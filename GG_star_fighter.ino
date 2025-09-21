#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SD_MMC.h>
#include <driver/gpio.h>
#include <driver/i2s.h>
#include <cmath>

// ==================== КОНСТАНТЫ ПИНОВ ====================
constexpr uint8_t ENCODER_CLK = 32;
constexpr uint8_t ENCODER_DT = 33;
constexpr uint8_t ENCODER_SW = 14;
constexpr uint8_t BUTTON_A = 2;
constexpr uint8_t BUTTON_B = 19;
constexpr uint8_t I2S_BCK_PIN = 26;
constexpr uint8_t I2S_WS_PIN = 25;
constexpr uint8_t I2S_DOUT_PIN = 22;
constexpr uint8_t BUZZER_PIN = 13;

// ==================== КОНСТАНТЫ ЭКРАНА ====================
constexpr int16_t SCREEN_WIDTH = 128;
constexpr int16_t SCREEN_HEIGHT = 160;

// ==================== КОНСТАНТЫ ПУТЕЙ ФАЙЛОВ ====================
constexpr const char* HIGHSCORE_FILE = "/highscore.txt";
constexpr const char* LOGO_FILE = "/spr_GG_logo.bin";
constexpr const char* START_BG_FILE = "/spr_start_BG.bin";
constexpr const char* MAIN_BG_FILE = "/spr_main_BG.bin";
constexpr const char* GAMEOVER_BG_FILE = "/spr_GO_BG.bin";
constexpr const char* NAME1_FILE = "/spr_main_name.bin";
constexpr const char* NAME2_FILE = "/spr_main_name2.bin";
constexpr const char* PRESS_FILE = "/spr_press_RB.bin";

// ==================== КОНСТАНТЫ РАЗМЕРОВ СПРАЙТОВ ====================
constexpr uint8_t SHIP_WIDTH = 17;
constexpr uint8_t SHIP_HEIGHT = 17;
constexpr uint8_t BULLET_WIDTH = 4;
constexpr uint8_t BULLET_HEIGHT = 4;
constexpr uint8_t ASTEROID_WIDTH = 9;
constexpr uint8_t ASTEROID_HEIGHT = 10;
constexpr uint8_t COMET_WIDTH = 24;
constexpr uint8_t COMET_HEIGHT = 12;
constexpr uint8_t EXPLOSION_WIDTH = 24;
constexpr uint8_t EXPLOSION_HEIGHT = 23;
constexpr uint8_t LOGO_WIDTH = 128;
constexpr uint8_t LOGO_HEIGHT = 160;
constexpr uint8_t BG_WIDTH = 128;
constexpr uint8_t BG_HEIGHT = 160;
constexpr uint8_t NAME_WIDTH = 117;
constexpr uint8_t NAME_HEIGHT = 48;
constexpr uint8_t PRESS_WIDTH = 48;
constexpr uint8_t PRESS_HEIGHT = 15;

// ==================== КОНСТАНТЫ ИГРЫ ====================
constexpr uint8_t MAX_BULLETS = 5;       
constexpr uint8_t MAX_ASTEROIDS = 10;    
constexpr uint32_t BULLET_DELAY = 300;
constexpr float SHIP_SPEED = 2.0f;
constexpr float BULLET_SPEED = 4.0f;
constexpr float ASTEROID_BASE_SPEED = 1.0f;
constexpr float COMET_SPEED_MULTIPLIER = 1.5f;
constexpr float SHIP_COLLISION_RADIUS = 6.0f;
constexpr float BULLET_COLLISION_RADIUS = 2.0f;
constexpr float ASTEROID_RADIUS_BASE = 4.0f;
constexpr uint32_t LOGO_DISPLAY_TIME = 2000;
constexpr unsigned long DEBOUNCE_MS = 50;

// ==================== ПРЕДВЫЧИСЛЕННАЯ ТАБЛИЦА КОСИНУСОВ ====================
constexpr int8_t COS_TABLE[36] = {
    100, 98, 94, 87, 77, 64, 50, 34, 17, 0,
    -17, -34, -50, -64, -77, -87, -94, -98, -100,
    -98, -94, -87, -77, -64, -50, -34, -17, 0,
    17, 34, 50, 64, 77, 87, 94, 98
};

// ==================== СТРУКТУРЫ ====================
struct InputState {
    int16_t encoderAngle;
    bool encoderPressed;
    bool buttonA;
    bool buttonB;
};

