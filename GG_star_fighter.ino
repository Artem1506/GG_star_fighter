#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SD_MMC.h>
#include <driver/gpio.h>
#include <driver/i2s.h>
#include <cmath>


// ==================== КОНСТАНТЫ ПИНОВ ====================
constexpr uint8_t ENCODER_CLK = 32;
constexpr uint8_t ENCODER_DT = 33;
constexpr uint8_t ENCODER_SW = 34;
constexpr uint8_t BUTTON_A = 2;
constexpr uint8_t BUTTON_B = 19;
constexpr uint8_t I2S_BCK_PIN = 26;
constexpr uint8_t I2S_WS_PIN = 25;
constexpr uint8_t I2S_DOUT_PIN = 22;
constexpr uint8_t BUZZER_PIN = 13;
constexpr uint8_t LED_PIN = 27;

// ==================== КОНСТАНТЫ ЭКРАНА ====================
constexpr int16_t SCREEN_WIDTH = 128;
constexpr int16_t SCREEN_HEIGHT = 160;

// ==================== КОНСТАНТЫ ПУТЕЙ ФАЙЛОВ ====================
constexpr const char* HIGHSCORE_FILE = "/highscore.txt";
constexpr const char* LOGO_FILE = "/spr_GG_logo.bin";
constexpr const char* START_BG_FILE = "/spr_start_BG.bin";
constexpr const char* NAME1_FILE = "/spr_main_name.bin";
constexpr const char* NAME2_FILE = "/spr_main_name2.bin";
constexpr const char* PRESS_FILE = "/spr_press_RB.bin"; //поменять на рисование текста
constexpr const char* MAIN_BG_FILE = "/spr_main_BG.bin";
constexpr const char* SHIP_BOOST_FILE = "/spr_ship_boost_000_1.bin";
constexpr const char* SHIP_STAY_FILE = "/spr_ship_stay_000.bin";
constexpr const char* BULLET_FILE = "/spr_bullet_000.bin";
constexpr const char* ASTEROID_FILE = "/spr_asteroid_1.bin";
constexpr const char* COMET_FILE = "/spr_comet_000.bin";
constexpr const char* BOOM_BIG_FILE = "/spr_boom_big1.bin";
constexpr const char* BOOM_SMALL_FILE = "/spr_boom_small1.bin";
constexpr const char* GAMEOVER_BG_FILE = "/spr_GO_BG.bin";
constexpr const char* GAMEOVER_TEXT_FILE = "/spr_GO_text.bin";

// ==================== КОНСТАНТЫ РАЗМЕРОВ СПРАЙТОВ ====================
constexpr uint8_t LOGO_WIDTH = 128;
constexpr uint8_t LOGO_HEIGHT = 160;
constexpr uint8_t BG_WIDTH = 128;
constexpr uint8_t BG_HEIGHT = 160;
constexpr uint8_t NAME_WIDTH = 117;
constexpr uint8_t NAME_HEIGHT = 48;
constexpr uint8_t PRESS_WIDTH = 48;
constexpr uint8_t PRESS_HEIGHT = 15;
constexpr uint8_t SHIP_WIDTH = 17;
constexpr uint8_t SHIP_HEIGHT = 17;
constexpr uint8_t BULLET_WIDTH = 4;
constexpr uint8_t BULLET_HEIGHT = 4;
constexpr uint8_t ASTEROID_WIDTH = 9;
constexpr uint8_t ASTEROID_HEIGHT = 10;
constexpr uint8_t COMET_WIDTH = 22;
constexpr uint8_t COMET_HEIGHT = 21;
constexpr uint8_t BOOMBIG_WIDTH = 65;
constexpr uint8_t BOOMBIG_HEIGHT = 62;
constexpr uint8_t BOOMSMALL_WIDTH = 24;
constexpr uint8_t BOOMSMALL_HEIGHT = 23;
constexpr uint8_t GOTEXT_WIDTH = 58;
constexpr uint8_t GOTEXT_HEIGHT = 35;

// ==================== КОНСТАНТЫ ИГРЫ ====================
constexpr uint8_t MAX_BULLETS = 5;
constexpr uint8_t MAX_ASTEROIDS = 10;
constexpr uint32_t BULLET_DELAY = 300;
constexpr float SHIP_SPEED = 2.0f;
constexpr float BULLET_SPEED = 4.0f;
constexpr float ASTEROID_BASE_SPEED = 1.0f;
constexpr float COMET_SPEED_MULTIPLIER = 1.5f;
constexpr uint32_t LOGO_DISPLAY_TIME = 2000;
constexpr unsigned long DEBOUNCE_MS = 50;

