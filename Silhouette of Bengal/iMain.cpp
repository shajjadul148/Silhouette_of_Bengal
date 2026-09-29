#include "iGraphics.h"
#include "globals.h"
#include "draw.h"
#include "input.h"
#include "story.h"
#include "enemy.h"
#include "background.h"
#include "hero.h"
#include "Obstacle.h"
#include "Level2.h"
#include "Level3.h"
#include "Level4.h"
#include "Level3Obstacle.h"
#include "Level4Obstacle.h"
#include "StairMechanics.h"
#include "hero2.h"
#include "hero3.h"
#include "hero4.h"
#include "Obstacle2.h"
#include "enemy2.h"
#include "enemy3.h"
#include "collision3.h"
#include "enemy4.h"
#include "collision4.h"
#include "SaveSystem.h"

/* =========================================================
GLOBAL DEFINITIONS
========================================================= */
GameState currentState = INTERFACE;
GameState lastMusicState = INTERFACE;
GameState pausedFromState = PLAYING;
int forestImage, interfaceImage, menuBgImage;
int gameModeImage, settingsBgImage, controlsBgImage;
int hoverOffset[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int interfaceBlinkFrame = 0;
int selectedMenu = 0, health = 100, score = 0, level = 1;
bool soundOn = true, musicOn = true;

Button startButton = { 390, 375, 240, 55 };
Button levelButton = { 390, 305, 240, 55 }; /* kept for compatibility */
Button optionsButton = { 390, 305, 240, 55 };
Button loadGameButton = { 390, 350, 240, 55 };
Button highScoreButton = { 390, 280, 240, 55 };
Button creditsButton = { 390, 210, 240, 55 };
Button level1Button = { 390, 400, 240, 55 };
Button level2Button = { 390, 330, 240, 55 };
Button level3Button = { 390, 260, 240, 55 };
Button level4Button = { 390, 190, 240, 55 };
Button modeButton = { 390, 315, 240, 55 };
Button settingsButton = { 390, 235, 240, 55 };
Button exitButton = { 390, 165, 240, 55 };
Button storyModeButton = { 360, 330, 300, 60 };
Button missionModeButton = { 360, 250, 300, 60 };
Button backButton = { 420, 100, 180, 50 };
Button soundButton = { 430, 350, 200, 55 };
Button musicButton = { 430, 280, 200, 55 };
Button controlsButton = { 430, 210, 200, 55 };
Button resumeButton = { 390, 350, 240, 55 };
Button pauseSettingsButton = { 390, 275, 240, 55 };
Button mainMenuButton = { 390, 200, 240, 55 };
Button restartButton = { 390, 280, 240, 55 };
Button backToMenuButton = { 10, 505, 100, 25 };
Button viewOptionsButton = { 412, 35, 200, 55 };
Button nextLevelButton = { 390, 280, 240, 55 };
Button nameConfirmButton = { 390, 230, 240, 55 };

/* where to go after the player confirms a name */
GameState nameNextState = MENU;


/* =========================================================
DRAW FUNCTIONS
========================================================= */
void drawBackground()
{
	iSetColor(20, 25, 45);
	iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	iSetColor(55, 20, 25);
	iFilledRectangle(0, 380, SCREEN_WIDTH, 220);
	iSetColor(30, 25, 20);
	iFilledRectangle(0, 0, SCREEN_WIDTH, 90);
	iSetColor(120, 80, 45);
	iFilledRectangle(0, 90, SCREEN_WIDTH, 5);
}

void drawButton(Button b, char text[], bool selected, int animOffset);

void drawImageWithAlpha(int x, int y, int width, int height,
	unsigned int image, float alpha)
{
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor4f(1.0f, 1.0f, 1.0f, alpha);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);  glVertex2f(x, y);
	glTexCoord2f(1, 0);  glVertex2f(x + width, y);
	glTexCoord2f(1, -1); glVertex2f(x + width, y + height);
	glTexCoord2f(0, -1); glVertex2f(x, y + height);
	glEnd();

	glColor4f(1, 1, 1, 1);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}

void drawSoftImage(unsigned int image)
{
	/* A small offset blend gives every interface background
	a subtle soft-focus effect without needing another image file. */
	iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, image);
	int offsets[3] = { -2, 0, 2 };

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (offsets[i] != 0 || offsets[j] != 0)
			{
				drawImageWithAlpha(offsets[i], offsets[j],
					SCREEN_WIDTH, SCREEN_HEIGHT, image, 0.035f);
			}
		}
	}
}

void drawBlurredInterface()
{
	drawSoftImage(interfaceImage);

	int offsets[3] = { -4, 0, 4 };
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (offsets[i] != 0 || offsets[j] != 0)
			{
				drawImageWithAlpha(offsets[i], offsets[j],
					SCREEN_WIDTH, SCREEN_HEIGHT, interfaceImage, 0.055f);
			}
		}
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.0f, 0.0f, 0.0f, 0.28f);
	glBegin(GL_QUADS);
	glVertex2f(0, 0);
	glVertex2f(SCREEN_WIDTH, 0);
	glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT);
	glVertex2f(0, SCREEN_HEIGHT);
	glEnd();
	glColor4f(1, 1, 1, 1);
	glDisable(GL_BLEND);
}

void drawInterface()
{
	drawSoftImage(interfaceImage);

	if (interfaceBlinkFrame < 30)
	{
		iSetColor(238, 224, 184);
		iText(440, 55, "PRESS ENTER", GLUT_BITMAP_HELVETICA_18);
	}
}

void drawPurpleTitleBox(int x, int y, int width, int height)
{
	iSetColor(177, 108, 222);
	iFilledRectangle(x - 4, y - 4, width + 8, height + 8);
	iSetColor(71, 30, 108);
	iFilledRectangle(x, y, width, height);
	iSetColor(129, 68, 174);
	iFilledRectangle(x + 2, y + 2, width - 4, height - 4);
	iSetColor(231, 207, 255);
	iRectangle(x, y, width, height);
}

void drawTitle()
{
	int tx = 245, ty = 465, tw = 534, th = 60;
	drawPurpleTitleBox(tx, ty, tw, th);
	iSetColor(255, 255, 255);
	iText(tx + 89, ty + 22, "SILHOUETTE OF BENGAL",
		GLUT_BITMAP_TIMES_ROMAN_24);
}

