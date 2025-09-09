#include <TFT_eSPI.h>
#include <SPI.h>
#include <SD_MMC.h>
#include <math.h>
#include "driver/i2s.h"

// ==================== DISPLAY CONFIG ====================
#define USER_SETUP_INFO "User_Setup"
#define ST7735_DRIVER
#define TFT_WIDTH  128
#define TFT_HEIGHT 160
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   5      
#define TFT_DC   21    
#define TFT_RST  4     
#define TFT_BL   21 //фактически подключено к 3.3v   
#define LOAD_GLCD
#define LOAD_FONT2
#define SPI_FREQUENCY  27000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000

TFT_eSPI tft = TFT_eSPI();

// Размеры спрайтов
#define LOGO_WIDTH   128
#define LOGO_HEIGHT  160
#define BG_WIDTH     128
#define BG_HEIGHT    160
#define NAME_WIDTH   117
#define NAME_HEIGHT  48
#define PRESS_WIDTH  48
#define PRESS_HEIGHT 15
#define GOBG_WIDTH  128
#define GOBG_HEIGHT 160
#define GOTEXT_WIDTH  58
#define GOTEXT_HEIGHT 35
#define MAIN_BG_WIDTH 128 
#define MAIN_BG_HEIGHT 160

#define BOOMS1_WIDTH  24
#define BOOMS1_HEIGHT 23
#define BOOMS2_WIDTH  24
#define BOOMS2_HEIGHT 23
#define BOOMS3_WIDTH  24
#define BOOMS3_HEIGHT 23
#define BOOMS4_WIDTH  24
#define BOOMS4_HEIGHT 23
#define BOOMS5_WIDTH  24
#define BOOMS5_HEIGHT 23

#define TRANSPARENT_COLOR 0xF81F

// ==================== LED ====================
#define LED_PIN 27

// ==================== I2S ====================
#define I2S_BCK_PIN   26
#define I2S_WS_PIN    25
#define I2S_DOUT_PIN  22
#define I2S_NUM_USED  I2S_NUM_0

// ==================== GAME CONSTANTS ====================
#define SHIP_SIZE 12
#define ASTEROID_SIZE 14
#define BULLET_SIZE 3
#define GAME_SPEED 5.0f
#define PHYSICS_FPS 40
#define RENDER_FPS 30

#define ENCODER_CLK 32
#define ENCODER_DT  33
#define ENCODER_SW  14
#define BUTTON_A    34
#define BUTTON_B    35

#define MAX_ASTEROIDS 10
#define SHOOTING_COOLDOWN 500
#define INITIAL_ASTEROIDS 1
#define ASTEROIDS_PER_LEVEL 2

#define BUZZER_PIN 13
#define LASER_STEP_DURATION 5
#define LASER_VIBRATO_RATE 30
#define LASER_VIBRATO_DEPTH 50

// Типы астероидов
#define ASTEROID_NORMAL 0
#define ASTEROID_FAST   1
#define FAST_ASTEROID_SPEED_MULTIPLIER 1.5f
#define FAST_ASTEROID_SPAWN_CHANCE 50

// Музыкальные треки
const char* introTracks[] = {"/snd_intro1.wav", "/snd_intro2.wav", "/snd_intro3.wav"};
const char* mainTracks[] = {"/snd_main1.wav", "/snd_main2.wav", "/snd_main3.wav", "/snd_main4.wav", "/snd_main5.wav"};
const int introTracksCount = 3;
const int mainTracksCount = 5;

// ==================== WAV STRUCT ====================
struct WavInfo {
  uint32_t dataPos;
  uint32_t dataLen;
  uint16_t channels;
  uint32_t sampleRate;
  uint16_t bitsPerSample;
};

// ==================== VARIABLES ====================
volatile int encoderPos = 0;
portMUX_TYPE encoderMux = portMUX_INITIALIZER_UNLOCKED;

struct LaserSoundEffect {
  unsigned long startTime;
  bool isPlaying;
  unsigned long lastUpdate;
};

