#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>

// ==================== ��������� ====================
bool storageInit();

unsigned long loadHighscore();
void saveHighscore(unsigned long score);

#endif
