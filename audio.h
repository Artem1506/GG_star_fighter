#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>
#include "driver/i2s.h"

// ==================== Пины ====================
#define I2S_BCK_PIN   26
#define I2S_WS_PIN    25
#define I2S_DOUT_PIN  22
#define I2S_NUM_USED  I2S_NUM_0

#define BUZZER_PIN 13

// ==================== Структура WAV файла ====================
struct WavInfo {
    uint32_t dataPos;
    uint32_t dataLen;
    uint16_t channels;
    uint32_t sampleRate;
    uint16_t bitsPerSample;
};

// ==================== Музыкальные треки ====================
const char* introTracks[] = { 
  "/snd_intro1.wav", "/snd_intro2.wav", "/snd_intro3.wav" 
};
const char* mainTracks[] = {
  "/snd_main1.wav", "/snd_main2.wav", "/snd_main3.wav",
  "/snd_main4.wav", "/snd_main5.wav"
};
const char* gameOverTracks[] = {
  "/snd_gameover1.wav", "/snd_gameover2.wav", "/snd_gameover3.wav"
};

class AudioManager {
public:
    void init();
    void update();

    void playRandomIntro();
    void playRandomMain();
    void playRandomGameOver();
    void playShot();
    void playHit();
    void playCrash();

private:
    void playWAV(const char* filename);
};

// ==================== Функции аудио ====================
void audioInit();
bool playWavFile(const char* filename);
void stopAudio();
void playRandomIntro();
void playRandomMain();
bool isAudioPlaying();
WavInfo parseWav(File& f);

// ==================== Типы звуковых эффектов ====================
enum SoundEffect {
    SOUND_LASER,
    SOUND_HIT,
    SOUND_EXPLOSION
};

// ==================== Эффекты ====================
void playEffect(SoundEffect effect);

// ==================== Обновление ====================
void updateAudio();

#endif