struct SoundEffect {
  int frequency;
  unsigned long duration;
  unsigned long startTime;
  bool isPlaying;
};

struct Bullet {
  float x, y, dx, dy;
  float prevX, prevY;
  bool active;
  uint32_t spawnTime;
};

struct Asteroid {
  float x, y, dx, dy;
  float prevX, prevY;
  bool active;
  uint8_t type;
};

struct Game {
  float shipX = 64, shipY = 64;
  float prevShipX = 64, prevShipY = 64;
  float shipAngle = 0;
  float prevShipAngle = 0;
  float shipCos = 1.0f, shipSin = 0.0f;
  float shipSpeedX = 0, shipSpeedY = 0;
  int score = 0;
  int level = 1;
  Bullet bullets[15];
  Asteroid asteroids[MAX_ASTEROIDS];
  uint32_t lastShotTime = 0;
  bool gameOver = false;
} game;

LaserSoundEffect soundLaser = {0, false, 0};
SoundEffect soundExplosion = {350, 200, 0, false};
bool sdInitialized = false;
bool showLogo = true;
uint32_t logoStartTime = 0;
bool musicPlaying = false;
int currentTrackIndex = -1;

// Game states
enum GameState {
  STATE_LOGO,
  STATE_MENU,
  STATE_GAME,
  STATE_GAME_OVER
};
GameState currentState = STATE_LOGO;

// ==================== SD CARD FUNCTIONS ====================
bool initSDCard() {
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("[WARN] SD Card Mount Failed");
    return false;
  }
  if (SD_MMC.cardType() == CARD_NONE) {
    Serial.println("[WARN] No SD_MMC card attached");
    return false;
  }
  return true;
}

// ==================== IMAGE FUNCTIONS ====================
const int IMG_BUFFER_PIXELS = 128;
uint16_t imgBuffer[IMG_BUFFER_PIXELS];

void displayRGB565File(const char* filename, int x, int y, int w, int h) {
  if (!SD_MMC.exists(filename)) {
    Serial.printf("File not found: %s\n", filename);
    return;
  }
  
  File f = SD_MMC.open(filename, FILE_READ);
  if (!f) return;
  
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col += IMG_BUFFER_PIXELS) {
      int toRead = min(IMG_BUFFER_PIXELS, w - col);
      f.read((uint8_t*)imgBuffer, toRead * 2);
      tft.pushImage(x + col, y + row, toRead, 1, imgBuffer);
    }
  }
  f.close();
}

void displayRGB565FileTransparent(const char* filename, int x, int y, int w, int h, uint16_t transparent) {
  if (!SD_MMC.exists(filename)) {
    Serial.printf("File not found: %s\n", filename);
    return;
  }
  
  File f = SD_MMC.open(filename, FILE_READ);
  if (!f) return;
  
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col += IMG_BUFFER_PIXELS) {
      int toRead = min(IMG_BUFFER_PIXELS, w - col);
      f.read((uint8_t*)imgBuffer, toRead * 2);

      int runStart = -1;
      for (int i = 0; i < toRead; i++) {
        if (imgBuffer[i] != transparent) {
          if (runStart == -1) runStart = i;
        } else {
          if (runStart != -1) {
            tft.pushImage(x + col + runStart, y + row, i - runStart, 1, &imgBuffer[runStart]);
            runStart = -1;
          }
        }
      }
      if (runStart != -1) {
        tft.pushImage(x + col + runStart, y + row, toRead - runStart, 1, &imgBuffer[runStart]);
      }
    }
  }
  f.close();
}