// ==================== СТРУКТУРЫ И ПЕРЕЧИСЛЕНИЯ ====================
struct Entity {
    float x, y;
    float vx, vy;
    bool active;
    int oldX, oldY;
};

struct Ship {
    Entity base;
    int16_t rotation;
    bool boosting;
    uint32_t lastShot;
    uint32_t lastBoostFrame;
    uint8_t boostFrame;
};

struct Bullet {
    Entity base;
    uint32_t spawnTime;
};

struct Asteroid {
    Entity base;
    uint8_t size;
    bool isComet;
};

struct WavInfo {
    uint32_t dataPos;
    uint32_t dataLen;
    uint16_t channels;
    uint32_t sampleRate;
    uint16_t bitsPerSample;
};

struct InputState {
    int16_t encoderAngle;
    bool encoderPressed;
    bool buttonA;
    bool buttonB;
};

enum GameState {
    STATE_LOGO,
    STATE_MENU,
    STATE_PLAY,
    STATE_GAME_OVER
};

// ==================== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ====================
TFT_eSPI tft;
GameState currentState = STATE_LOGO;
uint32_t stateStartTime = 0;
uint32_t lastFrameTime = 0;

Ship playerShip;
Bullet bullets[5];
Asteroid asteroids[10];
int score = 0;
int highScore = 0;
uint8_t activeAsteroids = 0;

volatile int32_t encoderPos = 0;
volatile int lastEncoderARaw = 0;

// ===== GLOBAL BACKGROUND DATA =====
uint16_t* bgBuffer = nullptr;      // буфер текущего фона в PSRAM
uint16_t* bgStart = nullptr;       // фон стартового меню
uint16_t* bgMain = nullptr;        // фон основной игры
uint16_t* bgGameOver = nullptr;    // фон экрана Game Over
size_t bgSizeStart = 0;
size_t bgSizeMain = 0;
size_t bgSizeGameOver = 0;
uint16_t* bgCurrent = nullptr;     // указатель на активный фон

// ==================== Мониторинг ресурсов ====================
unsigned long lastStatsTime = 0;
unsigned long frameCount = 0;

void printStats() {
  // CPU
  uint32_t cpuFreq = getCpuFrequencyMhz();

  // RAM
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t minFreeHeap = ESP.getMinFreeHeap();

  // PSRAM
  uint32_t freePsram = ESP.getFreePsram();
  uint32_t psramSize = ESP.getPsramSize();

  // FPS
  static unsigned long lastFpsTime = millis();
  static unsigned long lastFrameCount = 0;
  unsigned long now = millis();
  float fps = (frameCount - lastFrameCount) * 1000.0 / (now - lastFpsTime);
  lastFrameCount = frameCount;
  lastFpsTime = now;

  Serial.println("=== System Stats ===");
  Serial.printf("CPU: %u MHz\n", cpuFreq);
  Serial.printf("Heap Free: %u bytes (Min: %u)\n", freeHeap, minFreeHeap);
  Serial.printf("PSRAM: %u / %u bytes free\n", freePsram, psramSize);
  Serial.printf("FPS: %.2f\n", fps);
  Serial.println("====================");
}

// ==================== end ====================

// Утилита: пересечение двух осевых bbox (x,y,w,h)
bool rectsIntersect(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
  return !(x2 >= x1 + w1 || x2 + w2 <= x1 || y2 >= y1 + h1 || y2 + h2 <= y1);
}

// ==================== ФУНКЦИИ SD-КАРТЫ ====================
bool initSDCard() {
    if (!SD_MMC.begin("/sdcard", true)) {
        //Serial.println("SD Card Mount Failed");
        return false;
    }
    if (SD_MMC.cardType() == CARD_NONE) {
        //Serial.println("No SD card attached");
        return false;
    }
    return true;
}

int readHighScore() {
    File file = SD_MMC.open(HIGHSCORE_FILE, FILE_READ);
    if (!file) return 0;
    
    char buffer[16];
    uint8_t index = 0;
    while (file.available() && index < sizeof(buffer) - 1) {
        char c = file.read();
        if (c == '\n' || c == '\r') break;
        buffer[index++] = c;
    }
    buffer[index] = '\0';
    file.close();
    return atoi(buffer);
}

void writeHighScore(int score) {
    File file = SD_MMC.open(HIGHSCORE_FILE, FILE_WRITE);
    if (file) {
        file.seek(0);
        file.println(score);
        file.close();
    }
}

// ==================== ФУНКЦИИ ОТРИСОВКИ ====================
void drawImageFromPSRAM(const char* filename, int x, int y, int w, int h) {
  size_t size;
  uint8_t* data = getSpriteFromCache(filename, size); // уже есть
  if (!data) return;
  tft.pushImage(x, y, w, h, (uint16_t*)data);
}