void drawButton(Button b, char text[],
	bool selected, int animOffset)
{
	int drawX = b.x - animOffset;
	if (animOffset > 0 || selected)
	{
		iSetColor(185, 111, 229);
		iFilledRectangle(drawX - 4, b.y - 4,
			b.width + 8, b.height + 8);
	}

	iSetColor(52, 21, 82);
	iFilledRectangle(drawX, b.y, b.width, b.height);
	iSetColor(91, 43, 130);
	iFilledRectangle(drawX + 2, b.y + 2, b.width - 4, b.height - 4);
	iSetColor(148, 83, 192);
	iFilledRectangle(drawX + 2, b.y + b.height - 7, b.width - 4, 5);
	iSetColor(234, 213, 255);
	iRectangle(drawX, b.y, b.width, b.height);
	iSetColor(255, 255, 255);
	iText(drawX + 22, b.y + 20, text,
		GLUT_BITMAP_HELVETICA_18);
}

void drawOptions()
{
	drawBlurredInterface();
	drawPurpleTitleBox(382, 460, 260, 60);
	iSetColor(255, 255, 255);
	iText(425, 482, "OPTIONS", GLUT_BITMAP_TIMES_ROMAN_24);

	drawButton(loadGameButton, "LOAD GAME", false, hoverOffset[0]);
	drawButton(highScoreButton, "HIGH SCORE", false, hoverOffset[1]);
	drawButton(creditsButton, "CREDITS", false, hoverOffset[2]);
	drawButton(backButton, "BACK", false, hoverOffset[3]);
}

void drawHighScore()
{
	drawBlurredInterface();
	drawPurpleTitleBox(350, 460, 325, 60);
	iSetColor(255, 255, 255);
	iText(410, 482, "HIGH SCORE", GLUT_BITMAP_TIMES_ROMAN_24);

	hsDraw(205, 405);
	drawButton(backButton, "BACK", false, hoverOffset[0]);
}

void drawCredits()
{
	drawBlurredInterface();
	drawPurpleTitleBox(390, 460, 245, 60);
	iSetColor(255, 255, 255);
	iText(445, 482, "CREDITS", GLUT_BITMAP_TIMES_ROMAN_24);

	// High-contrast credit text: dark shadow + bright foreground
	// so the names and IDs remain clearly readable over the background.

	// 1st member
	iSetColor(20, 10, 35);
	iText(337, 398, "1. Mehedi Hasan Ibne Rais", GLUT_BITMAP_HELVETICA_18);
	iSetColor(255, 255, 255);
	iText(335, 400, "1. Mehedi Hasan Ibne Rais", GLUT_BITMAP_HELVETICA_18);

	iSetColor(20, 10, 35);
	iText(337, 363, "ID: 00725105101151", GLUT_BITMAP_HELVETICA_18);
	iSetColor(255, 235, 80);
	iText(335, 365, "ID: 00725105101151", GLUT_BITMAP_HELVETICA_18);

	// 2nd member
	iSetColor(20, 10, 35);
	iText(337, 313, "2. Md. Shajjadul Islam", GLUT_BITMAP_HELVETICA_18);
	iSetColor(255, 255, 255);
	iText(335, 315, "2. Md. Shajjadul Islam", GLUT_BITMAP_HELVETICA_18);

	iSetColor(20, 10, 35);
	iText(337, 278, "ID: 00725105101148", GLUT_BITMAP_HELVETICA_18);
	iSetColor(255, 235, 80);
	iText(335, 280, "ID: 00725105101148", GLUT_BITMAP_HELVETICA_18);

	// 3rd member
	iSetColor(20, 10, 35);
	iText(337, 228, "3. Md. Abed Bin Rahman", GLUT_BITMAP_HELVETICA_18);
	iSetColor(255, 255, 255);
	iText(335, 230, "3. Md. Abed Bin Rahman", GLUT_BITMAP_HELVETICA_18);

	iSetColor(20, 10, 35);
	iText(337, 193, "ID: 00725105101172", GLUT_BITMAP_HELVETICA_18);
	iSetColor(255, 235, 80);
	iText(335, 195, "ID: 00725105101172", GLUT_BITMAP_HELVETICA_18);

	drawButton(backButton, "BACK", false, hoverOffset[0]);
}

void loadSavedGame()
{
	int savedLevel = saveGetContinueLevel();
	if (savedLevel < 1 || savedLevel > 4) savedLevel = 1;

	health = 100;
	score = 0;
	level = savedLevel;

	if (savedLevel == 1)
	{
		resetEnemies();
		resetHero();
		resetObstacles();
		currentState = PLAYING;
	}
	else if (savedLevel == 2)
	{
		resetHero2();
		resetEnemies2();
		resetObstacles2();
		currentState = LEVEL2_PLAYING;
	}
	else if (savedLevel == 3)
	{
		resetHero3();
		resetEnemies3();
		currentState = LEVEL3_PLAYING;
	}
	else
	{
		resetHero4();
		resetEnemies4();
		currentState = LEVEL4_PLAYING;
	}
}

void drawMainMenu()
{
	drawBlurredInterface();
	drawTitle();
	drawButton(startButton, "START GAME",
		selectedMenu == 0, hoverOffset[0]);
	drawButton(optionsButton, "OPTIONS",
		selectedMenu == 1, hoverOffset[1]);
	drawButton(settingsButton, "SETTINGS",
		selectedMenu == 2, hoverOffset[2]);
	drawButton(exitButton, "EXIT",
		selectedMenu == 3, hoverOffset[3]);
	iSetColor(255, 255, 255);
	iText(355, 45, "A 2D STORY-DRIVEN ACTION GAME",
		GLUT_BITMAP_HELVETICA_12);
}


/* =========================================================
PLAYER NAME INPUT
========================================================= */
void startNewGame()
{
	health = 100; score = 0; level = 1;
	resetEnemies();
	resetHero();
	resetObstacles();
	saveOnGameStart();
	storyStart(STORY_INTRO);
}

/* open the name screen; the last used name is already filled in */
bool nameLoadMode = false;   /* true = LOAD GAME name screen, false = START GAME */

/* START GAME: always an EMPTY box, the player types a name every time */
void beginNameInput(GameState next)
{
	nameNextState = next;
	nameLoadMode = false;
	nameInputReset();
	saveClearMessage();
	currentState = NAME_INPUT;
}

/* LOAD GAME: type the name you saved with; the last player is pre-filled */
void beginLoadInput()
{
	nameNextState = PLAYING;
	nameLoadMode = true;
	if (saveHasPlayerName()) nameInputSet(saveGetPlayerName());
	else nameInputReset();
	saveClearMessage();
	currentState = NAME_INPUT;
}

void confirmPlayerName()
{
	if (nameLoadMode)
	{
		/* resume that player's saved level; unknown name -> stay here and show a message */
		if (saveLoadPlayer(nameInputGet())) loadSavedGame();
		return;
	}
	saveSetPlayerName(nameInputGet());
	if (nameNextState == LEVEL_SELECT) currentState = LEVEL_SELECT;
	else startNewGame();
}