// ==================== WAV FUNCTIONS ====================
bool parseWav(File &f, WavInfo &info) {
  if (!f) return false;
  f.seek(0);
  char riff[4]; f.read((uint8_t*)riff, 4);
  if (strncmp(riff, "RIFF", 4) != 0) return false;
  f.seek(8);
  char wave[4]; f.read((uint8_t*)wave, 4);
  if (strncmp(wave, "WAVE", 4) != 0) return false;

  while (f.position() + 8 <= f.size()) {
    char id[5]; id[4] = 0;
    f.read((uint8_t*)id, 4);
    uint32_t chunkSize; f.read((uint8_t*)&chunkSize, 4);

    if (strncmp(id, "fmt ", 4) == 0) {
      uint16_t audioFormat; f.read((uint8_t*)&audioFormat, 2);
      f.read((uint8_t*)&info.channels, 2);
      f.read((uint8_t*)&info.sampleRate, 4);
      uint32_t byteRate; f.read((uint8_t*)&byteRate, 4);
      uint16_t blockAlign; f.read((uint8_t*)&blockAlign, 2);
      f.read((uint8_t*)&info.bitsPerSample, 2);
      if (chunkSize > 16) f.seek(f.position() + (chunkSize - 16));
    } else if (strncmp(id, "data", 4) == 0) {
      info.dataPos = f.position();
      info.dataLen = chunkSize;
      return true;
    } else {
      f.seek(f.position() + chunkSize);
    }
  }
  return false;
}

bool i2sInit(uint32_t sampleRate, uint16_t bits, uint8_t ch) {
  i2s_driver_uninstall(I2S_NUM_USED);
  i2s_config_t cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = sampleRate,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = ch == 1 ? I2S_CHANNEL_FMT_ONLY_LEFT : I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S),
    .intr_alloc_flags = 0,
    .dma_buf_count = 6,
    .dma_buf_len = 160,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };
  
  if (i2s_driver_install(I2S_NUM_USED, &cfg, 0, NULL) != ESP_OK) return false;
  
  i2s_pin_config_t pins = {
    .bck_io_num = I2S_BCK_PIN,
    .ws_io_num = I2S_WS_PIN,
    .data_out_num = I2S_DOUT_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  
  if (i2s_set_pin(I2S_NUM_USED, &pins) != ESP_OK) return false;
  i2s_set_clk(I2S_NUM_USED, sampleRate, I2S_BITS_PER_SAMPLE_16BIT, 
              ch == 1 ? I2S_CHANNEL_MONO : I2S_CHANNEL_STEREO);
  return true;
}

// Фоновая задача воспроизведения
void playWavTask(void *param) {
  const char* filename = (const char*)param;
  
  if (!SD_MMC.exists(filename)) {
    Serial.printf("Audio file not found: %s\n", filename);
    vTaskDelete(NULL);
    return;
  }
  
  File f = SD_MMC.open(filename, FILE_READ);
  if (!f) {
    Serial.printf("Error opening audio file: %s\n", filename);
    vTaskDelete(NULL);
    return;
  }

  WavInfo info = {0};
  if (!parseWav(f, info)) {
    Serial.printf("Invalid WAV file: %s\n", filename);
    f.close();
    vTaskDelete(NULL);
    return;
  }
  
  if (!i2sInit(info.sampleRate, info.bitsPerSample, info.channels)) {
    Serial.printf("I2S init failed for: %s\n", filename);
    f.close();
    vTaskDelete(NULL);
    return;
  }

  f.seek(info.dataPos);
  uint8_t buf[1024];
  uint32_t left = info.dataLen;
  
  musicPlaying = true;
  
  while (left > 0) {
    size_t n = f.read(buf, min((uint32_t)sizeof(buf), left));
    if (n == 0) break;
    size_t written;
    i2s_write(I2S_NUM_USED, buf, n, &written, portMAX_DELAY);
    left -= n;
    
    // Проверяем, не нужно ли прервать воспроизведение
    if (currentState != STATE_MENU && currentState != STATE_GAME) {
      break;
    }
  }
  
  i2s_zero_dma_buffer(I2S_NUM_USED);
  i2s_driver_uninstall(I2S_NUM_USED);
  f.close();
  musicPlaying = false;
  
  vTaskDelete(NULL);
}