struct WavInfo {
    uint32_t dataPos;
    uint32_t dataLen;
    uint16_t channels;
    uint32_t sampleRate;
    uint16_t bitsPerSample;
};

struct Entity {
    float x, y;
    float vx, vy;
    bool active;
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

// ==================== ПЕРЕЧИСЛЕНИЯ ====================
enum SoundEffect {
    SOUND_LASER,
    SOUND_HIT,
    SOUND_EXPLOSION,
    SOUND_NONE
};

enum GameState {
    STATE_LOGO,
    STATE_MENU,
    STATE_PLAY,
    STATE_GAME_OVER
};

// ==================== КЛАСС ХРАНИЛИЩА ====================
class StorageManager {
public:
    bool init();
    int readHighScore();
    void writeHighScore(int score);
    File openFile(const char* filename, const char* mode = FILE_READ);

private:
    static const char* HIGHSCORE_FILE;
};

// ==================== КЛАСС ВВОДА ====================
class InputManager {
public:
    bool init();
    void update();
    InputState getState() const;

private:
    static void handleEncoderISR();

    InputState currentState;

    static volatile int32_t encoderPos;
    static volatile int lastEncoderARaw;

    static bool encoderSwRaw;
    static bool encoderSwStable;
    static bool lastEncoderSwStable;
    static unsigned long encoderSwLastDebounce;

    static bool btnARaw;
    static bool btnAStable;
    static bool lastBtnAStable;
    static unsigned long btnALastDebounce;

    static bool btnBRaw;
    static bool btnBStable;
    static bool lastBtnBStable;
    static unsigned long btnBLastDebounce;

    static bool encoderPressed;
};

// ==================== КЛАСС ГРАФИКИ ====================
class Graphics {
public:
    bool init();
    void present();
    void clear();

    void drawLogo();
    void drawStartMenu(uint8_t frame);
    void drawGameBackground();
    void drawGameOver(uint16_t score, uint16_t highScore);

    void drawShip(int16_t x, int16_t y, uint16_t angle, bool isBoosting = false, uint8_t boostFrame = 0);
    void drawBullet(int16_t x, int16_t y, uint16_t angle);
    void drawAsteroid(int16_t x, int16_t y, uint8_t size);
    void drawComet(int16_t x, int16_t y, uint16_t direction);
    void drawExplosion(int16_t x, int16_t y, uint8_t frame);
    void drawScore(uint16_t score);

    static int16_t centerX(int16_t x, uint8_t width) { return x - width / 2; }
    static int16_t centerY(int16_t y, uint8_t height) { return y - height / 2; }

private:
    void drawSpriteFromSD(int16_t x, int16_t y, const char* filename, uint8_t w, uint8_t h, uint8_t bpp = 4);
    bool isSpriteVisible(int16_t x, int16_t y, uint8_t w, uint8_t h);

    TFT_eSPI tft;
    uint16_t lineBuf[512]; // Статический буфер для отрисовки
};

// ==================== КЛАСС АУДИО ====================
class AudioManager {
public:
    bool init();
    void update();

    void playRandomIntro();
    void playRandomMain();
    void playRandomGameOver();
    void stopMusic();

    void playLaser() { tone(BUZZER_PIN, 2000, 100); }
    void playHit() { tone(BUZZER_PIN, 400, 150); }
    void playCrash() { tone(BUZZER_PIN, 200, 400); }

    bool isPlaying() const { return isPlayingMusic; }

private:
    void playWAV(const char* filename);
    WavInfo parseWav(File& file);
    void musicTask(void* filename);

    static const char* introTracks[3];
    static const char* mainTracks[5];
    static const char* gameOverTracks[3];

    bool isPlayingMusic = false;
    TaskHandle_t musicTaskHandle = NULL;
};

// ==================== КЛАСС ИГРЫ ====================
class GameManager {
public:
    void init();
    void update();
    void render();
    void changeState(GameState newState);

    GameState getCurrentState() const { return currentState; }
    int getScore() const { return score; }
    int getHighScore() const { return highScore; }

private:
    void resetGame();
    void spawnAsteroid(bool forceComet = false);
    void updateLogo();
    void updateMenu(const InputState& input);
    void updatePlay(const InputState& input);
    void updateGameOver(const InputState& input);
    void checkCollisions();
    void checkHighScore();