void restoreBgArea(int x, int y, int w, int h) {
  uint16_t* src = bgBuffer + y * BG_WIDTH + x;
  tft.pushImage(x, y, w, h, src);
}
// Вспомогательная функция: восстановить прямоугольник (x,y,w,h) из bgPtr
void restoreBgAreaFromBG(uint16_t* bgPtr, int bgW, int bgH, int x, int y, int w, int h) {
  if (!bgPtr) return;
  // Клиппинг
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  if (x >= bgW || y >= bgH) return;
  if (x + w > bgW) w = bgW - x;
  if (y + h > bgH) h = bgH - y;
  if (w <= 0 || h <= 0) return;

  // Указатель на первый пиксель (x,y)
  uint16_t* src = bgPtr + y * bgW + x;
  // Отправляем весь прямоугольник одним pushImage
  tft.pushImage(x, y, w, h, src);
}

// Главная функция: восстановить область и перерисовать объекты, пересекающиеся с ней
// bgPtr/bgW/bgH - указатель и размеры текущего фонового изображения (например bgMain)
void restoreAreaAndRedrawObjects(uint16_t* bgPtr, int bgW, int bgH,
                                 int rx, int ry, int rw, int rh) {
  if (!bgPtr) return;

  // 1) Восстанавливаем фон в заданной зоне
  restoreBgAreaFromBG(bgPtr, bgW, bgH, rx, ry, rw, rh);

  // 2) Перерисуем все объекты, чьи current bbox пересекаются с этой зоной.
  //    Порядок перерисовки отражает z-order: сначала астероиды, потом пули, затем корабль.
  char nameBuf[64];

  // ASTEROIDS
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    if (!asteroids[i].base.active) continue;
    int ax = (int)asteroids[i].base.x - (ASTEROID_WIDTH / 2);
    int ay = (int)asteroids[i].base.y - (ASTEROID_HEIGHT / 2);
    int aw = asteroids[i].isComet ? COMET_WIDTH : ASTEROID_WIDTH;
    int ah = asteroids[i].isComet ? COMET_HEIGHT : ASTEROID_HEIGHT;

    if (rectsIntersect(rx, ry, rw, rh, ax, ay, aw, ah)) {
      if (asteroids[i].isComet) {
        float dirf = atan2(asteroids[i].base.vy, asteroids[i].base.vx) * 180.0f / PI;
        int dir = (int)dirf;
        if (dir < 0) dir += 360;
        int norm = (dir / 10) * 10;
        snprintf(nameBuf, sizeof(nameBuf), "/spr_comet_%03d.bin", norm);
        drawImageFromPSRAM(nameBuf, ax, ay, COMET_WIDTH, COMET_HEIGHT);
      } else {
        drawImageFromPSRAM(ASTEROID_FILE, ax, ay, ASTEROID_WIDTH, ASTEROID_HEIGHT);
      }
    }
  }

  // BULLETS
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].base.active) continue;
    int bx = (int)bullets[i].base.x - (BULLET_WIDTH / 2);
    int by = (int)bullets[i].base.y - (BULLET_HEIGHT / 2);
    if (rectsIntersect(rx, ry, rw, rh, bx, by, BULLET_WIDTH, BULLET_HEIGHT)) {
      float angle = atan2(bullets[i].base.vy, bullets[i].base.vx) * 180.0f / PI;
      int ang = (int)angle;
      if (ang < 0) ang += 360;
      int norm = (ang / 10) * 10;
      snprintf(nameBuf, sizeof(nameBuf), "/spr_bullet_%03d.bin", norm);
      drawImageFromPSRAM(nameBuf, bx, by, BULLET_WIDTH, BULLET_HEIGHT);
    }
  }

  // PLAYER (на вершине)
  if (playerShip.base.active) {
    int sx = (int)playerShip.base.x - (SHIP_WIDTH / 2);
    int sy = (int)playerShip.base.y - (SHIP_HEIGHT / 2);
    if (rectsIntersect(rx, ry, rw, rh, sx, sy, SHIP_WIDTH, SHIP_HEIGHT)) {
      int norm = (playerShip.rotation / 10) * 10;
      if (playerShip.boosting) {
        snprintf(nameBuf, sizeof(nameBuf), "/spr_ship_boost_%03d_%d.bin", norm, playerShip.boostFrame + 1);
      } else {
        snprintf(nameBuf, sizeof(nameBuf), "/spr_ship_stay_%03d.bin", norm);
      }
      drawImageFromPSRAM(nameBuf, sx, sy, SHIP_WIDTH, SHIP_HEIGHT);
    }
  }
}

