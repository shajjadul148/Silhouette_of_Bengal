#ifndef SILHOUETTE_LEVEL4_H
#define SILHOUETTE_LEVEL4_H

#include "globals.h"
#include "background.h"
#include "iGraphics.h"

#define LEVEL4_BG_SEGMENT_COUNT 4
static unsigned int level4BgSegment[LEVEL4_BG_SEGMENT_COUNT];

inline void initLevel4()
{
    level4BgSegment[0] = iLoadImage("Images\\level4\\Level4BG_1.png");
    level4BgSegment[1] = iLoadImage("Images\\level4\\Level4BG_2.png");
    level4BgSegment[2] = iLoadImage("Images\\level4\\Level4BG_3.png");
    level4BgSegment[3] = iLoadImage("Images\\level4\\Level4BG_4.png");
}

inline void drawLevel4Background()
{
    updateCamera();
    for (int i = 0; i < LEVEL4_BG_SEGMENT_COUNT; ++i)
    {
        float sx = worldToScreenX(i * SCREEN_WIDTH);
        if (sx + SCREEN_WIDTH < 0 || sx > SCREEN_WIDTH)
            continue;
        drawSoftMapImage((int)sx, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
            level4BgSegment[i]);
    }
}

#endif
