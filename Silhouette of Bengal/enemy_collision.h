

#include "globals.h"

extern float playerX, playerY;   // defined in enemy.h

/* Player hitbox size — mirrors hero.h's HERO_DRAW_WIDTH / HERO_DRAW_HEIGHT. */
#define PLAYER_HITBOX_WIDTH   100
#define PLAYER_HITBOX_HEIGHT  100

/* How far in from each edge of the full 100x100 canvas the real,
   visible character is — see "HITBOX ACCURACY" above. */
#define ENEMY_HITBOX_INSET_X       22   // trimmed off left AND right
#define ENEMY_HITBOX_INSET_TOP      6
#define ENEMY_HITBOX_INSET_BOTTOM   5

#define PLAYER_HITBOX_INSET_X      22   // trimmed off left AND right
#define PLAYER_HITBOX_INSET_TOP     9
#define PLAYER_HITBOX_INSET_BOTTOM 13

/* =========================================================
   aabbOverlap — standard axis-aligned bounding box test.
   Both boxes use the same (x, y) = bottom-left corner
   convention as iShowBMP2 / drawHeroTexture, so this lines up
   with how everything is actually drawn on screen.
   ========================================================= */
inline bool aabbOverlap(float ax, float ay, float aw, float ah,
	float bx, float by, float bw, float bh)
{
	return (ax < bx + bw) && (ax + aw > bx) &&
	       (ay < by + bh) && (ay + ah > by);
}

/* =========================================================
   insetBox — shrinks a (x, y, w, h) box (bottom-left origin)
   inward by the given margins, used to turn a sprite's full
   drawn canvas into a tighter box around the actual character.
   ========================================================= */
inline void insetBox(float x, float y, float w, float h,
	float insetX, float insetTop, float insetBottom,
	float &outX, float &outY, float &outW, float &outH)
{
	outX = x + insetX;
	outW = w - insetX * 2;
	outY = y + insetBottom;
	outH = h - insetTop - insetBottom;
}

/* =========================================================
   playerInEnemyRange — crops both the enemy's box and the
   player's box down to their real visible silhouettes (see
   HITBOX ACCURACY above), expands the enemy's cropped box
   outward by `range` pixels on every side, then checks whether
   the player's cropped box overlaps that expanded box. Used
   both for "should this idle knight start attacking" and "did
   the player's swing land" — just called with different
   `range` values (see enemy.h).
   ========================================================= */
inline bool playerInEnemyRange(float enemyX, float enemyY,
	float enemyW, float enemyH, float range)
{
	float ex, ey, ew, eh;
	insetBox(enemyX, enemyY, enemyW, enemyH,
		ENEMY_HITBOX_INSET_X, ENEMY_HITBOX_INSET_TOP, ENEMY_HITBOX_INSET_BOTTOM,
		ex, ey, ew, eh);

	// Expand the (already-cropped) enemy box outward by the reach
	// being tested.
	ex -= range;
	ey -= range;
	ew += range * 2;
	eh += range * 2;

	float px, py, pw, ph;
	insetBox(playerX, playerY, PLAYER_HITBOX_WIDTH, PLAYER_HITBOX_HEIGHT,
		PLAYER_HITBOX_INSET_X, PLAYER_HITBOX_INSET_TOP, PLAYER_HITBOX_INSET_BOTTOM,
		px, py, pw, ph);

	return aabbOverlap(ex, ey, ew, eh, px, py, pw, ph);
}

/* =========================================================
   LEVEL 1 OBSTACLE BLOCKING (enemy side)
   -----------------------------------------------------------
   Bug fixed here: knights used to walk straight through the
   Level 1 obstacles while chasing the player, because
   moveTowardPlayer() in enemy.h only ever checked the level's
   left/right edges, never the obstacles in between.

   The obstacle x/width values below are copied from Obstacle.h
   (obstacleManager.obstacles[]) on purpose rather than reading
   that class directly: Obstacle.h/hero.h are a teammate's
   portion of the project, and this fix is scoped to only
   enemy.h + enemy_collision.h. If the obstacle positions in
   Obstacle.h are ever moved/resized/added/removed
   (LEVEL1_OBSTACLE_COUNT), UPDATE THE TABLE BELOW TO MATCH --
   this table drifting out of sync with Obstacle.h is exactly
   what let enemies walk through/stop at the wrong spot before.

   Current Obstacle.h obstacles (as of this fix):
     obstacles[0]: x = 720,  width = 70
     obstacles[1]: x = 1650, width = 70

   Unlike the hero (who can jump over a low obstacle), knights
   never leave the ground, so for them every obstacle is simply
   a solid wall at every height — no jump-over case to handle.
   ========================================================= */
#define ENEMY_OBSTACLE_COUNT 2
struct EnemyObstacleBox { float x; float width; };

static const EnemyObstacleBox enemyObstacleBoxes[ENEMY_OBSTACLE_COUNT] =
{
	{  720.0f, 70.0f },   // Obstacle.h obstacles[0] (stone1.png)
	{ 1650.0f, 70.0f }    // Obstacle.h obstacles[1] (stone2.png)
};

/* =========================================================
   blockEnemyAtObstacle — given where a knight currently is
   (oldX) and where it's about to move to (newX), returns a
   possibly-adjusted x so the knight's box can't cross into an
   obstacle's footprint from outside it. If the knight was
   already inside the footprint (shouldn't normally happen, but
   keeps things safe), it's left alone rather than shoved
   somewhere unexpected.
   ========================================================= */
inline float blockEnemyAtObstacle(float oldX, float newX, float width,
	float obstacleX, float obstacleWidth)
{
	bool wasOverlapping = (oldX + width > obstacleX) &&
		(oldX < obstacleX + obstacleWidth);
	bool willOverlap = (newX + width > obstacleX) &&
		(newX < obstacleX + obstacleWidth);

	if (willOverlap && !wasOverlapping)
	{
		if (oldX < obstacleX)
			newX = obstacleX - width;          // approaching from the left
		else
			newX = obstacleX + obstacleWidth;  // approaching from the right
	}

	return newX;
}

/* =========================================================
   blockEnemyAtAllObstacles — runs blockEnemyAtObstacle() against
   every Level 1 obstacle, so a knight moving toward the player
   stops right at the near edge of whichever obstacle it would
   otherwise have walked through.
   ========================================================= */
inline float blockEnemyAtAllObstacles(float oldX, float newX, float width)
{
	for (int i = 0; i < ENEMY_OBSTACLE_COUNT; i++)
	{
		newX = blockEnemyAtObstacle(oldX, newX, width,
			enemyObstacleBoxes[i].x, enemyObstacleBoxes[i].width);
	}
	return newX;
}
