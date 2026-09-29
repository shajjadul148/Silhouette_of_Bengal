#pragma once

#include "globals.h"
#include "enemy4.h"

/* =========================================================
LEVEL 4 COLLISION SYSTEM

This file keeps all Level 4 collision handling together:
1. Hero <-> enemy body collision
2. Hero gun bullet -> Enemy4 collision
   Hero4 owns projectile movement/visuals; this file only decides
   whether the moving projectile overlaps an enemy.
3. Enemy movement bounds
4. Healing pickup collision
5. Final Level 4 completion -> Level 4 ending story

Enemy gun bullets already use EnemyManager4::updateBullets()
for their movement and Hero hit detection. That code reduces
the global `health`, so the existing HUD health bar immediately
reflects every successful enemy-bullet hit.
========================================================= */

/* Bounding box inset constants for Level 4 entities */
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
inline bool checkAABBCollision4(
    float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2)
{
    return (
        x1 < x2 + w2 &&
        x1 + w1 > x2 &&
        y1 < y2 + h2 &&
        y1 + h1 > y2
        );
}

/* ---------------------------------------------------------
   HERO HITBOX
   --------------------------------------------------------- */
inline void getLevel4PlayerHitbox(
    float &px,
    float &py,
    float &pw,
    float &ph)
{
    px =
        playerX +
        HERO4_INSET_LEFT;

    py =
        playerY +
        HERO4_INSET_BOTTOM;

    pw =
        100.0f -
        (
            HERO4_INSET_LEFT +
            HERO4_INSET_RIGHT
        );

    ph =
        100.0f -
        (
            HERO4_INSET_BOTTOM +
            HERO4_INSET_TOP
        );
}

/* ---------------------------------------------------------
   ENEMY HITBOX
   --------------------------------------------------------- */
inline void getEnemy4Hitbox(
    const Enemy4 &e,
    float &ex,
    float &ey,
    float &ew,
    float &eh)
{
    if (e.type == TYPE4_BLUE)
    {
        ex =
            e.x +
            ENEMY4_BLUE_INSET_X;

        ey =
            e.y +
            ENEMY4_BLUE_INSET_Y;

        ew =
            e.width -
            (
                2.0f *
                ENEMY4_BLUE_INSET_X
            );

        eh =
            e.height -
            (
                2.0f *
                ENEMY4_BLUE_INSET_Y
            );
    }
    else if (e.type == TYPE4_SOLDIER)
    {
        ex =
            e.x +
            SOLDIER4_INSET_X;

        ey =
            e.y +
            SOLDIER4_INSET_Y;

        ew =
            e.width -
            (
                2.0f *
                SOLDIER4_INSET_X
            );

        eh =
            e.height -
            (
                2.0f *
                SOLDIER4_INSET_Y
            );
    }
    else
    {
        /* TYPE4_BOSS and TYPE4_BOSS2 */
        ex =
            e.x +
            BOSS4_INSET_X;

        ey =
            e.y +
            BOSS4_INSET_Y;

        ew =
            e.width -
            (
                2.0f *
                BOSS4_INSET_X
            );

        eh =
            e.height -
            (
                2.0f *
                BOSS4_INSET_Y
            );
    }
}

/* ---------------------------------------------------------
   HERO <-> ENEMY BODY COLLISION
   --------------------------------------------------------- */
inline void handleLevel4PlayerEnemyCollisions()
{
    float px, py, pw, ph;

    getLevel4PlayerHitbox(
        px,
        py,
        pw,
        ph
        );

    for (
        int i = 0;
        i < MAX_ACTIVE_ENEMIES4;
        i++
        )
    {
        Enemy4 &e =
            enemyManager4.activeEnemies[i];

        if (!e.alive)
            continue;

        float ex, ey, ew, eh;

        getEnemy4Hitbox(
            e,
            ex,
            ey,
            ew,
            eh
            );

        if (
            checkAABBCollision4(
                px, py, pw, ph,
                ex, ey, ew, eh
                )
            )
        {
            /* Push the two bodies apart instead of allowing
               them to occupy the same position. */
            if (playerX < e.x)
            {
                playerX -= 2.0f;

                if (
                    e.type == TYPE4_BLUE ||
                    e.type == TYPE4_BOSS ||
                    e.type == TYPE4_BOSS2
                    )
                {
                    e.x += 1.0f;
                }
            }
            else
            {
                playerX += 2.0f;

                if (
                    e.type == TYPE4_BLUE ||
                    e.type == TYPE4_BOSS ||
                    e.type == TYPE4_BOSS2
                    )
                {
                    e.x -= 1.0f;
                }
            }
        }
    }
}

/* ---------------------------------------------------------
   HERO GUN BULLET -> ENEMY COLLISION

   Every successful hit calls Enemy4::takeBulletHit(), which
   removes 1/N of the enemy's max HP. N = bullets needed to
   kill: normal 3, soldier 5, boss1 7, boss2 10.

   Bosses keep the existing escort-shield rule:
   Boss1 cannot be damaged until all four escorts are dead.
   Boss2 cannot be damaged until all six escorts are dead.
   The bullet is still consumed when it hits a shielded boss.
   --------------------------------------------------------- */
