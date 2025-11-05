#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SD_MMC.h>
#include <driver/gpio.h>
#include <driver/i2s.h>
#include <cmath>
#include <vector>

// ==================== КОНСТАНТЫ ПИНОВ ====================
constexpr uint8_t ENCODER_CLK = 32;
constexpr uint8_t ENCODER_DT = 33;
constexpr uint8_t ENCODER_SW = 34;
constexpr uint8_t BUTTON_A = 35;
constexpr uint8_t BUTTON_B = 19;
constexpr uint8_t I2S_BCK_PIN = 26;
constexpr uint8_t I2S_WS_PIN = 25;
constexpr uint8_t I2S_DOUT_PIN = 22;
constexpr uint8_t BUZZER_PIN = 13;
constexpr uint8_t LED_PIN = 27;

// ==================== КОНСТАНТЫ ЭКРАНА ====================
constexpr int16_t SCREEN_WIDTH = 128;
constexpr int16_t SCREEN_HEIGHT = 160;
#define TFT_TRANSPARENT_COLOR 0x1FF8

// === Настройки фиксированных параметров WAV ===
#define AUDIO_SAMPLE_RATE   8000
#define AUDIO_BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT
#define AUDIO_CHANNEL_MODE  I2S_CHANNEL_MONO

// ==================== КОНСТАНТЫ ПУТЕЙ ФАЙЛОВ ====================
constexpr const char* HIGHSCORE_FILE = "/highscore.txt";
const char* introTracks[3] = {
	"/snd_intro1.wav", "/snd_intro2.wav", "/snd_intro3.wav"};
const char* mainTracks[5] = {
	"/snd_main1.wav", "/snd_main2.wav", "/snd_main3.wav", "/snd_main4.wav", "/snd_main5.wav"};
const char* gameOverTracks[3] = {
	"/snd_gameover1.wav", "/snd_gameover2.wav", "/snd_gameover3.wav"};
constexpr const char* LOGO_FILE = "/spr_GG_logo.bin";
constexpr const char* START_BG_FILE = "/spr_start_BG.bin";
constexpr const char* NAME1_FILE = "/spr_main_name.bin";
constexpr const char* NAME2_FILE = "/spr_main_name2.bin";
constexpr const char* MAIN_BG_FILE = "/spr_main_BG.bin";
constexpr const char* SHIP_BOOST_FILES[36][3] = {
	{"/spr_ship_boost_000_1.bin", "/spr_ship_boost_000_2.bin", "/spr_ship_boost_000_3.bin"},
	{"/spr_ship_boost_010_1.bin", "/spr_ship_boost_010_2.bin", "/spr_ship_boost_010_3.bin"},
	{"/spr_ship_boost_020_1.bin", "/spr_ship_boost_020_2.bin", "/spr_ship_boost_020_3.bin"},
	{"/spr_ship_boost_030_1.bin", "/spr_ship_boost_030_2.bin", "/spr_ship_boost_030_3.bin"},
	{"/spr_ship_boost_040_1.bin", "/spr_ship_boost_040_2.bin", "/spr_ship_boost_040_3.bin"},
	{"/spr_ship_boost_050_1.bin", "/spr_ship_boost_050_2.bin", "/spr_ship_boost_050_3.bin"},
	{"/spr_ship_boost_060_1.bin", "/spr_ship_boost_060_2.bin", "/spr_ship_boost_060_3.bin"},
	{"/spr_ship_boost_070_1.bin", "/spr_ship_boost_070_2.bin", "/spr_ship_boost_070_3.bin"},
	{"/spr_ship_boost_080_1.bin", "/spr_ship_boost_080_2.bin", "/spr_ship_boost_080_3.bin"},
	{"/spr_ship_boost_090_1.bin", "/spr_ship_boost_090_2.bin", "/spr_ship_boost_090_3.bin"},
	{"/spr_ship_boost_100_1.bin", "/spr_ship_boost_100_2.bin", "/spr_ship_boost_100_3.bin"},
	{"/spr_ship_boost_110_1.bin", "/spr_ship_boost_110_2.bin", "/spr_ship_boost_110_3.bin"},
	{"/spr_ship_boost_120_1.bin", "/spr_ship_boost_120_2.bin", "/spr_ship_boost_120_3.bin"},
	{"/spr_ship_boost_130_1.bin", "/spr_ship_boost_130_2.bin", "/spr_ship_boost_130_3.bin"},
	{"/spr_ship_boost_140_1.bin", "/spr_ship_boost_140_2.bin", "/spr_ship_boost_140_3.bin"},
	{"/spr_ship_boost_150_1.bin", "/spr_ship_boost_150_2.bin", "/spr_ship_boost_150_3.bin"},
	{"/spr_ship_boost_160_1.bin", "/spr_ship_boost_160_2.bin", "/spr_ship_boost_160_3.bin"},
	{"/spr_ship_boost_170_1.bin", "/spr_ship_boost_170_2.bin", "/spr_ship_boost_170_3.bin"},
	{"/spr_ship_boost_180_1.bin", "/spr_ship_boost_180_2.bin", "/spr_ship_boost_180_3.bin"},
	{"/spr_ship_boost_190_1.bin", "/spr_ship_boost_190_2.bin", "/spr_ship_boost_190_3.bin"},
	{"/spr_ship_boost_200_1.bin", "/spr_ship_boost_200_2.bin", "/spr_ship_boost_200_3.bin"},
	{"/spr_ship_boost_210_1.bin", "/spr_ship_boost_210_2.bin", "/spr_ship_boost_210_3.bin"},
	{"/spr_ship_boost_220_1.bin", "/spr_ship_boost_220_2.bin", "/spr_ship_boost_220_3.bin"},
	{"/spr_ship_boost_230_1.bin", "/spr_ship_boost_230_2.bin", "/spr_ship_boost_230_3.bin"},
	{"/spr_ship_boost_240_1.bin", "/spr_ship_boost_240_2.bin", "/spr_ship_boost_240_3.bin"},
	{"/spr_ship_boost_250_1.bin", "/spr_ship_boost_250_2.bin", "/spr_ship_boost_250_3.bin"},
	{"/spr_ship_boost_260_1.bin", "/spr_ship_boost_260_2.bin", "/spr_ship_boost_260_3.bin"},
	{"/spr_ship_boost_270_1.bin", "/spr_ship_boost_270_2.bin", "/spr_ship_boost_270_3.bin"},
	{"/spr_ship_boost_280_1.bin", "/spr_ship_boost_280_2.bin", "/spr_ship_boost_280_3.bin"},
	{"/spr_ship_boost_290_1.bin", "/spr_ship_boost_290_2.bin", "/spr_ship_boost_290_3.bin"},
	{"/spr_ship_boost_300_1.bin", "/spr_ship_boost_300_2.bin", "/spr_ship_boost_300_3.bin"},
	{"/spr_ship_boost_310_1.bin", "/spr_ship_boost_310_2.bin", "/spr_ship_boost_310_3.bin"},
	{"/spr_ship_boost_320_1.bin", "/spr_ship_boost_320_2.bin", "/spr_ship_boost_320_3.bin"},
	{"/spr_ship_boost_330_1.bin", "/spr_ship_boost_330_2.bin", "/spr_ship_boost_330_3.bin"},
	{"/spr_ship_boost_340_1.bin", "/spr_ship_boost_340_2.bin", "/spr_ship_boost_340_3.bin"},
	{"/spr_ship_boost_350_1.bin", "/spr_ship_boost_350_2.bin", "/spr_ship_boost_350_3.bin"}};
