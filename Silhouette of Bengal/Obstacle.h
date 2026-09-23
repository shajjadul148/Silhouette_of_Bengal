#ifndef SILHOUETTE_LEVEL1_OBSTACLE_H
#define SILHOUETTE_LEVEL1_OBSTACLE_H

/* =========================================================
Obstacle.h
Picture obstacles for Level 1

stone1.png -> stone2.png
(Obstacle 3 removed)
========================================================= */

#include "globals.h"
#include "iGraphics.h"

/* hero.h and background.h are included before this file
from iMain.cpp. */

extern float cameraX;
extern float playerX;


/* =========================================================
SETTINGS
========================================================= */

// এখানে সংখ্যা 3 থেকে পরিবর্তন করে 2 করা হয়েছে
#define LEVEL1_OBSTACLE_COUNT 2 
#define LEVEL1_OBSTACLE_COLORKEY 195
#define LEVEL1_OBSTACLE_DAMAGE 5


/* =========================================================
OBSTACLE STRUCTURE
========================================================= */

struct Level1PictureObstacle
{
	float x;
	int width;
	int height;
	unsigned int texture;
	bool damageGivenThisJump;
};


/* =========================================================
LOAD OBSTACLE TEXTURE
========================================================= */

inline unsigned int loadLevel1ObstacleTexture(char filename[])
{
	int width, height, channels;

	unsigned char *data =
		stbi_load(
		filename,
		&width,
		&height,
		&channels,
		4
		);

	if (!data)
		return 0;


	/* -----------------------------------------
	Remove light/white background
	----------------------------------------- */

	int pixelCount = width * height;

	for (int i = 0; i < pixelCount; i++)
	{
		unsigned char *pixel = data + i * 4;

		bool lightNeutral =
			pixel[0] >= LEVEL1_OBSTACLE_COLORKEY &&
			pixel[1] >= LEVEL1_OBSTACLE_COLORKEY &&
			pixel[2] >= LEVEL1_OBSTACLE_COLORKEY &&

			abs(
			(int)pixel[0] -
			(int)pixel[1]
			) <= 4 &&

			abs(
			(int)pixel[1] -
			(int)pixel[2]
			) <= 4;

		if (lightNeutral)
			pixel[3] = 0;
		else
			pixel[3] = 255;
	}


	/* -----------------------------------------
	Create OpenGL texture
	----------------------------------------- */

	unsigned int texture;

	glGenTextures(
		1,
		&texture
		);

	glBindTexture(
		GL_TEXTURE_2D,
		texture
		);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		width,
		height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		data
		);


	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MIN_FILTER,
		GL_LINEAR
		);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MAG_FILTER,
		GL_LINEAR
		);


	stbi_image_free(data);

	return texture;
}


/* =========================================================
DRAW OBSTACLE TEXTURE
========================================================= */

/* Simple placeholder so the obstacle is still visible (and still
readable as "a rock") even if a stone PNG fails to load for some
reason -- e.g. wrong working directory, renamed file. Drops out
automatically once the real texture loads successfully. */
inline void drawLevel1ObstaclePlaceholder(int x, int y, int width, int height)
{
	iSetColor(120, 120, 120);
	iFilledRectangle(x, y, width, height);
	iSetColor(90, 90, 90);
	iRectangle(x, y, width, height);
}

inline void drawLevel1ObstacleTexture(
	int x,
	int y,
	int width,
	int height,
	unsigned int texture)
{
	if (texture == 0)
	{
		drawLevel1ObstaclePlaceholder(x, y, width, height);
		return;
	}


	glEnable(GL_TEXTURE_2D);

	glBindTexture(
		GL_TEXTURE_2D,
		texture
		);


	glTexParameterf(
		GL_TEXTURE_2D,
		GL_TEXTURE_MIN_FILTER,
		GL_LINEAR
		);

	glTexParameterf(
		GL_TEXTURE_2D,
		GL_TEXTURE_MAG_FILTER,
		GL_LINEAR
		);

	glTexParameterf(
		GL_TEXTURE_2D,
		GL_TEXTURE_WRAP_S,
		GL_REPEAT
		);

	glTexParameterf(
		GL_TEXTURE_2D,
		GL_TEXTURE_WRAP_T,
		GL_REPEAT
		);


	glTexEnvf(
		GL_TEXTURE_ENV,
		GL_TEXTURE_ENV_MODE,
		GL_REPLACE
		);


	glBegin(GL_QUADS);

	glTexCoord2f(0, 0);
	glVertex2f(
		x,
		y
		);

	glTexCoord2f(1, 0);
	glVertex2f(
		x + width,
		y
		);

	glTexCoord2f(1, -1);
	glVertex2f(
		x + width,
		y + height
		);

	glTexCoord2f(0, -1);
	glVertex2f(
		x,
		y + height
		);

	glEnd();


	glDisable(GL_TEXTURE_2D);
}


/* =========================================================
OBSTACLE MANAGER
========================================================= */

class ObstacleManager
{
public:

	Level1PictureObstacle obstacles[
		LEVEL1_OBSTACLE_COUNT
	];


	/* =====================================================
	INITIALIZE OBSTACLES
	===================================================== */

