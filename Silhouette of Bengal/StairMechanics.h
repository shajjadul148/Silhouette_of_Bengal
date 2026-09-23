#ifndef SILHOUETTE_STAIR_MECHANICS_H
#define SILHOUETTE_STAIR_MECHANICS_H

#include "globals.h"
#include <math.h>

/*
   PLAYER-CONTROLLED STAIRS FOR LEVEL 3 AND LEVEL 4
   -------------------------------------------------
   This is intentionally NOT automatic diagonal movement.

   - UP / W held  : Y increases only.
   - DOWN / S held: Y decreases only.
   - LEFT / A     : X decreases normally.
   - RIGHT / D    : X increases normally.

   The player must place the Hero inside a stair lane first.  While
   climbing, X never changes, so the player has full control over
   when to go up/down and when to leave the stair at the landing.
*/

struct PlayerStairLane
{
    float centerX;
    float yLow;
    float yHigh;
    float xHalfWidth;
};

#define STAIR_COUNT 4
#define STAIR_VERTICAL_SPEED 3.0f
#define STAIR_ENTRY_MARGIN 30.0f
#define STAIR_GROUND_Y 40.0f

/* The X values are world coordinates.  They are intentionally broad
   enough that a 100x100 Hero can step into the stair opening easily. */
static const PlayerStairLane level3StairLanes[STAIR_COUNT] =
{
    { 680.0f,  40.0f, 280.0f, 48.0f },
    { 1580.0f, 40.0f, 280.0f, 48.0f },
    { 2210.0f, 40.0f, 280.0f, 48.0f },
    { 3110.0f, 40.0f, 280.0f, 48.0f }
};

static const PlayerStairLane level4StairLanes[STAIR_COUNT] =
{
    { 720.0f,  40.0f, 285.0f, 48.0f },
    { 1535.0f, 40.0f, 285.0f, 48.0f },
    { 2365.0f, 40.0f, 285.0f, 48.0f },
    { 3185.0f, 40.0f, 285.0f, 48.0f }
};

inline const PlayerStairLane *getActiveStairLanes()
{
    return (currentState == LEVEL4_PLAYING) ? level4StairLanes : level3StairLanes;
}

inline bool playerIsInsideStairLane(float heroX, float heroY, int &laneIndex)
{
    const PlayerStairLane *lanes = getActiveStairLanes();
    float heroCenterX = heroX + 50.0f;

    for (int i = 0; i < STAIR_COUNT; ++i)
    {
        const PlayerStairLane &lane = lanes[i];
        if (fabs(heroCenterX - lane.centerX) <= lane.xHalfWidth &&
            heroY >= lane.yLow - STAIR_ENTRY_MARGIN &&
            heroY <= lane.yHigh + STAIR_ENTRY_MARGIN)
        {
            laneIndex = i;
            return true;
        }
    }
    laneIndex = -1;
    return false;
}

inline bool movePlayerUpOnStair(float &heroX, float &heroY)
{
    /* Player-controlled vertical movement.
       There is deliberately NO stair-lane/X-position check here.
       Pressing/holding UP or W always moves the Hero straight upward.
       X is never changed. */
    float maxY = (currentState == LEVEL4_PLAYING) ? 285.0f : 280.0f;
    float newY = heroY + STAIR_VERTICAL_SPEED;
    if (newY > maxY)
        newY = maxY;

    if (newY == heroY)
        return false;

    heroY = newY;
    return true;
}

inline bool movePlayerDownOnStair(float &heroX, float &heroY)
{
    /* Player-controlled vertical movement.
       There is deliberately NO stair-lane/X-position check here.
       Pressing/holding DOWN or S always moves the Hero straight downward.
       X is never changed. */
    float minY = STAIR_GROUND_Y;
    float newY = heroY - STAIR_VERTICAL_SPEED;
    if (newY < minY)
        newY = minY;

    if (newY == heroY)
        return false;

    heroY = newY;
    return true;
}

inline bool heroIsOnUpperFloor(float heroY)
{
    return heroY > 170.0f;
}

#endif