void drawNameInput()
{
	drawBlurredInterface();
	drawPurpleTitleBox(312, 450, 400, 60);
	iSetColor(255, 255, 255);
	if (nameLoadMode)
		iText(345, 472, "LOAD GAME - YOUR NAME", GLUT_BITMAP_TIMES_ROMAN_24);
	else
		iText(370, 472, "ENTER YOUR NAME", GLUT_BITMAP_TIMES_ROMAN_24);

	/* text box */
	iSetColor(52, 21, 82);
	iFilledRectangle(330, 335, 364, 60);
	iSetColor(234, 213, 255);
	iRectangle(330, 335, 364, 60);
	iSetColor(255, 255, 255);
	char shown[64];
	bool blink = (interfaceBlinkFrame < 30);
	sprintf_s(shown, "%s%s", nameInputGet(), blink ? "_" : "");
	iText(350, 358, shown, GLUT_BITMAP_TIMES_ROMAN_24);

	iSetColor(190, 170, 130);
	iText(345, 300, "Type your name (max 15 letters), then press ENTER",
		GLUT_BITMAP_HELVETICA_12);

	if (saveGetMessage()[0] != '\0')
	{
		iSetColor(255, 120, 120);
		iText(345, 320, (char*)saveGetMessage(), GLUT_BITMAP_HELVETICA_18);
	}
	if (nameLoadMode)
		saveDrawPlayerList(715, 395);

	drawButton(nameConfirmButton, "CONFIRM", false, 0);
	drawButton(backButton, "BACK", false, 0);
}

void drawLevelSelect()
{
	drawBlurredInterface();
	int tx = 362, ty = 460, tw = 300, th = 60;
	drawPurpleTitleBox(tx, ty, tw, th);
	iSetColor(255, 255, 255);
	iText(tx + 55, ty + 22, "SELECT LEVEL",
		GLUT_BITMAP_TIMES_ROMAN_24);
	drawButton(level1Button, "LEVEL 1", false, hoverOffset[0]);
	drawButton(level2Button, "LEVEL 2", false, hoverOffset[1]);
	drawButton(level3Button, "LEVEL 3", false, hoverOffset[2]);
	drawButton(level4Button, "LEVEL 4", false, hoverOffset[3]);
	drawButton(backButton, "BACK", false, hoverOffset[4]);
}

void drawGameMode()
{
	drawSoftImage(gameModeImage);
	int tx = 362, ty = 460, tw = 300, th = 60;
	drawPurpleTitleBox(tx, ty, tw, th);
	iSetColor(255, 255, 255);
	iText(tx + 60, ty + 22, "GAME MODE",
		GLUT_BITMAP_TIMES_ROMAN_24);
	drawButton(storyModeButton, "STORY MODE",
		false, hoverOffset[0]);
	drawButton(missionModeButton, "MISSION MODE",
		false, hoverOffset[1]);
	drawButton(backButton, "BACK",
		false, hoverOffset[2]);
	iSetColor(190, 170, 130);
	iText(360, 180, "Choose how you want to play.",
		GLUT_BITMAP_HELVETICA_18);
}

void drawSettings()
{
	drawBlurredInterface();
	int tx = 412, ty = 460, tw = 200, th = 60;
	drawPurpleTitleBox(tx, ty, tw, th);
	iSetColor(255, 255, 255);
	iText(tx + 30, ty + 22, "SETTINGS",
		GLUT_BITMAP_TIMES_ROMAN_24);
	char soundText[20];
	if (soundOn) strcpy_s(soundText, "SOUND : ON");
	else strcpy_s(soundText, "SOUND : OFF");
	char musicText[20];
	if (musicOn) strcpy_s(musicText, "MUSIC : ON");
	else strcpy_s(musicText, "MUSIC : OFF");
	drawButton(soundButton, soundText,
		false, hoverOffset[3]);
	drawButton(musicButton, musicText,
		false, hoverOffset[4]);
	drawButton(controlsButton, "CONTROLS",
		false, hoverOffset[5]);
	drawButton(backButton, "BACK",
		false, hoverOffset[6]);
}

void drawControls()
{
	drawBlurredInterface();
	int tx = 412, ty = 460, tw = 200, th = 60;
	drawPurpleTitleBox(tx, ty, tw, th);
	iSetColor(255, 255, 255);
	iText(tx + 30, ty + 22, "CONTROLS",
		GLUT_BITMAP_TIMES_ROMAN_24);
	iSetColor(40, 30, 20);
	iFilledRectangle(310, 180, 400, 240);
	iSetColor(210, 170, 90);
	iRectangle(310, 180, 400, 240);
	iSetColor(230, 220, 190);
	iText(330, 390, "A / LEFT    :  Move Left",
		GLUT_BITMAP_HELVETICA_18);
	iText(330, 350, "D / RIGHT   :  Move Right",
		GLUT_BITMAP_HELVETICA_18);
	iText(330, 310, "W / UP      :  Move Up",
		GLUT_BITMAP_HELVETICA_18);
	iText(330, 270, "J           :  Attack",
		GLUT_BITMAP_HELVETICA_18);
	iText(330, 230, "P           :  Pause",
		GLUT_BITMAP_HELVETICA_18);
	drawButton(backButton, "BACK", false, hoverOffset[7]);
}

void drawHealthBar()
{
	int bx = 50, by = 535, bw = 250, bh = 25;
	iSetColor(230, 210, 170);
	iRectangle(bx, by, bw, bh);
	iSetColor(70, 30, 30);
	iFilledRectangle(bx + 3, by + 3, bw - 6, bh - 6);
	iSetColor(150, 40, 40);
	iFilledRectangle(bx + 3, by + 3,
		(bw - 6) * health / 100, bh - 6);
	iSetColor(255, 255, 255);
	iText(bx, by + 35, "HEALTH", GLUT_BITMAP_HELVETICA_12);
}

void drawHUD()
{
	iSetColor(20, 20, 20);
	iFilledRectangle(0, 500, SCREEN_WIDTH, 100);
	drawHealthBar();
	iSetColor(230, 210, 170);
	iText(450, 545, "SCORE:", GLUT_BITMAP_HELVETICA_18);
	char sc[50];
	sprintf_s(sc, "%d", score);
	iText(530, 545, sc, GLUT_BITMAP_HELVETICA_18);
	iText(800, 545, "LEVEL:", GLUT_BITMAP_HELVETICA_18);
	char lv[20];
	sprintf_s(lv, "%d", level);
	iText(875, 545, lv, GLUT_BITMAP_HELVETICA_18);
}

