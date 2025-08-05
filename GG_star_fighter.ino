#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

#define SHIP_SIZE 12
#define ASTEROID_SIZE 14
#define BULLET_SIZE 3
#define GAME_SPEED 5.0
#define PHYSICS_FPS 40
#define RENDER_FPS 30

#define ENCODER_CLK 14
#define ENCODER_DT  12
#define BUTTON_A    15
#define BUTTON_B    19

#define MAX_ASTEROIDS 10

volatile int encoderPos = 0;
portMUX_TYPE encoderMux = portMUX_INITIALIZER_UNLOCKED;

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
};

struct Game {
  float shipX = 64, shipY = 64;
  float prevShipX = 64, prevShipY = 64;
  float shipAngle = 0;
  float shipSpeedX = 0, shipSpeedY = 0;
  int score = 0;
  Bullet bullets[15];
  Asteroid asteroids[MAX_ASTEROIDS];
} game;

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

void setup() {
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), encoderISR, CHANGE);
}

void loop() {
  static uint32_t lastPhysicsUpdate = 0;
  static uint32_t lastRenderUpdate = 0;
  static uint32_t lastFullClear = 0;
  static bool forceFullRedraw = false;

  uint32_t now = millis();

  // Очистка экрана каждые 100 мс
  if (now - lastFullClear >= 100) {
    tft.fillScreen(TFT_BLACK);
    forceFullRedraw = true;
    lastFullClear = now;
  }

  if (now - lastPhysicsUpdate >= (1000 / PHYSICS_FPS)) {
    updatePhysics();
    lastPhysicsUpdate = now;
  }

  if (now - lastRenderUpdate >= (1000 / RENDER_FPS)) {
    renderGame(forceFullRedraw);
    forceFullRedraw = false;
    lastRenderUpdate = now;
  }
}

void updatePhysics() {
  static int lastEncoder = 0;
  if (encoderPos != lastEncoder) {
    game.shipAngle += (encoderPos - lastEncoder) * 0.1;
    lastEncoder = encoderPos;
  }

  if (digitalRead(BUTTON_A) == LOW) {
    game.shipSpeedX += cos(game.shipAngle) * 0.04;
    game.shipSpeedY += sin(game.shipAngle) * 0.04;
  }

  if (digitalRead(BUTTON_B) == LOW) {
    static uint32_t lastShot = 0;
    if (millis() - lastShot > 100) {
      for (auto &b : game.bullets) {
        if (!b.active) {
          b.x = game.shipX;
          b.y = game.shipY;
          b.prevX = b.x;
          b.prevY = b.y;
          b.dx = cos(game.shipAngle) * 3;
          b.dy = sin(game.shipAngle) * 3;
          b.spawnTime = millis();
          b.active = true;
          lastShot = millis();
          break;
        }
      }
    }
  }

  game.prevShipX = game.shipX;
  game.prevShipY = game.shipY;
  game.shipX += game.shipSpeedX * GAME_SPEED;
  game.shipY += game.shipSpeedY * GAME_SPEED;
  game.shipSpeedX *= 0.95;
  game.shipSpeedY *= 0.95;

  if (game.shipX < 0) game.shipX = tft.width();
  if (game.shipX > tft.width()) game.shipX = 0;
  if (game.shipY < 0) game.shipY = tft.height();
  if (game.shipY > tft.height()) game.shipY = 0;

  for (auto &b : game.bullets) {
    if (b.active) {
      b.prevX = b.x;
      b.prevY = b.y;
      b.x += b.dx * GAME_SPEED;
      b.y += b.dy * GAME_SPEED;
      if (millis() - b.spawnTime > 1000 || b.x < 0 || b.x > tft.width() || b.y < 0 || b.y > tft.height())
        b.active = false;
    }
  }

  int desiredAsteroids = 1 + (game.score / 5);
  if (desiredAsteroids > MAX_ASTEROIDS) desiredAsteroids = MAX_ASTEROIDS;

  int activeAsteroids = 0;
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    auto &a = game.asteroids[i];

    if (!a.active && activeAsteroids < desiredAsteroids) {
      a.x = random(2) ? -ASTEROID_SIZE : tft.width() + ASTEROID_SIZE;
      a.y = random(tft.height());
      float dx = (tft.width() / 2 - a.x) * 0.01;
      float dy = (tft.height() / 2 - a.y) * 0.01;
      a.dx = dx;
      a.dy = dy;
      a.active = true;
    }

    if (a.active) {
      activeAsteroids++;

      a.prevX = a.x;
      a.prevY = a.y;
      a.x += a.dx * (GAME_SPEED / 4.0);
      a.y += a.dy * (GAME_SPEED / 4.0);
      if (a.x < -ASTEROID_SIZE) a.x = tft.width();
      if (a.x > tft.width()) a.x = -ASTEROID_SIZE;
      if (a.y < -ASTEROID_SIZE) a.y = tft.height();
      if (a.y > tft.height()) a.y = -ASTEROID_SIZE;

      // Столкновение с кораблем
      float dx = a.x - game.shipX;
      float dy = a.y - game.shipY;
      if (dx * dx + dy * dy < pow(ASTEROID_SIZE / 2 + SHIP_SIZE / 2, 2)) {
        game.score = 0;
      }

      // Столкновение с пулями
      for (auto &b : game.bullets) {
        if (b.active) {
          float bdx = a.x - b.x;
          float bdy = a.y - b.y;
          if (bdx * bdx + bdy * bdy < pow(ASTEROID_SIZE / 2 + BULLET_SIZE, 2)) {
            a.active = false;
            b.active = false;
            game.score++;
          }
        }
      }
    }
  }
}

void renderGame(bool fullRedraw) {
  // Счёт
  tft.fillRect(0, 0, 128, 20, TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(2, 2);
  tft.setTextSize(1);
  tft.print("Score: ");
  tft.print(game.score);

  // Пули
  for (auto &b : game.bullets) {
    if (!b.active || fullRedraw)
      tft.fillCircle((int)b.prevX, (int)b.prevY, BULLET_SIZE + 1, TFT_BLACK);
    if (b.active)
      tft.fillCircle((int)b.x, (int)b.y, BULLET_SIZE, TFT_RED);
  }

  // Астероиды
  for (auto &a : game.asteroids) {
    if (!a.active || fullRedraw)
      tft.fillCircle((int)a.prevX, (int)a.prevY, ASTEROID_SIZE / 2 + 1, TFT_BLACK);
    if (a.active)
      tft.fillCircle((int)a.x, (int)a.y, ASTEROID_SIZE / 2, TFT_WHITE);
  }

  // Корабль
  if (fullRedraw) {
    drawShip(game.shipX, game.shipY, game.shipAngle, TFT_BLUE);
  } else {
    drawShip(game.prevShipX, game.prevShipY, game.shipAngle, TFT_BLACK);
    drawShip(game.shipX, game.shipY, game.shipAngle, TFT_BLUE);
  }
}

void drawShip(float x, float y, float angle, uint16_t color) {
  float x1 = x + cos(angle) * SHIP_SIZE / 2;
  float y1 = y + sin(angle) * SHIP_SIZE / 2;
  float x2 = x + cos(angle + 2.5) * SHIP_SIZE / 2.5;
  float y2 = y + sin(angle + 2.5) * SHIP_SIZE / 2.5;
  float x3 = x + cos(angle - 2.5) * SHIP_SIZE / 2.5;
  float y3 = y + sin(angle - 2.5) * SHIP_SIZE / 2.5;
  tft.fillTriangle((int)x1, (int)y1, (int)x2, (int)y2, (int)x3, (int)y3, color);
}