constexpr const char* SHIP_STAY_FILES[36] = {"/spr_ship_stay_000.bin", "/spr_ship_stay_010.bin", "/spr_ship_stay_020.bin", "/spr_ship_stay_030.bin",
	"/spr_ship_stay_040.bin", "/spr_ship_stay_050.bin", "/spr_ship_stay_060.bin", "/spr_ship_stay_070.bin", "/spr_ship_stay_080.bin", "/spr_ship_stay_090.bin",
	"/spr_ship_stay_100.bin", "/spr_ship_stay_110.bin", "/spr_ship_stay_120.bin", "/spr_ship_stay_130.bin", "/spr_ship_stay_140.bin", "/spr_ship_stay_150.bin",
	"/spr_ship_stay_160.bin", "/spr_ship_stay_170.bin", "/spr_ship_stay_180.bin", "/spr_ship_stay_190.bin", "/spr_ship_stay_200.bin", "/spr_ship_stay_210.bin",
	"/spr_ship_stay_220.bin", "/spr_ship_stay_230.bin", "/spr_ship_stay_240.bin", "/spr_ship_stay_250.bin", "/spr_ship_stay_260.bin", "/spr_ship_stay_270.bin",
	"/spr_ship_stay_280.bin", "/spr_ship_stay_290.bin", "/spr_ship_stay_300.bin", "/spr_ship_stay_310.bin", "/spr_ship_stay_320.bin", "/spr_ship_stay_330.bin",
	"/spr_ship_stay_340.bin", "/spr_ship_stay_350.bin"};
constexpr const char* BULLET_FILES[36] = {"/spr_bullet_000.bin", "/spr_bullet_010.bin", "/spr_bullet_020.bin", "/spr_bullet_030.bin", "/spr_bullet_040.bin",
	"/spr_bullet_050.bin", "/spr_bullet_060.bin", "/spr_bullet_070.bin", "/spr_bullet_080.bin", "/spr_bullet_090.bin", "/spr_bullet_100.bin", "/spr_bullet_110.bin",
	"/spr_bullet_120.bin", "/spr_bullet_130.bin", "/spr_bullet_140.bin", "/spr_bullet_150.bin", "/spr_bullet_160.bin", "/spr_bullet_170.bin", "/spr_bullet_180.bin",
	"/spr_bullet_190.bin", "/spr_bullet_200.bin", "/spr_bullet_210.bin", "/spr_bullet_220.bin", "/spr_bullet_230.bin", "/spr_bullet_240.bin", "/spr_bullet_250.bin",
	"/spr_bullet_260.bin", "/spr_bullet_270.bin", "/spr_bullet_280.bin", "/spr_bullet_290.bin", "/spr_bullet_300.bin", "/spr_bullet_310.bin", "/spr_bullet_320.bin",
	"/spr_bullet_330.bin", "/spr_bullet_340.bin", "/spr_bullet_350.bin"};
constexpr const char* COMET_FILES[36] = {"/spr_comet_000.bin", "/spr_comet_010.bin", "/spr_comet_020.bin", "/spr_comet_030.bin", "/spr_comet_040.bin",
	"/spr_comet_050.bin", "/spr_comet_060.bin", "/spr_comet_070.bin", "/spr_comet_080.bin", "/spr_comet_090.bin", "/spr_comet_100.bin", "/spr_comet_110.bin",
	"/spr_comet_120.bin", "/spr_comet_130.bin", "/spr_comet_140.bin", "/spr_comet_150.bin", "/spr_comet_160.bin", "/spr_comet_170.bin", "/spr_comet_180.bin",
	"/spr_comet_190.bin", "/spr_comet_200.bin", "/spr_comet_210.bin", "/spr_comet_220.bin", "/spr_comet_230.bin", "/spr_comet_240.bin", "/spr_comet_250.bin",
	"/spr_comet_260.bin", "/spr_comet_270.bin", "/spr_comet_280.bin", "/spr_comet_290.bin", "/spr_comet_300.bin", "/spr_comet_310.bin", "/spr_comet_320.bin",
	"/spr_comet_330.bin", "/spr_comet_340.bin", "/spr_comet_350.bin"};
constexpr const char* BOOM_BIG_FILES[13] = {"/spr_boom_big1.bin", "/spr_boom_big2.bin", "/spr_boom_big3.bin", "/spr_boom_big4.bin", "/spr_boom_big5.bin",
	"/spr_boom_big6.bin", "/spr_boom_big7.bin", "/spr_boom_big8.bin", "/spr_boom_big9.bin", "/spr_boom_big10.bin", "/spr_boom_big11.bin", "/spr_boom_big12.bin",
	"/spr_boom_big13.bin"};
constexpr const char* BOOM_SMALL_FILES[5] = {"/spr_boom_small1.bin", "/spr_boom_small2.bin", "/spr_boom_small3.bin", "/spr_boom_small4.bin", "/spr_boom_small5.bin"};
constexpr const char* ASTEROID1_FILE = "/spr_asteroid_1.bin";
constexpr const char* ASTEROID2_FILE = "/spr_asteroid_2.bin";
constexpr const char* ASTEROID3_FILE = "/spr_asteroid_3.bin";
constexpr const char* GAMEOVER_BG_FILE = "/spr_GO_BG.bin";
constexpr const char* GOTEXT_FILE = "/spr_GO_text.bin";

// ==================== КОНСТАНТЫ РАЗМЕРОВ СПРАЙТОВ ====================
constexpr uint8_t BG_WIDTH = 128;
constexpr uint8_t BG_HEIGHT = 160;
constexpr uint8_t NAME_WIDTH = 117;
constexpr uint8_t NAME_HEIGHT = 48;
constexpr uint8_t SHIP_WIDTH = 17;
constexpr uint8_t SHIP_HEIGHT = 17;
constexpr uint8_t BULLET_WIDTH = 4;
constexpr uint8_t BULLET_HEIGHT = 4;
constexpr uint8_t ASTEROID1_WIDTH = 9;
constexpr uint8_t ASTEROID1_HEIGHT = 10;
constexpr uint8_t ASTEROID2_WIDTH = 12;
constexpr uint8_t ASTEROID2_HEIGHT = 15;
constexpr uint8_t ASTEROID3_WIDTH = 18;
constexpr uint8_t ASTEROID3_HEIGHT = 17;
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
constexpr uint8_t MAX_EXPLOSIONS = 10;
constexpr uint8_t ANIMATION_SPEED = 50;
constexpr uint32_t BULLET_DELAY = 300;
constexpr float SHIP_SPEED = 2.0f;
constexpr float BULLET_SPEED = 4.0f;
constexpr float ASTEROID_BASE_SPEED = 1.0f;
constexpr float COMET_SPEED_MULTIPLIER = 1.9f;
constexpr unsigned long DEBOUNCE_MS = 50;

// ==================== СТРУКТУРЫ И ПЕРЕЧИСЛЕНИЯ ====================
struct Entity
{
	float x, y;          // Позиция центра
	float vx, vy;        // Скорость
	bool active;         // Существует ли объект
	int oldX, oldY;      // Предыдущая позиция (для стирания)
	bool visible;        // Нужно ли рисовать
	int width, height;   // Размер спрайта
	const char* sprite;  // Имя файла спрайта
};

struct Ship
{
	Entity base;
	int16_t rotation;
	bool boosting;
	uint32_t lastShot;
	uint32_t lastBoostFrame;
	uint8_t boostFrame;
};

struct Bullet
{
	Entity base;
	uint32_t spawnTime;
};

struct Asteroid
{
	Entity base;
	uint8_t variant;
	bool isComet;
};

struct Explosion
{
	bool active;
	float x, y;
	int oldX, oldY;
	uint8_t frame;
	uint32_t lastFrameTime;
	bool isPlayer;
	bool isBig;
};

struct WavInfo
{
	uint32_t dataPos;
	uint32_t dataLen;
	uint16_t channels;
	uint32_t sampleRate;
	uint16_t bitsPerSample;
};

struct InputState
{
	int16_t encoderAngle;
	bool encoderPressed;
	bool buttonA;
	bool buttonB;
};

struct SpriteData
{
	String name;
	uint16_t* data;
	size_t size;
};

enum GameState
{
	STATE_MENU,
	STATE_PLAY,
	STATE_GAME_OVER
};

