#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// Настройки дисплея
#define TFT_CS    5
#define TFT_RST   4
#define TFT_DC    21
#define TFT_SCLK 18
#define TFT_MOSI 23

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// Пины управления
#define ENCODER_CLK 14
#define ENCODER_DT  12
#define BUTTON_A    15
#define BUTTON_B    19

// Размеры спрайтов
#define SHIP_SIZE 16
#define ASTEROID_SIZE 16
#define BULLET_SIZE 4

// Игровые константы
#define GAME_SPEED 5.0
#define PHYSICS_FPS 40
#define RENDER_FPS 20

// Состояние игры
struct {
  // Игрок
  float shipX = 64;
  float shipY = 64;
  float shipAngle = 0;
  float shipSpeedX = 0;
  float shipSpeedY = 0;
  
  // Пули
  struct {
    float x, y;
    float dx, dy;
    bool active;
    uint32_t spawnTime;
  } bullets[20];
  
  // Астероиды
  struct {
    float x, y;
    float dx, dy;
    bool active;
  } asteroids[15];
  
  // Игровые параметры
  int score = 0;
  int asteroidsCount = 1;
  uint32_t lastSpawnTime = 0;
} game;

// Состояние энкодера
volatile int encoderPos = 0;
portMUX_TYPE encoderMux = portMUX_INITIALIZER_UNLOCKED;

// Прототипы функций
void spawnAsteroids();
void checkCollisions();
void updatePhysics();
void handleControls();
void renderGame();
void drawToBuffer();
void drawSpriteToBuffer(int16_t x, int16_t y, const uint16_t sprite[][ASTEROID_SIZE], uint8_t w, uint8_t h);
void drawRotatedSpriteToBuffer(int16_t x, int16_t y, const uint16_t sprite[][SHIP_SIZE], uint8_t size, float angle);
void drawCircleToBuffer(int16_t x, int16_t y, int16_t r, uint16_t color);
void drawTextToBuffer(int16_t x, int16_t y, uint16_t color, const char *text);
void drawNumberToBuffer(int16_t x, int16_t y, uint16_t color, int number);

// Спрайты (замените на свои)
const uint16_t spr_01[SHIP_SIZE][SHIP_SIZE] = {0};
const uint16_t spr_02[ASTEROID_SIZE][ASTEROID_SIZE] = {0};

void IRAM_ATTR encoderISR() {
  portENTER_CRITICAL_ISR(&encoderMux);
  static uint8_t lastCLK = HIGH;
  uint8_t clk = digitalRead(ENCODER_CLK);
  
  if(clk != lastCLK) {
    if(digitalRead(ENCODER_DT) != clk) {
      encoderPos++;
    } else {
      encoderPos--;
    }
    lastCLK = clk;
  }
  portEXIT_CRITICAL_ISR(&encoderMux);
}

void setup() {
  Serial.begin(115200);
  
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, -1);  // SCLK, MISO, MOSI, SS
  tft.initR(INITR_BLACKTAB);
  tft.setSPISpeed(20000000); // 20 MHz — максимум для ST7735

  // Инициализация дисплея
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(0);
  tft.fillScreen(ST7735_BLACK);
  
  // Настройка управления
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), encoderISR, CHANGE);
  
  // Инициализация игры
  spawnAsteroids();
}

void loop() {
  static uint32_t lastPhysicsUpdate = 0;
  static uint32_t lastRenderUpdate = 0;
  uint32_t now = millis();
  
  // Обновление физики (120 FPS)
  if(now - lastPhysicsUpdate >= (1000/PHYSICS_FPS)) {
    updatePhysics();
    lastPhysicsUpdate = now;
  }
  
  // Отрисовка (60 FPS)
  if(now - lastRenderUpdate >= (1000/RENDER_FPS)) {
    renderGame();
    lastRenderUpdate = now;
  }
}

void spawnAsteroids() {
  for(int i = 0; i < game.asteroidsCount; i++) {
    if(!game.asteroids[i].active) {
      // Спавн у краев экрана
      if(random(2)) {
        game.asteroids[i].x = random(2) ? -ASTEROID_SIZE : tft.width() + ASTEROID_SIZE;
        game.asteroids[i].y = random(tft.height());
      } else {
        game.asteroids[i].x = random(tft.width());
        game.asteroids[i].y = random(2) ? -ASTEROID_SIZE : tft.height() + ASTEROID_SIZE;
      }
      
      // Направление движения к центру
      float dx = (tft.width()/2 - game.asteroids[i].x) * 0.001 * random(5, 15);
      float dy = (tft.height()/2 - game.asteroids[i].y) * 0.001 * random(5, 15);
      
      game.asteroids[i].dx = dx;
      game.asteroids[i].dy = dy;
      game.asteroids[i].active = true;
    }
  }
  game.lastSpawnTime = millis();
}

