#ifndef SILHOUETTE_LEVEL3_OBSTACLE_H
#define SILHOUETTE_LEVEL3_OBSTACLE_H
#include "globals.h"

#define LEVEL3_OBSTACLE_COUNT 2
#include "iGraphics.h"
#include <math.h>
#include <stdlib.h>

struct Level3Obstacle
{
	float x;
	float y;
	int width;
	int height;
	unsigned int texture;
};

static Level3Obstacle level3Obstacles[LEVEL3_OBSTACLE_COUNT];

inline unsigned int loadLevel3ObstacleTexture(const char *filename)
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

inline void drawLevel3Obstacle(const Level3Obstacle &o)
{
	float sx = o.x - cameraX;
	if (sx + o.width < 0 || sx > SCREEN_WIDTH) return;

	if (o.texture == 0)
	{
		iSetColor(90, 90, 90);
		iFilledRectangle((int)sx, (int)o.y, o.width, o.height);
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
	glTexCoord2f(0, 0); glVertex2f((int)sx, (int)o.y);
	glTexCoord2f(1, 0); glVertex2f((int)sx + o.width, (int)o.y);
	glTexCoord2f(1, -1); glVertex2f((int)sx + o.width, (int)o.y + o.height);
	glTexCoord2f(0, -1); glVertex2f((int)sx, (int)o.y + o.height);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

inline bool isLevel3CoverBetween(float heroX, float heroY, float enemyX, float enemyY)
{
	if (fabs(heroY - enemyY) > 55.0f) return false;

	float hCenter = heroX + 50.0f;
	float eCenter = enemyX + 50.0f;
	float left = (hCenter < eCenter) ? hCenter : eCenter;
	float right = (hCenter > eCenter) ? hCenter : eCenter;

	for (int i = 0; i < LEVEL3_OBSTACLE_COUNT; ++i)
	{
		const Level3Obstacle &o = level3Obstacles[i];
		float oLeft = o.x;
		float oRight = o.x + o.width;
		if (oRight > left && oLeft < right &&
			fabs(heroY - o.y) <= 55.0f &&
			fabs(enemyY - o.y) <= 55.0f)
			return true;
	}
	return false;
}

inline bool Level3ObstacleBlocksX(float oldX, float newX, float enemyY)
{
	float minX = (oldX < newX) ? oldX : newX;
	float maxX = (oldX > newX) ? oldX : newX;
	// Enemy and obstacle must be on the same floor/lane.
	for (int i = 0; i < LEVEL3_OBSTACLE_COUNT; ++i)
	{
		const Level3Obstacle &o = level3Obstacles[i];
		if (fabs(enemyY - o.y) > 55.0f) continue;
		if (maxX + 100.0f > o.x && minX < o.x + o.width)
			return true;
	}
	return false;
}

inline bool Level3EnemyMovementBlocked(float oldX, float newX, float enemyY)
{
	return Level3ObstacleBlocksX(oldX, newX, enemyY);
}

/* Decent-distance-from-obstacle margin used by level3SafeEnemySpawnX(). */
#define LEVEL3_ENEMY_OBSTACLE_MARGIN 80.0f

/* Nudges a candidate enemy spawn X away from any obstacle that sits on
   the same floor/lane as enemyY, so an enemy never spawns overlapping
   (or hugging) an obstacle. If desiredX is already a decent distance
   from every obstacle on that floor, it is returned unchanged. Called
   from enemy3.h at every spawn site instead of hard-coding raw X
   constants, so new/adjusted Level3Obstacle positions can never
   silently reintroduce an enemy-on-obstacle spawn. */
inline float level3SafeEnemySpawnX(float desiredX, float enemyY, float enemyWidth = 100.0f)
{
	// Multiple passes: pushing clear of one obstacle can land inside the
	// margin of a neighboring one when obstacles sit close together, so
	// re-scan until a pass makes no further change.
	for (int pass = 0; pass < LEVEL3_OBSTACLE_COUNT + 1; ++pass)
	{
		bool changed = false;
		for (int i = 0; i < LEVEL3_OBSTACLE_COUNT; ++i)
		{
			const Level3Obstacle &o = level3Obstacles[i];
			if (fabs(enemyY - o.y) > 55.0f) continue; // different floor/lane, can't overlap

			float lo = o.x - LEVEL3_ENEMY_OBSTACLE_MARGIN - enemyWidth; // safe left edge for desiredX
			float hi = o.x + o.width + LEVEL3_ENEMY_OBSTACLE_MARGIN;    // safe right edge for desiredX

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

inline void drawLevel3Obstacles()
{
	for (int i = 0; i < LEVEL3_OBSTACLE_COUNT; ++i)
		drawLevel3Obstacle(level3Obstacles[i]);
}

inline void resetLevel3Obstacles() {}


inline void initLevel3Obstacles()
{
	const float x[LEVEL3_OBSTACLE_COUNT] = { 900, 3000 };
	const float y[LEVEL3_OBSTACLE_COUNT] = { 10, 15};
	const char *files[LEVEL3_OBSTACLE_COUNT] = {
		"Images\\level3obstacle\\stone3.png",
		"Images\\level3obstacle\\stone4.jpg"
	};

	for (int i = 0; i < LEVEL3_OBSTACLE_COUNT; ++i)
	{
		level3Obstacles[i].x = x[i];
		level3Obstacles[i].y = y[i];
		level3Obstacles[i].width = 100;
		level3Obstacles[i].height = 100;
		level3Obstacles[i].texture = loadLevel3ObstacleTexture(files[i]);
	}
}

#endif