// ==================== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ====================
TFT_eSPI tft;
GameState currentState = STATE_MENU;
uint32_t stateStartTime = 0;
uint32_t lastFrameTime = 0;

Ship playerShip;
Bullet bullets[5];
Asteroid asteroids[10];
Explosion explosions[MAX_EXPLOSIONS];
bool playerExploding = false;  // true когда запущен взрыв корабля
int score = 0;
int highScore = 0;
uint8_t activeAsteroids = 0;

volatile int32_t encoderPos = 0;
volatile int lastEncoderARaw = 0;

bool firstFrameInPlay = true;

bool audioPlaying = false;
TaskHandle_t audioTaskHandle = nullptr;
File currentFile;

// ===== GLOBAL BACKGROUND DATA =====
static uint16_t rowBuf[128];     // буфер текущего фона в PSRAM
uint16_t* bgStart = nullptr;     // фон стартового меню
uint16_t* bgMain = nullptr;      // фон основной игры
uint16_t* bgGameOver = nullptr;  // фон экрана Game Over
size_t bgSizeStart = 0;
size_t bgSizeMain = 0;
size_t bgSizeGameOver = 0;
uint16_t* bgCurrent = nullptr;  // указатель на активный фон

// ========== TOOLS ==========
// Обмен байт в place: делает swap16 для массива uint16_t длиной count
static inline void swap16Buffer(uint16_t* data, size_t count)
{
	for (size_t i = 0; i < count; ++i)
	{
		uint16_t v = data[i];
		data[i] = (v >> 8) | (v << 8);
	}
}

// ==================== Мониторинг ресурсов ====================
unsigned long lastStatsTime = 0;
unsigned long frameCount = 0;