void checkCollisions() {
  // Пули с астероидами
  for(auto &bullet : game.bullets) {
    if(!bullet.active) continue;
    
    for(auto &asteroid : game.asteroids) {
      if(!asteroid.active) continue;
      
      float dist = sqrt(pow(bullet.x - asteroid.x, 2) + pow(bullet.y - asteroid.y, 2));
      if(dist < ASTEROID_SIZE/2) {
        bullet.active = false;
        asteroid.active = false;
        game.score++;
        
        // Каждые 5 очков увеличиваем сложность
        if(game.score % 5 == 0 && game.asteroidsCount < sizeof(game.asteroids)/sizeof(game.asteroids[0])) {
          game.asteroidsCount++;
        }
      }
    }
  }
  
  // Корабль с астероидами
  for(auto &asteroid : game.asteroids) {
    if(!asteroid.active) continue;
    
    float dist = sqrt(pow(game.shipX - asteroid.x, 2) + pow(game.shipY - asteroid.y, 2));
    if(dist < (SHIP_SIZE + ASTEROID_SIZE)/2) {
      // Столкновение - сброс игры
      game.shipX = tft.width()/2;
      game.shipY = tft.height()/2;
      game.shipSpeedX = 0;
      game.shipSpeedY = 0;
      game.score = max(0, game.score - 5);
      break;
    }
  }
}

void updatePhysics() {
  // Управление кораблем
  handleControls();
  
  // Движение корабля
  game.shipX += game.shipSpeedX * GAME_SPEED;
  game.shipY += game.shipSpeedY * GAME_SPEED;
  
  // Трение
  game.shipSpeedX *= 0.95;
  game.shipSpeedY *= 0.95;
  
  // Границы экрана
  if(game.shipX < 0) game.shipX = tft.width();
  if(game.shipX > tft.width()) game.shipX = 0;
  if(game.shipY < 0) game.shipY = tft.height();
  if(game.shipY > tft.height()) game.shipY = 0;
  
  // Движение пуль
  for(auto &bullet : game.bullets) {
    if(bullet.active) {
      bullet.x += bullet.dx * GAME_SPEED;
      bullet.y += bullet.dy * GAME_SPEED;
      
      // Удаление старых пуль
      if(millis() - bullet.spawnTime > 1000 || 
         bullet.x < 0 || bullet.x > tft.width() || 
         bullet.y < 0 || bullet.y > tft.height()) {
        bullet.active = false;
      }
    }
  }
  
  // Движение астероидов
  for(auto &asteroid : game.asteroids) {
    if(asteroid.active) {
      asteroid.x += asteroid.dx * GAME_SPEED;
      asteroid.y += asteroid.dy * GAME_SPEED;
      
      if(asteroid.x < -ASTEROID_SIZE) asteroid.x = tft.width();
      if(asteroid.x > tft.width()) asteroid.x = -ASTEROID_SIZE;
      if(asteroid.y < -ASTEROID_SIZE) asteroid.y = tft.height();
      if(asteroid.y > tft.height()) asteroid.y = -ASTEROID_SIZE;
    }
  }
  
  // Проверка столкновений
  checkCollisions();
  
  // Спавн новых астероидов
  if(millis() - game.lastSpawnTime > 2000) {
    spawnAsteroids();
  }
}

void handleControls() {
  // Поворот корабля
  static int lastEncoderPos = 0;
  if(encoderPos != lastEncoderPos) {
    game.shipAngle += (encoderPos - lastEncoderPos) * 0.1;
    lastEncoderPos = encoderPos;
  }
  
  // Движение вперед
  if(digitalRead(BUTTON_A) == LOW) {
    float force = 0.5;
    game.shipSpeedX += cos(game.shipAngle) * force;
    game.shipSpeedY += sin(game.shipAngle) * force;
  }
  
  // Выстрел
  static uint32_t lastFireTime = 0;
  if(digitalRead(BUTTON_B) == LOW && millis() - lastFireTime > 100) {
    for(auto &bullet : game.bullets) {
      if(!bullet.active) {
        bullet.x = game.shipX + cos(game.shipAngle) * SHIP_SIZE/2;
        bullet.y = game.shipY + sin(game.shipAngle) * SHIP_SIZE/2;
        bullet.dx = cos(game.shipAngle) * 3;
        bullet.dy = sin(game.shipAngle) * 3;
        bullet.active = true;
        bullet.spawnTime = millis();
        lastFireTime = millis();
        break;
      }
    }
  }
}

