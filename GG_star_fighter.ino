#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SD_MMC.h>

// ===== Подключаем модули =====
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include "game.h"
#include "storage.h"
#include "entities.h"

// ===== Глобальные объекты =====
TFT_eSPI tft;                   // Дисплей
Graphics graphics;              // Графика
InputManager input;             // Ввод
AudioManager audio;             // Аудио
GameManager game;               // Игра
StorageManager storage;         // Хранилище

void setup() {
    Serial.begin(115200);
    // === Инициализация подсистем ===
    graphics.init();
    input.init();
    storage.init();
    audio.init();
    initEntities();
    
    game.init();
}

void loop() {
    uint32_t frameStart = millis();
    
    input.update();
    InputState inputState = input.getState();
    game.update();
    audio.update();
    
    graphics.clear();
    game.render();
    graphics.present();
    
    // Контроль FPS
    uint32_t frameTime = millis() - frameStart;
    if (frameTime < 16) delay(16 - frameTime);
}