    Ship playerShip;
    Bullet bullets[MAX_BULLETS];
    Asteroid asteroids[MAX_ASTEROIDS];

    GameState currentState = STATE_LOGO;
    int score = 0;
    int highScore = 0;
    uint32_t stateStartTime = 0;
    uint32_t lastBulletTime = 0;
    uint32_t lastAsteroidSpawn = 0;
    uint8_t activeBullets = 0;
    uint8_t activeAsteroids = 0;
};

// ==================== СТАТИЧЕСКИЕ ПЕРЕМЕННЫЕ ====================
// InputManager
volatile int32_t InputManager::encoderPos = 0;
volatile int InputManager::lastEncoderARaw = 0;
bool InputManager::encoderSwRaw = false;
bool InputManager::encoderSwStable = false;
bool InputManager::lastEncoderSwStable = false;
unsigned long InputManager::encoderSwLastDebounce = 0;
bool InputManager::btnARaw = false;
bool InputManager::btnAStable = false;
bool InputManager::lastBtnAStable = false;
unsigned long InputManager::btnALastDebounce = 0;
bool InputManager::btnBRaw = false;
bool InputManager::btnBStable = false;
bool InputManager::lastBtnBStable = false;
unsigned long InputManager::btnBLastDebounce = 0;
bool InputManager::encoderPressed = false;

// AudioManager
const char* AudioManager::introTracks[3] = {
    "/snd_intro1.wav", "/snd_intro2.wav", "/snd_intro3.wav"
};
const char* AudioManager::mainTracks[5] = {
    "/snd_main1.wav", "/snd_main2.wav", "/snd_main3.wav",
    "/snd_main4.wav", "/snd_main5.wav"
};
const char* AudioManager::gameOverTracks[3] = {
    "/snd_gameover1.wav", "/snd_gameover2.wav", "/snd_gameover3.wav"
};

// StorageManager
const char* StorageManager::HIGHSCORE_FILE = "/highscore.txt";

// ==================== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ====================
TFT_eSPI tft;
Graphics graphics;
InputManager input;
AudioManager audio;
GameManager game;
StorageManager storage;

// ==================== РЕАЛИЗАЦИЯ STORAGEMANAGER ====================
bool StorageManager::init() {
    if (!SD_MMC.begin("/sdcard", true)) {
        return false;
    }

    if (!SD_MMC.exists(HIGHSCORE_FILE)) {
        File file = SD_MMC.open(HIGHSCORE_FILE, FILE_WRITE);
        if (file) {
            file.println("0");
            file.close();
        }
    }
    return true;
}

int StorageManager::readHighScore() {
    File file = SD_MMC.open(HIGHSCORE_FILE, FILE_READ);
    if (!file) return 0;

    // Исправление: эффективное чтение числа
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

void StorageManager::writeHighScore(int score) {
    File file = SD_MMC.open(HIGHSCORE_FILE, FILE_WRITE);
    if (file) {
        file.seek(0);
        file.println(score);
        file.close();
    }
}

File StorageManager::openFile(const char* filename, const char* mode) {
    return SD_MMC.open(filename, mode);
}

// ==================== РЕАЛИЗАЦИЯ INPUTMANAGER ====================
void IRAM_ATTR InputManager::handleEncoderISR() {
    int A = gpio_get_level((gpio_num_t)ENCODER_CLK);
    int B = gpio_get_level((gpio_num_t)ENCODER_DT);

    if (A != lastEncoderARaw) {
        if (A == B) {
            encoderPos++;
        }
        else {
            encoderPos--;
        }
    }
    lastEncoderARaw = A;
}

bool InputManager::init() {
    pinMode(ENCODER_CLK, INPUT_PULLUP);
    pinMode(ENCODER_DT, INPUT_PULLUP);
    pinMode(ENCODER_SW, INPUT_PULLUP);
    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(BUTTON_B, INPUT_PULLUP);

    lastEncoderARaw = gpio_get_level((gpio_num_t)ENCODER_CLK);

    encoderSwRaw = (digitalRead(ENCODER_SW) == LOW);
    encoderSwStable = encoderSwRaw;
    lastEncoderSwStable = encoderSwStable;

    btnARaw = (digitalRead(BUTTON_A) == LOW);
    btnAStable = btnARaw;
    lastBtnAStable = btnAStable;

    btnBRaw = (digitalRead(BUTTON_B) == LOW);
    btnBStable = btnBRaw;
    lastBtnBStable = btnBStable;

    attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handleEncoderISR, CHANGE);

    return true;
}

void InputManager::update() {
    encoderPressed = false;
    currentState.encoderAngle = (encoderPos * 10) % 360;
    if (currentState.encoderAngle < 0) currentState.encoderAngle += 360;

    unsigned long now = millis();

    // Дебаунс кнопки энкодера
    bool swReading = (digitalRead(ENCODER_SW) == LOW);
    if (swReading != encoderSwRaw) {
        encoderSwRaw = swReading;
        encoderSwLastDebounce = now;
    }
    else if ((now - encoderSwLastDebounce) > DEBOUNCE_MS) {
        if (encoderSwRaw != encoderSwStable) {
            lastEncoderSwStable = encoderSwStable;
            encoderSwStable = encoderSwRaw;

            if (encoderSwStable && !lastEncoderSwStable) {
                encoderPressed = true;
            }
        }
    }

    // Дебаунс кнопки A
    bool aReading = (digitalRead(BUTTON_A) == LOW);
    if (aReading != btnARaw) {
        btnARaw = aReading;
        btnALastDebounce = now;
    }
    else if ((now - btnALastDebounce) > DEBOUNCE_MS) {
        btnAStable = btnARaw;
    }

    // Дебаунс кнопки B
    bool bReading = (digitalRead(BUTTON_B) == LOW);
    if (bReading != btnBRaw) {
        btnBRaw = bReading;
        btnBLastDebounce = now;
    }
    else if ((now - btnBLastDebounce) > DEBOUNCE_MS) {
        btnBStable = btnBRaw;
    }

    currentState.encoderPressed = encoderPressed;
    currentState.buttonA = btnAStable;
    currentState.buttonB = btnBStable;
}

InputState InputManager::getState() const {
    return currentState;
}

// ==================== РЕАЛИЗАЦИЯ AUDIOMANAGER ====================
bool AudioManager::init() {
    // Инициализация I2S для WAV файлов
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 6,
        .dma_buf_len = 64,
        .use_apll = false
    };
    