void drawShipFromPSRAM(int16_t x, int16_t y, int16_t angle, bool boosting, uint8_t boostFrame) {
    char filename[40];

    // нормализация угла
    int normAngle = angle % 360;
    if (normAngle < 0) normAngle += 360;
    normAngle = (normAngle / 10) * 10;

    // имя файла
    if (boosting) {
        if (boostFrame > 2) boostFrame = 2;
        snprintf(filename, sizeof(filename),
                 "/spr_ship_boost_%03d_%d.bin", normAngle, boostFrame + 1);
    } else {
        snprintf(filename, sizeof(filename),
                 "/spr_ship_stay_%03d.bin", normAngle);
    }

    size_t fileSize;
    uint16_t* sprite = (uint16_t*)getSpriteFromCache(filename, fileSize);
    if (!sprite) return;

    // рисуем
    tft.pushImage(x - SHIP_WIDTH/2, y - SHIP_HEIGHT/2, SHIP_WIDTH, SHIP_HEIGHT, sprite, TFT_MAGENTA);
}


void drawLogo() {
    drawImageFromPSRAM(LOGO_FILE, 0, 0, LOGO_WIDTH, LOGO_HEIGHT);
}

void drawMenuBackground() {
    drawImageFromPSRAM(START_BG_FILE, 0, 0, BG_WIDTH, BG_HEIGHT);
}

void drawGameBackground() {
    drawImageFromPSRAM(MAIN_BG_FILE, 0, 0, BG_WIDTH, BG_HEIGHT);
}

void drawGameOverBackground() {
    drawImageFromPSRAM(GAMEOVER_BG_FILE, 0, 0, BG_WIDTH, BG_HEIGHT);
}

void drawShip(int16_t x, int16_t y, uint16_t angle, bool isBoosting = false, uint8_t boostFrame = 0) {
    char filename[32];
    uint16_t normalizedAngle = (angle / 10) * 10;
    
    if (isBoosting) {
        snprintf(filename, sizeof(filename), "/spr_ship_boost_%03d_%d.bin", normalizedAngle, boostFrame + 1);
    } else {
        snprintf(filename, sizeof(filename), "/spr_ship_stay_%03d.bin", normalizedAngle);
    }
    
    drawImageFromPSRAM(filename, x - SHIP_WIDTH/2, y - SHIP_HEIGHT/2, SHIP_WIDTH, SHIP_HEIGHT);
}

void drawBullet(int16_t x, int16_t y, uint16_t angle) {
    char filename[24];
    uint16_t normalizedAngle = (angle / 10) * 10;
    snprintf(filename, sizeof(filename), "/spr_bullet_%03d.bin", normalizedAngle);
    
    drawImageFromPSRAM(filename, x - BULLET_WIDTH/2, y - BULLET_HEIGHT/2, BULLET_WIDTH, BULLET_HEIGHT);
}

void drawAsteroid(int16_t x, int16_t y, uint8_t size, bool isComet, uint16_t direction) {
    char filename[24];
    if (isComet) {
        uint16_t normalizedDirection = (direction / 10) * 10;
        snprintf(filename, sizeof(filename), "/spr_comet_%03d.bin", normalizedDirection);
    } else {
        //snprintf(filename, sizeof(filename), "/spr_asteroid_%d.bin", size);
        snprintf(filename, sizeof(filename), "/spr_asteroid_1.bin", size);
    }
    
    drawImageFromPSRAM(filename, x - (isComet ? COMET_WIDTH : ASTEROID_WIDTH)/2, 
                   y - (isComet ? COMET_HEIGHT : ASTEROID_HEIGHT)/2, 
                   isComet ? COMET_WIDTH : ASTEROID_WIDTH, 
                   isComet ? COMET_HEIGHT : ASTEROID_HEIGHT);
}

void drawScore(uint16_t score) {
    char scoreText[16];
    snprintf(scoreText, sizeof(scoreText), "SCORE: %d", score);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(scoreText, 5, 5, 2);
}

// ==================== АУДИО СИСТЕМА ====================
const char* introTracks[3] = {"/snd_intro1.wav", "/snd_intro2.wav", "/snd_intro3.wav"};
const char* mainTracks[5] = {"/snd_main1.wav", "/snd_main2.wav", "/snd_main3.wav", "/snd_main4.wav", "/snd_main5.wav"};
const char* gameOverTracks[3] = {"/snd_gameover1.wav", "/snd_gameover2.wav", "/snd_gameover3.wav"};

