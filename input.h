#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>

// ==================== Пины ====================
#define ENCODER_CLK 32
#define ENCODER_DT  33
#define ENCODER_SW  14
#define BUTTON_A    2
#define BUTTON_B    19

// ==================== Структура ввода ====================
struct InputState {
    int encoderAngle;     // угол (в градусах, кратный 10)
    bool encoderPressed;  // нажата ли кнопка энкодера (одноразовое событие)
    bool buttonA;         // кнопка движения
    bool buttonB;         // кнопка стрельбы
};

// ==================== Инициализация ====================
void inputInit();

// ==================== Обновление ====================
void updateInput();

// ==================== Получение состояния ====================
InputState getInput();

#endif
