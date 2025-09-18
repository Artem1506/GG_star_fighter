#include <Arduino.h>

// ===== Подключаем модули =====
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include "game.h"
#include "storage.h"
#include "entities.h"

#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();   // создаём глобальный дисплей

GameManager game;
StorageManager storage;
AudioManager audio;
StateMachine stateMachine;
StorageManager storage;
CollisionSystem collisionSystem;

void setup() {
    Serial.begin(115200);

    // === Инициализация подсистем ===
    display.init();
    input.init();
    storage.initSD();
    audio.init();
  
    stateMachine.changeState(new LogoState());
    
    game.changeState(STATE_LOGO);
}

void loop() {
    uint32_t frameStart = millis();
    
    // 1. Ввод (максимально быстро)
    readInputs();
    
    // 2. Обновление игры
    updateGame();
    
    // 3. Отрисовка
    graphics.clear();
    drawGame();
    graphics.present();
    
    // 4. Точный контроль FPS (60 FPS = 16.67ms на кадр)
    uint32_t frameTime = millis() - frameStart;
    if (frameTime < 16) {
        delay(16 - frameTime);
    }
}