inline void handleLevel4HeroBulletCollisions()
{
    for (
        int b = 0;
        b < HERO4_MAX_BULLETS;
        b++
        )
    {
        if (!hero4Bullets[b].active)
            continue;

        for (
            int i = 0;
            i < MAX_ACTIVE_ENEMIES4;
            i++
            )
        {
            Enemy4 &e =
                enemyManager4.activeEnemies[i];

            if (!e.alive)
                continue;

            /* Preserve the existing boss escort protection. */
            if (
                e.type == TYPE4_BOSS &&
                enemyManager4.waveEscortsDefeated <
                BOSS4_ESCORT_COUNT
                )
            {
                float ex, ey, ew, eh;

                getEnemy4Hitbox(
                    e,
                    ex, ey, ew, eh
                    );

                if (
                    checkAABBCollision4(
                        hero4Bullets[b].x,
                        hero4Bullets[b].y,
                        (float)hero4Bullets[b].width,
                        (float)hero4Bullets[b].height,
                        ex, ey, ew, eh
                        )
                    )
                {
                    hero4Bullets[b].active =
                        false;

                    break;
                }

                continue;
            }

            if (
                e.type == TYPE4_BOSS2 &&
                enemyManager4.waveEscorts2Defeated <
                BOSS2_ESCORT_COUNT
                )
            {
                float ex, ey, ew, eh;

                getEnemy4Hitbox(
                    e,
                    ex, ey, ew, eh
                    );

                if (
                    checkAABBCollision4(
                        hero4Bullets[b].x,
                        hero4Bullets[b].y,
                        (float)hero4Bullets[b].width,
                        (float)hero4Bullets[b].height,
                        ex, ey, ew, eh
                        )
                    )
                {
                    hero4Bullets[b].active =
                        false;

                    break;
                }

                continue;
            }

            float ex, ey, ew, eh;

            getEnemy4Hitbox(
                e,
                ex, ey, ew, eh
                );

            bool hit =
                checkAABBCollision4(
                    hero4Bullets[b].x,
                    hero4Bullets[b].y,
                    (float)hero4Bullets[b].width,
                    (float)hero4Bullets[b].height,
                    ex, ey, ew, eh
                    );

            if (!hit)
                continue;

            /* A wall/fort cover can block the projectile. */
            if (
                isLevel4CoverBetween(
                    playerX,
                    playerY,
                    e.x,
                    e.y
                    )
                )
            {
                hero4Bullets[b].active =
                    false;

                break;
            }

            /* Bullet hit: 3 (normal) / 5 (soldier) / 7 (boss1) / 10 (boss2) hits to kill. */
            e.takeBulletHit();

            hero4Bullets[b].active =
                false;

            break;
        }

        /* Remove bullets that travel beyond the Level 4 world. */
        if (hero4Bullets[b].active)
        {
            if (
                hero4Bullets[b].x < -100.0f ||
                hero4Bullets[b].x >
                (float)LEVEL4_WIDTH + 100.0f
                )
            {
                hero4Bullets[b].active =
                    false;
            }
        }
    }
}

/* ---------------------------------------------------------
   ENEMY WORLD BOUNDS
   --------------------------------------------------------- */
inline void checkEnemy4WorldBounds()
{
    for (
        int i = 0;
        i < MAX_ACTIVE_ENEMIES4;
        i++
        )
    {
        Enemy4 &e =
            enemyManager4.activeEnemies[i];

        if (!e.alive)
            continue;

        if (e.x < 50.0f)
            e.x = 50.0f;

        if (e.x > 4000.0f)
            e.x = 4000.0f;
    }
}

/* ---------------------------------------------------------
   HEALING PICKUP COLLISION
   --------------------------------------------------------- */
inline void handleLevel4HealingPickupCollisions()
{
    float px, py, pw, ph;

    getLevel4PlayerHitbox(
        px,
        py,
        pw,
        ph
        );

    for (
        int i = 0;
        i < MAX_HEALING_PICKUPS4;
        i++
        )
    {
        HealingPickup4 &pk =
            enemyManager4.healingPickups[i];

        if (!pk.active)
            continue;

        if (
            checkAABBCollision4(
                px, py, pw, ph,
                pk.x,
                pk.y,
                (float)HEALING4_PICKUP_WIDTH,
                (float)HEALING4_PICKUP_HEIGHT
                )
            )
        {
            health +=
                HEALING4_PICKUP_AMOUNT;

            if (health > 100)
                health = 100;

            pk.active =
                false;
        }
    }
}

/* ---------------------------------------------------------
   MASTER LEVEL 4 COLLISION LOOP
   --------------------------------------------------------- */
inline void updateLevel4Collisions()
{
    handleLevel4PlayerEnemyCollisions();

    /* Hero bullets kill enemies. */
    handleLevel4HeroBulletCollisions();

    checkEnemy4WorldBounds();

    handleLevel4HealingPickupCollisions();

    /* EnemyManager4 changes the state to LEVEL_COMPLETE when
       the final Boss_2 is defeated. Immediately replace that
       screen with the Level 4 ending story so the after-Level-4
       caption + Level4Audio2.wav play before the final
       completion screen. */
    if (
        currentState == LEVEL_COMPLETE &&
        enemyManager4.waveState == L4_COMPLETE
        )
    {
        storyStart(
            STORY_LEVEL4_END
            );
    }
}