void printStats()
{
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

// ==================== ФУНКЦИИ SD-КАРТЫ ====================
bool initSDCard()
{
	if (!SD_MMC.begin("/sdcard", true))
	{
		// Serial.println("SD Card Mount Failed");
		return false;
	}
	if (SD_MMC.cardType() == CARD_NONE)
	{
		// Serial.println("No SD card attached");
		return false;
	}
	return true;
}

int readHighScore()
{
	File file = SD_MMC.open(HIGHSCORE_FILE, FILE_READ);
	if (!file) return 0;

	char buffer[16];
	uint8_t index = 0;
	while (file.available() && index < sizeof(buffer) - 1)
	{
		char c = file.read();
		if (c == '\n' || c == '\r') break;
		buffer[index++] = c;
	}
	buffer[index] = '\0';
	file.close();
	return atoi(buffer);
}

void writeHighScore(int score)
{
	File file = SD_MMC.open(HIGHSCORE_FILE, FILE_WRITE);
	if (file)
	{
		file.seek(0);
		file.println(score);
		file.close();
	}
}

// ==================== КЭШ СПРАЙТОВ В PSRAM ====================
std::vector<SpriteData> spriteCache;

void loadFileToPSRAM(const char* filename)
{
	File f = SD_MMC.open(filename, FILE_READ);
	if (!f)
	{
		Serial.printf("[ERR] Не найден файл %s\n", filename);
		return;
	}
	size_t sz = f.size();
	uint8_t* buf = (uint8_t*)malloc(sz);
	;
	if (!buf)
	{
		Serial.printf("[ERR] Нет памяти для %s\n", filename);
		f.close();
		return;
	}
	f.read(buf, sz);
	f.close();

	uint16_t* pixels = reinterpret_cast<uint16_t*>(buf);
	size_t pixelCount = sz / 2;
	swap16Buffer(pixels, pixelCount);

	SpriteData entry;
	entry.name = String(filename);
	entry.data = reinterpret_cast<uint16_t*>(buf);
	entry.size = sz / 2;  // количество пикселей
	spriteCache.push_back(entry);
	Serial.printf("[OK] Загружен %s (%u байт, %u пикселей)\n", filename, sz, entry.size);
}

// Загружаем все спрайты в PSRAM
void loadAllSpritesToPSRAM()
{
	const char* files[] = {START_BG_FILE, MAIN_BG_FILE, GAMEOVER_BG_FILE, GOTEXT_FILE, NAME1_FILE, NAME2_FILE, ASTEROID1_FILE, ASTEROID2_FILE, ASTEROID3_FILE};
	for (auto file : files)
		loadFileToPSRAM(file);
	// Загружаем массивы
	for (int i = 0; i < 36; i++)
	{
		loadFileToPSRAM(SHIP_STAY_FILES[i]);
		loadFileToPSRAM(BULLET_FILES[i]);
		loadFileToPSRAM(COMET_FILES[i]);
		for (int f = 0; f < 3; f++)
			loadFileToPSRAM(SHIP_BOOST_FILES[i][f]);
	}
	for (int i = 0; i < 5; i++)
	{
		loadFileToPSRAM(BOOM_SMALL_FILES[i]);
	}
	for (int i = 0; i < 13; ++i)
	{
		loadFileToPSRAM(BOOM_BIG_FILES[i]);
	}
}

// Поиск спрайта в кеше
uint16_t* getSpriteFromCache(const char* name, size_t& outSize)
{
	for (auto& s : spriteCache)
	{
		if (s.name.equals(name))
		{
			outSize = s.size;
			return s.data;
		}
	}
	return nullptr;
}

// ==================== ФУНКЦИИ ОТРИСОВКИ ====================
bool drawLogoStremed()
{
	File file = SD_MMC.open(LOGO_FILE, FILE_READ);
	const int CHUNK_HEIGHT = 4;
	uint16_t chunkBuffer[BG_WIDTH * CHUNK_HEIGHT];
	for (int y = 0; y < BG_HEIGHT; y += CHUNK_HEIGHT)
	{
		int currentChunkHeight = min(CHUNK_HEIGHT, BG_HEIGHT - y);
		size_t chunkSize = BG_WIDTH * currentChunkHeight * 2;

		// Читаем чанк из файла
		size_t bytesRead = file.read((uint8_t*)chunkBuffer, chunkSize);
		for (int i = 0; i < (bytesRead / 2); i++)
		{
			uint16_t v = chunkBuffer[i];
			chunkBuffer[i] = (v >> 8) | (v << 8);
		}
		tft.pushImage(0, y, BG_WIDTH, currentChunkHeight, chunkBuffer);
	}
	file.close();
	return true;
}

static inline void drawSpriteFromPSRAM(const char* filename, int x, int y, int w, int h, uint16_t* bgPtr = nullptr, int bgW = BG_WIDTH, int bgH = BG_HEIGHT)
{
	// Получаем спрайт из кеша (в PSRAM, RGB565)
	size_t spriteSizePixels = 0;
	uint16_t* sprite = (uint16_t*)getSpriteFromCache(filename, spriteSizePixels);
	if (!sprite) return;

	// Если bgPtr не передан — используем bgCurrent (глобальная переменная в твоем коде)
	if (!bgPtr) bgPtr = bgCurrent;
	if (!bgPtr)
	{
		// Без фона прозрачные пиксели нельзя корректно подставить — просто нарисуем без прозрачности
		tft.pushImage(x, y, w, h, sprite);
		return;
	}

	// Видимый прямоугольник с учётом экрана
	int sx = x, sy = y, sw = w, sh = h;
	// clip по экрану (0..SCREEN_WIDTH-1 / 0..SCREEN_HEIGHT-1)
	if (sx + sw <= 0 || sy + sh <= 0 || sx >= SCREEN_WIDTH || sy >= SCREEN_HEIGHT) return;  // полностью вне
	int srcXOffset = 0, srcYOffset = 0;
	if (sx < 0)
	{
		srcXOffset = -sx;
		sw += sx;
		sx = 0;
	}  // sx was negative
	if (sy < 0)
	{
		srcYOffset = -sy;
		sh += sy;
		sy = 0;
	}  // sy was negative
	if (sx + sw > SCREEN_WIDTH) sw = SCREEN_WIDTH - sx;
	if (sy + sh > SCREEN_HEIGHT) sh = SCREEN_HEIGHT - sy;
	if (sw <= 0 || sh <= 0) return;

	// Временный буфер для одной строки (макс ширина 128). static — один буфер для всех вызовов.
	static uint16_t rowBuf[128];  // 128 * 2 = 256 bytes — очень мало
	// Проверка размеров на случай, если w > 128 (в твоём проекте max 128, но на всякий)
	if (w > 128)
	{
		// не ожидается, но на всякий случай — отрезаем (ниже мы работаем с sw)
	}

	// Для каждой видимой строки:
	for (int row = 0; row < sh; ++row)
	{
		int spriteRowIndex = (srcYOffset + row) * w + srcXOffset;  // индекс первого пикселя этой видимой строки в sprite
		uint16_t* spriteRowPtr = sprite + spriteRowIndex;

		// Сначала проверим, есть ли в этой видимой строке хоть один прозрачный пиксель.
		bool hasTransparent = false;
		for (int col = 0; col < sw; ++col)
		{
			if (spriteRowPtr[col] == (uint16_t)TFT_TRANSPARENT_COLOR)
			{
				hasTransparent = true;
				break;
			}
		}

		if (!hasTransparent)
		{
			// Строка полностью непрозрачна — отправляем напрямую (уменьшаем указатель, т.к. pushImage ожидает полный ряд)
			// Но pushImage принимает указатель на начало строки нужной длины: spriteRowPtr указывает именно на первый видимый пиксель.
			tft.pushImage(sx, sy + row, sw, 1, spriteRowPtr);
		}
		else
		{
			// Нужно собрать строку, заменив прозрачные пиксели на фон
			// Вычисляем указатель на фоновые пиксели
			// bgPtr хранит фон полной ширины bgW (обычно BG_WIDTH), берём пиксели с координат (sx .. sx+sw-1, sy+row)
			uint16_t* bgRowPtr = bgPtr + (sy + row) * bgW + sx;

			// Собираем временный буфер
			for (int col = 0; col < sw; ++col)
			{
				uint16_t sp = spriteRowPtr[col];
				if (sp == (uint16_t)TFT_TRANSPARENT_COLOR) { rowBuf[col] = bgRowPtr[col]; }
				else { rowBuf[col] = sp; }
			}
			// Отправляем собранную строку
			tft.pushImage(sx, sy + row, sw, 1, rowBuf);
		}
	}
}

void restoreBgArea(int x, int y, int w, int h)
{
	if (!bgCurrent) return;
	// ограничение по экрану
	if (x < 0)
	{
		w += x;
		x = 0;
	}
	if (y < 0)
	{
		h += y;
		y = 0;
	}
	if (x + w > BG_WIDTH) w = BG_WIDTH - x;
	if (y + h > BG_HEIGHT) h = BG_HEIGHT - y;
	if (w <= 0 || h <= 0) return;

	// Восстановление кусочка фона
	for (int row = 0; row < h; row++)
	{
		memcpy(rowBuf, bgCurrent + (y + row) * BG_WIDTH + x, w * 2);
		tft.pushImage(x, y + row, w, 1, rowBuf);
	}
}
// Восстановить фон под одной сущностью (Entity). width/height передаются в пикселях.
void restoreEntityBg(const Entity& e, int spriteW, int spriteH)
{

	int x = e.oldX - spriteW / 2;
	int y = e.oldY - spriteH / 2;
	restoreBgArea(x, y, spriteW, spriteH);
}
void restoreShipBg()
{
	restoreEntityBg(playerShip.base, SHIP_WIDTH, SHIP_HEIGHT);
}
void restoreBulletsBg()
{
	for (int i = 0; i < MAX_BULLETS; ++i)
	{
		if (bullets[i].base.active || bullets[i].base.oldX >= 0) { restoreEntityBg(bullets[i].base, BULLET_WIDTH, BULLET_HEIGHT); }
	}
}
void restoreAsteroidsBg()
{
	for (int i = 0; i < MAX_ASTEROIDS; ++i)
	{
		if (asteroids[i].base.active || asteroids[i].base.oldX >= 0)
		{
			int w = 0, h = 0;
			if (asteroids[i].isComet)
			{
				w = COMET_WIDTH;
				h = COMET_HEIGHT;
			}
			else
			{
				switch (asteroids[i].variant)
				{
					case 1:
						w = ASTEROID1_WIDTH;
						h = ASTEROID1_HEIGHT;
						break;
					case 2:
						w = ASTEROID2_WIDTH;
						h = ASTEROID2_HEIGHT;
						break;
					case 3:
					default:
						w = ASTEROID3_WIDTH;
						h = ASTEROID3_HEIGHT;
						break;
				}
			}
			restoreEntityBg(asteroids[i].base, w, h);
		}
	}
}
void restoreExplosionsBg()
{
	for (int i = 0; i < MAX_EXPLOSIONS; ++i)
	{
		if (explosions[i].active)
		{
			int w = explosions[i].isBig ? BOOMBIG_WIDTH : BOOMSMALL_WIDTH;
			int h = explosions[i].isBig ? BOOMBIG_HEIGHT : BOOMSMALL_HEIGHT;
			explosions[i].oldX = explosions[i].x;
			explosions[i].oldY = explosions[i].y;
			int x = explosions[i].x - w / 2;
			int y = explosions[i].y - h / 2;
			restoreBgArea(x, y, w, h);
		}
	}
}

void drawShip(int16_t x, int16_t y, uint16_t angle, bool isBoosting = false, uint8_t boostFrame = 0)
{
	char filename[32];
	uint16_t normalizedAngle = (angle / 10) * 10;

	if (isBoosting) { snprintf(filename, sizeof(filename), "/spr_ship_boost_%03d_%d.bin", normalizedAngle, boostFrame + 1); }
	else
	{
		snprintf(filename, sizeof(filename), "/spr_ship_stay_%03d.bin", normalizedAngle);
	}
	drawSpriteFromPSRAM(filename, x - SHIP_WIDTH / 2, y - SHIP_HEIGHT / 2, SHIP_WIDTH, SHIP_HEIGHT);
}

void drawBullet(int16_t x, int16_t y, uint16_t angle)
{
	char filename[24];
	uint16_t normalizedAngle = (angle / 10) * 10;
	snprintf(filename, sizeof(filename), "/spr_bullet_%03d.bin", normalizedAngle);

	drawSpriteFromPSRAM(filename, x - BULLET_WIDTH / 2, y - BULLET_HEIGHT / 2, BULLET_WIDTH, BULLET_HEIGHT);
}

void drawAsteroid(int16_t x, int16_t y, uint8_t variant, bool isComet, uint16_t direction)
{
	char filename[32];
	if (isComet)
	{
		uint16_t normalizedDirection = (direction / 10) * 10;
		snprintf(filename, sizeof(filename), "/spr_comet_%03d.bin", normalizedDirection);
		drawSpriteFromPSRAM(filename, x - COMET_WIDTH / 2, y - COMET_HEIGHT / 2, COMET_WIDTH, COMET_HEIGHT);
	}
	else
	{
		const char* spriteFile = nullptr;
		uint8_t w = 0, h = 0;
		switch (variant)
		{
			case 1:
				spriteFile = ASTEROID1_FILE;
				w = ASTEROID1_WIDTH;
				h = ASTEROID1_HEIGHT;
				break;
			case 2:
				spriteFile = ASTEROID2_FILE;
				w = ASTEROID2_WIDTH;
				h = ASTEROID2_HEIGHT;
				break;
			case 3:
			default:
				spriteFile = ASTEROID3_FILE;
				w = ASTEROID3_WIDTH;
				h = ASTEROID3_HEIGHT;
				break;
		}
		drawSpriteFromPSRAM(spriteFile, x - w / 2, y - h / 2, w, h);
	}
}

void drawExplosions()
{
	for (int i = 0; i < MAX_EXPLOSIONS; i++)
	{
		if (!explosions[i].active) continue;
		const char* file;
		int w, h;
		if (explosions[i].isBig)
		{  // Большой взрыв

			file = BOOM_BIG_FILES[explosions[i].frame];
			w = BOOMBIG_WIDTH;
			h = BOOMBIG_HEIGHT;
		}
		else
		{  // Маленький взрыв
			file = BOOM_SMALL_FILES[explosions[i].frame];
			w = BOOMSMALL_WIDTH;
			h = BOOMSMALL_HEIGHT;
		}
		int x = (int)explosions[i].x - w / 2;
		int y = (int)explosions[i].y - h / 2;
		drawSpriteFromPSRAM(file, x, y, w, h, bgCurrent, BG_WIDTH, BG_HEIGHT);
	}
}

void storeOldPositions()
{
	// Сохраняем предыдущее положение корабля
	playerShip.base.oldX = (int)playerShip.base.x;
	playerShip.base.oldY = (int)playerShip.base.y;
	// Сохраняем пули
	for (int i = 0; i < MAX_BULLETS; ++i)
	{
		bullets[i].base.oldX = (int)bullets[i].base.x;
		bullets[i].base.oldY = (int)bullets[i].base.y;
	}
	// Сохраняем астероиды
	for (int i = 0; i < MAX_ASTEROIDS; ++i)
	{
		asteroids[i].base.oldX = (int)asteroids[i].base.x;
		asteroids[i].base.oldY = (int)asteroids[i].base.y;
	}
	// Сохраняем взрывы
	for (int i = 0; i < MAX_EXPLOSIONS; ++i)
	{
		explosions[i].oldX = (int)explosions[i].x;
		explosions[i].oldY = (int)explosions[i].y;
	}
}

void drawScore(uint16_t score)
{
	const int scoreX = 2;
	const int scoreY = 2;
	const int scoreW = 60;  // ширина зоны под текст
	const int scoreH = 9;   // высота
	// Восстанавливаем фон под текстом счёта
	restoreBgArea(scoreX, scoreY, scoreW, scoreH);
	// Рисуем текст заново
	char scoreText[16];
	snprintf(scoreText, sizeof(scoreText), "SCORE: %d", score);
	tft.setTextColor(TFT_WHITE);
	tft.drawString(scoreText, scoreX, scoreY, 1);
}

// ==================== АУДИО СИСТЕМА ====================
bool parseWav(File& f, WavInfo& info)
{
	if (!f) return false;
	f.seek(0);
	char riff[4];
	f.read((uint8_t*)riff, 4);
	if (strncmp(riff, "RIFF", 4) != 0) return false;
	f.seek(8);
	char wave[4];
	f.read((uint8_t*)wave, 4);
	if (strncmp(wave, "WAVE", 4) != 0) return false;

	while (f.position() + 8 <= f.size())
	{
		char id[5] = {0};
		f.read((uint8_t*)id, 4);
		uint32_t chunkSize;
		f.read((uint8_t*)&chunkSize, 4);

		if (strncmp(id, "fmt ", 4) == 0)
		{
			uint16_t audioFormat;
			f.read((uint8_t*)&audioFormat, 2);
			f.read((uint8_t*)&info.channels, 2);
			f.read((uint8_t*)&info.sampleRate, 4);
			uint32_t byteRate;
			f.read((uint8_t*)&byteRate, 4);
			uint16_t blockAlign;
			f.read((uint8_t*)&blockAlign, 2);
			f.read((uint8_t*)&info.bitsPerSample, 2);
		}
		else if (strncmp(id, "data", 4) == 0)
		{
			info.dataPos = f.position();
			info.dataLen = chunkSize;
			return true;
		}
		else { f.seek(f.position() + chunkSize); }
	}
	return false;
}

void initI2S()
{
	const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),  // мастер, передача данных
		.sample_rate = AUDIO_SAMPLE_RATE,
		.bits_per_sample = AUDIO_BITS_PER_SAMPLE,
		.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
		.communication_format = I2S_COMM_FORMAT_I2S,
		.intr_alloc_flags = 0,
		.dma_buf_count = 8,          // количество DMA буферов
		.dma_buf_len = 256,          // длина одного DMA буфера (чем больше — тем плавнее)
		.use_apll = false,           // можно true для более стабильного звука
		.tx_desc_auto_clear = true,  // автоматически очищать TX
		.fixed_mclk = 0};

	// Настройки пинов
	const i2s_pin_config_t pin_config = {
		.bck_io_num = I2S_BCK_PIN,        // BCK — Bit Clock
		.ws_io_num = I2S_WS_PIN,          // WS — Word Select (LRCK)
		.data_out_num = I2S_DOUT_PIN,     // DATA — Data Out
		.data_in_num = I2S_PIN_NO_CHANGE  // Не используем вход
	};

	if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) == ESP_OK)
  	Serial.println("[I2S] driver installed");
	else
    Serial.println("[ERR] i2s_driver_install failed");

	if (i2s_set_pin(I2S_NUM_0, &pin_config) == ESP_OK)
    Serial.println("[I2S] pins set");
	else
    Serial.println("[ERR] i2s_set_pin failed");

	i2s_zero_dma_buffer(I2S_NUM_0);
	i2s_start(I2S_NUM_0);
}