bool parseWav(File &f, WavInfo &info) {
    if (!f) return false;
    f.seek(0);
    char riff[4]; f.read((uint8_t*)riff, 4);
    if (strncmp(riff, "RIFF", 4) != 0) return false;
    f.seek(8);
    char wave[4]; f.read((uint8_t*)wave, 4);
    if (strncmp(wave, "WAVE", 4) != 0) return false;

    while (f.position() + 8 <= f.size()) {
        char id[5] = {0};
        f.read((uint8_t*)id, 4);
        uint32_t chunkSize; f.read((uint8_t*)&chunkSize, 4);

        if (strncmp(id, "fmt ", 4) == 0) {
            uint16_t audioFormat; f.read((uint8_t*)&audioFormat, 2);
            f.read((uint8_t*)&info.channels, 2);
            f.read((uint8_t*)&info.sampleRate, 4);
            uint32_t byteRate; f.read((uint8_t*)&byteRate, 4);
            uint16_t blockAlign; f.read((uint8_t*)&blockAlign, 2);
            f.read((uint8_t*)&info.bitsPerSample, 2);
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
    i2s_driver_uninstall(I2S_NUM_0);
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
    if (i2s_driver_install(I2S_NUM_0, &cfg, 0, NULL) != ESP_OK) return false;
    
    i2s_pin_config_t pins = {
        .bck_io_num = I2S_BCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_DOUT_PIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    if (i2s_set_pin(I2S_NUM_0, &pins) != ESP_OK) return false;
    
    i2s_set_clk(I2S_NUM_0, sampleRate, I2S_BITS_PER_SAMPLE_16BIT, ch == 1 ? I2S_CHANNEL_MONO : I2S_CHANNEL_STEREO);
    return true;
}

void playWavTask(void *param) {
    const char* fn = (const char*)param;
    File f = SD_MMC.open(fn, FILE_READ);
    if (!f) { vTaskDelete(NULL); return; }

    WavInfo info = {0};
    if (!parseWav(f, info)) { f.close(); vTaskDelete(NULL); return; }
    if (!i2sInit(info.sampleRate, info.bitsPerSample, info.channels)) { f.close(); vTaskDelete(NULL); return; }

    f.seek(info.dataPos);
    uint8_t buf[1024];
    uint32_t left = info.dataLen;
    
    while (left > 0) {
        size_t n = f.read(buf, min((uint32_t)sizeof(buf), left));
        if (n == 0) break;
        size_t written;
        i2s_write(I2S_NUM_0, buf, n, &written, portMAX_DELAY);
        left -= n;
    }
    
    i2s_zero_dma_buffer(I2S_NUM_0);
    i2s_driver_uninstall(I2S_NUM_0);
    f.close();
    vTaskDelete(NULL);
}

void playRandomTrack(const char** tracks, uint8_t count) {
    uint8_t idx = random(count);
    xTaskCreatePinnedToCore(playWavTask, "AudioTask", 8192, (void*)tracks[idx], 1, NULL, 0);
}

void playSoundEffect(uint16_t frequency, uint32_t duration) {
    tone(BUZZER_PIN, frequency, duration);
}

// ==================== ВВОД ДАННЫХ ====================
void IRAM_ATTR handleEncoderISR() {
    int clkState = digitalRead(ENCODER_CLK);
    int dtState = digitalRead(ENCODER_DT);
    
    if (clkState != lastEncoderARaw) {
        if (clkState == dtState) {
            encoderPos++;
        } else {
            encoderPos--;
        }
    }
    lastEncoderARaw = clkState;
}

InputState getInputState() {
    InputState state;

    // Угол из энкодера
    state.encoderAngle = (encoderPos * 10) % 360;
    if (state.encoderAngle < 0) state.encoderAngle += 360;

    // Чтение кнопок
    int swState = digitalRead(ENCODER_SW);
    int clkState = digitalRead(ENCODER_CLK);
    int dtState = digitalRead(ENCODER_DT);

    state.encoderPressed = (swState == LOW);
    state.buttonA = (digitalRead(BUTTON_A) == LOW);
    state.buttonB = (digitalRead(BUTTON_B) == LOW);

    // --- Логирование в стиле тестового проекта ---
    //Serial.printf("Pos: %5d | Angle: %3d° | SW: %d | CLK: %d | DT: %d\n", 
    //              encoderPos, state.encoderAngle, state.encoderPressed, clkState, dtState);

    static bool lastSW = false;
    if (state.encoderPressed && !lastSW) {
        Serial.println(">>> ЭНКОДЕР НАЖАТ!");
    }
    if (!state.encoderPressed && lastSW) {
        Serial.println(">>> ЭНКОДЕР ОТПУЩЕН!");
    }
    lastSW = state.encoderPressed;

    static bool lastA = false, lastB = false;
    if (state.buttonA && !lastA) Serial.println(">>> КНОПКА A НАЖАТА!");
    if (!state.buttonA && lastA) Serial.println(">>> КНОПКА A ОТПУЩЕНА!");
    if (state.buttonB && !lastB) Serial.println(">>> КНОПКА B НАЖАТА!");
    if (!state.buttonB && lastB) Serial.println(">>> КНОПКА B ОТПУЩЕНА!");
    lastA = state.buttonA;
    lastB = state.buttonB;

    return state;
}

// ==================== ИГРОВАЯ ЛОГИКА ====================
void changeState(GameState newState) {
    currentState = newState;
    switch (newState) {
        case STATE_MENU: bgCurrent = bgStart; break;
        case STATE_PLAY: bgCurrent = bgMain; break;
        case STATE_GAME_OVER: bgCurrent = bgGameOver; break;
    }
}


void resetGame() {
    playerShip.base.x = SCREEN_WIDTH / 2;
    playerShip.base.y = SCREEN_HEIGHT / 2;
    playerShip.base.vx = 0;
    playerShip.base.vy = 0;
    playerShip.rotation = 0;
    playerShip.boosting = false;
    playerShip.base.active = true;
    playerShip.lastShot = 0;

    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].base.active = false;
    for (int i = 0; i < MAX_ASTEROIDS; i++) asteroids[i].base.active = false;

    activeAsteroids = 0;
    score = 0;
}

void spawnAsteroid(bool forceComet = false) {
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!asteroids[i].base.active) {
            uint8_t side = random(4);
            switch (side) {
                case 0: asteroids[i].base.x = -20; asteroids[i].base.y = random(SCREEN_HEIGHT); break;
                case 1: asteroids[i].base.x = SCREEN_WIDTH + 20; asteroids[i].base.y = random(SCREEN_HEIGHT); break;
                case 2: asteroids[i].base.x = random(SCREEN_WIDTH); asteroids[i].base.y = -20; break;
                case 3: asteroids[i].base.x = random(SCREEN_WIDTH); asteroids[i].base.y = SCREEN_HEIGHT + 20; break;
            }

            float targetX = SCREEN_WIDTH / 2 + random(-20, 20);
            float targetY = SCREEN_HEIGHT / 2 + random(-20, 20);
            float dx = targetX - asteroids[i].base.x;
            float dy = targetY - asteroids[i].base.y;
            float distance = sqrt(dx * dx + dy * dy);

            asteroids[i].base.vx = (dx / distance) * ASTEROID_BASE_SPEED;
            asteroids[i].base.vy = (dy / distance) * ASTEROID_BASE_SPEED;

            asteroids[i].isComet = forceComet || (random(100) < (score * 5));
            if (asteroids[i].isComet) {
                asteroids[i].base.vx *= COMET_SPEED_MULTIPLIER;
                asteroids[i].base.vy *= COMET_SPEED_MULTIPLIER;
            }

            asteroids[i].base.active = true;
            activeAsteroids++;
            break;
        }
    }
}

