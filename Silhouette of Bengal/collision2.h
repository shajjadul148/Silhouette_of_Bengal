

#include "globals.h"

extern float playerX, playerY;   // defined in enemy.h, shared by both levels

/* Player hitbox size -- mirrors hero.h's HERO_DRAW_WIDTH / HERO_DRAW_HEIGHT. */
#define PLAYER2_HITBOX_WIDTH   100
#define PLAYER2_HITBOX_HEIGHT  100

/* How far in from each edge of the full 100x100 canvas the real,
   visible Level 2 knight/hero is -- see "HITBOX ACCURACY" above. */
#define ENEMY2_HITBOX_INSET_X       22   // trimmed off left AND right
#define ENEMY2_HITBOX_INSET_TOP      6
#define ENEMY2_HITBOX_INSET_BOTTOM   5

#define PLAYER2_HITBOX_INSET_X      22   // trimmed off left AND right
#define PLAYER2_HITBOX_INSET_TOP     9
#define PLAYER2_HITBOX_INSET_BOTTOM 13

/* =========================================================
   aabbOverlap2 -- standard axis-aligned bounding box test.
   Both boxes use the same (x, y) = bottom-left corner
   convention as everything else drawn on screen.
   ========================================================= */
inline bool aabbOverlap2(float ax, float ay, float aw, float ah,
	float bx, float by, float bw, float bh)
{
	return (ax < bx + bw) && (ax + aw > bx) &&
	       (ay < by + bh) && (ay + ah > by);
}

/* =========================================================
   insetBox2 -- shrinks a (x, y, w, h) box (bottom-left origin)
   inward by the given margins, used to turn a sprite's full
   drawn canvas into a tighter box around the actual character.
   ========================================================= */
inline void insetBox2(float x, float y, float w, float h,
	float insetX, float insetTop, float insetBottom,
	float &outX, float &outY, float &outW, float &outH)
{
	outX = x + insetX;
	outW = w - insetX * 2;
	outY = y + insetBottom;
	outH = h - insetTop - insetBottom;
}

/* =========================================================
   playerInEnemy2Range -- crops both the knight's box and the
   player's box down to their real visible silhouettes, expands
   the knight's cropped box outward by `range` pixels on every
   side, then checks whether the player's cropped box overlaps
   that expanded box. Used both for "should this knight start
   attacking" and "did the player's swing land" -- just called
   with different `range` values (see enemy2.h).
   ========================================================= */
inline bool playerInEnemy2Range(float enemyX, float enemyY,
	float enemyW, float enemyH, float range)
{
	float ex, ey, ew, eh;
	insetBox2(enemyX, enemyY, enemyW, enemyH,
		ENEMY2_HITBOX_INSET_X, ENEMY2_HITBOX_INSET_TOP, ENEMY2_HITBOX_INSET_BOTTOM,
		ex, ey, ew, eh);

	// Expand the (already-cropped) knight box outward by the reach
	// being tested.
	ex -= range;
	ey -= range;
	ew += range * 2;
	eh += range * 2;

	float px, py, pw, ph;
	insetBox2(playerX, playerY, PLAYER2_HITBOX_WIDTH, PLAYER2_HITBOX_HEIGHT,
		PLAYER2_HITBOX_INSET_X, PLAYER2_HITBOX_INSET_TOP, PLAYER2_HITBOX_INSET_BOTTOM,
		px, py, pw, ph);

	return aabbOverlap2(ex, ey, ew, eh, px, py, pw, ph);
}

/* =========================================================
   enemy2sOverlap -- plain box overlap between two Level 2
   knights (full 100x100 boxes, no inset needed here -- this is
   just used to nudge knights apart, not to land a hit). Lets
   enemy2.h keep up to 10 knights from drawing exactly on top of
   each other while several are chasing/fighting the hero at once.
   ========================================================= */
inline bool enemy2sOverlap(float ax, float ay, float aw, float ah,
	float bx, float by, float bw, float bh)
{
	return aabbOverlap2(ax, ay, aw, ah, bx, by, bw, bh);
}

/* =========================================================
   LEVEL 2 OBSTACLE BLOCKING (enemy side)
   -----------------------------------------------------------
   Mirrors enemy_collision.h's Level 1 fix: Level 2 knights
   (enemy2.h) used to walk straight through obstacle2.h's stone
   obstacles while chasing/patrolling/entering from the gate,
   because none of enemy2.h's movement functions ever checked
   them.

   The obstacle x/width values below are copied from obstacle2.h
   (obstacleManager2.obstacles[]) on purpose rather than reading
   that class directly, same reasoning as enemy_collision.h: this
   keeps enemy2.h's fix scoped to enemy2.h + collision2.h only.
   If the obstacle positions in obstacle2.h are ever moved/
   resized/added/removed (LEVEL2_OBSTACLE_COUNT), UPDATE THE
   TABLE BELOW TO MATCH.

   Current obstacle2.h obstacles (as of this fix):
     obstacles[0]: x = 1050, width = 70
     obstacles[1]: x = 1900, width = 70

   Like Level 1's knights, Level 2 knights never leave the
   ground, so every obstacle is simply a solid wall at every
   height -- no jump-over case to handle.
   ========================================================= */
#define ENEMY2_OBSTACLE_COUNT 2
struct Enemy2ObstacleBox { float x; float width; };

static const Enemy2ObstacleBox enemy2ObstacleBoxes[ENEMY2_OBSTACLE_COUNT] =
{
	{ 1050.0f, 70.0f },   // obstacle2.h obstacles[0] (stone1.png)
	{ 1900.0f, 70.0f }    // obstacle2.h obstacles[1] (stone2.png)
};

/* =========================================================
   blockEnemy2AtObstacle -- given where a knight currently is
   (oldX) and where it's about to move to (newX), returns a
   possibly-adjusted x so the knight's box can't cross into an
   obstacle's footprint from outside it. If the knight was
   already inside the footprint, it's left alone rather than
   shoved somewhere unexpected.
   ========================================================= */
inline float blockEnemy2AtObstacle(float oldX, float newX, float width,
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
   blockEnemy2AtAllObstacles -- runs blockEnemy2AtObstacle()
   against every Level 2 obstacle, so a knight moving (whether
   chasing, patrolling, or walking in from the gate) stops right
   at the near edge of whichever obstacle it would otherwise
   have walked through.
   ========================================================= */
inline float blockEnemy2AtAllObstacles(float oldX, float newX, float width)
{
	for (int i = 0; i < ENEMY2_OBSTACLE_COUNT; i++)
	{
		newX = blockEnemy2AtObstacle(oldX, newX, width,
			enemy2ObstacleBoxes[i].x, enemy2ObstacleBoxes[i].width);
	}
	return newX;
}