// Функция для запуска случайного трека
void playRandomTrack(bool isIntro) {
  if (musicPlaying) return;
  
  const char** tracks = isIntro ? introTracks : mainTracks;
  int trackCount = isIntro ? introTracksCount : mainTracksCount;
  
  int newIndex;
  do {
    newIndex = random(trackCount);
  } while (trackCount > 1 && newIndex == currentTrackIndex);
  
  currentTrackIndex = newIndex;
  xTaskCreatePinnedToCore(playWavTask, "playWav", 8192, (void*)tracks[newIndex], 1, NULL, 0);
}

// ==================== INTERRUPT ====================
void IRAM_ATTR encoderISR() {
  portENTER_CRITICAL_ISR(&encoderMux);
  static uint8_t lastCLK = HIGH;
  uint8_t clk = digitalRead(ENCODER_CLK);
  if (clk != lastCLK) {
    if (digitalRead(ENCODER_DT) != clk) encoderPos++;
    else encoderPos--;
    lastCLK = clk;
  }
  portEXIT_CRITICAL_ISR(&encoderMux);
}

// ==================== SOUND FUNCTIONS ====================
void playSound(SoundEffect &effect) {
  effect.startTime = millis();
  effect.isPlaying = true;
  tone(BUZZER_PIN, effect.frequency, effect.duration);
}

void playLaserSound() {
  soundLaser.startTime = millis();
  soundLaser.isPlaying = true;
  soundLaser.lastUpdate = 0;
  noTone(BUZZER_PIN);
}

void updateLaserSound() {
  if (!soundLaser.isPlaying) return;
  unsigned long currentTime = millis();
  if (currentTime - soundLaser.startTime > 60) {
    soundLaser.isPlaying = false;
    noTone(BUZZER_PIN);
    return;
  }
  if (currentTime - soundLaser.lastUpdate < LASER_STEP_DURATION) return;
  soundLaser.lastUpdate = currentTime;

  unsigned long elapsed = currentTime - soundLaser.startTime;
  float baseFreq = 1200.0f + (800.0f * (min(elapsed, 30UL) / 30.0f));
  float vibrato = sinf(elapsed * LASER_VIBRATO_RATE * 0.01f) * LASER_VIBRATO_DEPTH;
  int finalFreq = (int)(baseFreq + vibrato);
  tone(BUZZER_PIN, finalFreq);
}

// ==================== GAME FUNCTIONS ====================
void spawnAsteroid(int index) {
  Asteroid &a = game.asteroids[index];
  a.x = (random(2) ? -ASTEROID_SIZE : tft.width() + ASTEROID_SIZE);
  a.y = random(tft.height());

  float targetX = random(tft.width());
  float targetY = random(tft.height());
  float dx = (targetX - a.x) * 0.01f;
  float dy = (targetY - a.y) * 0.01f;

  float length = sqrtf(dx * dx + dy * dy);
  if (length > 0.0001f) {
    dx = dx / length * 0.5f;
    dy = dy / length * 0.5f;
  }

  if (random(100) < FAST_ASTEROID_SPAWN_CHANCE && game.level > 1) {
    a.type = ASTEROID_FAST;
    dx *= FAST_ASTEROID_SPEED_MULTIPLIER;
    dy *= FAST_ASTEROID_SPEED_MULTIPLIER;
  } else {
    a.type = ASTEROID_NORMAL;
  }

  a.dx = dx;
  a.dy = dy;
  a.active = true;
  a.prevX = a.x;
  a.prevY = a.y;
}

