#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>

// ==================== Èםעונפויס ====================
bool storageInit();

unsigned long loadHighscore();
void saveHighscore(unsigned long score);

class StorageManager {
public:
	bool initSD();
	int readHighScore();
	void writeHighScore(int score);
	File openSprite(const char* filename);
	File openAudio(const char* filename);
};

#endif
