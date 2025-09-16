#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>
#include "driver/i2s.h"

// ==================== ���� ====================
#define I2S_BCK_PIN   26
#define I2S_WS_PIN    25
#define I2S_DOUT_PIN  22
#define I2S_NUM_USED  I2S_NUM_0

#define BUZZER_PIN 13

// ==================== ��������� WAV ����� ====================
struct WavInfo {
    uint32_t dataPos;
    uint32_t dataLen;
    uint16_t channels;
    uint32_t sampleRate;
    uint16_t bitsPerSample;
};

// ==================== ����������� ����� ====================
extern const char* introTracks[];
extern const char* mainTracks[];
extern const int introTracksCount;
extern const int mainTracksCount;

// ==================== ������� ����� ====================
void audioInit();
bool playWavFile(const char* filename);
void stopAudio();
void playRandomIntro();
void playRandomMain();
bool isAudioPlaying();
WavInfo parseWav(File& f);

// ==================== ���� �������� �������� ====================
enum SoundEffect {
    SOUND_LASER,
    SOUND_HIT,
    SOUND_EXPLOSION
};

// ==================== ������� ====================
void playEffect(SoundEffect effect);

// ==================== ���������� ====================
void updateAudio();

#endif
