#pragma once

#include <stdio.h>
#include <string.h>
#include <windows.h>

#pragma comment(lib, "winmm.lib")

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 600

// ============================================================
// GLUT FONT COMPATIBILITY
// ============================================================
// This project uses an older GLUT header where
// GLUT_BITMAP_TIMES_ROMAN_18 is not defined.  Map it to the
// available 18-pixel Helvetica bitmap font so existing code in
// iMain.cpp does not need to be changed.
#ifndef GLUT_BITMAP_TIMES_ROMAN_18
#define GLUT_BITMAP_TIMES_ROMAN_18 GLUT_BITMAP_HELVETICA_18
#endif

// Levels 1 and 2 each use 3 background segments side by side.
#define LEVEL1_WIDTH  (SCREEN_WIDTH * 3)

// Level 3 contains four serial background segments.
#define LEVEL3_WIDTH  (SCREEN_WIDTH * 4)

// Level 4 contains four serial background segments.
#define LEVEL4_WIDTH  (SCREEN_WIDTH * 4)

enum GameState
{
    INTERFACE,
    MENU,
    NAME_INPUT,
    LEVEL_SELECT,
    GAME_MODE,
    SETTINGS,
    CONTROLS,
    OPTIONS,
    HIGH_SCORE,
    CREDITS,

    PLAYING,
    LEVEL2_PLAYING,
    LEVEL3_PLAYING,
    LEVEL4_PLAYING,
    PAUSED,
    GAME_OVER,
    LEVEL_COMPLETE,

    STORY_INTRO_STATE,
    STORY_LEVEL1_STATE,
    STORY_ENDING_STATE,

    STORY_LEVEL2_START_STATE,
    STORY_LEVEL2_END_STATE,
    STORY_LEVEL3_START_STATE,
    STORY_LEVEL3_END_STATE,
    STORY_LEVEL4_START_STATE,
    STORY_LEVEL4_END_STATE
};

struct Button
{
    int x;
    int y;
    int width;
    int height;
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

// Music state
extern GameState lastMusicState;

// Main menu
extern Button startButton;
extern Button levelButton;
extern Button optionsButton;

// Options menu
extern Button loadGameButton;
extern Button highScoreButton;
extern Button creditsButton;

// Level select
extern Button level1Button;
extern Button level2Button;
extern Button level3Button;
extern Button level4Button;

// Game mode
extern Button modeButton;
extern Button storyModeButton;
extern Button missionModeButton;

// Settings / controls
extern Button settingsButton;
extern Button controlsButton;
extern Button soundButton;
extern Button musicButton;

// General / navigation
extern Button exitButton;
extern Button backButton;

// Pause menu
extern Button resumeButton;
extern Button pauseSettingsButton;
extern Button mainMenuButton;
extern Button restartButton;
extern Button backToMenuButton;

// Other UI buttons
extern Button viewOptionsButton;
extern Button nextLevelButton;
extern Button nameConfirmButton;
