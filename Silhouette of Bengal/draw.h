#pragma once
#include "globals.h"

void drawBackground();
void drawTitle();
void drawButton(Button b, char text[],
	bool selected, int animOffset = 0);
void drawMainMenu();
void drawLevelSelect();
void drawGameMode();
void drawSettings();
void drawControls();
void drawHealthBar();
void drawHUD();
void drawLevel1();
void drawPauseMenu();
void drawGameOver();
void drawLevelComplete();