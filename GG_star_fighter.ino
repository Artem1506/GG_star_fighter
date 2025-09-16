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

void setup() {
    Serial.begin(115200);

    // === Инициализация подсистем ===
    display.init();
    input.init();
    storage.initSD();
    audio.init();
  
    game.changeState(STATE_LOGO);
}

void loop() {
  input.update();
  audio.update();
  
  switch(game.getCurrentState()) {
    case STATE_LOGO:
      updateLogoState();
      break;
    case STATE_MENU:
      updateMenuState();
      break;
    case STATE_PLAY:
      updatePlayState();
      break;
    case STATE_GAME_OVER:
      updateGameOverState();
      break;
  }

    animationManager.update();
    
    // Отрисовка анимаций
    animationManager.render();

  display.render();
}