    if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) != ESP_OK) {
        return false;
    }

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_DOUT_PIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    
    i2s_set_pin(I2S_NUM_0, &pin_config);

    // Инициализация бузера
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    return true;
}

void AudioManager::musicTask(void* filename) {
    const char* path = (const char*)filename;
    File file = SD_MMC.open(path);
    if (!file) {
        isPlayingMusic = false;
        musicTaskHandle = NULL;
        vTaskDelete(NULL);
        return;
    }

    WavInfo info = parseWav(file);
    if (info.dataLen == 0) {
        file.close();
        isPlayingMusic = false;
        musicTaskHandle = NULL;
        vTaskDelete(NULL);
        return;
    }

    file.seek(info.dataPos);

    // Исправление: статический буфер для экономии стека
    static uint8_t buffer[256];
    size_t bytesRead;

    isPlayingMusic = true;
    while (file.available() && isPlayingMusic) {
        bytesRead = file.read(buffer, sizeof(buffer));
        if (bytesRead > 0) {
            size_t bytesWritten;
            i2s_write(I2S_NUM_0, buffer, bytesRead, &bytesWritten, portMAX_DELAY);
        }
    }

    file.close();
    isPlayingMusic = false;
    musicTaskHandle = NULL;
    vTaskDelete(NULL);
}

WavInfo AudioManager::parseWav(File& file) {
    WavInfo info = {};

    if (file.size() < 44) return info;

    file.seek(22);
    file.read((uint8_t*)&info.channels, 2);
    file.read((uint8_t*)&info.sampleRate, 4);
    file.seek(34);
    file.read((uint8_t*)&info.bitsPerSample, 2);

    file.seek(36);
    uint32_t chunkId, chunkSize;

    while (file.available() > 8) {
        file.read((uint8_t*)&chunkId, 4);
        file.read((uint8_t*)&chunkSize, 4);

        if (chunkId == 0x61746164) { // "data"
            info.dataPos = file.position();
            info.dataLen = chunkSize;
            break;
        }

        if (chunkSize > file.available()) break;
        file.seek(file.position() + chunkSize);
    }

    return info;
}

void AudioManager::playWAV(const char* filename) {
    stopMusic();

    xTaskCreatePinnedToCore(
        [](void* param) { ((AudioManager*)param)->musicTask(param); },
        "AudioTask",
        4096,
        (void*)filename,
        1,
        &musicTaskHandle,
        1
    );
}

