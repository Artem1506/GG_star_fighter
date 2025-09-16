#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>

// ==================== ���� ====================
#define ENCODER_CLK 32
#define ENCODER_DT  33
#define ENCODER_SW  14
#define BUTTON_A    2
#define BUTTON_B    19

// ==================== ��������� ����� ====================
struct InputState {
    int encoderAngle;     // ���� (� ��������, ������� 10)
    bool encoderPressed;  // ������ �� ������ �������� (����������� �������)
    bool buttonA;         // ������ ��������
    bool buttonB;         // ������ ��������
};

// ==================== ������������� ====================
void inputInit();

// ==================== ���������� ====================
void updateInput();

// ==================== ��������� ��������� ====================
InputState getInput();

#endif
