#include "input.h"
#include <driver/gpio.h>

// ==================== ����������� ���������� ====================
volatile int32_t InputManager::encoderPos = 0;
volatile int InputManager::lastEncoderARaw = 0;

bool InputManager::encoderSwRaw = false;
bool InputManager::encoderSwStable = false;
bool InputManager::lastEncoderSwStable = false;
unsigned long InputManager::encoderSwLastDebounce = 0;

bool InputManager::btnARaw = false;
bool InputManager::btnAStable = false;
bool InputManager::lastBtnAStable = false;
unsigned long InputManager::btnALastDebounce = 0;

bool InputManager::btnBRaw = false;
bool InputManager::btnBStable = false;
bool InputManager::lastBtnBStable = false;
unsigned long InputManager::btnBLastDebounce = 0;

bool InputManager::encoderPressed = false;

constexpr unsigned long DEBOUNCE_MS = 50;

// ==================== ���������� ���������� ====================
void IRAM_ATTR InputManager::handleEncoderISR() {
    int A = gpio_get_level((gpio_num_t)ENCODER_CLK);
    int B = gpio_get_level((gpio_num_t)ENCODER_DT);

    if (A != lastEncoderARaw) {
        if (A == B) {
            encoderPos++;
        }
        else {
            encoderPos--;
        }
    }
    lastEncoderARaw = A;
}

// ==================== ������������� ====================
bool InputManager::init() {
    // ��������� �����
    pinMode(ENCODER_CLK, INPUT_PULLUP);
    pinMode(ENCODER_DT, INPUT_PULLUP);
    pinMode(ENCODER_SW, INPUT_PULLUP);
    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(BUTTON_B, INPUT_PULLUP);

    // ������������� ���������
    lastEncoderARaw = gpio_get_level((gpio_num_t)ENCODER_CLK);

    encoderSwRaw = (digitalRead(ENCODER_SW) == LOW);
    encoderSwStable = encoderSwRaw;
    lastEncoderSwStable = encoderSwStable;

    btnARaw = (digitalRead(BUTTON_A) == LOW);
    btnAStable = btnARaw;
    lastBtnAStable = btnAStable;

    btnBRaw = (digitalRead(BUTTON_B) == LOW);
    btnBStable = btnBRaw;
    lastBtnBStable = btnBStable;

    // ��������� ����������
    attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handleEncoderISR, CHANGE);

    return true;
}

// ==================== ���������� ��������� ====================
void InputManager::update() {
    // ����� ������������ �������
    encoderPressed = false;

    // ���������� ���� ��������
    currentState.encoderAngle = (encoderPos * 10) % 360;
    if (currentState.encoderAngle < 0) currentState.encoderAngle += 360;

    unsigned long now = millis();

    // ---------- ������� ������ �������� ----------
    bool swReading = (digitalRead(ENCODER_SW) == LOW);
    if (swReading != encoderSwRaw) {
        encoderSwRaw = swReading;
        encoderSwLastDebounce = now;
    }
    else if ((now - encoderSwLastDebounce) > DEBOUNCE_MS) {
        if (encoderSwRaw != encoderSwStable) {
            lastEncoderSwStable = encoderSwStable;
            encoderSwStable = encoderSwRaw;

            // ����������� ������� ��� �������
            if (encoderSwStable && !lastEncoderSwStable) {
                encoderPressed = true;
            }
        }
    }

    // ---------- ������� ������ A ----------
    bool aReading = (digitalRead(BUTTON_A) == LOW);
    if (aReading != btnARaw) {
        btnARaw = aReading;
        btnALastDebounce = now;
    }
    else if ((now - btnALastDebounce) > DEBOUNCE_MS) {
        btnAStable = btnARaw;
    }

    // ---------- ������� ������ B ----------
    bool bReading = (digitalRead(BUTTON_B) == LOW);
    if (bReading != btnBRaw) {
        btnBRaw = bReading;
        btnBLastDebounce = now;
    }
    else if ((now - btnBLastDebounce) > DEBOUNCE_MS) {
        btnBStable = btnBRaw;
    }

    // ���������� �������� ���������
    currentState.encoderPressed = encoderPressed;
    currentState.buttonA = btnAStable;
    currentState.buttonB = btnBStable;
}

// ==================== ��������� ��������� ====================
InputState InputManager::getState() const {
    return currentState;
}