	void init()
	{
		/* -----------------------------------------
		OBSTACLE 1
		Same size as obstacle2.h stone1
		70 x 70
		----------------------------------------- */

		obstacles[0].x = 720;

		obstacles[0].width = 70;

		obstacles[0].height = 70;

		obstacles[0].texture =
			loadLevel1ObstacleTexture(
			"Images\\level1obstacle\\stone1.png"
			);

		obstacles[0].damageGivenThisJump = false;


		/* -----------------------------------------
		OBSTACLE 2
		Same size as obstacle2.h stone2
		70 x 73
		----------------------------------------- */

		obstacles[1].x = 1650;

		obstacles[1].width = 70;

		obstacles[1].height = 73;

		obstacles[1].texture =
			loadLevel1ObstacleTexture(
			"Images\\level1obstacle\\stone2.png"
			);

		obstacles[1].damageGivenThisJump = false;

		// OBSTACLE 3 এখান থেকে রিমুভ করা হয়েছে
	}


	/* =====================================================
	RESET OBSTACLES
	===================================================== */

	void reset()
	{
		for (
			int i = 0;
			i < LEVEL1_OBSTACLE_COUNT;
		i++
			)
		{
			obstacles[i].damageGivenThisJump = false;
		}
	}


	/* =====================================================
	CHECK HERO / OBSTACLE OVERLAP
	===================================================== */

	bool overlapsHero(
		const Level1PictureObstacle &obstacle)
	{
		return
			hero.x + hero.width > obstacle.x &&
			hero.x <
			obstacle.x + obstacle.width;
	}


	/* =====================================================
	DAMAGE HERO WHEN LANDING
	===================================================== */

	void damageHeroWhenLanding(
		Level1PictureObstacle &obstacle)
	{
		/* Hero is on ground */

		if (hero.grounded)
		{
			obstacle.damageGivenThisJump = false;

			return;
		}


		/* Already damaged during this jump */

		if (obstacle.damageGivenThisJump)
			return;


		/* Hero must be falling */

		if (hero.velY >= 0)
			return;


		/* Hero must be horizontally over obstacle */

		if (!overlapsHero(obstacle))
			return;


		/* Top of obstacle */

		float obstacleTop =
			HERO_GROUND_Y +
			obstacle.height;


		/* Previous hero position */

		float previousHeroY =
			hero.y - hero.velY;


		/* Landing detection */

		if (
			previousHeroY >= obstacleTop &&
			hero.y <= obstacleTop
			)
		{
			health -= LEVEL1_OBSTACLE_DAMAGE;


			if (health < 0)
				health = 0;


			/* Prevent unwanted knockback */

			hero.lastHealth = health;


			obstacle.damageGivenThisJump = true;


			/* Game over */

			if (health == 0)
			{
				currentState = GAME_OVER;
			}
		}
	}


	/* =====================================================
	BLOCK HERO AT OBSTACLE
	===================================================== */

	void blockHeroAtObstacle(
		const Level1PictureObstacle &obstacle)
	{
		if (!overlapsHero(obstacle))
			return;


		/* Do not block while jumping */

		if (!hero.grounded)
			return;


		float obstacleMiddle =
			obstacle.x +
			obstacle.width / 2.0f;


		float heroMiddle =
			hero.x +
			hero.width / 2.0f;


		/* Hero is on left side */

		if (heroMiddle < obstacleMiddle)
		{
			hero.x =
				obstacle.x -
				hero.width;
		}

		/* Hero is on right side */

		else
		{
			hero.x =
				obstacle.x +
				obstacle.width;
		}


		/* Keep hero inside level */

		hero.clampToLevel();


		/* Update player position */

		playerX = hero.x;
	}


	/* =====================================================
	UPDATE
	===================================================== */

	void update()
	{
		for (
			int i = 0;
			i < LEVEL1_OBSTACLE_COUNT;
		i++
			)
		{
			damageHeroWhenLanding(
				obstacles[i]
				);

			blockHeroAtObstacle(
				obstacles[i]
				);
		}
	}


	/* =====================================================
	DRAW
	===================================================== */

	void draw()
	{
		for (
			int i = 0;
			i < LEVEL1_OBSTACLE_COUNT;
		i++
			)
		{
			int screenX =
				(int)(
				obstacles[i].x -
				cameraX
				);


			/* Outside screen */

			if (
				screenX +
				obstacles[i].width < 0
				)
			{
				continue;
			}


			if (
				screenX >
				SCREEN_WIDTH
				)
			{
				continue;
			}


			/* Draw obstacle */

			drawLevel1ObstacleTexture(
				screenX,
				(int)HERO_GROUND_Y,
				obstacles[i].width,
				obstacles[i].height,
				obstacles[i].texture
				);
		}
	}
};


/* =========================================================
GLOBAL OBSTACLE MANAGER
========================================================= */

ObstacleManager obstacleManager;


/* =========================================================
FUNCTIONS USED BY iMain.cpp
========================================================= */

inline void initObstacles()
{
	obstacleManager.init();
}


inline void resetObstacles()
{
	obstacleManager.reset();
}


inline void updateObstacles()
{
	obstacleManager.update();
}


inline void drawObstacles()
{
	obstacleManager.draw();
}


#endif