void stopCurrentTrack()
{
	if (audioPlaying) {
		audioPlaying = false;
    for (int i=0; i<50; ++i) {
            if (eTaskGetState(audioTaskHandle) == eDeleted) break;
            vTaskDelay(pdMS_TO_TICKS(10));
        }
			audioTaskHandle = NULL;	
		i2s_zero_dma_buffer(I2S_NUM_0);
		//i2s_stop(I2S_NUM_0);
		Serial.println("[AUDIO] Track stopped.");
	}
}
void playRandomTrack(const char** tracks, uint8_t count)
{
    if (count == 0 || tracks == nullptr) return;

    stopCurrentTrack();          // останавливаем предыдущее воспроизведение
    delay(50);                   // короткая пауза для стабильности I2S

    uint8_t idx = random(count); // выбираем случайный трек
    const char* fn = tracks[idx];
    if (!fn) return;

    Serial.printf("[AUDIO] Selected track: %s\n", fn);

    // Создаём задачу для параллельного воспроизведения
    xTaskCreatePinnedToCore(
    [](void* param) {
        const char* fn = (const char*)param;

        File* f = new File(SD_MMC.open(fn, FILE_READ));
        if (!*f) {
            Serial.printf("[AUDIO] Failed to open file: %s\n", fn);
            delete f;
            vTaskDelete(NULL);
            return;
        }

        WavInfo info = { 0 };
        if (!parseWav(*f, info)) {
            Serial.printf("[AUDIO] Bad WAV: %s\n", fn);
            f->close();
            delete f;
            vTaskDelete(NULL);
            return;
        }

        i2s_bits_per_sample_t bits =
            (info.bitsPerSample == 32) ? I2S_BITS_PER_SAMPLE_32BIT : I2S_BITS_PER_SAMPLE_16BIT;
        i2s_channel_t ch = (info.channels == 1) ? I2S_CHANNEL_MONO : I2S_CHANNEL_STEREO;

        i2s_set_clk(I2S_NUM_0, info.sampleRate, bits, ch);
        i2s_start(I2S_NUM_0);

        Serial.printf("[AUDIO] playing %s: %u Hz, %u bits, %u ch, dataPos=%u, len=%u\n",
            fn, info.sampleRate, info.bitsPerSample, info.channels,
            (unsigned)info.dataPos, (unsigned)info.dataLen);

        f->seek(info.dataPos);
        const size_t BUF_SZ = 1024;
        uint8_t buf[BUF_SZ];
        uint32_t left = info.dataLen;
        size_t written = 0;

        audioPlaying = true;

        while (audioPlaying && left > 0) {
            size_t toRead = (left > BUF_SZ) ? BUF_SZ : left;
            size_t n = f->read(buf, toRead);
            if (n == 0) break;
            i2s_write(I2S_NUM_0, buf, n, &written, portMAX_DELAY);
            left -= n;
            vTaskDelay(pdMS_TO_TICKS(1));
        }

        f->close();
        delete f;
        i2s_zero_dma_buffer(I2S_NUM_0);
        audioPlaying = false;
        Serial.println("[AUDIO] done.");
        vTaskDelete(NULL);
        },
        "AudioTask",
        8192,                // стек
        (void*)fn,           // путь к файлу
        1,                   // приоритет
        &audioTaskHandle,
        0                    // ядро 0 (второе свободное от графики)
    );
}

