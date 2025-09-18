#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>

// ==================== ��������� ����� ====================
constexpr uint8_t ENCODER_CLK = 32;
constexpr uint8_t ENCODER_DT = 33;
constexpr uint8_t ENCODER_SW = 14;
constexpr uint8_t BUTTON_A = 2;
constexpr uint8_t BUTTON_B = 19;

// ==================== ��������� ��������� ����� ====================
struct InputState {
    int16_t encoderAngle;     // ���� (� ��������, ������� 10)
    bool encoderPressed;      // ������ �� ������ ��������
    bool buttonA;             // ������ ��������
    bool buttonB;             // ������ ��������
};

// ==================== ����� ���������� ������ ====================
class InputManager {
public:
    bool init();
    void update();
    InputState getState() const;

private:
    static void handleEncoderISR();

    // ��������� �����
    InputState currentState;

    // ���������� ��� ��������
    static volatile int32_t encoderPos;
    static volatile int lastEncoderARaw;

    // ���������� ��� ��������
    static bool encoderSwRaw;
    static bool encoderSwStable;
    static bool lastEncoderSwStable;
    static unsigned long encoderSwLastDebounce;

    static bool btnARaw;
    static bool btnAStable;
    static bool lastBtnAStable;
    static unsigned long btnALastDebounce;

    static bool btnBRaw;
    static bool btnBStable;
    static bool lastBtnBStable;
    static unsigned long btnBLastDebounce;

    static bool encoderPressed;
};

#endif