void AudioManager::playRandomIntro() {
    uint8_t idx = random(3);
    playWAV(introTracks[idx]);
}

void AudioManager::playRandomMain() {
    uint8_t idx = random(5);
    playWAV(mainTracks[idx]);
}

void AudioManager::playRandomGameOver() {
    uint8_t idx = random(3);
    playWAV(gameOverTracks[idx]);
}

void AudioManager::stopMusic() {
    isPlayingMusic = false;
    if (musicTaskHandle != NULL) {
        vTaskDelete(musicTaskHandle);
        musicTaskHandle = NULL;
    }
}

void AudioManager::update() {
    // Автопереключение треков в меню
    if (!isPlayingMusic && (game.getCurrentState() == STATE_MENU || game.getCurrentState() == STATE_PLAY)) {
        if (game.getCurrentState() == STATE_MENU) {
            playRandomIntro();
        } else {
            playRandomMain();
        }
    }
}

// ==================== РЕАЛИЗАЦИЯ GRAPHICS ====================
bool Graphics::init() {
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    return true;
}

void Graphics::present() {
}

void Graphics::clear() {
    tft.fillScreen(TFT_BLACK);
}

void Graphics::drawSpriteFromSD(int16_t x, int16_t y, const char* filename, uint8_t w, uint8_t h, uint8_t bpp) {
    if (!isSpriteVisible(x, y, w, h)) return;

    File file = storage.openFile(filename, FILE_READ);
    if (!file) return;

    // Исправление: гарантированное закрытие файла
    for (uint8_t row = 0; row < h; row++) {
        if (y + row < 0 || y + row >= SCREEN_HEIGHT) continue;
        
        if (bpp == 4) {
            for (uint8_t col = 0; col < w; col += 2) {
                uint8_t byte;
                if (file.read(&byte, 1) != 1) break;
                
                uint8_t nibble1 = (byte >> 4) & 0x0F;
                uint8_t nibble2 = byte & 0x0F;
                
                uint8_t v1 = nibble1 * 17;
                uint8_t v2 = nibble2 * 17;
                
                lineBuf[col] = ((v1 & 0xF8) << 8) | ((v1 & 0xFC) << 3) | (v1 >> 3);
                if (col + 1 < w) {
                    lineBuf[col + 1] = ((v2 & 0xF8) << 8) | ((v2 & 0xFC) << 3) | (v2 >> 3);
                }
            }
        } else if (bpp == 1) {
            for (uint8_t col = 0; col < w; col += 8) {
                uint8_t byte;
                if (file.read(&byte, 1) != 1) break;
                
                for (uint8_t bit = 0; bit < 8 && col + bit < w; bit++) {
                    bool on = (byte >> (7 - bit)) & 1;
                    lineBuf[col + bit] = on ? TFT_WHITE : TFT_BLACK;
                }
            }
        }
        
        tft.pushImage(x, y + row, w, 1, lineBuf);
    }
    file.close(); // Гарантированное закрытие
}

void Graphics::drawLogo() {
    drawSpriteFromSD(0, 0, LOGO_FILE, LOGO_WIDTH, LOGO_HEIGHT, 4);
}

void Graphics::drawStartMenu(uint8_t frame) {
    // Фон
    drawSpriteFromSD(0, 0, START_BG_FILE, BG_WIDTH, BG_HEIGHT, 4);
    
    // Анимация названия (меняется каждую секунду)
    if ((frame / 20) % 2 == 0) {
        drawSpriteFromSD(5, 20, NAME1_FILE, NAME_WIDTH, NAME_HEIGHT, 4);
    } else {
        drawSpriteFromSD(5, 20, NAME2_FILE, NAME_WIDTH, NAME_HEIGHT, 4);
    }
    
    // Мигающая надпись (меняется каждые 0.5 секунды)
    if ((frame / 10) % 2 == 0) {
        drawSpriteFromSD(40, 120, PRESS_FILE, PRESS_WIDTH, PRESS_HEIGHT, 4);
    }
}

void Graphics::drawGameBackground() {
    drawSpriteFromSD(0, 0, MAIN_BG_FILE, BG_WIDTH, BG_HEIGHT, 4);
}