void playSoundEffect(uint16_t frequency, uint32_t duration)
{
	tone(BUZZER_PIN, frequency, duration);
}

// ==================== ВВОД ДАННЫХ ====================
void IRAM_ATTR handleEncoderISR()
{
	int clkState = digitalRead(ENCODER_CLK);
	int dtState = digitalRead(ENCODER_DT);

	if (clkState != lastEncoderARaw)
	{
		if (clkState == dtState) { encoderPos++; }
		else { encoderPos--; }
	}
	lastEncoderARaw = clkState;
}

InputState getInputState()
{
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

	static bool lastSW = false;
	if (state.encoderPressed && !lastSW) { Serial.println(">>> ЭНКОДЕР НАЖАТ!"); }
	if (!state.encoderPressed && lastSW) { Serial.println(">>> ЭНКОДЕР ОТПУЩЕН!"); }
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
void changeState(GameState newState)
{
	currentState = newState;
	stateStartTime = millis();
	firstFrameInPlay = true;

	stopCurrentTrack();
	digitalWrite(LED_PIN, LOW);

	switch (newState)
	{
		case STATE_MENU:
			bgCurrent = bgStart;
			playRandomTrack(introTracks, 3);
			break;
		case STATE_PLAY:
			bgCurrent = bgMain;
			playRandomTrack(mainTracks, 5);
			break;
		case STATE_GAME_OVER:
			bgCurrent = bgGameOver;
			playRandomTrack(gameOverTracks, 3);
			break;
	}
}

void resetGame()
{
	playerShip.base.x = SCREEN_WIDTH / 2;
	playerShip.base.y = SCREEN_HEIGHT / 2;
	playerShip.base.vx = 0;
	playerShip.base.vy = 0;
	playerShip.rotation = 0;
	playerShip.boosting = false;
	playerShip.base.active = true;
	playerShip.lastShot = 0;
	firstFrameInPlay = true;  // чтобы не применялось движение в первый кадр todo попробовать выпилить проблема была апаратная

	for (int i = 0; i < MAX_BULLETS; i++)
		bullets[i].base.active = false;
	for (int i = 0; i < MAX_ASTEROIDS; i++)
		asteroids[i].base.active = false;
	for (int i = 0; i < MAX_EXPLOSIONS; i++)
		explosions[i].active = false;

	activeAsteroids = 0;
	score = 0;
	delay(50);  // маленькая пауза стабилизирует питание дисплея
}

void spawnAsteroid(bool forceComet = false)
{
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (!asteroids[i].base.active)
		{
			uint8_t side = random(4);
			switch (side)
			{
				case 0:
					asteroids[i].base.x = -20;
					asteroids[i].base.y = random(SCREEN_HEIGHT);
					break;
				case 1:
					asteroids[i].base.x = SCREEN_WIDTH + 20;
					asteroids[i].base.y = random(SCREEN_HEIGHT);
					break;
				case 2:
					asteroids[i].base.x = random(SCREEN_WIDTH);
					asteroids[i].base.y = -20;
					break;
				case 3:
					asteroids[i].base.x = random(SCREEN_WIDTH);
					asteroids[i].base.y = SCREEN_HEIGHT + 20;
					break;
			}

			float targetX = SCREEN_WIDTH / 2 + random(-20, 20);
			float targetY = SCREEN_HEIGHT / 2 + random(-20, 20);
			float dx = targetX - asteroids[i].base.x;
			float dy = targetY - asteroids[i].base.y;
			float distance = sqrt(dx * dx + dy * dy);

			asteroids[i].base.vx = (dx / distance) * ASTEROID_BASE_SPEED;
			asteroids[i].base.vy = (dy / distance) * ASTEROID_BASE_SPEED;

			uint8_t variant = random(1, 4);
			asteroids[i].variant = variant;
			if (variant == 1)
			{
				asteroids[i].base.width = ASTEROID1_WIDTH;
				asteroids[i].base.height = ASTEROID1_HEIGHT;
				asteroids[i].base.sprite = ASTEROID1_FILE;
			}
			else if (variant == 2)
			{
				asteroids[i].base.width = ASTEROID2_WIDTH;
				asteroids[i].base.height = ASTEROID2_HEIGHT;
				asteroids[i].base.sprite = ASTEROID2_FILE;
			}
			else
			{
				asteroids[i].base.width = ASTEROID3_WIDTH;
				asteroids[i].base.height = ASTEROID3_HEIGHT;
				asteroids[i].base.sprite = ASTEROID3_FILE;
			}

			asteroids[i].isComet = forceComet || (random(100) < (score * 2));
			if (asteroids[i].isComet)
			{
				asteroids[i].base.vx *= COMET_SPEED_MULTIPLIER;
				asteroids[i].base.vy *= COMET_SPEED_MULTIPLIER;
			}

			asteroids[i].base.active = true;
			activeAsteroids++;
			break;
		}
	}
}

void spawnExplosion(float x, float y)
{
	for (int i = 0; i < MAX_EXPLOSIONS; i++)
	{
		if (!explosions[i].active)
		{
			explosions[i].active = true;
			explosions[i].x = x;
			explosions[i].y = y;
			explosions[i].oldX = (int)x;
			explosions[i].oldY = (int)y;
			explosions[i].frame = 0;
			explosions[i].lastFrameTime = millis();
			explosions[i].isBig = false;
			explosions[i].isPlayer = false;
			return;
		}
	}
}

void spawnBigExplosion(float x, float y, bool isPlayerExplosion = false) {
	digitalWrite(LED_PIN, HIGH); //test
	for (int i = 0; i < MAX_EXPLOSIONS; ++i)
	{
		if (!explosions[i].active)
		{
			explosions[i].active = true;
			explosions[i].x = x;
			explosions[i].y = y;
			explosions[i].oldX = (int)x;
			explosions[i].oldY = (int)y;
			explosions[i].frame = 0;
			explosions[i].lastFrameTime = millis();
			explosions[i].isBig = true;
			explosions[i].isPlayer = isPlayerExplosion;
			if (isPlayerExplosion) playerExploding = true;
			return;
		}
	}
}

void updateExplosions()
{
	uint32_t now = millis();
	for (int i = 0; i < MAX_EXPLOSIONS; i++)
	{
		if (!explosions[i].active) continue;

		if (explosions[i].frame > 0)
		{
			int w = explosions[i].isBig ? BOOMBIG_WIDTH : BOOMSMALL_WIDTH;
			int h = explosions[i].isBig ? BOOMBIG_HEIGHT : BOOMSMALL_HEIGHT;
			int x = (int)explosions[i].oldX - w / 2;
			int y = (int)explosions[i].oldY - h / 2;
			restoreBgArea(x, y, w, h);
		}

		explosions[i].oldX = explosions[i].x;
		explosions[i].oldY = explosions[i].y;

		if (now - explosions[i].lastFrameTime > ANIMATION_SPEED)
		{  // задержка между кадрами
			explosions[i].frame++;
			explosions[i].lastFrameTime = now;

			uint8_t maxFrames = explosions[i].isBig ? 13 : 5;
			if (explosions[i].frame >= maxFrames)
			{
				explosions[i].active = false;
				// Если это был взрыв игрока — завершаем игру
				if (explosions[i].isPlayer)
				{
					playerExploding = false;
					if (score > highScore)
					{
						highScore = score;
						writeHighScore(highScore);
					}
					changeState(STATE_GAME_OVER);
				}
			}
		}
	}
}

