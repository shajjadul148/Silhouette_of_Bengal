#pragma once

/* =========================================================
level2.h
Scrolling background for Level 2.

Required image paths:
Images\Level2\level2_bg_1.png
Images\Level2\level2_bg_2.png
Images\Level2\level2_bg_3.png       (courtyard/gate scene -- 3rd
                                     screen, world x [2*SCREEN_WIDTH,
                                     3*SCREEN_WIDTH); this is where
                                     enemy2.h's last 4 knights fight
                                     and where they walk in from --
                                     see ENEMY2_GATE_X in enemy2.h)
========================================================= */

#include "globals.h"
#include "iGraphics.h"
#include "background.h"

static unsigned int level2BgSegment[3];

inline void initLevel2()
{
	level2BgSegment[0] = iLoadImage("Images\\Level2\\level2_bg_1.png");
	level2BgSegment[1] = iLoadImage("Images\\Level2\\level2_bg_2.png");
	level2BgSegment[2] = iLoadImage("Images\\Level2\\level2_bg_3.png");
}

inline void drawLevel2Background()
{
	/* The three Level 2 images are placed one after another. */
	updateCamera();

	for (int i = 0; i < 3; i++)
	{
		float worldX = i * SCREEN_WIDTH;
		float screenX = worldToScreenX(worldX);

		if (screenX + SCREEN_WIDTH < 0 || screenX > SCREEN_WIDTH)
			continue;

		drawSoftMapImage((int)screenX, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
			level2BgSegment[i]);
	}
}