void Graphics::drawGameOver(uint16_t score, uint16_t highScore) {
    drawSpriteFromSD(0, 0, GAMEOVER_BG_FILE, BG_WIDTH, BG_HEIGHT, 4);
    
    char scoreText[16];
    snprintf(scoreText, sizeof(scoreText), "SCORE: %d", score);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(scoreText, SCREEN_WIDTH / 2 - 30, SCREEN_HEIGHT / 2 - 10, 2);
    
    char highScoreText[20];
    snprintf(highScoreText, sizeof(highScoreText), "BEST: %d", highScore);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString(highScoreText, SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT / 2 + 10, 2);
}

void Graphics::drawScore(uint16_t score) {
    char scoreText[16];
    snprintf(scoreText, sizeof(scoreText), "SCORE: %d", score);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(scoreText, 5, 5, 2);
}

void Graphics::drawShip(int16_t x, int16_t y, uint16_t angle, bool isBoosting, uint8_t boostFrame) {
    if (!isSpriteVisible(x - SHIP_WIDTH/2, y - SHIP_HEIGHT/2, SHIP_WIDTH, SHIP_HEIGHT)) return;
    
    char filename[32];
    uint16_t normalizedAngle = (angle / 10) * 10;
    
    if (isBoosting) {
        snprintf(filename, sizeof(filename), "/spr_ship_boost_%03d_%d.bin", normalizedAngle, boostFrame + 1);
    } else {
        snprintf(filename, sizeof(filename), "/spr_ship_stay_%03d.bin", normalizedAngle);
    }
    
    drawSpriteFromSD(centerX(x, SHIP_WIDTH), centerY(y, SHIP_HEIGHT), filename, SHIP_WIDTH, SHIP_HEIGHT, 4);
}

void Graphics::drawBullet(int16_t x, int16_t y, uint16_t angle) {
    if (!isSpriteVisible(x - BULLET_WIDTH/2, y - BULLET_HEIGHT/2, BULLET_WIDTH, BULLET_HEIGHT)) return;
    
    char filename[24];
    uint16_t normalizedAngle = (angle / 10) * 10;
    snprintf(filename, sizeof(filename), "/spr_bullet_%03d.bin", normalizedAngle);
    
    drawSpriteFromSD(centerX(x, BULLET_WIDTH), centerY(y, BULLET_HEIGHT), filename, BULLET_WIDTH, BULLET_HEIGHT, 1);
}

void Graphics::drawAsteroid(int16_t x, int16_t y, uint8_t size) {
    if (!isSpriteVisible(x - ASTEROID_WIDTH/2, y - ASTEROID_HEIGHT/2, ASTEROID_WIDTH, ASTEROID_HEIGHT)) return;
    
    char filename[24];
    snprintf(filename, sizeof(filename), "/spr_asteroid_%d.bin", size);
    
    drawSpriteFromSD(centerX(x, ASTEROID_WIDTH), centerY(y, ASTEROID_HEIGHT), filename, ASTEROID_WIDTH, ASTEROID_HEIGHT, 4);
}

void Graphics::drawComet(int16_t x, int16_t y, uint16_t direction) {
    if (!isSpriteVisible(x - COMET_WIDTH/2, y - COMET_HEIGHT/2, COMET_WIDTH, COMET_HEIGHT)) return;
    
    char filename[24];
    uint16_t normalizedDirection = (direction / 10) * 10;
    snprintf(filename, sizeof(filename), "/spr_comet_%03d.bin", normalizedDirection);
    
    drawSpriteFromSD(centerX(x, COMET_WIDTH), centerY(y, COMET_HEIGHT), filename, COMET_WIDTH, COMET_HEIGHT, 4);
}

bool Graphics::isSpriteVisible(int16_t x, int16_t y, uint8_t w, uint8_t h) {
    return !(x + w <= 0 || x >= SCREEN_WIDTH || y + h <= 0 || y >= SCREEN_HEIGHT);
}

// ==================== РЕАЛИЗАЦИЯ GAMEMANAGER ====================
void GameManager::init() {
    highScore = storage.readHighScore();
    changeState(STATE_LOGO);
}

void GameManager::changeState(GameState newState) {
    currentState = newState;
    stateStartTime = millis();

    switch (newState) {
    case STATE_LOGO:
        break;
    case STATE_MENU:
        audio.playRandomIntro();
        break;
    case STATE_PLAY:
        resetGame();
        audio.playRandomMain();
        break;
    case STATE_GAME_OVER:
        checkHighScore();
        audio.playRandomGameOver();
        break;
    }
}

