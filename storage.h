#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <SD_MMC.h>

// ==================== ������ ��������� ====================
class StorageManager {
public:
    bool init(); // ������ �������������
    int readHighScore();
    void writeHighScore(int score);

    // �������������� ������� �������
    File openFile(const char* filename, const char* mode = FILE_READ);

private:
    static const char* HIGHSCORE_FILE;
};

#endif
