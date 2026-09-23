

#include "globals.h"
#include "iGraphics.h"
#include "enemy_collision.h"
#include <math.h>

/* -------- placeholder player position -------- */
float playerX = 500, playerY = 40;   // TODO: sync with real player position

/* Camera scroll offset — defined in background.h. Enemies live in
world-space (same space as playerX), so this is what converts
an enemy's world x into the screen x it should draw at. */
extern float cameraX;

/* -------- tunables -------- */
#define MAX_ENEMIES            5
#define ENEMY_IDLE_FRAMES      4
#define ENEMY_ATTACK_FRAMES    2
#define ENEMY_ANIM_SPEED       8        // frames between sprite changes (lower = faster)
#define ENEMY_ATTACK_COOLDOWN  60       // frames before an enemy can attack again
#define IGNORE_WHITE           0xFFFFFF // sprite background treated as transparent

#define PLAYER_ATTACK_RANGE    20.0f    // reach of the player's 'J' attack
#define PLAYER_ATTACK_DAMAGE   20       // Level 1 enemies lose health faster per J attack

#define ENEMY_RESPAWN_DELAY    45       // frames of pause after a knight dies before the next one appears

#define ENEMY_CHASE_RANGE      300.0f   // player must be this close before the knight starts walking over
#define ENEMY_CHASE_SPEED      1.6f     // px/frame the knight closes the gap by while chasing (hero walks at 4.0f)

enum EnemyState { ENEMY_IDLE, ENEMY_CHASE, ENEMY_ATTACKING, ENEMY_DEAD };

/* Idle / walk-stance loop */
static char enemyIdleFrames[ENEMY_IDLE_FRAMES][100] = {
	"Images\\Enemy\\Enemy_01.bmp",
	"Images\\Enemy\\Enemy_02.bmp",
	"Images\\Enemy\\Enemy_03.bmp",
	"Images\\Enemy\\Enemy_04.bmp"
};

/* Attack swing (thrust then strike) */
static char enemyAttackFrames[ENEMY_ATTACK_FRAMES][100] = {
	"Images\\Enemy\\Enemy_05.bmp",
	"Images\\Enemy\\Enemy_06.bmp"
};

/* Enemy_07.bmp and Enemy_08.bmp (overhead strike / shield-guard) are
included in the Images\Enemy folder but not wired up yet — spare
frames if a 3rd attack frame or a distinct guard pose is wanted later. */


/* =========================================================
Enemy — a single knight
========================================================= */
class Enemy
{
public:
	float x, y;              // bottom-left draw position
	int width, height;       // draw size on screen
	int hp, maxHp;
	int damage;               // damage dealt to player per attack
	float attackRange;        // distance at which the enemy starts attacking
	bool alive;
	int scoreValue;           // score awarded to player when killed

	EnemyState state;
	int currentFrame;
	int frameTimer;
	int attackCooldownTimer;

	Enemy()
	{
		alive = false;
	}

	void spawn(float px, float py, int hpVal, int dmgVal,
		float range, int scoreVal)
	{
		x = px; y = py;

		// The knight_*.bmp sprites are 100x100 px, and iShowBMP2 has
		// no width/height params of its own — it always draws a sprite
		// at its real pixel size. So these MUST match the actual bmp
		// dimensions, or the health bar (which is positioned off of
		// these) drifts away from where the sprite is really drawn.
		width = 100; height = 100;
		hp = hpVal; maxHp = hpVal;
		damage = dmgVal;
		attackRange = range;
		scoreValue = scoreVal;
		alive = true;
		state = ENEMY_IDLE;
		currentFrame = 0;
		frameTimer = 0;
		attackCooldownTimer = 0;
	}

	float distanceToPlayer() const
	{
		float dx = playerX - x;
		float dy = playerY - y;
		return sqrtf(dx * dx + dy * dy);
	}

	// Box-based version of the same idea (see enemy_collision.h):
	// is the player's hitbox overlapping this knight's box once
	// it's expanded outward by `range` pixels? Used instead of
	// distanceToPlayer() for both "start attacking" and "was the
	// player's swing a hit" — a box test reads as more forgiving/
	// natural than a circular radius for side-scroller sprites
	// that are wider than they are deep.
	bool playerInRange(float range) const
	{
		return playerInEnemyRange(x, y, (float)width, (float)height, range);
	}

	void takeDamage(int amount)
	{
		if (!alive) return;
		hp -= amount;
		if (hp <= 0)
		{
			hp = 0;
			alive = false;
			state = ENEMY_DEAD;
			score += scoreValue;   // global from globals.h
		}
	}

