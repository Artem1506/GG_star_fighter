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

void setup() {
    Serial.begin(115200);

    // === Инициализация подсистем ===
    graphicsInit(&tft);
    inputInit();
    audioInit();
    storageInit();
    initEntities();

    // === Запуск игры ===
    gameInit();
}

void loop() {
    // === Игровой цикл ===
    gameUpdate();   // обновление логики
    gameRender();   // отрисовка
    updateAudio();  // обновление звука
}
