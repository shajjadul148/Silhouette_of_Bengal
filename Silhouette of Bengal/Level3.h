#ifndef SILHOUETTE_LEVEL3_H
#define SILHOUETTE_LEVEL3_H

/* =========================================================
   Level3.h
   Four Level 3 backgrounds are placed serially in one
   scrolling world. Required files:
   Images\level3\Level_3_bg_1.png
   Images\level3\Level_3_bg_2.png
   Images\level3\Level_3_bg_3.png
   Images\level3\Level_3_bg_4.png
   ========================================================= */

#include "globals.h"
#include "iGraphics.h"
#include "background.h"

#define LEVEL3_BG_SEGMENT_COUNT 4

static unsigned int level3BgSegment[LEVEL3_BG_SEGMENT_COUNT];

inline void initLevel3()
{
	level3BgSegment[0] = iLoadImage("Images\\level3\\Level_3_bg_1.png");
	level3BgSegment[1] = iLoadImage("Images\\level3\\Level_3_bg_2.png");
	level3BgSegment[2] = iLoadImage("Images\\level3\\Level_3_bg_3.png");
	level3BgSegment[3] = iLoadImage("Images\\level3\\Level_3_bg_4.png");
}

inline void drawLevel3Background()
{
	updateCamera();

	for (int i = 0; i < LEVEL3_BG_SEGMENT_COUNT; i++)
	{
		float worldX = i * SCREEN_WIDTH;
		float screenX = worldToScreenX(worldX);

		if (screenX + SCREEN_WIDTH < 0 || screenX > SCREEN_WIDTH)
			continue;

		drawSoftMapImage((int)screenX, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
			level3BgSegment[i]);
	}
}

#endif
