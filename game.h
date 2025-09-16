#ifndef GAME_H
#define GAME_H

#include <Arduino.h>
#include "entities.h"

enum GameState {
	STATE_LOGO,
	STATE_MENU,
	STATE_PLAY,
	STATE_GAME_OVER
};

class GameManager {
public:
	void init();
	void update();
	void changeState(GameState newState);

	GameState getCurrentState() { return currentState; }
	int getScore() { return score; }
	void setScore(int newScore) { score = newScore; }

private:
	GameState currentState;
	int score;
	int highScore;
	unsigned long stateStartTime;
};

#endif