void updatePhysics() {
  static int lastEncoder = 0;

  portENTER_CRITICAL(&encoderMux);
  int curEnc = encoderPos;
  portEXIT_CRITICAL(&encoderMux);

  if (curEnc != lastEncoder) {
    game.prevShipAngle = game.shipAngle;
    game.shipAngle += (curEnc - lastEncoder) * 0.1f;
    lastEncoder = curEnc;
    game.shipCos = cosf(game.shipAngle);
    game.shipSin = sinf(game.shipAngle);
  }

  // Ускорение
  if (digitalRead(BUTTON_A) == LOW) {
    game.shipSpeedX += game.shipCos * 0.04f;
    game.shipSpeedY += game.shipSin * 0.04f;
  }

  // Выстрел
  if (digitalRead(BUTTON_B) == LOW) {
    if (millis() - game.lastShotTime > SHOOTING_COOLDOWN) {
      for (int i = 0; i < (int)(sizeof(game.bullets) / sizeof(game.bullets[0])); ++i) {
        Bullet &b = game.bullets[i];
        if (!b.active) {
          b.x = game.shipX;
          b.y = game.shipY;
          b.prevX = b.x;
          b.prevY = b.y;
          b.dx = game.shipCos * 3.0f;
          b.dy = game.shipSin * 3.0f;
          b.spawnTime = millis();
          b.active = true;
          game.lastShotTime = millis();
          playLaserSound();
          break;
        }
      }
    }
  }

  // Двигаем корабль
  game.prevShipX = game.shipX;
  game.prevShipY = game.shipY;
  game.shipX += game.shipSpeedX * GAME_SPEED;
  game.shipY += game.shipSpeedY * GAME_SPEED;
  game.shipSpeedX *= 0.95f;
  game.shipSpeedY *= 0.95f;

  // Телепорт по краям
  if (game.shipX < 0) game.shipX = tft.width();
  if (game.shipX > tft.width()) game.shipX = 0;
  if (game.shipY < 0) game.shipY = tft.height();
  if (game.shipY > tft.height()) game.shipY = 0;

  // Двигаем пули
  for (int i = 0; i < (int)(sizeof(game.bullets) / sizeof(game.bullets[0])); ++i) {
    Bullet &b = game.bullets[i];
    if (b.active) {
      b.prevX = b.x;
      b.prevY = b.y;
      b.x += b.dx * GAME_SPEED;
      b.y += b.dy * GAME_SPEED;
      if ((millis() - b.spawnTime > 1000) || b.x < 0 || b.x > tft.width() || b.y < 0 || b.y > tft.height()) {
        b.active = false;
      }
    }
  }

  // Подсчёт активных астероидов
  int activeAsteroids = 0;
  for (int i = 0; i < MAX_ASTEROIDS; ++i) if (game.asteroids[i].active) activeAsteroids++;

  int desiredAsteroids = INITIAL_ASTEROIDS + ((game.level - 1) * ASTEROIDS_PER_LEVEL);
  if (desiredAsteroids > MAX_ASTEROIDS) desiredAsteroids = MAX_ASTEROIDS;

  if (activeAsteroids < desiredAsteroids) {
    for (int i = 0; i < MAX_ASTEROIDS && activeAsteroids < desiredAsteroids; ++i) {
      if (!game.asteroids[i].active) {
        spawnAsteroid(i);
        activeAsteroids++;
      }
    }
  }

  int newLevel = 1 + (game.score / 10);
  if (newLevel > game.level) game.level = newLevel;

  // Обновление положения астероидов и столкновений
  float shipCollisionRadius = (ASTEROID_SIZE / 2.0f) + (SHIP_SIZE / 2.0f);
  float shipCollisionRadius2 = shipCollisionRadius * shipCollisionRadius;
  float bulletCollisionRadius = (ASTEROID_SIZE / 2.0f) + BULLET_SIZE;
  float bulletCollisionRadius2 = bulletCollisionRadius * bulletCollisionRadius;

  for (int i = 0; i < MAX_ASTEROIDS; ++i) {
    Asteroid &a = game.asteroids[i];
    if (!a.active) continue;

    a.prevX = a.x;
    a.prevY = a.y;

    float speedMultiplier = (a.type == ASTEROID_FAST) ? FAST_ASTEROID_SPEED_MULTIPLIER : 1.0f;
    a.x += a.dx * (GAME_SPEED / 4.0f) * speedMultiplier;
    a.y += a.dy * (GAME_SPEED / 4.0f) * speedMultiplier;

    if (a.x < -ASTEROID_SIZE * 2) a.x = tft.width() + ASTEROID_SIZE;
    if (a.x > tft.width() + ASTEROID_SIZE * 2) a.x = -ASTEROID_SIZE;
    if (a.y < -ASTEROID_SIZE * 2) a.y = tft.height() + ASTEROID_SIZE;
    if (a.y > tft.height() + ASTEROID_SIZE * 2) a.y = -ASTEROID_SIZE;

    float dx = a.x - game.shipX;
    float dy = a.y - game.shipY;
    float dist2 = dx * dx + dy * dy;

    if (dist2 < shipCollisionRadius2) {
      // Столкновение с кораблем
      game.gameOver = true;
      currentState = STATE_GAME_OVER;
      
      // Мигание светодиода при столкновении
      for (int i = 0; i < 5; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
      }
      break;
    }

    // Столкновения с пулями
    for (int bidx = 0; bidx < (int)(sizeof(game.bullets) / sizeof(game.bullets[0])); ++bidx) {
      Bullet &b = game.bullets[bidx];
      if (!b.active) continue;
      float bdx = a.x - b.x;
      float bdy = a.y - b.y;
      float bdist2 = bdx * bdx + bdy * bdy;
      if (bdist2 < bulletCollisionRadius2) {
        a.active = false;
        b.active = false;
        game.score++;
        playSound(soundExplosion);
      }
    }
  }
}

