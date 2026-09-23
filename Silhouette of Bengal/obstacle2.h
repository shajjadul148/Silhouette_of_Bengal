#ifndef SILHOUETTE_LEVEL2_OBSTACLE_H
#define SILHOUETTE_LEVEL2_OBSTACLE_H

/* =========================================================
Obstacle2.h
Picture obstacles for Level 2.

stone1.png and stone2.png are placed in that serial order.
========================================================= */

#include "globals.h"
#include "iGraphics.h"

/* hero2.h and background.h are included before this file from iMain.cpp.
(Level 2 obstacles collide against hero2, the axe-wielding Level 2
player character -- not hero.h's Level 1 hero.) */
extern float cameraX;
extern float playerX;

#define LEVEL2_OBSTACLE_COUNT 2
#define LEVEL2_OBSTACLE_COLORKEY 195
#define LEVEL2_OBSTACLE_DAMAGE 5

struct Level2PictureObstacle
{
	float x;
	int width;
	int height;
	unsigned int texture;
	bool damageGivenThisJump;
};

inline unsigned int loadLevel2ObstacleTexture(char filename[])
{
	int width, height, channels;
	unsigned char *data = stbi_load(filename, &width, &height, &channels, 4);
	if (!data)
		return 0;

	int pixelCount = width * height;
	for (int i = 0; i < pixelCount; i++)
	{
		unsigned char *pixel = data + i * 4;
		bool lightNeutral =
			pixel[0] >= LEVEL2_OBSTACLE_COLORKEY &&
			pixel[1] >= LEVEL2_OBSTACLE_COLORKEY &&
			pixel[2] >= LEVEL2_OBSTACLE_COLORKEY &&
			abs((int)pixel[0] - (int)pixel[1]) <= 4 &&
			abs((int)pixel[1] - (int)pixel[2]) <= 4;
		pixel[3] = lightNeutral ? 0 : 255;
	}

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
	return texture;
}

/* Same placeholder idea as Obstacle.h -- keeps Level 2's stones
visible (and blockable/hazardous) even if a stone PNG fails to load. */
inline void drawLevel2ObstaclePlaceholder(int x, int y, int width, int height)
{
	iSetColor(120, 120, 120);
	iFilledRectangle(x, y, width, height);
	iSetColor(90, 90, 90);
	iRectangle(x, y, width, height);
}

inline void drawLevel2ObstacleTexture(int x, int y, int width, int height,
	unsigned int texture)
{
	if (texture == 0)
	{
		drawLevel2ObstaclePlaceholder(x, y, width, height);
		return;
	}

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);  glVertex2f(x, y);
	glTexCoord2f(1, 0);  glVertex2f(x + width, y);
	glTexCoord2f(1, -1); glVertex2f(x + width, y + height);
	glTexCoord2f(0, -1); glVertex2f(x, y + height);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

class ObstacleManager2
{
public:
	Level2PictureObstacle obstacles[LEVEL2_OBSTACLE_COUNT];

	void init()
	{
		/* Far apart world positions keep the Level 2 path open and scenic. */
		obstacles[0].x = 1050;
		obstacles[0].width = 70;
		obstacles[0].height = 70;
		obstacles[0].texture = loadLevel2ObstacleTexture(
			"Images\\level2obstacle\\stone1.png");
		obstacles[0].damageGivenThisJump = false;

		obstacles[1].x = 1900;
		obstacles[1].width = 70;
		obstacles[1].height = 73;
		obstacles[1].texture = loadLevel2ObstacleTexture(
			"Images\\level2obstacle\\stone2.png");
		obstacles[1].damageGivenThisJump = false;
	}

	void reset()
	{
		for (int i = 0; i < LEVEL2_OBSTACLE_COUNT; i++)
			obstacles[i].damageGivenThisJump = false;
	}

	bool overlapsHero(const Level2PictureObstacle &obstacle)
	{
		return hero2.x + hero2.width > obstacle.x &&
			hero2.x < obstacle.x + obstacle.width;
	}

	void damageHeroWhenLanding(Level2PictureObstacle &obstacle)
	{
		if (hero2.grounded)
		{
			obstacle.damageGivenThisJump = false;
			return;
		}

		if (obstacle.damageGivenThisJump || hero2.velY >= 0 ||
			!overlapsHero(obstacle))
			return;

		float obstacleTop = HERO_GROUND_Y + obstacle.height;
		float previousHeroY = hero2.y - hero2.velY;
		if (previousHeroY >= obstacleTop && hero2.y <= obstacleTop)
		{
			health -= LEVEL2_OBSTACLE_DAMAGE;
			if (health < 0)
				health = 0;
			/* Keep obstacle damage from triggering hero.h's enemy-hit
			knockback, so the jump can still carry the hero forward. */
			hero2.lastHealth = health;
			obstacle.damageGivenThisJump = true;

			if (health == 0)
				currentState = GAME_OVER;
		}
	}

	void blockHeroAtObstacle(const Level2PictureObstacle &obstacle)
	{
		if (!overlapsHero(obstacle))
			return;

		/* Do not push the hero back during a jump. This keeps the picture
		obstacle at full visual size while making it crossable in one jump. */
		if (!hero2.grounded)
			return;

		float obstacleMiddle = obstacle.x + obstacle.width / 2.0f;
		float heroMiddle = hero2.x + hero2.width / 2.0f;
		if (heroMiddle < obstacleMiddle)
			hero2.x = obstacle.x - hero2.width;
		else
			hero2.x = obstacle.x + obstacle.width;

		hero2.clampToLevel();
		playerX = hero2.x;
	}

	void update()
	{
		for (int i = 0; i < LEVEL2_OBSTACLE_COUNT; i++)
		{
			damageHeroWhenLanding(obstacles[i]);
			blockHeroAtObstacle(obstacles[i]);
		}
	}

	void draw()
	{
		for (int i = 0; i < LEVEL2_OBSTACLE_COUNT; i++)
		{
			int screenX = (int)(obstacles[i].x - cameraX);
			if (screenX + obstacles[i].width < 0 || screenX > SCREEN_WIDTH)
				continue;

			drawLevel2ObstacleTexture(screenX, (int)HERO_GROUND_Y,
				obstacles[i].width, obstacles[i].height, obstacles[i].texture);
		}
	}
};

ObstacleManager2 obstacleManager2;

inline void initObstacles2()   { obstacleManager2.init(); }
inline void resetObstacles2()  { obstacleManager2.reset(); }
inline void updateObstacles2() { obstacleManager2.update(); }
inline void drawObstacles2()   { obstacleManager2.draw(); }

#endif
