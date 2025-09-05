// config.h
#pragma once

// ==================== STATES ====================
enum GameState {
    STATE_SPLASH,      // Экран загрузки и логотипа
    STATE_MAIN_MENU,   // Главное меню (можно добавить позже)
    STATE_GAMEPLAY,    // Игровой процесс
    STATE_GAME_OVER,   // Экран завершения игры
    STATE_HIGHSCORES   // Таблица рекордов (можно добавить позже)
};

// ==================== DISPLAY ====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 160
#define LOGO_WIDTH 128
#define LOGO_HEIGHT 120

// ==================== TIMING ====================
#define SPLASH_DURATION 3000    // 3 секунды показа логотипа
#define PHYSICS_FPS 40
#define RENDER_FPS 30
#define ANIMATION_FPS 60

// ==================== GAME ====================
#define MAX_ASTEROIDS 10
#define MAX_BULLETS 15
#define INITIAL_ASTEROIDS 1
#define ASTEROIDS_PER_LEVEL 2
#define SHOOTING_COOLDOWN 500

// ==================== HARDWARE ====================
#define ENCODER_CLK 14
#define ENCODER_DT 12
#define BUTTON_A 15
#define BUTTON_B 19
#define BUZZER_PIN 25