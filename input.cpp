#include "input.h"
#include <Arduino.h>
#include <driver/gpio.h> // для gpio_get_level в ISR (ESP32)

// ========== Настройки debounce ==========
static const unsigned long DEBOUNCE_MS = 50; // задержка стабильности (можно уменьшить/увеличить)

// ========== Глобальные переменные ==========
volatile int encoderPos = 0;    // "сырое" значение энкодера (из ISR)
int encoderAngle = 0;          // угол в градусах
// Для ISR: предыдущее сырое состояние A
volatile int lastEncoderARaw = 0;

// encoder switch (SW) — raw + debounce
static bool encoderSwRaw = false;
static bool encoderSwStable = false;
static bool lastEncoderSwStable = false;
static unsigned long encoderSwLastDebounce = 0;

// кнопка A
static bool btnARaw = false;
static bool btnAStable = false;
static bool lastBtnAStable = false;
static unsigned long btnALastDebounce = 0;

// кнопка B
static bool btnBRaw = false;
static bool btnBStable = false;
static bool lastBtnBStable = false;
static unsigned long btnBLastDebounce = 0;

// одноразовое событие "нажатие энкодера"
static bool encoderPressed = false;

// ========== Обработчик прерываний для энкодера ==========
void IRAM_ATTR handleEncoder() {
    // Быстрый безопасный считыватель состояния GPIO в ISR
    int A = gpio_get_level((gpio_num_t)ENCODER_CLK);
    int B = gpio_get_level((gpio_num_t)ENCODER_DT);

    // Срабатываем только по изменению A (одна из распространённых схем чтения энкодера)
    if (A != lastEncoderARaw) {
        if (A == B) {
            encoderPos++;   // направление зависит от фаз
        }
        else {
            encoderPos--;
        }
    }
    lastEncoderARaw = A;
}

// ========== Инициализация ==========
void inputInit() {
    // Настроим пины как INPUT_PULLUP (схема: пин -> кнопка -> GND)
    pinMode(ENCODER_CLK, INPUT_PULLUP);
    pinMode(ENCODER_DT, INPUT_PULLUP);
    pinMode(ENCODER_SW, INPUT_PULLUP);

    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(BUTTON_B, INPUT_PULLUP);

    // Инициализируем "сырые" и стабильные состояния по текущему уровню пинов
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

    // Прерывание только на CLK (A) — уменьшает ложные события
    attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handleEncoder, CHANGE);
}

// ========== Обновление состояния (вызывайте в loop()) ==========
void updateInput() {
    // Сбрасываем одноразовое событие — оно выставится ниже если будет смена
    encoderPressed = false;

    // Обновляем вычисляемый угол (шаг 10 градусов)
    encoderAngle = (encoderPos * 10) % 360;
    if (encoderAngle < 0) encoderAngle += 360;

    unsigned long now = millis();

    // ---------- Encoder SW (debounce) ----------
    bool swReading = (digitalRead(ENCODER_SW) == LOW); // LOW = нажата
    if (swReading != encoderSwRaw) {
        // изменилось "сырое" чтение — стартуем таймер стабилизации
        encoderSwRaw = swReading;
        encoderSwLastDebounce = now;
    }
    else {
        // если прошло достаточно времени и "сырое" отличается от стабильного — считаем смену состоявшейся
        if ((now - encoderSwLastDebounce) > DEBOUNCE_MS && encoderSwRaw != encoderSwStable) {
            lastEncoderSwStable = encoderSwStable;
            encoderSwStable = encoderSwRaw;

            if (encoderSwStable) {
                Serial.println("Encoder SW: PRESSED");
            }
            else {
                Serial.println("Encoder SW: RELEASED");
            }

            // одноразовое событие при переходе RELEASED -> PRESSED
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

// ========== Возврат состояния в game/другим модулям ==========
InputState getInput() {
    InputState st;
    st.encoderAngle = encoderAngle;
    st.encoderPressed = encoderPressed;   // одноразовое событие (true только в цикле, где произошло стабилизированное нажатие)
    st.buttonA = btnAStable;
    st.buttonB = btnBStable;
    return st;
}
