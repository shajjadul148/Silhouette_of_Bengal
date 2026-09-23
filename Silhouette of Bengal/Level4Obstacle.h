#ifndef SILHOUETTE_LEVEL4_OBSTACLE_H
#define SILHOUETTE_LEVEL4_OBSTACLE_H

#include "globals.h"
#include "iGraphics.h"
#include <math.h>
#include <stdlib.h>

#define LEVEL4_OBSTACLE_COUNT 3

struct Level4Obstacle
{
    float x;
    float y;
    int width;
    int height;
    unsigned int texture;

    /* Pixels to nudge the DRAWN sprite down by, purely visual. The
       stone PNGs each have a different amount of empty transparent
       margin baked in below the actual rock art (measured per-file:
       stone1 ~0%, stone4 ~7.7%, stone5 ~8.8% of the 100px draw
       height), so rendering them all at the same top-left `y` made
       some stones look like they were floating above the ground
       line while others sat flush with it -- the literal "obstacle
       mismatch with ground position" bug. `y` itself stays the
       single shared ground line so every floor/lane comparison
       elsewhere is untouched; only the draw call is shifted.
       See initLevel4Obstacles(). */
    float visualYOffset;
};

static Level4Obstacle level4Obstacles[LEVEL4_OBSTACLE_COUNT];

inline unsigned int loadLevel4ObstacleTexture(const char *filename)
{
    int width, height, channels;
    unsigned char *data = stbi_load((char*)filename, &width, &height, &channels, 4);
    if (!data) return 0;

    int pixels = width * height;
    for (int i = 0; i < pixels; ++i)
    {
        unsigned char *p = data + i * 4;
        bool nearWhite =
            p[0] >= 195 && p[1] >= 195 && p[2] >= 195 &&
            abs((int)p[0] - (int)p[1]) <= 6 &&
            abs((int)p[1] - (int)p[2]) <= 6;
        p[3] = nearWhite ? 0 : 255;
    }

    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
    return tex;
}