void checkCollisions() {
    // Пули -> Астероиды
    for (int b = 0; b < MAX_BULLETS; b++) {
        if (!bullets[b].base.active) continue;

        for (int a = 0; a < MAX_ASTEROIDS; a++) {
            if (!asteroids[a].base.active) continue;

            float dx = bullets[b].base.x - asteroids[a].base.x;
            float dy = bullets[b].base.y - asteroids[a].base.y;
            float distanceSq = dx * dx + dy * dy;

            if (distanceSq < 64.0f) {
                bullets[b].base.active = false;
                asteroids[a].base.active = false;
                activeAsteroids--;
                score++;
                playSoundEffect(400, 150);
                spawnAsteroid();
                break;
            }
        }
    }

    // Корабль -> Астероиды
    for (int a = 0; a < MAX_ASTEROIDS; a++) {
        if (!asteroids[a].base.active) continue;

        float dx = playerShip.base.x - asteroids[a].base.x;
        float dy = playerShip.base.y - asteroids[a].base.y;
        float distanceSq = dx * dx + dy * dy;

        if (distanceSq < 144.0f) {
            playerShip.base.active = false;
            playSoundEffect(200, 400);
            changeState(STATE_GAME_OVER);
            
            if (score > highScore) {
                highScore = score;
                writeHighScore(highScore);
            }
            break;
        }
    }
}

// ==================== КЭШ СПРАЙТОВ В PSRAM ====================
struct SpriteData {
  String name;
  uint8_t* data;
  size_t size;
};

std::vector<SpriteData> spriteCache;

