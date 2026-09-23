#pragma once

#include "globals.h"
#include "enemy4.h"

/* Bounding box inset constants for Level 4 entities
   (independent copy of the Level 3 insets -- no shared code with
   collision3.h). */
#define HERO4_INSET_LEFT        22.0f
#define HERO4_INSET_RIGHT       22.0f
#define HERO4_INSET_BOTTOM      13.0f
#define HERO4_INSET_TOP          9.0f

#define ENEMY4_BLUE_INSET_X     20.0f
#define ENEMY4_BLUE_INSET_Y     10.0f

#define SOLDIER4_INSET_X        18.0f
#define SOLDIER4_INSET_Y        12.0f

#define BOSS4_INSET_X           30.0f
#define BOSS4_INSET_Y           18.0f

/* Axis-Aligned Bounding Box (AABB) Overlap Helper */
inline bool checkAABBCollision4(float x1, float y1, float w1, float h1,
	float x2, float y2, float w2, float h2)
{
	return (x1 < x2 + w2 &&
		x1 + w1 > x2 &&
		y1 < y2 + h2 &&
		y1 + h1 > y2);
}

/* Get player precise collision box */
inline void getLevel4PlayerHitbox(float &px, float &py, float &pw, float &ph)
{
	px = playerX + HERO4_INSET_LEFT;
	py = playerY + HERO4_INSET_BOTTOM;
	pw = 100.0f - (HERO4_INSET_LEFT + HERO4_INSET_RIGHT);
	ph = 100.0f - (HERO4_INSET_BOTTOM + HERO4_INSET_TOP);
}

/* Get Enemy4 precise collision box */
inline void getEnemy4Hitbox(const Enemy4 &e, float &ex, float &ey, float &ew, float &eh)
{
	if (e.type == TYPE4_BLUE)
	{
		ex = e.x + ENEMY4_BLUE_INSET_X;
		ey = e.y + ENEMY4_BLUE_INSET_Y;
		ew = e.width - (2.0f * ENEMY4_BLUE_INSET_X);
		eh = e.height - (2.0f * ENEMY4_BLUE_INSET_Y);
	}
	else if (e.type == TYPE4_SOLDIER)
	{
		ex = e.x + SOLDIER4_INSET_X;
		ey = e.y + SOLDIER4_INSET_Y;
		ew = e.width - (2.0f * SOLDIER4_INSET_X);
		eh = e.height - (2.0f * SOLDIER4_INSET_Y);
	}
	else // TYPE4_BOSS / TYPE4_BOSS2 -- both use the same boss-sized insets
	{
		ex = e.x + BOSS4_INSET_X;
		ey = e.y + BOSS4_INSET_Y;
		ew = e.width - (2.0f * BOSS4_INSET_X);
		eh = e.height - (2.0f * BOSS4_INSET_Y);
	}
}

/* Handles physical body collisions and bounds between Hero and Active Level 4 Enemies */
inline void handleLevel4PlayerEnemyCollisions()
{
	float px, py, pw, ph;
	getLevel4PlayerHitbox(px, py, pw, ph);

	for (int i = 0; i < MAX_ACTIVE_ENEMIES4; i++)
	{
		Enemy4 &e = enemyManager4.activeEnemies[i];
		if (!e.alive) continue;

		float ex, ey, ew, eh;
		getEnemy4Hitbox(e, ex, ey, ew, eh);

		if (checkAABBCollision4(px, py, pw, ph, ex, ey, ew, eh))
		{
			/* Simple push-back physics to avoid overlapping bodies */
			if (playerX < e.x)
			{
				playerX -= 2.0f;
				if (e.type == TYPE4_BLUE || e.type == TYPE4_BOSS || e.type == TYPE4_BOSS2) e.x += 1.0f;
			}
			else
			{
				playerX += 2.0f;
				if (e.type == TYPE4_BLUE || e.type == TYPE4_BOSS || e.type == TYPE4_BOSS2) e.x -= 1.0f;
			}
		}
	}
}

/* Restricts Level 4 Enemies from walking off edge bounds */
inline void checkEnemy4WorldBounds()
{
	for (int i = 0; i < MAX_ACTIVE_ENEMIES4; i++)
	{
		Enemy4 &e = enemyManager4.activeEnemies[i];
		if (!e.alive) continue;

		if (e.x < 50.0f) e.x = 50.0f;
		if (e.x > 4000.0f) e.x = 4000.0f;
	}
}

/* Handles the hero walking over a dropped Healing.png pickup: heals
   the hero (capped at 100) and removes the pickup. Pickups themselves
   are spawned in enemy4.h (EnemyManager4::spawnHealingPickup) by the
   "last Soldier of the wave" death rule. */
inline void handleLevel4HealingPickupCollisions()
{
	float px, py, pw, ph;
	getLevel4PlayerHitbox(px, py, pw, ph);

	for (int i = 0; i < MAX_HEALING_PICKUPS4; i++)
	{
		HealingPickup4 &pk = enemyManager4.healingPickups[i];
		if (!pk.active) continue;

		if (checkAABBCollision4(px, py, pw, ph,
			pk.x, pk.y, (float)HEALING4_PICKUP_WIDTH, (float)HEALING4_PICKUP_HEIGHT))
		{
			health += HEALING4_PICKUP_AMOUNT;
			if (health > 100) health = 100;
			pk.active = false;
		}
	}
}

/* Main master collision loop for Level 4 */
inline void updateLevel4Collisions()
{
	handleLevel4PlayerEnemyCollisions();
	checkEnemy4WorldBounds();
	handleLevel4HealingPickupCollisions();
}
