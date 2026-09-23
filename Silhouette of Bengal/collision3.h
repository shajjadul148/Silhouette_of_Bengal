#pragma once

#include "globals.h"
#include "enemy3.h"

/* Bounding box inset constants for Level 3 entities (HERO_INSET_*,
   ENEMY3_BLUE_INSET_*, SOLDIER_INSET_*, BOSS_INSET_*) now live in
   enemy3.h, which this file includes above -- Enemy3::playerInRange()
   needs them too, and enemy3.h can't include this file back (this
   file already includes enemy3.h for the Enemy3 type), so they were
   moved there as the single source of truth instead of being
   duplicated in both places. */

/* Axis-Aligned Bounding Box (AABB) Overlap Helper */
inline bool checkAABBCollision(float x1, float y1, float w1, float h1,
	float x2, float y2, float w2, float h2)
{
	return (x1 < x2 + w2 &&
		x1 + w1 > x2 &&
		y1 < y2 + h2 &&
		y1 + h1 > y2);
}

/* Get player precise collision box */
inline void getLevel3PlayerHitbox(float &px, float &py, float &pw, float &ph)
{
	px = playerX + HERO_INSET_LEFT;
	py = playerY + HERO_INSET_BOTTOM;
	pw = 100.0f - (HERO_INSET_LEFT + HERO_INSET_RIGHT);
	ph = 100.0f - (HERO_INSET_BOTTOM + HERO_INSET_TOP);
}

/* Get Enemy3 precise collision box */
inline void getEnemy3Hitbox(const Enemy3 &e, float &ex, float &ey, float &ew, float &eh)
{
	if (e.type == TYPE_BLUE)
	{
		ex = e.x + ENEMY3_BLUE_INSET_X;
		ey = e.y + ENEMY3_BLUE_INSET_Y;
		ew = e.width - (2.0f * ENEMY3_BLUE_INSET_X);
		eh = e.height - (2.0f * ENEMY3_BLUE_INSET_Y);
	}
	else if (e.type == TYPE_SOLDIER)
	{
		ex = e.x + SOLDIER_INSET_X;
		ey = e.y + SOLDIER_INSET_Y;
		ew = e.width - (2.0f * SOLDIER_INSET_X);
		eh = e.height - (2.0f * SOLDIER_INSET_Y);
	}
	else // TYPE_BOSS
	{
		ex = e.x + BOSS_INSET_X;
		ey = e.y + BOSS_INSET_Y;
		ew = e.width - (2.0f * BOSS_INSET_X);
		eh = e.height - (2.0f * BOSS_INSET_Y);
	}
}

/* Handles physical body collisions and bounds between Hero and Active Level 3 Enemies */
inline void handleLevel3PlayerEnemyCollisions()
{
	float px, py, pw, ph;
	getLevel3PlayerHitbox(px, py, pw, ph);

	for (int i = 0; i < MAX_ACTIVE_ENEMIES3; i++)
	{
		Enemy3 &e = enemyManager3.activeEnemies[i];
		if (!e.alive) continue;

		float ex, ey, ew, eh;
		getEnemy3Hitbox(e, ex, ey, ew, eh);

		if (checkAABBCollision(px, py, pw, ph, ex, ey, ew, eh))
		{
			/* Simple push-back physics to avoid overlapping bodies */
			if (playerX < e.x)
			{
				playerX -= 2.0f;
				if (e.type == TYPE_BLUE || e.type == TYPE_BOSS) e.x += 1.0f;
			}
			else
			{
				playerX += 2.0f;
				if (e.type == TYPE_BLUE || e.type == TYPE_BOSS) e.x -= 1.0f;
			}
		}
	}
}

/* Restricts Level 3 Enemies from walking off edge bounds */
inline void checkEnemy3WorldBounds()
{
	for (int i = 0; i < MAX_ACTIVE_ENEMIES3; i++)
	{
		Enemy3 &e = enemyManager3.activeEnemies[i];
		if (!e.alive) continue;

		if (e.x < 50.0f) e.x = 50.0f;
		if (e.x > LEVEL3_WIDTH - 100.0f) e.x = LEVEL3_WIDTH - 100.0f;
	}
}

/* Handles the hero walking over a dropped Healing.png pickup: heals
   the hero (capped at 100) and removes the pickup. Pickups themselves
   are spawned in enemy3.h (EnemyManager3::spawnHealingPickup) by the
   "last Soldier of the wave" death rule. */
inline void handleLevel3HealingPickupCollisions()
{
	float px, py, pw, ph;
	getLevel3PlayerHitbox(px, py, pw, ph);

	for (int i = 0; i < MAX_HEALING_PICKUPS; i++)
	{
		HealingPickup &pk = enemyManager3.healingPickups[i];
		if (!pk.active) continue;

		if (checkAABBCollision(px, py, pw, ph,
			pk.x, pk.y, (float)HEALING_PICKUP_WIDTH, (float)HEALING_PICKUP_HEIGHT))
		{
			health += HEALING_PICKUP_AMOUNT;
			if (health > 100) health = 100;
			pk.active = false;
		}
	}
}

/* Main master collision loop for Level 3 */
inline void updateLevel3Collisions()
{
	handleLevel3PlayerEnemyCollisions();
	checkEnemy3WorldBounds();
	handleLevel3HealingPickupCollisions();
}