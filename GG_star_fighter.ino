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
  input.update();
  audio.update();
  stateMachine.update();

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

// Отрисовка
    display.clear();
    stateMachine.render();
    display.present();
    
    // Управление FPS
    delay(16); // ~60 FPS

  display.render();
}