void drawLevel1()
{
	drawLevel1Background();
	drawObstacles();
	drawPurpleTitleBox(315, 440, 394, 38);
	iSetColor(255, 255, 255);
	iText(350, 452, "MISSION 1 : THE HIDDEN TRAIL",
		GLUT_BITMAP_HELVETICA_18);
	drawEnemies();
	drawHero();
	drawHUD();
	iSetColor(74, 28, 110);
	iFilledRectangle(10, 505, 100, 25);
	iSetColor(181, 112, 228);
	iRectangle(10, 505, 100, 25);
	iSetColor(255, 255, 255);
	iText(18, 512, "< BACK", GLUT_BITMAP_HELVETICA_12);
	backToMenuButton.x = 10;
	backToMenuButton.y = 505;
	backToMenuButton.width = 100;
	backToMenuButton.height = 25;
}

void drawLevel2()
{
	drawLevel2Background();
	drawObstacles2();
	drawPurpleTitleBox(325, 440, 394, 38);
	iSetColor(255, 255, 255);
	iText(345, 452, "MISSION 2 : THE VILLAGE OUTPOST",
		GLUT_BITMAP_HELVETICA_18);
	drawEnemies2();
	drawHero2();
	drawHUD();
	iSetColor(74, 28, 110);
	iFilledRectangle(10, 505, 100, 25);
	iSetColor(181, 112, 228);
	iRectangle(10, 505, 100, 25);
	iSetColor(255, 255, 255);
	iText(18, 512, "< BACK", GLUT_BITMAP_HELVETICA_12);
	backToMenuButton.x = 10;
	backToMenuButton.y = 505;
	backToMenuButton.width = 100;
	backToMenuButton.height = 25;
}

void drawLevel3()
{
	drawLevel3Background();
	/* Draw actors first so the obstacle is a true foreground cover. */
	drawEnemies3();
	drawHero3();
	drawLevel3Obstacles();
	drawPurpleTitleBox(335, 440, 354, 38);
	iSetColor(255, 255, 255);
	iText(362, 452, "MISSION 3 : THE FINAL PATH", GLUT_BITMAP_HELVETICA_18);
	drawHUD();
	iSetColor(74, 28, 110); iFilledRectangle(10, 505, 100, 25);
	iSetColor(181, 112, 228); iRectangle(10, 505, 100, 25);
	iSetColor(255, 255, 255); iText(18, 512, "< BACK", GLUT_BITMAP_HELVETICA_12);
	backToMenuButton.x = 10; backToMenuButton.y = 505; backToMenuButton.width = 100; backToMenuButton.height = 25;
}

void drawLevel4()
{
	drawLevel4Background();
	/* Draw actors before obstacles so Hero/Enemy are hidden behind cover. */
	drawEnemies4();
	drawHero4();
	drawLevel4Obstacles();
	drawPurpleTitleBox(335, 440, 354, 38);
	iSetColor(255,255,255);
	iText(362,452,"MISSION 4 : THE FINAL FORT",GLUT_BITMAP_HELVETICA_18);
	drawHUD();
	iSetColor(74,28,110); iFilledRectangle(10,505,100,25);
	iSetColor(181,112,228); iRectangle(10,505,100,25);
	iSetColor(255,255,255); iText(18,512,"< BACK",GLUT_BITMAP_HELVETICA_12);
	backToMenuButton.x=10; backToMenuButton.y=505; backToMenuButton.width=100; backToMenuButton.height=25;
}

/* J-key / click attack for Level 3, with cover blocking. */
inline void playerAttackEnemies3Cover()
{
	for (int i = 0; i < MAX_ACTIVE_ENEMIES3; ++i)
	{
		Enemy3 &e = enemyManager3.activeEnemies[i];
		if (!e.alive || !e.playerInRange(PLAYER3_ATTACK_RANGE)) continue;
		bool blocked = isLevel3CoverBetween(playerX, playerY, e.x, e.y);
		if (!blocked) e.takeDamage(PLAYER3_ATTACK_DAMAGE);
	}
}

/* J-key / click attack for Level 4, with cover blocking. Fully
   independent from Level 3's version above -- no shared enemy logic. */
inline void playerAttackEnemies4Cover()
{
	for (int i = 0; i < MAX_ACTIVE_ENEMIES4; ++i)
	{
		Enemy4 &e = enemyManager4.activeEnemies[i];
		if (!e.alive || !e.playerInRange(PLAYER4_ATTACK_RANGE)) continue;
		/* Bosses are shielded by their escorts -- can't be hurt until
		   all of them are dead (matches their passive state in
		   Enemy4::update()). This was previously only enforced in
		   EnemyManager4::playerAttack(), which this cover-checking
		   path doesn't go through, so the shield had no effect on
		   the actual attack input -- fixed here for both bosses. */
		if (e.type == TYPE4_BOSS && enemyManager4.waveEscortsDefeated < BOSS4_ESCORT_COUNT) continue;
		if (e.type == TYPE4_BOSS2 && enemyManager4.waveEscorts2Defeated < BOSS2_ESCORT_COUNT) continue;
		bool blocked = isLevel4CoverBetween(playerX, playerY, e.x, e.y);
		if (!blocked) e.takeDamage(PLAYER4_ATTACK_DAMAGE);
	}
}

/* Dispatches to the right level's attack function. */
inline void playerAttackEnemies34()
{
	if (currentState == LEVEL3_PLAYING)
		playerAttackEnemies3Cover();
	else if (currentState == LEVEL4_PLAYING)
		playerAttackEnemies4Cover();
}

void drawPauseMenu()
{
	drawBlurredInterface();
	drawPurpleTitleBox(402, 450, 220, 55);
	iSetColor(255, 255, 255);
	iText(455, 468, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);
	drawButton(resumeButton, "RESUME", false, 0);
	drawButton(pauseSettingsButton, "SETTINGS", false, 0);
	drawButton(mainMenuButton, "MAIN MENU", false, 0);
}

