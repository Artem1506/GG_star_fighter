#include "audio.h"
#include <SD_MMC.h>

// ==================== СТАТИЧЕСКИЕ МАССИВЫ ТРЕКОВ ====================
const char* AudioManager::introTracks[3] = {
    "/snd_intro1.wav", "/snd_intro2.wav", "/snd_intro3.wav"
};

const char* AudioManager::mainTracks[5] = {
    "/snd_main1.wav", "/snd_main2.wav", "/snd_main3.wav",
    "/snd_main4.wav", "/snd_main5.wav"
};

const char* AudioManager::gameOverTracks[3] = {
    "/snd_gameover1.wav", "/snd_gameover2.wav", "/snd_gameover3.wav"
};

// ==================== ИНИЦИАЛИЗАЦИЯ ====================
bool AudioManager::init() {
    // Инициализация I2S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 6,    // Уменьшено для экономии памяти
        .dma_buf_len = 64,
        .use_apll = false
    };
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_DOUT_PIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    i2s_set_pin(I2S_NUM_0, &pin_config);

    // Инициализация бузера
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    return true;
}

// ==================== ЗАДАЧА ВОСПРОИЗВЕДЕНИЯ МУЗЫКИ ====================
void AudioManager::musicTask(void* filename) {
    const char* path = (const char*)filename;
    File file = SD_MMC.open(path);
    if (!file) {
        isPlayingMusic = false;
        vTaskDelete(NULL);
        return;
    }

    // Парсинг WAV файла
    WavInfo info = parseWav(file);
    if (info.dataLen == 0) {
        file.close();
        isPlayingMusic = false;
        vTaskDelete(NULL);
        return;
    }

    file.seek(info.dataPos);

    // Буфер для чтения данных
    uint8_t buffer[256];  // Уменьшенный буфер для экономии памяти
    size_t bytesRead;

    isPlayingMusic = true;
    while (file.available() && isPlayingMusic) {
        bytesRead = file.read(buffer, sizeof(buffer));
        if (bytesRead > 0) {
            size_t bytesWritten;
            i2s_write(I2S_NUM_0, buffer, bytesRead, &bytesWritten, portMAX_DELAY);
        }
    }

    file.close();
    isPlayingMusic = false;
    vTaskDelete(NULL);
}

// ==================== ПАРСИНГ WAV ФАЙЛА ====================
WavInfo AudioManager::parseWav(File& file) {
    WavInfo info = {};

    if (file.size() < 44) return info; // Минимальный размер WAV

    // Чтение заголовка
    file.seek(22);
    file.read((uint8_t*)&info.channels, 2);
    file.read((uint8_t*)&info.sampleRate, 4);
    file.seek(34);
    file.read((uint8_t*)&info.bitsPerSample, 2);

    // Поиск блока данных
    file.seek(36);
    uint32_t chunkId, chunkSize;

    while (file.available() > 8) {
        file.read((uint8_t*)&chunkId, 4);
        file.read((uint8_t*)&chunkSize, 4);

        if (chunkId == 0x61746164) { // "data"
            info.dataPos = file.position();
            info.dataLen = chunkSize;
            break;
        }

        if (chunkSize > file.available()) break;
        file.seek(file.position() + chunkSize);
    }

    return info;
}

// ==================== ВОСПРОИЗВЕДЕНИЕ МУЗЫКИ ====================
void AudioManager::playWAV(const char* filename) {
    stopMusic(); // Останавливаем текущее воспроизведение

    // Создаем задачу для воспроизведения
    xTaskCreatePinnedToCore(
        [](void* param) { ((AudioManager*)param)->musicTask(param); },
        "AudioTask",
        2048,       // Уменьшенный стек
        (void*)filename,
        1,
        &musicTaskHandle,
        1
    );
}

void AudioManager::playRandomIntro() {
    uint8_t idx = random(3);
    playWAV(introTracks[idx]);
}

void AudioManager::playRandomMain() {
    uint8_t idx = random(5);
    playWAV(mainTracks[idx]);
}

void AudioManager::playRandomGameOver() {
    uint8_t idx = random(3);
    playWAV(gameOverTracks[idx]);
}

void AudioManager::stopMusic() {
    isPlayingMusic = false;
    if (musicTaskHandle) {
        vTaskDelete(musicTaskHandle);
        musicTaskHandle = NULL;
    }
}

// ==================== ЗВУКОВЫЕ ЭФФЕКТЫ ====================
void AudioManager::playLaser() {
    tone(BUZZER_PIN, 2000, 100); // Высокий тон, 100ms
}

void AudioManager::playHit() {
    tone(BUZZER_PIN, 400, 150);  // Низкий тон, 150ms
}

void AudioManager::playCrash() {
    tone(BUZZER_PIN, 200, 400);  // Очень низкий тон, 400ms
}

// ==================== ОБНОВЛЕНИЕ ====================
void AudioManager::update() {
    // Периодические проверки или обновления эффектов
}