void checkCollisions()
{
	// Пули -> Астероиды
	for (int b = 0; b < MAX_BULLETS; b++)
	{
		if (!bullets[b].base.active) continue;

		for (int a = 0; a < MAX_ASTEROIDS; a++)
		{
			if (!asteroids[a].base.active) continue;

			float dx = bullets[b].base.x - asteroids[a].base.x;
			float dy = bullets[b].base.y - asteroids[a].base.y;
			float distanceSq = dx * dx + dy * dy;

			float bulletRadius = max(BULLET_WIDTH, BULLET_HEIGHT) / 2.0f;

			float asteroidRadius = 0.0f;
			if (asteroids[a].isComet) { asteroidRadius = max(COMET_WIDTH, COMET_HEIGHT) / 2.0f; }
			else
			{
				switch (asteroids[a].variant)
				{
					case 1:
						asteroidRadius = max(ASTEROID1_WIDTH, ASTEROID1_HEIGHT) / 2.0f;
						break;
					case 2:
						asteroidRadius = max(ASTEROID2_WIDTH, ASTEROID2_HEIGHT) / 2.0f;
						break;
					case 3:
					default:
						asteroidRadius = max(ASTEROID3_WIDTH, ASTEROID3_HEIGHT) / 2.0f;
						break;
				}
			}

			float collideDist = bulletRadius + asteroidRadius;

			if (distanceSq < collideDist * collideDist)
			{
				// --- Столкновение: уничтожаем пулю и астероид
				bullets[b].base.active = false;
				asteroids[a].base.active = false;
				activeAsteroids--;
				score++;
				playSoundEffect(400, 300);
				spawnExplosion(asteroids[a].base.x, asteroids[a].base.y);
				spawnAsteroid();
				break;
			}
		}
	}

	// Корабль -> Астероиды
	float shipRadius = max(SHIP_WIDTH, SHIP_HEIGHT) / 2.0f;
	for (int a = 0; a < MAX_ASTEROIDS; a++)
	{
		if (!asteroids[a].base.active) continue;

		float dx = playerShip.base.x - asteroids[a].base.x;
		float dy = playerShip.base.y - asteroids[a].base.y;
		float distanceSq = dx * dx + dy * dy;

		float asteroidRadius = 0.0f;
		if (asteroids[a].isComet) { asteroidRadius = max(COMET_WIDTH, COMET_HEIGHT) / 2.0f; }
		else
		{
			switch (asteroids[a].variant)
			{
				case 1:
					asteroidRadius = max(ASTEROID1_WIDTH, ASTEROID1_HEIGHT) / 2.0f;
					break;
				case 2:
					asteroidRadius = max(ASTEROID2_WIDTH, ASTEROID2_HEIGHT) / 2.0f;
					break;
				case 3:
				default:
					asteroidRadius = max(ASTEROID3_WIDTH, ASTEROID3_HEIGHT) / 2.0f;
					break;
			}
		}
		float collideDist = shipRadius + asteroidRadius;
		if (distanceSq < collideDist * collideDist)
		{
			// --- Столкновение корабля с астероидом/кометой ---
			playerShip.base.active = false;
			asteroids[a].base.active = false;
			playerExploding = true;

			playSoundEffect(50, 1000);
			spawnBigExplosion(playerShip.base.x, playerShip.base.y, true);  // isPlayerExplosion = true
			break;                                                          // достаточно одного столкновения
		}
	}
}

// ==================== ОСНОВНЫЕ ФУНКЦИИ ====================
void setup()
{
	Serial.begin(115200);
	delay(1000);

	tft.init();
	tft.setRotation(0);

	pinMode(ENCODER_CLK, INPUT_PULLUP);
	pinMode(ENCODER_DT, INPUT_PULLUP);
	pinMode(ENCODER_SW, INPUT);
	pinMode(BUTTON_A, INPUT);
	pinMode(BUTTON_B, INPUT_PULLUP);
	pinMode(LED_PIN, OUTPUT);
	pinMode(BUZZER_PIN, OUTPUT);

	Serial.printf("ENCODER_SW pin mode: %d\n", digitalRead(ENCODER_SW));
	Serial.printf("ENCODER_CLK pin mode: %d\n", digitalRead(ENCODER_CLK));
	Serial.printf("ENCODER_DT pin mode: %d\n", digitalRead(ENCODER_DT));

	lastEncoderARaw = digitalRead((gpio_num_t)ENCODER_CLK);
	attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handleEncoderISR, CHANGE);

	if (!initSDCard())
	{
		tft.drawString("SD Card Error", 20, 70, 2);
		while (1);
	}

	drawLogoStremed();

	Serial.println("[INFO] Загружаем спрайты в PSRAM...");
	loadAllSpritesToPSRAM();
	Serial.println("[INFO] Спрайты готовы!");

	bgStart = (uint16_t*)getSpriteFromCache(START_BG_FILE, bgSizeStart);
	bgMain = (uint16_t*)getSpriteFromCache(MAIN_BG_FILE, bgSizeMain);
	bgGameOver = (uint16_t*)getSpriteFromCache(GAMEOVER_BG_FILE, bgSizeGameOver);

	initI2S();

	highScore = readHighScore();
	changeState(STATE_MENU);
}