void drawShip(float x, float y, float angle, uint16_t color) {
  float x1 = x + cosf(angle) * (SHIP_SIZE / 2.0f);
  float y1 = y + sinf(angle) * (SHIP_SIZE / 2.0f);
  float x2 = x + cosf(angle + 2.5f) * (SHIP_SIZE / 2.5f);
  float y2 = y + sinf(angle + 2.5f) * (SHIP_SIZE / 2.5f);
  float x3 = x + cosf(angle - 2.5f) * (SHIP_SIZE / 2.5f);
  float y3 = y + sinf(angle - 2.5f) * (SHIP_SIZE / 2.5f);
  tft.fillTriangle((int)x1, (int)y1, (int)x2, (int)y2, (int)x3, (int)y3, color);
}

void renderGame(bool fullRedraw) {
  // ✅ Отрисовываем фон геймплея вместо черного экрана
  if (fullRedraw) {
    displayRGB565File("/spr_main_BG.bin", 0, 0, MAIN_BG_WIDTH, MAIN_BG_HEIGHT);
  }

  // Счёт и уровень
  tft.fillRect(0, 0, tft.width(), 20, TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(2, 2);
  tft.setTextSize(1);
  tft.print("Score: ");
  tft.print(game.score);
  tft.print(" L");
  tft.print(game.level);

  if (millis() - game.lastShotTime < SHOOTING_COOLDOWN) {
    float progress = (float)(millis() - game.lastShotTime) / SHOOTING_COOLDOWN;
    int barWidth = (int)(progress * 20);
    tft.fillRect(tft.width() - 28, 5, 20, 5, TFT_DARKGREY);
    tft.fillRect(tft.width() - 28, 5, barWidth, 5, TFT_GREEN);
  }

  // Пули
  for (int i = 0; i < (int)(sizeof(game.bullets) / sizeof(game.bullets[0])); ++i) {
    Bullet &b = game.bullets[i];
    int px = (int)b.prevX;
    int py = (int)b.prevY;
    if ((!b.active || fullRedraw) && px >= 0 && px < tft.width() && py >= 0 && py < tft.height()) {
      // ✅ Восстанавливаем фон вместо черного
      displayRGB565File("/spr_main_BG.bin", px - BULLET_SIZE - 1, py - BULLET_SIZE - 1, 
                       BULLET_SIZE * 2 + 2, BULLET_SIZE * 2 + 2);
    }
    if (b.active) {
      tft.fillCircle((int)b.x, (int)b.y, BULLET_SIZE, TFT_RED);
    }
  }

  // Астероиды
  for (int i = 0; i < MAX_ASTEROIDS; ++i) {
    Asteroid &a = game.asteroids[i];
    int px = (int)a.prevX;
    int py = (int)a.prevY;
    if ((!a.active || fullRedraw) && px >= 0 && px < tft.width() && py >= 0 && py < tft.height()) {
      // ✅ Восстанавливаем фон вместо черного
      displayRGB565File("/spr_main_BG.bin", px - ASTEROID_SIZE/2 - 1, py - ASTEROID_SIZE/2 - 1, 
                       ASTEROID_SIZE + 2, ASTEROID_SIZE + 2);
    }
    if (a.active) {
      uint16_t color = (a.type == ASTEROID_FAST) ? TFT_GREEN : TFT_WHITE;
      tft.fillCircle((int)a.x, (int)a.y, ASTEROID_SIZE / 2, color);
    }
  }

  // Корабль
  if (fullRedraw) {
    drawShip(game.shipX, game.shipY, game.shipAngle, TFT_BLUE);
  } else {
    // ✅ Восстанавливаем фон под предыдущей позицией корабля
    displayRGB565File("/spr_main_BG.bin", (int)game.prevShipX - SHIP_SIZE/2 - 1, 
                     (int)game.prevShipY - SHIP_SIZE/2 - 1, SHIP_SIZE + 2, SHIP_SIZE + 2);
    drawShip(game.shipX, game.shipY, game.shipAngle, TFT_BLUE);
  }
}

void showGameOverScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 60);
  tft.println("GAME OVER");
  tft.setTextSize(1);
  tft.setCursor(30, 80);
  tft.print("Score: ");
  tft.print(game.score);
  tft.setCursor(20, 100);
  tft.println("Press encoder");
  tft.setCursor(30, 110);
  tft.println("to restart");
}

