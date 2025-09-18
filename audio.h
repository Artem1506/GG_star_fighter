#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>
#include <driver/i2s.h>
#include <SD_MMC.h>

// ==================== ÊÎÍÑÒÀÍÒÛ ÏÈÍÎÂ ====================
constexpr uint8_t I2S_BCK_PIN = 26;
constexpr uint8_t I2S_WS_PIN = 25;
constexpr uint8_t I2S_DOUT_PIN = 22;
constexpr uint8_t BUZZER_PIN = 13;

// ==================== ÑÒĞÓÊÒÓĞÀ WAV ÔÀÉËÀ ====================
struct WavInfo {
    uint32_t dataPos;
    uint32_t dataLen;
    uint16_t channels;
    uint32_t sampleRate;
    uint16_t bitsPerSample;
};

// ==================== ÒÈÏÛ ÇÂÓÊÎÂÛÕ İÔÔÅÊÒÎÂ ====================
enum SoundEffect {
    SOUND_LASER,
    SOUND_HIT,
    SOUND_EXPLOSION,
    SOUND_NONE
};

// ==================== ÎÑÍÎÂÍÎÉ ÊËÀÑÑ ÀÓÄÈÎ ====================
class AudioManager {
public:
    bool init();
    void update();

    // Ìóçûêàëüíûå òğåêè
    void playRandomIntro();
    void playRandomMain();
    void playRandomGameOver();
    void stopMusic();

    // Çâóêîâûå ıôôåêòû
    void playLaser();
    void playHit();
    void playCrash();

    bool isPlaying() const { return isPlayingMusic; }

private:
    void playWAV(const char* filename);
    WavInfo parseWav(File& file);
    void musicTask(void* filename);

    static const char* introTracks[3];
    static const char* mainTracks[5];
    static const char* gameOverTracks[3];

    bool isPlayingMusic = false;
    TaskHandle_t musicTaskHandle = NULL;
};

#endif