	// Steps x one frame closer to playerX, at ENEMY_CHASE_SPEED, without
	// overshooting/jittering past the target once it's within one step.
	void moveTowardPlayer()
	{
		float oldX = x;
		float newX = x;

		float dx = playerX - x;
		if (dx > ENEMY_CHASE_SPEED)       newX = x + ENEMY_CHASE_SPEED;
		else if (dx < -ENEMY_CHASE_SPEED) newX = x - ENEMY_CHASE_SPEED;
		else                               newX = playerX;

		// FIX: knights used to walk straight through the Level 1
		// obstacles while chasing the player. blockEnemyAtAllObstacles()
		// (enemy_collision.h) stops the step right at the obstacle's
		// edge instead of letting it cross through.
		newX = blockEnemyAtAllObstacles(oldX, newX, (float)width);
		x = newX;

		// Keep the knight from wandering off the edges of the level.
		if (x < 0) x = 0;
		if (x > LEVEL1_WIDTH - width) x = LEVEL1_WIDTH - width;
	}

	void update()
	{
		if (!alive) return;

		bool inAttackRange = playerInRange(attackRange);
		bool inChaseRange = playerInRange(ENEMY_CHASE_RANGE);
		frameTimer++;

		if (state == ENEMY_ATTACKING)
		{
			if (frameTimer >= ENEMY_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame++;
				if (currentFrame >= ENEMY_ATTACK_FRAMES)
				{
					// Attack animation finished — apply damage if the
					// player is still in range at the moment of impact.
					if (inAttackRange)
					{
						health -= damage;   // global player health from globals.h
						if (health < 0) health = 0;
					}
					currentFrame = 0;
					attackCooldownTimer = ENEMY_ATTACK_COOLDOWN;
					// Drop back to chase/idle depending on whether the
					// player is still nearby once the swing is over.
					state = inChaseRange ? ENEMY_CHASE : ENEMY_IDLE;
				}
			}
		}
		else if (state == ENEMY_CHASE)
		{
			if (attackCooldownTimer > 0)
				attackCooldownTimer--;

			// Walking loop reuses the idle frames (no separate walk
			// sprites yet) — same animation, just while x is changing.
			if (frameTimer >= ENEMY_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame = (currentFrame + 1) % ENEMY_IDLE_FRAMES;
			}

			if (inAttackRange && attackCooldownTimer == 0)
			{
				state = ENEMY_ATTACKING;
				currentFrame = 0;
				frameTimer = 0;
			}
			else if (!inChaseRange)
			{
				// Player wandered back out of chase range — give up
				// and go back to idling in place.
				state = ENEMY_IDLE;
			}
			else if (!inAttackRange)
			{
				// Still chasing: close the gap. Once in attack range
				// the knight plants its feet and swings instead of
				// walking into the player.
				moveTowardPlayer();
			}
		}
		else // ENEMY_IDLE — stays put and does nothing until the
			// player enters this knight's chase range.
		{
			if (attackCooldownTimer > 0)
				attackCooldownTimer--;

			if (frameTimer >= ENEMY_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame = (currentFrame + 1) % ENEMY_IDLE_FRAMES;
			}

			if (inAttackRange && attackCooldownTimer == 0)
			{
				state = ENEMY_ATTACKING;
				currentFrame = 0;
				frameTimer = 0;
			}
			else if (inChaseRange)
			{
				state = ENEMY_CHASE;
			}
		}
	}

	void draw()
	{
		if (!alive) return;

		// Convert this enemy's world position into a screen position
		// now that the level scrolls (see background.h).
		float screenX = x - cameraX;

		char *frameFile = (state == ENEMY_ATTACKING)
			? enemyAttackFrames[currentFrame]
			: enemyIdleFrames[currentFrame];

		iShowBMP2((int)screenX, (int)y, frameFile, IGNORE_WHITE);

		// Health bar centered directly over the knight's head. The
		// sprite canvas has a few px of transparent padding above the
		// actual helmet, and the visible knight is narrower than the
		// full 100px canvas, so the bar is inset both ways rather than
		// spanning the full sprite bounding box.
		// NOTE: top padding re-measured for the new Enemy_*.bmp art
		// (~6px, vs. 4px on the old placeholder sprites) — retune the
		// "-6" below if the bar still looks off with the new frames.
		int barW = (int)(width * 0.55f);
		int barH = 6;
		int bx = (int)screenX + (width - barW) / 2;
		int by = (int)y + height - 6 + 6;   // -6: sprite's top padding, +6: gap above head

		iSetColor(60, 20, 20);
		iFilledRectangle(bx, by, barW, barH);
		iSetColor(180, 40, 40);
		iFilledRectangle(bx, by, barW * hp / maxHp, barH);
		iSetColor(230, 210, 170);
		iRectangle(bx, by, barW, barH);
	}
};