void resetGame() {
  game.shipX = 64;
  game.shipY = 64;
  game.prevShipX = 64;
  game.prevShipY = 64;
  game.shipAngle = 0;
  game.prevShipAngle = 0;
  game.shipCos = 1.0f;
  game.shipSin = 0.0f;
  game.shipSpeedX = 0;
  game.shipSpeedY = 0;
  game.score = 0;
  game.level = 1;
  game.lastShotTime = 0;
  game.gameOver = false;

  for (int i = 0; i < (int)(sizeof(game.bullets) / sizeof(game.bullets[0])); ++i) {
    game.bullets[i].active = false;
  }
  for (int i = 0; i < MAX_ASTEROIDS; ++i) {
    game.asteroids[i].active = false;
  }

  for (int i = 0; i < INITIAL_ASTEROIDS; ++i) {
    spawnAsteroid(i);
  }
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(500);
  
  // ✅ Правильная инициализация кнопок для избежания ложных срабатываний
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_SW, INPUT);
  pinMode(BUTTON_A, INPUT);
  pinMode(BUTTON_B, INPUT);
  delay(100);
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  delay(100);
  
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), encoderISR, CHANGE);
  
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  sdInitialized = initSDCard();

  if (sdInitialized) {
    // 1) Показываем логотип 2 секунды
    displayRGB565File("/spr_GG_logo.bin", 0, 0, LOGO_WIDTH, LOGO_HEIGHT);
    delay(2000);
    
    // 2) Запускаем случайный интро-трек
    playRandomTrack(true);
    
    // 3) Показываем фон меню
    displayRGB565File("/spr_start_BG.bin", 0, 0, BG_WIDTH, BG_HEIGHT);
    currentState = STATE_MENU;
  } else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(10, 70);
    tft.println("SD Card FAILED!");
    delay(2000);
  }

  // Инициализация игровых массивов
  for (int i = 0; i < (int)(sizeof(game.bullets) / sizeof(game.bullets[0])); ++i) {
    game.bullets[i].active = false;
    game.bullets[i].prevX = -1000;
    game.bullets[i].prevY = -1000;
  }
  for (int i = 0; i < MAX_ASTEROIDS; ++i) {
    game.asteroids[i].active = false;
    game.asteroids[i].prevX = -1000;
    game.asteroids[i].prevY = -1000;
  }

  randomSeed(esp_random());
  resetGame();
}

