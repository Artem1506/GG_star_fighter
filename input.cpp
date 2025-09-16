#include "input.h"
#include <Arduino.h>
#include <driver/gpio.h> // ��� gpio_get_level � ISR (ESP32)

// ========== ��������� debounce ==========
static const unsigned long DEBOUNCE_MS = 50; // �������� ������������ (����� ���������/���������)

// ========== ���������� ���������� ==========
volatile int encoderPos = 0;    // "�����" �������� �������� (�� ISR)
int encoderAngle = 0;          // ���� � ��������
// ��� ISR: ���������� ����� ��������� A
volatile int lastEncoderARaw = 0;

// encoder switch (SW) � raw + debounce
static bool encoderSwRaw = false;
static bool encoderSwStable = false;
static bool lastEncoderSwStable = false;
static unsigned long encoderSwLastDebounce = 0;

// ������ A
static bool btnARaw = false;
static bool btnAStable = false;
static bool lastBtnAStable = false;
static unsigned long btnALastDebounce = 0;

// ������ B
static bool btnBRaw = false;
static bool btnBStable = false;
static bool lastBtnBStable = false;
static unsigned long btnBLastDebounce = 0;

// ����������� ������� "������� ��������"
static bool encoderPressed = false;

// ========== ���������� ���������� ��� �������� ==========
void IRAM_ATTR handleEncoder() {
    // ������� ���������� ����������� ��������� GPIO � ISR
    int A = gpio_get_level((gpio_num_t)ENCODER_CLK);
    int B = gpio_get_level((gpio_num_t)ENCODER_DT);

    // ����������� ������ �� ��������� A (���� �� ��������������� ���� ������ ��������)
    if (A != lastEncoderARaw) {
        if (A == B) {
            encoderPos++;   // ����������� ������� �� ���
        }
        else {
            encoderPos--;
        }
    }
    lastEncoderARaw = A;
}

// ========== ������������� ==========
void inputInit() {
    // �������� ���� ��� INPUT_PULLUP (�����: ��� -> ������ -> GND)
    pinMode(ENCODER_CLK, INPUT_PULLUP);
    pinMode(ENCODER_DT, INPUT_PULLUP);
    pinMode(ENCODER_SW, INPUT_PULLUP);

    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(BUTTON_B, INPUT_PULLUP);

    // �������������� "�����" � ���������� ��������� �� �������� ������ �����
    lastEncoderARaw = gpio_get_level((gpio_num_t)ENCODER_CLK);

    encoderSwRaw = (digitalRead(ENCODER_SW) == LOW);
    encoderSwStable = encoderSwRaw;
    lastEncoderSwStable = encoderSwStable;
    encoderSwLastDebounce = 0;

    btnARaw = (digitalRead(BUTTON_A) == LOW);
    btnAStable = btnARaw;
    lastBtnAStable = btnAStable;
    btnALastDebounce = 0;

    btnBRaw = (digitalRead(BUTTON_B) == LOW);
    btnBStable = btnBRaw;
    lastBtnBStable = btnBStable;
    btnBLastDebounce = 0;

    // ���������� ������ �� CLK (A) � ��������� ������ �������
    attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handleEncoder, CHANGE);
}

// ========== ���������� ��������� (��������� � loop()) ==========
void updateInput() {
    // ���������� ����������� ������� � ��� ���������� ���� ���� ����� �����
    encoderPressed = false;

    // ��������� ����������� ���� (��� 10 ��������)
    encoderAngle = (encoderPos * 10) % 360;
    if (encoderAngle < 0) encoderAngle += 360;

    unsigned long now = millis();

    // ---------- Encoder SW (debounce) ----------
    bool swReading = (digitalRead(ENCODER_SW) == LOW); // LOW = ������
    if (swReading != encoderSwRaw) {
        // ���������� "�����" ������ � �������� ������ ������������
        encoderSwRaw = swReading;
        encoderSwLastDebounce = now;
    }
    else {
        // ���� ������ ���������� ������� � "�����" ���������� �� ����������� � ������� ����� ������������
        if ((now - encoderSwLastDebounce) > DEBOUNCE_MS && encoderSwRaw != encoderSwStable) {
            lastEncoderSwStable = encoderSwStable;
            encoderSwStable = encoderSwRaw;

            if (encoderSwStable) {
                Serial.println("Encoder SW: PRESSED");
            }
            else {
                Serial.println("Encoder SW: RELEASED");
            }

            // ����������� ������� ��� �������� RELEASED -> PRESSED
            if (encoderSwStable && !lastEncoderSwStable) {
                encoderPressed = true;
            }
        }
    }

    // ---------- Button A (debounce) ----------
    bool aReading = (digitalRead(BUTTON_A) == LOW);
    if (aReading != btnARaw) {
        btnARaw = aReading;
        btnALastDebounce = now;
    }
    else {
        if ((now - btnALastDebounce) > DEBOUNCE_MS && btnARaw != btnAStable) {
            lastBtnAStable = btnAStable;
            btnAStable = btnARaw;
            if (btnAStable) Serial.println("Button A: PRESSED"); else Serial.println("Button A: RELEASED");
        }
    }

    // ---------- Button B (debounce) ----------
    bool bReading = (digitalRead(BUTTON_B) == LOW);
    if (bReading != btnBRaw) {
        btnBRaw = bReading;
        btnBLastDebounce = now;
    }
    else {
        if ((now - btnBLastDebounce) > DEBOUNCE_MS && btnBRaw != btnBStable) {
            lastBtnBStable = btnBStable;
            btnBStable = btnBRaw;
            if (btnBStable) Serial.println("Button B: PRESSED"); else Serial.println("Button B: RELEASED");
        }
    }
}

// ========== ������� ��������� � game/������ ������� ==========
InputState getInput() {
    InputState st;
    st.encoderAngle = encoderAngle;
    st.encoderPressed = encoderPressed;   // ����������� ������� (true ������ � �����, ��� ��������� ����������������� �������)
    st.buttonA = btnAStable;
    st.buttonB = btnBStable;
    return st;
}