/* =========================================================
EnemyManager — holds and drives every Enemy in the level
========================================================= */
class EnemyManager
{
public:
	// One knight is ever "live" (spawned + drawn + updated) at a
	// time. All 5 configs still live here so the encounter order
	// and per-knight tuning is unchanged from before — only *how
	// many are on screen at once* changed (was: all 5 up front,
	// now: one at a time, next one spawns after the current one
	// is defeated).
	Enemy enemies[MAX_ENEMIES];

	int defeatedCount;     // how many of the 5 have been killed so far
	int activeIndex;       // index into enemies[] currently alive, -1 = none
	bool waitingToRespawn; // true during the pause between a death and the next spawn
	int respawnTimer;

	EnemyManager() { defeatedCount = 0; activeIndex = -1; waitingToRespawn = false; respawnTimer = 0; }

	// Per-knight spawn config, in encounter order. Shifted to start
	// well clear of the hero's own spawn point (hero.h spawns the
	// hero at x=500, same ground line — the first knight used to
	// spawn at that exact same spot). Still spread across the
	// 3-screen-wide Level 1 world (world x goes from 0 to
	// LEVEL1_WIDTH = 3072) so the player meets them one after
	// another while walking right. Adjust y to match wherever the
	// ground/path sits in the level art.
	static float configX(int i)
	{
		static const float x[MAX_ENEMIES] = { 930, 1410, 2330, 2670, 2940 };
		return x[i];
	}
	static int configHp(int i)
	{
		static const int hp[MAX_ENEMIES] = { 40, 40, 45, 45, 50 };
		return hp[i];
	}
	static int configDamage(int i)
	{
		static const int dmg[MAX_ENEMIES] = { 10, 15, 15, 20, 25 };
		return dmg[i];
	}
	static int configScore(int i)
	{
		static const int score[MAX_ENEMIES] = { 25, 25, 30, 30, 35 };
		return score[i];
	}
	static const float configRange; // same attackRange (90) for every knight

	// Spawns enemies[defeatedCount] as the new active knight, unless
	// all 5 have already been defeated.
	void spawnNext()
	{
		if (defeatedCount >= MAX_ENEMIES)
		{
			activeIndex = -1;
			return;
		}

		int i = defeatedCount;
		enemies[i].spawn(configX(i), 40, configHp(i), configDamage(i),
			configRange, configScore(i));
		activeIndex = i;
	}

	// Called whenever a level/game (re)starts
	void reset()
	{
		defeatedCount = 0;
		activeIndex = -1;
		waitingToRespawn = false;
		respawnTimer = 0;

		for (int i = 0; i < MAX_ENEMIES; i++)
			enemies[i].alive = false;

		spawnNext();   // first knight (index 0) appears immediately
	}

	void init()
	{
		reset();
	}

	void updateAll()
	{
		if (activeIndex != -1)
		{
			enemies[activeIndex].update();

			// The active knight just died — start the short pause
			// before the next one (if any) is generated.
			if (!enemies[activeIndex].alive && !waitingToRespawn)
			{
				defeatedCount++;
				activeIndex = -1;
				waitingToRespawn = true;
				respawnTimer = ENEMY_RESPAWN_DELAY;
			}
		}

		if (waitingToRespawn)
		{
			respawnTimer--;
			if (respawnTimer <= 0)
			{
				waitingToRespawn = false;
				spawnNext();
			}
		}

		if (health <= 0)
		{
			health = 0;
			currentState = GAME_OVER;
		}

		// Level 1 is complete only once all five knights have been
		// generated and defeated in turn (not just "none alive right
		// now", since that's also briefly true during the respawn pause).
		bool allDefeated = (defeatedCount >= MAX_ENEMIES) && !waitingToRespawn && activeIndex == -1;

		if (allDefeated && currentState == PLAYING)
		{
			currentState = STORY_ENDING_STATE;
			storyStart(STORY_ENDING);
		}
	}

	void drawAll()
	{
		if (activeIndex != -1)
			enemies[activeIndex].draw();
	}

	// Called when the player presses the attack key ('J')
	void playerAttack()
	{
		if (activeIndex == -1) return; // no knight generated right now

		Enemy &e = enemies[activeIndex];
		if (!e.alive) return;

		if (e.playerInRange(PLAYER_ATTACK_RANGE))
			e.takeDamage(PLAYER_ATTACK_DAMAGE);
	}
};

const float EnemyManager::configRange = 40.0f;

/* Single global instance — this is what iMain.cpp talks to */
EnemyManager enemyManager;

/* Thin wrapper functions so the call sites read the same as
plain function calls (initEnemies(), updateEnemies(), ...) */
inline void initEnemies()          { enemyManager.init(); }
inline void resetEnemies()         { enemyManager.reset(); }
inline void updateEnemies()        { enemyManager.updateAll(); }
inline void drawEnemies()          { enemyManager.drawAll(); }
inline void playerAttackEnemies()  { enemyManager.playerAttack(); }