// ==================== LOOP ====================
void loop() {
  static unsigned long lastToggle = 0;
  static bool showAlt = false;
  static unsigned long lastPressToggle = 0;
  static bool showPress = false;
  static uint32_t lastPhysicsUpdate = 0;
  static uint32_t lastRenderUpdate = 0;
  static uint32_t lastFullClear = 0;
  static bool forceFullRedraw = false;
  static unsigned long lastMusicCheck = 0;

  unsigned long now = millis();

  // ✅ Проверяем завершение музыки и запускаем новую
  if (now - lastMusicCheck >= 1000) {
    lastMusicCheck = now;
    if (!musicPlaying && (currentState == STATE_MENU || currentState == STATE_GAME)) {
      playRandomTrack(currentState == STATE_MENU);
    }
  }

  // Проверка нажатия кнопки энкодера
  static bool lastButtonState = HIGH;
  bool currentButtonState = digitalRead(ENCODER_SW);
  
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    // Кнопка нажата
    if (currentState == STATE_MENU) {
      // Переход из меню в игру
      currentState = STATE_GAME;
      // ✅ Запускаем игровой трек
      playRandomTrack(false);
      // ✅ Отрисовываем игровой фон
      displayRGB565File("/spr_main_BG.bin", 0, 0, MAIN_BG_WIDTH, MAIN_BG_HEIGHT);
    } else if (currentState == STATE_GAME_OVER) {
      // Переход из Game Over в игру
      resetGame();
      currentState = STATE_GAME;
      // ✅ Запускаем игровой трек
      playRandomTrack(false);
      // ✅ Отрисовываем игровой фон
      displayRGB565File("/spr_main_BG.bin", 0, 0, MAIN_BG_WIDTH, MAIN_BG_HEIGHT);
    }
  }
  lastButtonState = currentButtonState;

  switch (currentState) {
    case STATE_MENU:
      // Анимация меню
      if (now - lastToggle >= 1000) { // ✅ Изменено на 1 секунду
        lastToggle = now;
        showAlt = !showAlt;
        if (showAlt) {
          displayRGB565FileTransparent("/spr_main_name.bin", 3, 109, NAME_WIDTH, NAME_HEIGHT, TRANSPARENT_COLOR);
        } else {
          displayRGB565FileTransparent("/spr_main_name2.bin", 3, 109, NAME_WIDTH, NAME_HEIGHT, TRANSPARENT_COLOR);
        }
      }

      if (now - lastPressToggle >= 500) { // ✅ Изменено на 0.5 секунды
        lastPressToggle = now;
        showPress = !showPress;
        if (showPress) {
          displayRGB565FileTransparent("/spr_press_RB.bin", 37, 71, PRESS_WIDTH, PRESS_HEIGHT, TRANSPARENT_COLOR);
          digitalWrite(LED_PIN, HIGH);
        } else {
          // ✅ Восстанавливаем фон вместо черного
          displayRGB565File("/spr_start_BG.bin", 37, 71, PRESS_WIDTH, PRESS_HEIGHT);
          digitalWrite(LED_PIN, LOW);
        }
      }
      break;

    case STATE_GAME:
      // Игровой цикл
      if (now - lastFullClear >= 300) {
        forceFullRedraw = true;
        lastFullClear = now;
      }

      if (now - lastPhysicsUpdate >= (1000 / PHYSICS_FPS)) {
        updatePhysics();
        lastPhysicsUpdate = now;
      }

      if (soundLaser.isPlaying) {
        updateLaserSound();
      }

      if (now - lastRenderUpdate >= (1000 / RENDER_FPS)) {
        renderGame(forceFullRedraw);
        forceFullRedraw = false;
        lastRenderUpdate = now;
      }

      if (soundExplosion.isPlaying && (now - soundExplosion.startTime >= soundExplosion.duration)) {
        soundExplosion.isPlaying = false;
        noTone(BUZZER_PIN);
      }
      break;

    case STATE_GAME_OVER:
      // Экран Game Over
      showGameOverScreen();
      break;
  }
}