// Загружаем все спрайты в PSRAM
void loadAllSpritesToPSRAM() {
  const char* files[] = {
    LOGO_FILE, START_BG_FILE, MAIN_BG_FILE, GAMEOVER_BG_FILE, NAME1_FILE, NAME2_FILE, PRESS_FILE, ASTEROID_FILE, BOOM_BIG_FILE, 
    BOOM_SMALL_FILE, BULLET_FILE, COMET_FILE, GAMEOVER_TEXT_FILE, SHIP_BOOST_FILE, SHIP_STAY_FILE };
  size_t fileCount = sizeof(files) / sizeof(files[0]);

  for (size_t i = 0; i < fileCount; i++) {
    File f = SD_MMC.open(files[i], FILE_READ);
    if (!f) {
      Serial.printf("[ERR] Не найден файл %s\n", files[i]);
      continue;
    }
    size_t sz = f.size();
    uint8_t* buf = (uint8_t*)ps_malloc(sz);
    if (!buf) {
      Serial.printf("[ERR] Нет памяти для %s\n", files[i]);
      f.close();
      continue;
    }
    f.read(buf, sz);
    f.close();

    SpriteData s = {String(files[i]), buf, sz};
    spriteCache.push_back(s);
    Serial.printf("[OK] Загружен %s (%u байт)\n", files[i], sz);
  }
}

// Поиск спрайта в кеше
uint8_t* getSpriteFromCache(const char* filename, size_t& outSize) {
  for (auto& s : spriteCache) {
    if (s.name.equals(filename)) {
      outSize = s.size;
      return s.data;
    }
  }
  return nullptr;
}


// ==================== ОСНОВНЫЕ ФУНКЦИИ ====================
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    
    pinMode(ENCODER_CLK, INPUT_PULLUP);
    pinMode(ENCODER_DT, INPUT_PULLUP);
    pinMode(ENCODER_SW, INPUT);
    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(BUTTON_B, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    Serial.printf("ENCODER_SW pin mode: %d\n", digitalRead(ENCODER_SW));
    Serial.printf("ENCODER_CLK pin mode: %d\n", digitalRead(ENCODER_CLK));
    Serial.printf("ENCODER_DT pin mode: %d\n", digitalRead(ENCODER_DT));

    lastEncoderARaw = digitalRead((gpio_num_t)ENCODER_CLK);
    attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handleEncoderISR, CHANGE);

    if (!initSDCard()) {
        tft.drawString("SD Card Error", 20, 70, 2);
        while(1);
    }

    Serial.println("[INFO] Загружаем спрайты в PSRAM...");
    loadAllSpritesToPSRAM();
    Serial.println("[INFO] Спрайты готовы!");

    bgStart = (uint16_t*)getSpriteFromCache(START_BG_FILE, bgSizeStart);
    bgMain  = (uint16_t*)getSpriteFromCache(MAIN_BG_FILE, bgSizeMain);
    bgGameOver = (uint16_t*)getSpriteFromCache(GAMEOVER_BG_FILE, bgSizeGameOver);
    
    highScore = readHighScore();
    changeState(STATE_LOGO);
}