void drawGameOver()
{
	drawBlurredInterface();
	drawPurpleTitleBox(390, 430, 244, 55);
	iSetColor(255, 255, 255);
	iText(420, 448, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
	iSetColor(255, 255, 255);
	iText(390, 390, "Shamsher Kazi has fallen.",
		GLUT_BITMAP_HELVETICA_18);
	char pn[40];
	sprintf_s(pn, "PLAYER : %s", saveGetPlayerName());
	iText(420, 365, pn, GLUT_BITMAP_HELVETICA_18);
	char fs[50];
	sprintf_s(fs, "FINAL SCORE : %d", score);
	iText(420, 335, fs, GLUT_BITMAP_HELVETICA_18);
	drawButton(restartButton, "RESTART", false, 0);
	drawButton(mainMenuButton, "MAIN MENU", false, 0);
}

void drawLevelComplete()
{
	drawBlurredInterface();
	drawPurpleTitleBox(350, 430, 324, 55);
	iSetColor(255, 255, 255);
	iText(378, 448, "LEVEL COMPLETE!",
		GLUT_BITMAP_TIMES_ROMAN_24);
	iSetColor(255, 255, 255);
	if (level == 1)
		iText(385, 395, "Mission 1 completed successfully.",
			GLUT_BITMAP_HELVETICA_18);
	else
		iText(385, 395, "Mission 2 completed successfully.",
			GLUT_BITMAP_HELVETICA_18);
	char sc[50];
	sprintf_s(sc, "SCORE : %d", score);
	iText(450, 345, sc, GLUT_BITMAP_HELVETICA_18);
	drawButton(nextLevelButton, "NEXT LEVEL", false, hoverOffset[0]);
	drawButton(mainMenuButton, "MAIN MENU", false, hoverOffset[1]);
}


/* =========================================================
INPUT FUNCTIONS
========================================================= */
bool isInside(Button b, int mx, int my)
{
	return (mx >= b.x && mx <= b.x + b.width &&
		my >= b.y && my <= b.y + b.height);
}

void iMouseMove(int mx, int my) {}

/* =========================================================
iPassiveMouseMove — শুধু HOVER animation
সমস্যা ছিল: Settings block এ hover এর বদলে
click code ছিল — এখন ঠিক করা হয়েছে
========================================================= */
void iPassiveMouseMove(int mx, int my)
{
	/* -------- INTERFACE -------- */
	if (currentState == INTERFACE)
	{
		hoverOffset[9] = 0;
	}

	/* -------- MENU -------- */
	else if (currentState == MENU)
	{
		if (isInside(startButton, mx, my))
			hoverOffset[0] = 10;
		else if (hoverOffset[0] > 0)
			hoverOffset[0] = 0;

		if (isInside(optionsButton, mx, my))
			hoverOffset[1] = 10;
		else if (hoverOffset[1] > 0)
			hoverOffset[1] = 0;

		if (isInside(settingsButton, mx, my))
			hoverOffset[2] = 10;
		else if (hoverOffset[2] > 0)
			hoverOffset[2] = 0;

		if (isInside(exitButton, mx, my))
			hoverOffset[3] = 10;
		else if (hoverOffset[3] > 0)
			hoverOffset[3] = 0;
	}

	/* -------- OPTIONS -------- */
	else if (currentState == OPTIONS)
	{
		if (isInside(loadGameButton, mx, my))
			hoverOffset[0] = 10;
		else if (hoverOffset[0] > 0)
			hoverOffset[0] = 0;

		if (isInside(highScoreButton, mx, my))
			hoverOffset[1] = 10;
		else if (hoverOffset[1] > 0)
			hoverOffset[1] = 0;

		if (isInside(creditsButton, mx, my))
			hoverOffset[2] = 10;
		else if (hoverOffset[2] > 0)
			hoverOffset[2] = 0;

		if (isInside(backButton, mx, my))
			hoverOffset[3] = 10;
		else if (hoverOffset[3] > 0)
			hoverOffset[3] = 0;
	}
	else if (currentState == HIGH_SCORE || currentState == CREDITS)
	{
		if (isInside(backButton, mx, my))
			hoverOffset[0] = 10;
		else if (hoverOffset[0] > 0)
			hoverOffset[0] = 0;
	}

	/* -------- LEVEL SELECT -------- */
	else if (currentState == LEVEL_SELECT)
	{
		if (isInside(level1Button, mx, my))
			hoverOffset[0] = 10;
		else if (hoverOffset[0] > 0)
			hoverOffset[0] = 0;

		if (isInside(level2Button, mx, my))
			hoverOffset[1] = 10;
		else if (hoverOffset[1] > 0)
			hoverOffset[1] = 0;

		if (isInside(level3Button, mx, my))
			hoverOffset[2] = 10;
		else if (hoverOffset[2] > 0)
			hoverOffset[2] = 0;

		if (isInside(level4Button, mx, my))
			hoverOffset[3] = 10;
		else if (hoverOffset[3] > 0)
			hoverOffset[3] = 0;

		if (isInside(backButton, mx, my))
			hoverOffset[4] = 10;
		else if (hoverOffset[4] > 0)
			hoverOffset[4] = 0;
	}

	/* -------- GAME MODE -------- */
	else if (currentState == GAME_MODE)
	{
		if (isInside(storyModeButton, mx, my))
			hoverOffset[0] = 10;
		else if (hoverOffset[0] > 0)
			hoverOffset[0] = 0;

		if (isInside(missionModeButton, mx, my))
			hoverOffset[1] = 10;
		else if (hoverOffset[1] > 0)
			hoverOffset[1] = 0;

		if (isInside(backButton, mx, my))
			hoverOffset[2] = 10;
		else if (hoverOffset[2] > 0)
			hoverOffset[2] = 0;
	}

	/* -------- SETTINGS -------- */
	/* আগে এখানে click code ছিল — এখন শুধু hover */
	else if (currentState == SETTINGS)
	{
		if (isInside(soundButton, mx, my))
			hoverOffset[3] = 10;
		else if (hoverOffset[3] > 0)
			hoverOffset[3] = 0;

		if (isInside(musicButton, mx, my))
			hoverOffset[4] = 10;
		else if (hoverOffset[4] > 0)
			hoverOffset[4] = 0;

		if (isInside(controlsButton, mx, my))
			hoverOffset[5] = 10;
		else if (hoverOffset[5] > 0)
			hoverOffset[5] = 0;

		if (isInside(backButton, mx, my))
			hoverOffset[6] = 10;
		else if (hoverOffset[6] > 0)
			hoverOffset[6] = 0;
	}

	/* -------- CONTROLS -------- */
	else if (currentState == CONTROLS)
	{
		if (isInside(backButton, mx, my))
			hoverOffset[7] = 10;
		else if (hoverOffset[7] > 0)
			hoverOffset[7] = 0;
	}

	/* -------- PLAYING -------- */
	else if (currentState == PLAYING || currentState == LEVEL2_PLAYING ||
		currentState == LEVEL3_PLAYING || currentState == LEVEL4_PLAYING)
	{
		if (isInside(backToMenuButton, mx, my))
			hoverOffset[8] = 10;
		else if (hoverOffset[8] > 0)
			hoverOffset[8] = 0;
	}

	/* -------- PAUSED -------- */
	else if (currentState == PAUSED)
	{
		if (isInside(resumeButton, mx, my))
			hoverOffset[0] = 10;
		else if (hoverOffset[0] > 0)
			hoverOffset[0] = 0;

		if (isInside(pauseSettingsButton, mx, my))
			hoverOffset[1] = 10;
		else if (hoverOffset[1] > 0)
			hoverOffset[1] = 0;

		if (isInside(mainMenuButton, mx, my))
			hoverOffset[2] = 10;
		else if (hoverOffset[2] > 0)
			hoverOffset[2] = 0;
	}

	/* -------- GAME OVER -------- */
	else if (currentState == GAME_OVER)
	{
		if (isInside(restartButton, mx, my))
			hoverOffset[0] = 10;
		else if (hoverOffset[0] > 0)
			hoverOffset[0] = 0;

		if (isInside(mainMenuButton, mx, my))
			hoverOffset[1] = 10;
		else if (hoverOffset[1] > 0)
			hoverOffset[1] = 0;
	}

	/* -------- LEVEL COMPLETE -------- */
	else if (currentState == LEVEL_COMPLETE)
	{
		if (isInside(nextLevelButton, mx, my))
			hoverOffset[0] = 10;
		else if (hoverOffset[0] > 0)
			hoverOffset[0] = 0;

		if (isInside(mainMenuButton, mx, my))
			hoverOffset[1] = 10;
		else if (hoverOffset[1] > 0)
			hoverOffset[1] = 0;
	}
}

/* =========================================================
iMouse — শুধু CLICK action
সমস্যা ছিল: Settings এ music toggle এ
PlaySound ছিল না — এখন ঠিক করা হয়েছে
========================================================= */
void iMouse(int button, int state, int mx, int my)
{
	if (button != GLUT_LEFT_BUTTON ||
		state != GLUT_DOWN) return;

	/* -------- INTERFACE -------- */
	if (currentState == INTERFACE)
	{
		return;
	}
	/* -------- PLAYING -------- */
	if (currentState == PLAYING)
	{
		if (isInside(backToMenuButton, mx, my))
		{
			saveCurrentLevel(level, score);   /* save this level only, no unlock */
			health = 100; score = 0; level = 1;
			currentState = MENU;
		}
		else
		{
			heroAttack();
			playerAttackEnemies();
		}
		return;
	}
	else if (currentState == LEVEL2_PLAYING)
	{
		if (isInside(backToMenuButton, mx, my))
		{
			saveCurrentLevel(level, score);   /* save this level only, no unlock */
			health = 100; score = 0; level = 1;
			currentState = MENU;
		}
		else
		{
			hero2Attack();
			playerAttackEnemies2();
		}
		return;
	}
	else if (currentState == LEVEL3_PLAYING)
	{
		if (isInside(backToMenuButton, mx, my))
		{
			saveCurrentLevel(level, score);   /* save this level only, no unlock */
			health = 100; score = 0; level = 1;
			currentState = MENU;
		}
		else
		{
			hero3Attack();
			playerAttackEnemies34();
		}
		return;
	}
	else if (currentState == LEVEL4_PLAYING)
	{
		if (isInside(backToMenuButton, mx, my))
		{
			saveCurrentLevel(level, score);   /* save this level only, no unlock */
			health = 100; score = 0; level = 1;
			currentState = MENU;
		}
		else
		{
			hero4Attack();
			playerAttackEnemies34();
		}
		return;
	}

	/* -------- NAME INPUT -------- */
	else if (currentState == NAME_INPUT)
	{
		if (isInside(nameConfirmButton, mx, my))
		{
			if (nameInputKey(13)) confirmPlayerName();
		}
		else if (isInside(backButton, mx, my))
			currentState = nameLoadMode ? OPTIONS : MENU;
	}

	/* -------- MENU -------- */
	else if (currentState == MENU)
	{
		if (isInside(startButton, mx, my))
			beginNameInput(PLAYING);
		else if (isInside(optionsButton, mx, my))
			currentState = OPTIONS;
		else if (isInside(settingsButton, mx, my))
			currentState = SETTINGS;
		else if (isInside(exitButton, mx, my))
			exit(0);
	}

	/* -------- OPTIONS -------- */
	else if (currentState == OPTIONS)
	{
		if (isInside(loadGameButton, mx, my))
		{
			beginLoadInput();
		}
		else if (isInside(highScoreButton, mx, my))
			currentState = HIGH_SCORE;
		else if (isInside(creditsButton, mx, my))
			currentState = CREDITS;
		else if (isInside(backButton, mx, my))
			currentState = MENU;
	}
	/* -------- HIGH SCORE -------- */
	else if (currentState == HIGH_SCORE)
	{
		if (isInside(backButton, mx, my))
			currentState = OPTIONS;
	}
	/* -------- CREDITS -------- */
	else if (currentState == CREDITS)
	{
		if (isInside(backButton, mx, my))
			currentState = OPTIONS;
	}

	/* -------- LEVEL SELECT -------- */
	else if (currentState == LEVEL_SELECT)
	{
		if (isInside(level1Button, mx, my))
		{
			health = 100; score = 0; level = 1;
			resetEnemies();
			resetHero();
			resetObstacles();
			currentState = PLAYING;
		}
		else if (isInside(level2Button, mx, my))
		{
			health = 100; score = 0; level = 2;
			resetHero2();
			resetEnemies2();
			resetObstacles2();
			currentState = LEVEL2_PLAYING;
		}
		else if (isInside(level3Button, mx, my))
		{
			health = 100; score = 0; level = 3;
			resetHero3();
			resetEnemies3();
			currentState = LEVEL3_PLAYING;
		}
		else if (isInside(level4Button, mx, my))
		{
			health = 100; score = 0; level = 4;
			resetHero4();
			resetEnemies4();
			currentState = LEVEL4_PLAYING;
		}
		else if (isInside(backButton, mx, my))
			currentState = MENU;
	}

	/* -------- GAME MODE -------- */
	else if (currentState == GAME_MODE)
	{
		if (isInside(storyModeButton, mx, my))
		{
			health = 100; score = 0; level = 1;
			resetEnemies(); resetHero(); resetObstacles();
			storyStart(STORY_INTRO);
		}
		else if (isInside(missionModeButton, mx, my))
		{
			health = 100; score = 0; level = 1;
			resetEnemies(); resetHero(); resetObstacles();
			storyStart(STORY_INTRO);
		}
		else if (isInside(backButton, mx, my))
			currentState = MENU;
	}

	/* -------- SETTINGS -------- */
	/* আগে music button এ PlaySound ছিল না — এখন আছে */
	else if (currentState == SETTINGS)
	{
		if (isInside(soundButton, mx, my))
		{
			soundOn = !soundOn;
			saveSettings();
		}
		else if (isInside(musicButton, mx, my))
		{
			musicOn = !musicOn;
			saveSettings();
			if (musicOn)
			{
				// যে state এ ছিল সেই music চালাও
				PlaySound(
					TEXT("Images\\interface.wav"),
					NULL,
					SND_FILENAME | SND_ASYNC | SND_LOOP
					);
			}
			else
			{
				PlaySound(NULL, NULL, SND_PURGE);
			}
		}
		else if (isInside(controlsButton, mx, my))
			currentState = CONTROLS;
		else if (isInside(backButton, mx, my))
			currentState = MENU;
	}

	/* -------- CONTROLS -------- */
	else if (currentState == CONTROLS)
	{
		if (isInside(backButton, mx, my))
			currentState = SETTINGS;
	}

	/* -------- PAUSED -------- */
	else if (currentState == PAUSED)
	{
		if (isInside(resumeButton, mx, my))
			currentState = pausedFromState;
		else if (isInside(pauseSettingsButton, mx, my))
			currentState = SETTINGS;
		else if (isInside(mainMenuButton, mx, my))
			currentState = MENU;
	}

	/* -------- GAME OVER -------- */
	else if (currentState == GAME_OVER)
	{
		if (isInside(restartButton, mx, my))
		{
			health = 100; score = 0; level = 1;
			resetEnemies();
			resetHero();
			resetObstacles();
			currentState = PLAYING;
		}
		else if (isInside(mainMenuButton, mx, my))
			currentState = MENU;
	}

	/* -------- LEVEL COMPLETE -------- */
	else if (currentState == LEVEL_COMPLETE)
	{
		if (isInside(nextLevelButton, mx, my))
		{
			if (level == 1)
			{
				health = 100;
				score = 0;
				level = 2;
				resetHero2();
				resetEnemies2();
				resetObstacles2();
				storyStart(STORY_LEVEL2_START);
			}
			else if (level == 2)
			{
				health = 100; score = 0; level = 3;
				resetHero3(); resetEnemies3();
				storyStart(STORY_LEVEL3_START);
			}
			else if (level == 3)
			{
				health = 100; level = 4;
				resetHero4(); resetEnemies4();
				storyStart(STORY_LEVEL4_START);
			}
			else if (level == 4)
			{
				currentState = MENU;
			}
		}
		else if (isInside(mainMenuButton, mx, my))
			currentState = MENU;
	}
}

void iKeyboard(unsigned char key)
{
	if (currentState == NAME_INPUT)
	{
		if (key == 27)
			currentState = nameLoadMode ? OPTIONS : MENU;
		else
		{
			if (key != 13) saveClearMessage();
			if (nameInputKey(key)) confirmPlayerName();
		}
	}
	else if (currentState == INTERFACE)
	{
		if (key == 13 || key == '\r' || key == '\n')
			currentState = MENU;
	}
	else if (currentState == MENU)
	{
		if (key == 'w' || key == 'W')
		{
			selectedMenu--;
			if (selectedMenu < 0) selectedMenu = 3;
		}
		else if (key == 's' || key == 'S')
		{
			selectedMenu++;
			if (selectedMenu > 3) selectedMenu = 0;
		}
		else if (key == 13)
		{
			if (selectedMenu == 0)
				beginNameInput(PLAYING);
			else if (selectedMenu == 1)
				currentState = OPTIONS;
			else if (selectedMenu == 2)
				currentState = SETTINGS;
			else if (selectedMenu == 3)
				exit(0);
		}
	}
	else if (currentState == OPTIONS)
	{
		if (key == 27)
			currentState = MENU;
		else if (key == '1')
			beginLoadInput();
		else if (key == '2')
			currentState = HIGH_SCORE;
		else if (key == '3')
			currentState = CREDITS;
	}
	else if (currentState == HIGH_SCORE)
	{
		if (key == 27) currentState = OPTIONS;
	}
	else if (currentState == CREDITS)
	{
		if (key == 27) currentState = OPTIONS;
	}
	else if (currentState == LEVEL_SELECT)
	{
		if (key == 27)
			currentState = MENU;
	}
	else if (currentState == STORY_INTRO_STATE ||
		currentState == STORY_LEVEL1_STATE ||
		currentState == STORY_ENDING_STATE ||
		currentState == STORY_LEVEL2_START_STATE ||
		currentState == STORY_LEVEL2_END_STATE ||
		currentState == STORY_LEVEL3_START_STATE ||
		currentState == STORY_LEVEL3_END_STATE ||
		currentState == STORY_LEVEL4_START_STATE ||
		currentState == STORY_LEVEL4_END_STATE)
	{
		if (key == 13 || key == '\r' || key == '\n')
			storyContinue();
	}
	else if (currentState == PLAYING)
	{
		if (key == 'p' || key == 'P')
		{
			pausedFromState = PLAYING;
			currentState = PAUSED;
		}
		else if (key == 'j' || key == 'J')
		{
			heroAttack();
			playerAttackEnemies();
		}
	}
	else if (currentState == LEVEL2_PLAYING)
	{
		if (key == 'p' || key == 'P')
		{
			pausedFromState = LEVEL2_PLAYING;
			currentState = PAUSED;
		}
		else if (key == 'j' || key == 'J')
		{
			hero2Attack();
			playerAttackEnemies2();
		}
		else if (key == 27)
			currentState = MENU;
	}
	else if (currentState == LEVEL3_PLAYING)
	{
		if (key == 'p' || key == 'P')
		{
			pausedFromState = currentState;
			currentState = PAUSED;
		}
		else if (key == 'j' || key == 'J')
		{
			hero3Attack();
			playerAttackEnemies34();
		}
		else if (key == 27) currentState = MENU;
	}
	else if (currentState == LEVEL4_PLAYING)
	{
		if (key == 'p' || key == 'P')
		{
			pausedFromState = currentState;
			currentState = PAUSED;
		}
		else if (key == 'j' || key == 'J')
		{
			hero4Attack();
			playerAttackEnemies34();
		}
		else if (key == 27) currentState = MENU;
	}
	else if (currentState == PAUSED)
	{
		if (key == 'p' || key == 'P')
			currentState = pausedFromState;
		else if (key == 27)
			currentState = MENU;
	}
	else if (currentState == SETTINGS)
	{
		if (key == 27) currentState = MENU;
	}
	else if (currentState == CONTROLS)
	{
		if (key == 27) currentState = SETTINGS;
	}
	else
	{
		if (key == 27) currentState = MENU;
	}
}

void iSpecialKeyboard(unsigned char key)
{
	/* Arrow keys are held/read by hero2.h.  This callback is intentionally
	   empty so GLUT does not convert an arrow press into a one-shot jump. */
}



/* =========================================================
MAIN DRAW
========================================================= */
void iDraw()
{
	iClear();
	if (currentState == INTERFACE)
		drawInterface();
	else if (currentState == MENU)
		drawMainMenu();
	else if (currentState == LEVEL_SELECT)
		drawLevelSelect();
	else if (currentState == OPTIONS)
		drawOptions();
	else if (currentState == HIGH_SCORE)
		drawHighScore();
	else if (currentState == CREDITS)
		drawCredits();
	else if (currentState == NAME_INPUT)
		drawNameInput();
	else if (currentState == GAME_MODE)
		drawGameMode();
	else if (currentState == SETTINGS)
		drawSettings();
	else if (currentState == CONTROLS)
		drawControls();
	else if (currentState == STORY_INTRO_STATE ||
		currentState == STORY_LEVEL1_STATE ||
		currentState == STORY_ENDING_STATE ||
		currentState == STORY_LEVEL2_START_STATE ||
		currentState == STORY_LEVEL2_END_STATE ||
		currentState == STORY_LEVEL3_START_STATE ||
		currentState == STORY_LEVEL3_END_STATE ||
		currentState == STORY_LEVEL4_START_STATE ||
		currentState == STORY_LEVEL4_END_STATE)
		drawStory();
	else if (currentState == PLAYING)
		drawLevel1();
	else if (currentState == LEVEL2_PLAYING)
		drawLevel2();
	else if (currentState == LEVEL3_PLAYING)
		drawLevel3();
	else if (currentState == LEVEL4_PLAYING)
		drawLevel4();
	else if (currentState == PAUSED)
		drawPauseMenu();
	else if (currentState == GAME_OVER)
		drawGameOver();
	else if (currentState == LEVEL_COMPLETE)
		drawLevelComplete();
}


/* =========================================================
FIXED UPDATE
========================================================= */
void fixedUpdate()
{
	/* save-file hooks: react once when the state changes */
	static GameState prevSaveState = INTERFACE;
	if (currentState != prevSaveState)
	{
		if (currentState == GAME_OVER)
		{
			saveOnGameOver(level, score);   /* also puts the run total on the score board */
		}
		else if (currentState == LEVEL_COMPLETE)
			saveOnLevelComplete(level, score);
		else if (currentState == MENU && prevSaveState == PAUSED)
			saveOnQuitRun(level, score);    /* quit a run from the pause menu */
		prevSaveState = currentState;
	}

	interfaceBlinkFrame++;
	if (interfaceBlinkFrame >= 60)
		interfaceBlinkFrame = 0;

	if (currentState == STORY_INTRO_STATE ||
		currentState == STORY_LEVEL1_STATE ||
		currentState == STORY_ENDING_STATE ||
		currentState == STORY_LEVEL2_START_STATE ||
		currentState == STORY_LEVEL2_END_STATE ||
		currentState == STORY_LEVEL3_START_STATE ||
		currentState == STORY_LEVEL3_END_STATE ||
		currentState == STORY_LEVEL4_START_STATE ||
		currentState == STORY_LEVEL4_END_STATE)
	{
		storyUpdate();
	}

	if (lastMusicState != currentState)
	{
		if (currentState == MENU ||
			currentState == NAME_INPUT ||
			currentState == INTERFACE ||
			currentState == LEVEL_SELECT ||
			currentState == OPTIONS ||
			currentState == HIGH_SCORE ||
			currentState == CREDITS ||
			currentState == GAME_MODE ||
			currentState == SETTINGS ||
			currentState == CONTROLS ||
			currentState == PAUSED ||
			currentState == GAME_OVER ||
			currentState == LEVEL_COMPLETE)
		{
			if (musicOn)
			{
				PlaySound(
					TEXT("Images\\interface.wav"),
					NULL,
					SND_FILENAME | SND_ASYNC | SND_LOOP
					);
			}
			else
			{
				PlaySound(NULL, NULL, SND_PURGE);
			}
		}
		else if (currentState == PLAYING || currentState == LEVEL2_PLAYING ||
			currentState == LEVEL3_PLAYING || currentState == LEVEL4_PLAYING)
		{
			if (musicOn)
			{
				PlaySound(
					TEXT("Images\\level 1.wav"),
					NULL,
					SND_FILENAME | SND_ASYNC | SND_LOOP
					);
			}
			else
			{
				PlaySound(NULL, NULL, SND_PURGE);
			}
		}

		lastMusicState = currentState;
	}

	if (currentState == PLAYING)
	{
		updateHero();
		updateObstacles();
		updateEnemies();
	}
	else if (currentState == LEVEL2_PLAYING)
	{
		updateHero2();
		updateObstacles2();
		updateEnemies2();
	}
	else if (currentState == LEVEL3_PLAYING)
	{
		updateHero3();
		updateEnemies3();
		updateLevel3Collisions();
	}
	else if (currentState == LEVEL4_PLAYING)
	{
		updateHero4();
		updateEnemies4();
		updateLevel4Collisions();
	}

	if (currentState == INTERFACE ||
		currentState == MENU ||
		currentState == NAME_INPUT ||
		currentState == LEVEL_SELECT ||
		currentState == OPTIONS ||
		currentState == HIGH_SCORE ||
		currentState == CREDITS ||
		currentState == STORY_INTRO_STATE ||
		currentState == STORY_LEVEL1_STATE ||
		currentState == STORY_ENDING_STATE ||
		currentState == STORY_LEVEL2_START_STATE ||
		currentState == STORY_LEVEL2_END_STATE ||
		currentState == STORY_LEVEL3_START_STATE ||
		currentState == STORY_LEVEL3_END_STATE ||
		currentState == STORY_LEVEL4_START_STATE ||
		currentState == STORY_LEVEL4_END_STATE ||
		currentState == GAME_MODE ||
		currentState == SETTINGS ||
		currentState == CONTROLS ||
		currentState == PLAYING ||
		currentState == LEVEL2_PLAYING ||
		currentState == LEVEL3_PLAYING || currentState == LEVEL4_PLAYING)
	{
		iDraw();
	}
}


/* =========================================================
MAIN
========================================================= */
int main()
{
	iInitialize(SCREEN_WIDTH, SCREEN_HEIGHT,
		"Silhouette of Bengal - Interface");

	forestImage = iLoadImage("Images\\forest.png");
	interfaceImage = iLoadImage("Images\\Interface.png");
	menuBgImage = iLoadImage("Images\\forest.png");
	gameModeImage = iLoadImage("Images\\forest.png");
	settingsBgImage = iLoadImage("Images\\forest.png");
	controlsBgImage = iLoadImage("Images\\forest.png");

	initEnemies();
	initHero();
	initHero2();
	initHero3();
	initHero4();
	initBackground();
	initObstacles();
	initLevel2();
	initLevel3();
	initLevel4();
	initLevel3Obstacles();
	initLevel4Obstacles();
	initObstacles2();
	initEnemies2();
	initEnemies3();
	initEnemies4();

	saveSystemInit();

	if (musicOn)
	{
		PlaySound(
			TEXT("Images\\interface.wav"),
			NULL,
			SND_FILENAME | SND_ASYNC | SND_LOOP
			);
	}

	iStart();
	return 0;
}