void GameManager::update() {
    InputState inputState = input.getState();

    switch (currentState) {
        case STATE_LOGO: updateLogo(); break;
        case STATE_MENU: updateMenu(inputState); break;
        case STATE_PLAY: updatePlay(inputState); break;
        case STATE_GAME_OVER: updateGameOver(inputState); break;
    }

    audio.update();
}

void GameManager::render() {
    graphics.clear();

    switch (currentState) {
    case STATE_LOGO:
        graphics.drawLogo();
        break;
    case STATE_MENU:
        graphics.drawStartMenu((millis() - stateStartTime) / 50);
        break;
    case STATE_PLAY:
        graphics.drawGameBackground();
        
        // Рисуем только если корабль активен
        if (playerShip.base.active) {
            graphics.drawShip(playerShip.base.x, playerShip.base.y,
                playerShip.rotation, playerShip.boosting);
        }

        // Рисуем пули
        for (uint8_t i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].base.active) {
                float angle = atan2(bullets[i].base.vy, bullets[i].base.vx) * 180 / M_PI;
                graphics.drawBullet(bullets[i].base.x, bullets[i].base.y, angle);
            }
        }

        // Рисуем астероиды
        for (uint8_t i = 0; i < MAX_ASTEROIDS; i++) {
            if (asteroids[i].base.active) {
                if (asteroids[i].isComet) {
                    float direction = atan2(asteroids[i].base.vy, asteroids[i].base.vx) * 180 / M_PI;
                    graphics.drawComet(asteroids[i].base.x, asteroids[i].base.y, direction);
                }
                else {
                    graphics.drawAsteroid(asteroids[i].base.x, asteroids[i].base.y, asteroids[i].size);
                }
            }
        }

        graphics.drawScore(score);
        break;
    case STATE_GAME_OVER:
        graphics.drawGameOver(score, highScore);
        break;
    }
}

void GameManager::updateLogo() {
    if (millis() - stateStartTime >= LOGO_DISPLAY_TIME) {
        changeState(STATE_MENU);
    }
}

void GameManager::updateMenu(const InputState& inputState) {
    if (inputState.encoderPressed) {
        changeState(STATE_PLAY);
    }
}

void GameManager::updatePlay(const InputState& inputState) {
    playerShip.rotation = inputState.encoderAngle;

    if (inputState.buttonA) {
        float rad = playerShip.rotation * M_PI / 180.0f;
        playerShip.base.vx += cos(rad) * SHIP_SPEED;
        playerShip.base.vy += sin(rad) * SHIP_SPEED;
        playerShip.boosting = true;
    }
    else {
        playerShip.boosting = false;
    }

    // Обновление позиции
    playerShip.base.x += playerShip.base.vx;
    playerShip.base.y += playerShip.base.vy;

    // Телепортация на границах
    if (playerShip.base.x < 0) playerShip.base.x = SCREEN_WIDTH;
    if (playerShip.base.x > SCREEN_WIDTH) playerShip.base.x = 0;
    if (playerShip.base.y < 0) playerShip.base.y = SCREEN_HEIGHT;
    if (playerShip.base.y > SCREEN_HEIGHT) playerShip.base.y = 0;

    // Стрельба
    if (inputState.buttonB && millis() - lastBulletTime > BULLET_DELAY && activeBullets < MAX_BULLETS) {
        for (uint8_t i = 0; i < MAX_BULLETS; i++) {
            if (!bullets[i].base.active) {
                float rad = playerShip.rotation * M_PI / 180.0f;
                bullets[i].base.x = playerShip.base.x;
                bullets[i].base.y = playerShip.base.y;
                bullets[i].base.vx = cos(rad) * BULLET_SPEED;
                bullets[i].base.vy = sin(rad) * BULLET_SPEED;
                bullets[i].base.active = true;
                bullets[i].spawnTime = millis();
                activeBullets++;
                lastBulletTime = millis();
                audio.playLaser();
                break;
            }
        }
    }

    // Обновление пуль
    for (uint8_t i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].base.active) {
            bullets[i].base.x += bullets[i].base.vx;
            bullets[i].base.y += bullets[i].base.vy;

            // Удаление старых пуль
            if (millis() - bullets[i].spawnTime > 2000) {
                bullets[i].base.active = false;
                activeBullets--;
            }
        }
    }

    // Спавн астероидов
    if (activeAsteroids < 1 + (score / 5) && millis() - lastAsteroidSpawn > 2000) {
        spawnAsteroid();
        lastAsteroidSpawn = millis();
    }

    // Обновление астероидов
    for (uint8_t i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].base.active) {
            asteroids[i].base.x += asteroids[i].base.vx;
            asteroids[i].base.y += asteroids[i].base.vy;

            // Телепортация астероидов
            if (asteroids[i].base.x < -20) asteroids[i].base.x = SCREEN_WIDTH + 20;
            if (asteroids[i].base.x > SCREEN_WIDTH + 20) asteroids[i].base.x = -20;
            if (asteroids[i].base.y < -20) asteroids[i].base.y = SCREEN_HEIGHT + 20;
            if (asteroids[i].base.y > SCREEN_HEIGHT + 20) asteroids[i].base.y = -20;
        }
    }

    checkCollisions();
}

