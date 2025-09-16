#include "audio.h"
#include <driver/i2s.h>
#include <SD_MMC.h>

// ==================== Глобальные переменные ====================
static TaskHandle_t musicTaskHandle = NULL;
static bool isPlayingMusic = false;

// ==================== Реализация музыкальных треков ====================
const char* introTracks[] = {
    "/snd_intro1.wav",
    "/snd_intro2.wav",
    "/snd_intro3.wav"
};

const char* mainTracks[] = {
    "/snd_main1.wav",
    "/snd_main2.wav",
    "/snd_main3.wav",
    "/snd_main4.wav",
    "/snd_main5.wav"
};

const int introTracksCount = 3;
const int mainTracksCount = 5;

// ==================== WAV ====================
struct WavInfo {
    uint32_t dataPos;
    uint32_t dataLen;
    uint16_t channels;
    uint32_t sampleRate;
    uint16_t bitsPerSample;
};

static WavInfo parseWav(File f) {
    WavInfo info = {};
    f.seek(22);
    f.read((uint8_t*)&info.channels, 2);
    f.read((uint8_t*)&info.sampleRate, 4);
    f.seek(34);
    f.read((uint8_t*)&info.bitsPerSample, 2);

    uint32_t chunkId;
    uint32_t chunkSize;
    while (f.available()) {
        f.read((uint8_t*)&chunkId, 4);
        f.read((uint8_t*)&chunkSize, 4);
        if (chunkId == 0x61746164) { // "data"
            info.dataPos = f.position();
            info.dataLen = chunkSize;
            break;
        }
        f.seek(f.position() + chunkSize);
    }
    return info;
}

// ==================== Воспроизведение WAV ====================
static void playWavTask(void* param) {
    String path = String((char*)param);
    File f = SD_MMC.open(path);
    if (!f) {
        vTaskDelete(NULL);
        return;
    }

    WavInfo info = parseWav(f);
    f.seek(info.dataPos);

    uint8_t buffer[512];
    size_t bytesRead;

    isPlayingMusic = true;
    while (f.available()) {
        bytesRead = f.read(buffer, sizeof(buffer));
        size_t written;
        i2s_write(I2S_NUM_USED, buffer, bytesRead, &written, portMAX_DELAY);
    }
    isPlayingMusic = false;
    f.close();
    vTaskDelete(NULL);
}

// ==================== I2S Init ====================
void audioInit() {
    // I2S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false
    };
    i2s_driver_install(I2S_NUM_USED, &i2s_config, 0, NULL);

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_DOUT_PIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    i2s_set_pin(I2S_NUM_USED, &pin_config);

    // Buzzer
    pinMode(BUZZER_PIN, OUTPUT);
}

// ==================== Музыка ====================
void playRandomIntro() {
    if (isPlayingMusic) return;
    int idx = random(introTracksCount);
    xTaskCreatePinnedToCore(playWavTask, "music", 4096, (void*)introTracks[idx], 1, &musicTaskHandle, 1);
}

void playRandomMain() {
    if (isPlayingMusic) return;
    int idx = random(mainTracksCount);
    xTaskCreatePinnedToCore(playWavTask, "music", 4096, (void*)mainTracks[idx], 1, &musicTaskHandle, 1);
}

// ==================== Эффекты через buzzer ====================
void playEffect(SoundEffect effect) {
    switch (effect) {
    case SOUND_LASER:
        tone(BUZZER_PIN, 2000, 100); // высокий тон
        break;
    case SOUND_HIT:
        tone(BUZZER_PIN, 400, 150);  // низкий короткий
        break;
    case SOUND_EXPLOSION:
        tone(BUZZER_PIN, 200, 400);  // очень низкий длинный
        break;
    }
}

// ==================== Обновление ====================
void updateAudio() {
    // Здесь можно обновлять спецэффекты (например вибрато лазера)
    // Сейчас заглушка
}