inline void drawLevel4Obstacle(const Level4Obstacle &o)
{
    float sx = o.x - cameraX;
    if (sx + o.width < 0 || sx > SCREEN_WIDTH) return;

    /* Only the DRAWN position is nudged -- o.y (the logical ground
       line used by collision/lane checks) is left alone. */
    float drawY = o.y + o.visualYOffset;

    if (o.texture == 0)
    {
        iSetColor(90, 90, 90);
        iFilledRectangle((int)sx, (int)drawY, o.width, o.height);
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, o.texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBegin(GL_QUADS);
        glTexCoord2f(0,0); glVertex2f((int)sx, (int)drawY);
        glTexCoord2f(1,0); glVertex2f((int)sx + o.width, (int)drawY);
        glTexCoord2f(1,-1); glVertex2f((int)sx + o.width, (int)drawY + o.height);
        glTexCoord2f(0,-1); glVertex2f((int)sx, (int)drawY + o.height);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

inline bool isLevel4CoverBetween(float heroX, float heroY, float enemyX, float enemyY)
{
    if (fabs(heroY - enemyY) > 55.0f) return false;

    float hCenter = heroX + 50.0f;
    float eCenter = enemyX + 50.0f;
    float left = (hCenter < eCenter) ? hCenter : eCenter;
    float right = (hCenter > eCenter) ? hCenter : eCenter;

    for (int i = 0; i < LEVEL4_OBSTACLE_COUNT; ++i)
    {
        const Level4Obstacle &o = level4Obstacles[i];
        if (o.x + o.width > left && o.x < right &&
            fabs(heroY - o.y) <= 55.0f &&
            fabs(enemyY - o.y) <= 55.0f)
            return true;
    }
    return false;
}

inline bool Level4ObstacleBlocksX(float oldX, float newX, float enemyY)
{
    float minX = (oldX < newX) ? oldX : newX;
    float maxX = (oldX > newX) ? oldX : newX;
    // Enemy and obstacle must be on the same floor/lane.
    for (int i = 0; i < LEVEL4_OBSTACLE_COUNT; ++i)
    {
        const Level4Obstacle &o = level4Obstacles[i];
        if (fabs(enemyY - o.y) > 55.0f) continue;
        if (maxX + 100.0f > o.x && minX < o.x + o.width)
            return true;
    }
    return false;
}

inline bool Level4EnemyMovementBlocked(float oldX, float newX, float enemyY)
{
    return Level4ObstacleBlocksX(oldX, newX, enemyY);
}

/* Decent-distance-from-obstacle margin used by level4SafeEnemySpawnX(). */
#define LEVEL4_ENEMY_OBSTACLE_MARGIN 80.0f

/* Nudges a candidate enemy spawn X away from any obstacle that sits on
   the same floor/lane as enemyY, so an enemy never spawns overlapping
   (or hugging) an obstacle. If desiredX is already a decent distance
   from every obstacle on that floor, it is returned unchanged. Called
   from enemy4.h at every spawn site instead of hard-coding raw X
   constants, so new/adjusted Level4Obstacle positions can never
   silently reintroduce an enemy-on-obstacle spawn. */
inline float level4SafeEnemySpawnX(float desiredX, float enemyY, float enemyWidth = 100.0f)
{
    // Multiple passes: pushing clear of one obstacle can land inside the
    // margin of a neighboring one when obstacles sit close together, so
    // re-scan until a pass makes no further change (a handful of
    // obstacles converges in at most a couple of passes).
    for (int pass = 0; pass < LEVEL4_OBSTACLE_COUNT + 1; ++pass)
    {
        bool changed = false;
        for (int i = 0; i < LEVEL4_OBSTACLE_COUNT; ++i)
        {
            const Level4Obstacle &o = level4Obstacles[i];
            if (fabs(enemyY - o.y) > 55.0f) continue; // different floor/lane, can't overlap

            float lo = o.x - LEVEL4_ENEMY_OBSTACLE_MARGIN - enemyWidth; // safe left edge for desiredX
            float hi = o.x + o.width + LEVEL4_ENEMY_OBSTACLE_MARGIN;    // safe right edge for desiredX

            if (desiredX > lo && desiredX < hi)
            {
                // Too close (or overlapping) -- push out to whichever
                // safe edge is nearer to the original spot.
                float distToLeft = fabs(desiredX - lo);
                float distToRight = fabs(hi - desiredX);
                desiredX = (distToLeft <= distToRight) ? lo : hi;
                changed = true;
            }
        }
        if (!changed) break;
    }
    return desiredX;
}

inline void drawLevel4Obstacles()
{
    for (int i = 0; i < LEVEL4_OBSTACLE_COUNT; ++i)
        drawLevel4Obstacle(level4Obstacles[i]);
}

inline void initLevel4Obstacles()
{
    /* stone2/stone3/stone6 were removed on purpose -- only stone1,
       stone4 and stone5 remain, keeping each one's original world-X
       spot so already-tuned enemy spawn positions stay a safe
       distance from them (see level4SafeEnemySpawnX()). */
    const float x[LEVEL4_OBSTACLE_COUNT] = { 850, 2350, 2940 };
    const float y[LEVEL4_OBSTACLE_COUNT] = { 60, 60, 60 };
    const char *files[LEVEL4_OBSTACLE_COUNT] = {
        "Images\\level4obstacle\\stone1.png",
        "Images\\level4obstacle\\stone4.png",
        "Images\\level4obstacle\\stone5.png"
    };
    /* Measured empty transparent margin below the actual rock art in
       each source PNG, as a percentage of its own height -- e.g.
       stone1.png has ~0% dead space under the rock, stone5.png has
       ~8.8%. Scaled to the 100px draw height below so every stone
       still sits with its visible base on the shared ground line
       instead of floating above it by a different amount each. */
    const float visualYOffset[LEVEL4_OBSTACLE_COUNT] = { 0.0f, 7.7f, 8.8f };

    for (int i = 0; i < LEVEL4_OBSTACLE_COUNT; ++i)
    {
        level4Obstacles[i].x = x[i];
        level4Obstacles[i].y = y[i];
        level4Obstacles[i].width = 100;
        level4Obstacles[i].height = 100;
        level4Obstacles[i].texture = loadLevel4ObstacleTexture(files[i]);
        level4Obstacles[i].visualYOffset = visualYOffset[i];
    }
}

inline void resetLevel4Obstacles() {}

#endif
