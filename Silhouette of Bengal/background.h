#pragma once
/* =========================================================
background.h
Scrolling Level-1 background.

The art is split into 3 equal segments (each SCREEN_WIDTH
wide), placed side by side, so Level 1 is 3 screens wide
in total (LEVEL1_WIDTH). A camera follows the player and
only the segment(s) currently on screen are drawn.

HOOKS INTO THE PLAYER:
The camera is driven by playerX (declared in enemy.h,
the shared world-space player position). Nothing here
needs to change once real movement code lands — as soon
as playerX is updated every frame by the movement/player
code, the background will scroll automatically.

ART ASSETS (drop these into Images\, same names, same
1024x500 size, once the licensed/final art is ready):
Images\level1_bg_1.png   (world x:    0 - 1024)
Images\level1_bg_2.png   (world x: 1024 - 2048)
Images\level1_bg_3.png   (world x: 2048 - 3072)
========================================================= */

#include "globals.h"
#include "iGraphics.h"

#define BG_SEGMENT_COUNT   3
#define BG_SEGMENT_WIDTH   SCREEN_WIDTH
#define BG_SEGMENT_HEIGHT  500
// LEVEL1_WIDTH (3 * SCREEN_WIDTH = 3072) is defined once in globals.h
// so background.h and enemy.h always agree on how wide the level is.

static unsigned int level1BgSegment[BG_SEGMENT_COUNT];

/* World-space camera (left edge of the visible screen). Declared
here, but enemy.h also reads it (extern) to convert enemy world
positions into screen positions when it draws them. */
float cameraX = 0.0f;

inline void initBackground()
{
	level1BgSegment[0] = iLoadImage("Images\\level1_bg_1.png");
	level1BgSegment[1] = iLoadImage("Images\\level1_bg_2.png");
	level1BgSegment[2] = iLoadImage("Images\\level1_bg_3.png");
}

/* Level 3 is one screen longer than the earlier levels. Both the
   camera and Hero2 use this helper, so the fourth background image
   remains reachable and the camera does not stop at image three. */
inline float getActiveLevelWidth()
{
	if (currentState == LEVEL3_PLAYING)
		return (float)LEVEL3_WIDTH;
	if (currentState == LEVEL4_PLAYING)
		return (float)LEVEL4_WIDTH;
	return (float)LEVEL1_WIDTH;
}

/* Keeps the camera centered on the player, clamped so it never
shows past either end of the level. */
inline void updateCamera()
{
	cameraX = playerX - SCREEN_WIDTH / 2.0f;
	if (cameraX < 0)
		cameraX = 0;
	float maximumCameraX = getActiveLevelWidth() - SCREEN_WIDTH;
	if (cameraX > maximumCameraX)
		cameraX = maximumCameraX;
}

/* Converts a world-space x (used by enemies/props/etc.) into the
screen-space x that should actually be drawn at. */
inline float worldToScreenX(float worldX)
{
	return worldX - cameraX;
}

inline void drawSoftMapImage(int x, int y, int width, int height,
	unsigned int image)
{
	/* Light multi-layer offset blur for scrolling map backgrounds. */
	iShowImage(x, y, width, height, image);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor4f(1.0f, 1.0f, 1.0f, 0.07f);

	int offsets[4][2] = { { -2, 0 }, { 2, 0 }, { 0, -2 }, { 0, 2 } };
	for (int i = 0; i < 4; i++)
	{
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);  glVertex2f(x + offsets[i][0], y + offsets[i][1]);
		glTexCoord2f(1, 0);  glVertex2f(x + width + offsets[i][0], y + offsets[i][1]);
		glTexCoord2f(1, -1); glVertex2f(x + width + offsets[i][0], y + height + offsets[i][1]);
		glTexCoord2f(0, -1); glVertex2f(x + offsets[i][0], y + height + offsets[i][1]);
		glEnd();
	}

	glColor4f(1, 1, 1, 1);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}

inline void drawLevel1Background()
{
	updateCamera();

	for (int i = 0; i < BG_SEGMENT_COUNT; i++)
	{
		float segWorldX = i * BG_SEGMENT_WIDTH;
		float screenX = worldToScreenX(segWorldX);

		// Skip segments that are fully off-screen either side.
		if (screenX + BG_SEGMENT_WIDTH < 0 || screenX > SCREEN_WIDTH)
			continue;

		drawSoftMapImage((int)screenX, 0, BG_SEGMENT_WIDTH, BG_SEGMENT_HEIGHT,
			level1BgSegment[i]);
	}
}