void loop() {
    uint32_t now = millis();
    if (now - lastFrameTime < 33) return; // 30 FPS
    lastFrameTime = now;
    InputState input = getInputState();

    uint32_t currentTime = millis();

        
        switch (currentState) {

            case STATE_LOGO:
                drawImageFromPSRAM(LOGO_FILE, 0, 0, 128, 160);
                if (currentTime - stateStartTime >= LOGO_DISPLAY_TIME) {
                    changeState(STATE_MENU);
                    //playRandomTrack(introTracks, 3);
                }
                drawLogo();
                break;
                
            case STATE_MENU:
                tft.pushImage(0, 0, BG_WIDTH, BG_HEIGHT, bgCurrent);
                if ((now / 700) % 2 == 0)
                    drawImageFromPSRAM(PRESS_FILE, 40, 120, PRESS_WIDTH, PRESS_HEIGHT);
                if (input.encoderPressed) {
                    changeState(STATE_PLAY);
                    //playRandomTrack(mainTracks, 5);
                }
                
                drawMenuBackground();
                
                // Анимация названия
                if ((currentTime / 1000) % 2 == 0) {
                    drawImageFromPSRAM(NAME1_FILE, 5, 20, NAME_WIDTH, NAME_HEIGHT);
                } else {
                    drawImageFromPSRAM(NAME2_FILE, 5, 20, NAME_WIDTH, NAME_HEIGHT);
                }
                
                // Мигающая надпись
                if ((currentTime / 500) % 2 == 0) {
                    drawImageFromPSRAM(PRESS_FILE, 40, 120, PRESS_WIDTH, PRESS_HEIGHT);
                    digitalWrite(LED_PIN, HIGH);
                } else {
                    digitalWrite(LED_PIN, LOW);
                }
                break;
                
            case STATE_PLAY:
                // Обновление игры

                bgCurrent = bgMain;

                playerShip.rotation = input.encoderAngle;

                if (input.buttonA) {
                    float rad = playerShip.rotation * PI / 180.0f;
                    playerShip.base.vx += cos(rad) * SHIP_SPEED;
                    playerShip.base.vy += sin(rad) * SHIP_SPEED;
                    playerShip.boosting = true;
                } else {
                    playerShip.boosting = false;
                }

                // Движение корабля
                playerShip.base.x += playerShip.base.vx;
                playerShip.base.y += playerShip.base.vy;

                // Телепортация
                if (playerShip.base.x < 0) playerShip.base.x = SCREEN_WIDTH;
                if (playerShip.base.x > SCREEN_WIDTH) playerShip.base.x = 0;
                if (playerShip.base.y < 0) playerShip.base.y = SCREEN_HEIGHT;
                if (playerShip.base.y > SCREEN_HEIGHT) playerShip.base.y = 0;

                // Стрельба
                if (input.buttonB && currentTime - playerShip.lastShot > BULLET_DELAY) {
                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (!bullets[i].base.active) {
                            float rad = playerShip.rotation * PI / 180.0f;
                            bullets[i].base.x = playerShip.base.x;
                            bullets[i].base.y = playerShip.base.y;
                            bullets[i].base.vx = cos(rad) * BULLET_SPEED;
                            bullets[i].base.vy = sin(rad) * BULLET_SPEED;
                            bullets[i].base.active = true;
                            bullets[i].spawnTime = currentTime;
                            playerShip.lastShot = currentTime;
                            playSoundEffect(2000, 100);
                            break;
                        }
                    }
                }

                // Обновление пуль и астероидов
                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (bullets[i].base.active) {
                        bullets[i].base.x += bullets[i].base.vx;
                        bullets[i].base.y += bullets[i].base.vy;
                        if (currentTime - bullets[i].spawnTime > 2000) bullets[i].base.active = false;
                    }
                }

                for (int i = 0; i < MAX_ASTEROIDS; i++) {
                    if (asteroids[i].base.active) {
                        asteroids[i].base.x += asteroids[i].base.vx;
                        asteroids[i].base.y += asteroids[i].base.vy;
                        
                        if (asteroids[i].base.x < -20) asteroids[i].base.x = SCREEN_WIDTH + 20;
                        if (asteroids[i].base.x > SCREEN_WIDTH + 20) asteroids[i].base.x = -20;
                        if (asteroids[i].base.y < -20) asteroids[i].base.y = SCREEN_HEIGHT + 20;
                        if (asteroids[i].base.y > SCREEN_HEIGHT + 20) asteroids[i].base.y = -20;
                    }
                }

                // Спавн астероидов
                if (activeAsteroids < 1 + (score / 5)) {
                    spawnAsteroid();
                }

                checkCollisions();

                // Отрисовка игры
                drawGameBackground();
                static uint32_t lastBoostAnimTime = 0;
if (playerShip.boosting) {
    if (millis() - lastBoostAnimTime > 80) {
        playerShip.boostFrame = (playerShip.boostFrame + 1) % 3;
        lastBoostAnimTime = millis();
    }
}

drawShipFromPSRAM(
    (int)playerShip.base.x,
    (int)playerShip.base.y,
    playerShip.rotation,
    playerShip.boosting,
    playerShip.boostFrame
);

                
                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (bullets[i].base.active) {
                        float angle = atan2(bullets[i].base.vy, bullets[i].base.vx) * 180 / PI;
                        drawBullet(bullets[i].base.x, bullets[i].base.y, angle);
                    }
                }

                for (int i = 0; i < MAX_ASTEROIDS; i++) {
                    if (asteroids[i].base.active) {
                        float direction = atan2(asteroids[i].base.vy, asteroids[i].base.vx) * 180 / PI;
                        drawAsteroid(asteroids[i].base.x, asteroids[i].base.y, asteroids[i].size, asteroids[i].isComet, direction);
                    }
                }

                drawScore(score);
                break;
                
            case STATE_GAME_OVER:
                if (input.encoderPressed) {
                    resetGame();
                    changeState(STATE_MENU);
                    playRandomTrack(introTracks, 3);
                }
                
                drawGameOverBackground();
                
                char scoreText[32];
                snprintf(scoreText, sizeof(scoreText), "SCORE: %d", score);
                tft.drawString(scoreText, SCREEN_WIDTH/2 - 30, SCREEN_HEIGHT/2 - 10, 2);
                
                snprintf(scoreText, sizeof(scoreText), "BEST: %d", highScore);
                tft.drawString(scoreText, SCREEN_WIDTH/2 - 25, SCREEN_HEIGHT/2 + 10, 2);
                break;
        }
        
        frameCount++;

  // Каждые 5 секунд печатаем статистику
  //unsigned long now = millis();
  if (now - lastStatsTime >= 5000) {
    lastStatsTime = now;
    printStats();
  }
    } 
/*    delay(1);
}*/