void loop()
{
	uint32_t now = millis();
	if (now - lastFrameTime < 33) return;  // 30 FPS
	lastFrameTime = now;

	InputState input = getInputState();

	uint32_t currentTime = millis();

	switch (currentState)
	{
		case STATE_MENU: {
			if (firstFrameInPlay)
			{
				tft.pushImage(0, 0, BG_WIDTH, BG_HEIGHT, bgCurrent);  // отрисовка фона
				firstFrameInPlay = false;
			}
			bgCurrent = bgStart;

			// playRandomTrack(introTracks, 3);

			storeOldPositions();

			// Координаты и размеры логотипа
			const int nameX = 3, nameY = 109;
			const int nameW = NAME_WIDTH, nameH = NAME_HEIGHT;

			// Координаты и размеры мигающего текста
			const int pressX = 28, pressY = 71;
			const int pressW = 80, pressH = 24;
			// 1) Восстанавливаем фон под логотип
			restoreBgArea(nameX, nameY, nameW, nameH);
			// 2) Восстанавливаем фон под мигающим текстом
			restoreBgArea(pressX, pressY, pressW, pressH);
			// 3) Рисуем логотип
			if ((millis() / 1000) % 2 == 0) { drawSpriteFromPSRAM(NAME1_FILE, nameX, nameY, nameW, nameH, bgStart, BG_WIDTH, BG_HEIGHT); }
			else { drawSpriteFromPSRAM(NAME2_FILE, nameX, nameY, nameW, nameH, bgStart, BG_WIDTH, BG_HEIGHT); }

			// 4) Рисуем мигающий текст
			if ((millis() / 700) % 2 == 0)
			{
				tft.setTextColor(TFT_WHITE);  // прозрачный фон
				tft.setTextSize(1);
				tft.setCursor(pressX + 19, pressY);
				tft.print("press");
				tft.setCursor(pressX, pressY + 10);
				tft.print("right button");
				//digitalWrite(LED_PIN, HIGH);
			}
			//else { digitalWrite(LED_PIN, LOW); }
			if (input.encoderPressed)
			{
				delay(500);
				resetGame();
				changeState(STATE_PLAY);
				// playRandomTrack(mainTracks, 5);
			}
			break;
		}
		case STATE_PLAY:
			if (firstFrameInPlay)
			{
				tft.pushImage(0, 0, BG_WIDTH, BG_HEIGHT, bgCurrent);  // отрисовка фона
				firstFrameInPlay = false;
			}

			bgCurrent = bgMain;
			// === 1. Сохранить старые позиции перед движением ===
			storeOldPositions();

			playerShip.rotation = input.encoderAngle;

			if (!firstFrameInPlay)
			{
				if (input.buttonA)
				{
					float rad = playerShip.rotation * PI / 180.0f;
					playerShip.base.vx += cos(rad) * SHIP_SPEED;
					playerShip.base.vy += sin(rad) * SHIP_SPEED;

					// Ограничиваем скорость по модулю
					float speed = sqrt(playerShip.base.vx * playerShip.base.vx + playerShip.base.vy * playerShip.base.vy);
					if (speed > SHIP_SPEED)
					{
						playerShip.base.vx = (playerShip.base.vx / speed) * SHIP_SPEED;
						playerShip.base.vy = (playerShip.base.vy / speed) * SHIP_SPEED;
					}

					playerShip.boosting = true;
				}
				else { playerShip.boosting = false; }
			}

			// Движение корабля
			playerShip.base.x += playerShip.base.vx;
			playerShip.base.y += playerShip.base.vy;

			// Телепортация
			if (playerShip.base.x < 0) playerShip.base.x = SCREEN_WIDTH;
			if (playerShip.base.x > SCREEN_WIDTH) playerShip.base.x = 0;
			if (playerShip.base.y < 0) playerShip.base.y = SCREEN_HEIGHT;
			if (playerShip.base.y > SCREEN_HEIGHT) playerShip.base.y = 0;

			// --- 3. Применяем трение
			playerShip.base.vx *= 0.95f;  // плавное затухание скорости по X
			playerShip.base.vy *= 0.95f;  // плавное затухание скорости по Y

			// Стрельба
			if (input.buttonB && currentTime - playerShip.lastShot > BULLET_DELAY)
			{
				for (int i = 0; i < MAX_BULLETS; i++)
				{
					if (!bullets[i].base.active)
					{
						float rad = playerShip.rotation * PI / 180.0f;
						bullets[i].base.x = playerShip.base.x;
						bullets[i].base.y = playerShip.base.y;
						bullets[i].base.vx = cos(rad) * BULLET_SPEED;
						bullets[i].base.vy = sin(rad) * BULLET_SPEED;
						bullets[i].base.active = true;
						bullets[i].spawnTime = currentTime;

						playerShip.lastShot = currentTime;
						playSoundEffect(2000, 100);
						digitalWrite(LED_PIN, HIGH);
						delay(10);
						digitalWrite(LED_PIN, LOW);
						break;
					}
				}
			}

			// Обновление пуль и астероидов
			for (int i = 0; i < MAX_BULLETS; i++)
			{
				if (bullets[i].base.active)
				{
					bullets[i].base.x += bullets[i].base.vx;
					bullets[i].base.y += bullets[i].base.vy;
					if (currentTime - bullets[i].spawnTime > 2000) bullets[i].base.active = false;
				}
			}
			for (int i = 0; i < MAX_ASTEROIDS; i++)
			{
				if (asteroids[i].base.active)
				{
					asteroids[i].base.x += asteroids[i].base.vx;
					asteroids[i].base.y += asteroids[i].base.vy;

					if (asteroids[i].base.x < -20) asteroids[i].base.x = SCREEN_WIDTH + 20;
					if (asteroids[i].base.x > SCREEN_WIDTH + 20) asteroids[i].base.x = -20;
					if (asteroids[i].base.y < -20) asteroids[i].base.y = SCREEN_HEIGHT + 20;
					if (asteroids[i].base.y > SCREEN_HEIGHT + 20) asteroids[i].base.y = -20;
				}
			}

			// Обновление анимаций взрывов
			updateExplosions();  // todo проверить нужна ли она вообще

			// Спавн астероидов
			if (activeAsteroids < 1 + (score / 5)) { spawnAsteroid(); }

			checkCollisions();

			// 3) ВОССТАНОВЛЯЕМ фон там, где раньше были объекты
			if (!firstFrameInPlay)
			{
				restoreShipBg();
				restoreBulletsBg();
				restoreAsteroidsBg();
				restoreExplosionsBg();
			}

			drawShip((int)playerShip.base.x, (int)playerShip.base.y, playerShip.rotation, playerShip.boosting, playerShip.boostFrame);

			for (int i = 0; i < MAX_BULLETS; i++)
			{
				if (bullets[i].base.active)
				{
					float angle = atan2(bullets[i].base.vy, bullets[i].base.vx) * 180.0f / PI;
					if (angle < 0) angle += 360;
					drawBullet((int)bullets[i].base.x, (int)bullets[i].base.y, (uint16_t)angle);
				}
			}

			for (int i = 0; i < MAX_ASTEROIDS; ++i)
			{
				if (asteroids[i].base.active)
				{
					float direction = atan2(asteroids[i].base.vy, asteroids[i].base.vx) * 180.0f / PI;
					if (direction < 0) direction += 360;
					int w, h;
					if (asteroids[i].isComet)
					{
						w = COMET_WIDTH;
						h = COMET_HEIGHT;
					}
					else
					{
						switch (asteroids[i].variant)
						{
							case 1:
								w = ASTEROID1_WIDTH;
								h = ASTEROID1_HEIGHT;
								break;
							case 2:
								w = ASTEROID2_WIDTH;
								h = ASTEROID2_HEIGHT;
								break;
							case 3:
							default:
								w = ASTEROID3_WIDTH;
								h = ASTEROID3_HEIGHT;
								break;
						}
					}
					drawAsteroid((int)asteroids[i].base.x, (int)asteroids[i].base.y, asteroids[i].variant, asteroids[i].isComet, (uint16_t)direction);
				}
			}

			// Отрисовка активных взрывов
			drawExplosions();

			drawScore(score);

			// обновление анимации boost
			static uint32_t lastBoostAnimTime = 0;
			if (playerShip.boosting)
			{
				if (millis() - lastBoostAnimTime > ANIMATION_SPEED)
				{
					playerShip.boostFrame = (playerShip.boostFrame + 1) % 3;
					lastBoostAnimTime = millis();
				}
			}
			break;

		case STATE_GAME_OVER:

			bgCurrent = bgGameOver;

			if (firstFrameInPlay)
			{
				tft.pushImage(0, 0, BG_WIDTH, BG_HEIGHT, bgCurrent);  // отрисовка фона
				firstFrameInPlay = false;
			}

			storeOldPositions();

			// Координаты и размеры мигающего текста
			const int pressX = 28, pressY = 100;
			const int pressW = 80, pressH = 24;

			restoreBgArea(pressX, pressY, pressW, pressH);

			// Рисуем мигающий текст
			if ((millis() / 700) % 2 == 0)
			{
				tft.setTextColor(TFT_WHITE);  // прозрачный фон
				tft.setTextSize(1);
				tft.setCursor(pressX + 19, pressY);
				tft.print("press");
				tft.setCursor(pressX, pressY + 10);
				tft.print("right button");
			}

			drawSpriteFromPSRAM(GOTEXT_FILE, 35, 40, GOTEXT_WIDTH, GOTEXT_HEIGHT, bgCurrent, BG_WIDTH, BG_HEIGHT);

			char scoreText[32];
			snprintf(scoreText, sizeof(scoreText), "SCORE: %d", score);
			tft.setTextColor(TFT_WHITE);
			tft.drawString(scoreText, 2, 2, 1);

			snprintf(scoreText, sizeof(scoreText), "RECORD: %d", highScore);
			tft.setTextColor(TFT_GREEN);
			tft.drawString(scoreText, 2, 13, 1);

			if (input.encoderPressed)
			{
				delay(500);
				changeState(STATE_MENU);
				// playRandomTrack(introTracks, 3);
			}
			break;
	}

	frameCount++;

	if (now - lastStatsTime >= 5000)
	{
		lastStatsTime = now;
		//printStats();
	}
	// даём планировщику "глоток" CPU — помогает предотвратить WDT
vTaskDelay(pdMS_TO_TICKS(1));

}
// TODO 1) логику моргания диодом придумать сделать.