void GameManager::checkCollisions() {
    // Пули -> Астероиды
    for (uint8_t b = 0; b < MAX_BULLETS; b++) {
        if (!bullets[b].base.active) continue;

        for (uint8_t a = 0; a < MAX_ASTEROIDS; a++) {
            if (!asteroids[a].base.active) continue;

            float dx = bullets[b].base.x - asteroids[a].base.x;
            float dy = bullets[b].base.y - asteroids[a].base.y;
            float distanceSq = dx * dx + dy * dy;

            if (distanceSq < 64.0f) { // 8^2 = 64
                bullets[b].base.active = false;
                asteroids[a].base.active = false;
                activeBullets--;
                activeAsteroids--;
                score++;
                audio.playHit();
                break;
            }
        }
    }

    // Корабль -> Астероиды
    for (uint8_t a = 0; a < MAX_ASTEROIDS; a++) {
        if (!asteroids[a].base.active) continue;

        float dx = playerShip.base.x - asteroids[a].base.x;
        float dy = playerShip.base.y - asteroids[a].base.y;
        float distanceSq = dx * dx + dy * dy;

        if (distanceSq < 144.0f) { // 12^2 = 144
            playerShip.base.active = false;
            changeState(STATE_GAME_OVER);
            audio.playCrash();
            break;
        }
    }
}

void GameManager::updateGameOver(const InputState& inputState) {
    if (inputState.encoderPressed) {
        changeState(STATE_MENU);
    }
}

void GameManager::resetGame() {
    playerShip.base.x = SCREEN_WIDTH / 2;
    playerShip.base.y = SCREEN_HEIGHT / 2;
    playerShip.base.vx = 0;
    playerShip.base.vy = 0;
    playerShip.rotation = 0;
    playerShip.boosting = false;
    playerShip.base.active = true;

    for (uint8_t i = 0; i < MAX_BULLETS; i++) {
        bullets[i].base.active = false;
    }
    for (uint8_t i = 0; i < MAX_ASTEROIDS; i++) {
        asteroids[i].base.active = false;
    }

    activeBullets = 0;
    activeAsteroids = 0;
    score = 0;

    spawnAsteroid();
}

void GameManager::spawnAsteroid(bool forceComet) {
    for (uint8_t i = 0; i < MAX_ASTEROIDS; i++) {
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

            bool isComet = forceComet || (random(100) < (score * 5));
            if (isComet) {
                asteroids[i].base.vx *= COMET_SPEED_MULTIPLIER;
                asteroids[i].base.vy *= COMET_SPEED_MULTIPLIER;
                asteroids[i].isComet = true;
            } else {
                asteroids[i].isComet = false;
            }

            asteroids[i].base.active = true;
            activeAsteroids++;
            break;
        }
    }
}

void GameManager::checkHighScore() {
    if (score > highScore) {
        highScore = score;
        storage.writeHighScore(highScore);
    }
}

// ==================== ОСНОВНОЙ КОД ====================
void setup() {
    Serial.begin(115200);
    delay(10000);
    graphics.init();
    input.init();
    audio.init();
    storage.init();
    game.init();
}

void loop() {
    uint32_t frameStart = millis();
    
    input.update();
    game.update();
    audio.update();
    
    graphics.clear();
    game.render();
    graphics.present();
    
    uint32_t frameTime = millis() - frameStart;
    if (frameTime < 33) delay(33 - frameTime);
}