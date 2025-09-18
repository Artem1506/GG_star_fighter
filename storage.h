#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <SD_MMC.h>

// ==================== ЕДИНЫЙ ИНТЕРФЕЙС ====================
class StorageManager {
public:
    bool init(); // Единая инициализация
    int readHighScore();
    void writeHighScore(int score);

    // Дополнительные функции доступа
    File openFile(const char* filename, const char* mode = FILE_READ);

private:
    static const char* HIGHSCORE_FILE;
};

#endif