void renderGame() {
  static float lastShipX = 0, lastShipY = 0;
  static float lastBulletX[20] = {0}, lastBulletY[20] = {0};
  static float lastAsteroidX[15] = {0}, lastAsteroidY[15] = {0};

  // Очистка старого положения астероидов
  for (int i = 0; i < game.asteroidsCount; i++) {
    if (game.asteroids[i].active) {
      tft.fillCircle((int16_t)lastAsteroidX[i], (int16_t)lastAsteroidY[i], ASTEROID_SIZE / 2, ST7735_BLACK);
      lastAsteroidX[i] = game.asteroids[i].x;
      lastAsteroidY[i] = game.asteroids[i].y;
    }
  }

  // Очистка старого положения пуль
  for (int i = 0; i < 20; i++) {
    if (game.bullets[i].active) {
      tft.fillCircle((int16_t)lastBulletX[i], (int16_t)lastBulletY[i], BULLET_SIZE / 2, ST7735_BLACK);
      lastBulletX[i] = game.bullets[i].x;
      lastBulletY[i] = game.bullets[i].y;
    }
  }

  // Очистка корабля
  tft.fillTriangle((int16_t)(lastShipX + cos(game.shipAngle) * SHIP_SIZE / 2),
                   (int16_t)(lastShipY + sin(game.shipAngle) * SHIP_SIZE / 2),
                   (int16_t)(lastShipX + cos(game.shipAngle + 2.5) * SHIP_SIZE / 3),
                   (int16_t)(lastShipY + sin(game.shipAngle + 2.5) * SHIP_SIZE / 3),
                   (int16_t)(lastShipX + cos(game.shipAngle - 2.5) * SHIP_SIZE / 3),
                   (int16_t)(lastShipY + sin(game.shipAngle - 2.5) * SHIP_SIZE / 3),
                   ST7735_BLACK);

  // Отрисовка астероидов
  for (int i = 0; i < game.asteroidsCount; i++) {
    if (game.asteroids[i].active) {
      tft.fillCircle((int16_t)game.asteroids[i].x, (int16_t)game.asteroids[i].y, ASTEROID_SIZE / 2, ST7735_WHITE);
    }
  }

  // Отрисовка пуль
  for (int i = 0; i < 20; i++) {
    if (game.bullets[i].active) {
      tft.fillCircle((int16_t)game.bullets[i].x, (int16_t)game.bullets[i].y, BULLET_SIZE / 2, ST7735_RED);
    }
  }

  // Отрисовка корабля
  float x1 = game.shipX + cos(game.shipAngle) * SHIP_SIZE / 2;
  float y1 = game.shipY + sin(game.shipAngle) * SHIP_SIZE / 2;
  float x2 = game.shipX + cos(game.shipAngle + 2.5) * SHIP_SIZE / 3;
  float y2 = game.shipY + sin(game.shipAngle + 2.5) * SHIP_SIZE / 3;
  float x3 = game.shipX + cos(game.shipAngle - 2.5) * SHIP_SIZE / 3;
  float y3 = game.shipY + sin(game.shipAngle - 2.5) * SHIP_SIZE / 3;

  tft.fillTriangle((int16_t)x1, (int16_t)y1,
                   (int16_t)x2, (int16_t)y2,
                   (int16_t)x3, (int16_t)y3,
                   ST7735_BLUE);

  lastShipX = game.shipX;
  lastShipY = game.shipY;

  // HUD (перерисовываем целиком, можно вынести в отдельный слой позже)
  tft.fillRect(0, 0, 80, 20, ST7735_BLACK);
  tft.setTextSize(1);
  tft.setCursor(5, 5);
  tft.setTextColor(ST7735_WHITE);
  tft.print("Score: ");
  tft.print(game.score);

  tft.setCursor(5, 15);
  tft.print("Asteroids: ");
  tft.print(game.asteroidsCount);
}
