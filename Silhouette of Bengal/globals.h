#pragma once
#include <stdio.h>
#include <string.h>
#include <windows.h>      // ? ????
#pragma comment(lib, "winmm.lib")  // ? ????

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 600

// Levels 1 and 2 each use 3 background segments side by side.
#define LEVEL1_WIDTH  (SCREEN_WIDTH * 3)
// Level 3 contains four serial background segments.
#define LEVEL3_WIDTH  (SCREEN_WIDTH * 4)
#define LEVEL4_WIDTH  (SCREEN_WIDTH * 4)

enum GameState
{
	INTERFACE, MENU, LEVEL_SELECT, GAME_MODE, SETTINGS, CONTROLS,
	PLAYING, LEVEL2_PLAYING, LEVEL3_PLAYING, LEVEL4_PLAYING, PAUSED, GAME_OVER, LEVEL_COMPLETE,
	STORY_INTRO_STATE, STORY_LEVEL1_STATE, STORY_ENDING_STATE,
	STORY_LEVEL2_START_STATE, STORY_LEVEL2_END_STATE,
	STORY_LEVEL3_START_STATE, STORY_LEVEL3_END_STATE
};

struct Button
{
	int x, y, width, height;
};

extern GameState currentState;
extern int forestImage;
extern int interfaceImage;
extern int menuBgImage;
extern int gameModeImage;
extern int settingsBgImage;
extern int controlsBgImage;
extern int hoverOffset[10];
extern int selectedMenu;
extern int health;
extern int score;
extern int level;
extern bool soundOn;
extern bool musicOn;

// Music state    ? ????
extern GameState lastMusicState;

extern Button startButton;
extern Button levelButton;
extern Button level1Button;
extern Button level2Button;
extern Button level3Button;
extern Button level4Button;
extern Button modeButton;
extern Button settingsButton;
extern Button exitButton;
extern Button storyModeButton;
extern Button missionModeButton;
extern Button backButton;
extern Button soundButton;
extern Button musicButton;
extern Button controlsButton;
extern Button resumeButton;
extern Button pauseSettingsButton;
extern Button mainMenuButton;
extern Button restartButton;
extern Button backToMenuButton;
extern Button viewOptionsButton;
extern Button nextLevelButton;
