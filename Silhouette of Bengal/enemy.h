

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
#define ENEMY_WALK_FRAMES      4
#define ENEMY_FIGHT_FRAMES     5
#define ENEMY_ANIM_SPEED       8        // frames between sprite changes (lower = faster)
#define ENEMY_ATTACK_COOLDOWN  60       // frames before an enemy can attack again
#define ENEMY_COLORKEY_TOLERANCE 24     // how close to pure white still counts as "background"
#define IGNORE_WHITE           0xFFFFFF // (legacy) BMP transparent colour, no longer used by Level 1 enemies

#define ENEMY_DRAW_WIDTH       100
#define ENEMY_DRAW_HEIGHT      100
#define ENEMY_GROUND_Y         40.0f    // same ground line as hero.h's HERO_GROUND_Y

#define PLAYER_ATTACK_RANGE    20.0f    // reach of the player's 'J' attack
#define PLAYER_ATTACK_DAMAGE   20       // Level 1 enemies lose health faster per J attack

#define ENEMY_RESPAWN_DELAY    45       // frames of pause after a soldier dies before the next one appears

#define ENEMY_CHASE_RANGE      300.0f   // "detection range" -- player must be this close before the soldier breaks off and engages
#define ENEMY_CHASE_SPEED      1.6f     // px/frame the soldier closes the gap by while chasing (hero walks at 4.0f)

/* Same standing-guard / shadow-the-hero behaviour as Level 2 (enemy2.h). */
#define ENEMY_PATROL_SPEED     1.2f     // px/frame once a soldier starts shadowing the hero (post-pass, outside detection range)
#define ENEMY_PATROL_DEADZONE  6.0f     // how close to the hero's x counts as "level with him" (stand still, don't jitter)
#define ENEMY_PASS_MARGIN      80.0f    // how far the hero must walk past a soldier's post before it starts shadowing him

enum EnemyFacing { ENEMY_FACING_LEFT = -1, ENEMY_FACING_RIGHT = 1 };
enum EnemyState { ENEMY_STANDING, ENEMY_PATROL, ENEMY_CHASE, ENEMY_ATTACKING, ENEMY_DEAD };

/* -------- sprite filenames (Images\\Enemy1\\...) -------- */
static char enemyWalkLeftFiles[ENEMY_WALK_FRAMES][100] = {
	"Images\\Enemy1\\Walking\\walking_left_1.png",
	"Images\\Enemy1\\Walking\\walking_left_2.png",
	"Images\\Enemy1\\Walking\\walking_left_3.png",
	"Images\\Enemy1\\Walking\\walking_left_4.png"
};
static char enemyWalkRightFiles[ENEMY_WALK_FRAMES][100] = {
	"Images\\Enemy1\\Walking\\walking_right_1.png",
	"Images\\Enemy1\\Walking\\walking_right_2.png",
	"Images\\Enemy1\\Walking\\walking_right_3.png",
	"Images\\Enemy1\\Walking\\walking_right_4.png"
};
static char enemyFightLeftFiles[ENEMY_FIGHT_FRAMES][100] = {
	"Images\\Enemy1\\Fighting\\fighting_left_1.png",
	"Images\\Enemy1\\Fighting\\fighting_left_2.png",
	"Images\\Enemy1\\Fighting\\fighting_left_3.png",
	"Images\\Enemy1\\Fighting\\fighting_left_4.png",
	"Images\\Enemy1\\Fighting\\fighting_left_5.png"
};
static char enemyFightRightFiles[ENEMY_FIGHT_FRAMES][100] = {
	"Images\\Enemy1\\Fighting\\fighting_right_1.png",
	"Images\\Enemy1\\Fighting\\fighting_right_2.png",
	"Images\\Enemy1\\Fighting\\fighting_right_3.png",
	"Images\\Enemy1\\Fighting\\fighting_right_4.png",
	"Images\\Enemy1\\Fighting\\fighting_right_5.png"
};
static char enemyStandLeftFile[100]  = "Images\\Enemy1\\Standing\\standing_left.png";
static char enemyStandRightFile[100] = "Images\\Enemy1\\Standing\\standing_right.png";

/* -------- loaded textures (filled in by initEnemyTextures()) -------- */
static unsigned int enemyWalkLeftTex[ENEMY_WALK_FRAMES];
static unsigned int enemyWalkRightTex[ENEMY_WALK_FRAMES];
static unsigned int enemyFightLeftTex[ENEMY_FIGHT_FRAMES];
static unsigned int enemyFightRightTex[ENEMY_FIGHT_FRAMES];
static unsigned int enemyStandLeftTex;
static unsigned int enemyStandRightTex;

/* loadEnemyTexture -- same white-colorkey trick as enemy2.h / hero.h:
   the Images\\Enemy1 frames are flat-white-background PNGs, so any pixel
   close to pure white is made fully transparent before upload. */
inline unsigned int loadEnemyTexture(char filename[])
{
	int w, h, channels;
	unsigned char *data = stbi_load(filename, &w, &h, &channels, 4);
	if (!data)
		return 0; // missing/bad file -- draws nothing rather than crashing

	int nPixels = w * h;
	for (int i = 0; i < nPixels; i++)
	{
		unsigned char *p = data + i * 4;
		bool nearWhite =
			p[0] >= 255 - ENEMY_COLORKEY_TOLERANCE &&
			p[1] >= 255 - ENEMY_COLORKEY_TOLERANCE &&
			p[2] >= 255 - ENEMY_COLORKEY_TOLERANCE;
		p[3] = nearWhite ? 0 : 255;
	}

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, data);

	stbi_image_free(data);
	return texture;
}

inline void initEnemyTextures()
{
	for (int i = 0; i < ENEMY_WALK_FRAMES; i++)
	{
		enemyWalkLeftTex[i]  = loadEnemyTexture(enemyWalkLeftFiles[i]);
		enemyWalkRightTex[i] = loadEnemyTexture(enemyWalkRightFiles[i]);
	}
	for (int i = 0; i < ENEMY_FIGHT_FRAMES; i++)
	{
		enemyFightLeftTex[i]  = loadEnemyTexture(enemyFightLeftFiles[i]);
		enemyFightRightTex[i] = loadEnemyTexture(enemyFightRightFiles[i]);
	}
	enemyStandLeftTex  = loadEnemyTexture(enemyStandLeftFile);
	enemyStandRightTex = loadEnemyTexture(enemyStandRightFile);
}

/* drawEnemyTexture -- textured quad (left/right art is separate, so the
   caller picks the right texture up front instead of mirroring). */
inline void drawEnemyTexture(int x, int y, int w, int h, unsigned int texture)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glBegin(GL_QUADS);
		glTexCoord2f(0, 0);  glVertex2f(x, y);
		glTexCoord2f(1, 0);  glVertex2f(x + w, y);
		glTexCoord2f(1, -1); glVertex2f(x + w, y + h);
		glTexCoord2f(0, -1); glVertex2f(x, y + h);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}


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

	int facing;               // ENEMY_FACING_LEFT / ENEMY_FACING_RIGHT
	EnemyState state;
	int currentFrame;
	int frameTimer;
	int attackCooldownTimer;

	bool hasPassedPlayer;     // true once the hero has walked past this soldier's post

	Enemy()
	{
		alive = false;
	}

	void spawn(float px, float py, int hpVal, int dmgVal,
		float range, int scoreVal)
	{
		x = px; y = py;

		// The sprites are 100x100 px and are drawn 1:1, so the health
		// bar (positioned off width/height) sits right over the head.
		width = ENEMY_DRAW_WIDTH; height = ENEMY_DRAW_HEIGHT;
		hp = hpVal; maxHp = hpVal;
		damage = dmgVal;
		attackRange = range;
		scoreValue = scoreVal;
		alive = true;

		facing = ENEMY_FACING_RIGHT;
		state = ENEMY_STANDING;
		hasPassedPlayer = false;
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

	// Box-based range test (see enemy_collision.h): is the player's
	// hitbox overlapping this soldier's box once it's expanded outward
	// by `range` pixels? Used for both "start attacking" and "was the
	// player's swing a hit".
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

	/* Turns to face whichever side the hero is CURRENTLY on, with
	   ENEMY_PATROL_DEADZONE as hysteresis so the art doesn't flicker
	   left/right while the hero is level with the soldier. Every
	   walk/fight/stand pose goes through this. */
	void faceTowardPlayer()
	{
		float dx = playerX - x;
		if (dx > ENEMY_PATROL_DEADZONE)
			facing = ENEMY_FACING_RIGHT;
		else if (dx < -ENEMY_PATROL_DEADZONE)
			facing = ENEMY_FACING_LEFT;
	}

	/* "Shadow the hero": one step toward whichever side he is on, hold
	   still once roughly level with him. */
	void trackPlayerSide()
	{
		float oldX = x;
		float newX = x;
		float dx = playerX - x;
		faceTowardPlayer();

		if (dx > ENEMY_PATROL_DEADZONE)
			newX = x + ENEMY_PATROL_SPEED;
		else if (dx < -ENEMY_PATROL_DEADZONE)
			newX = x - ENEMY_PATROL_SPEED;

		// Soldiers can't walk through the Level 1 obstacles.
		newX = blockEnemyAtAllObstacles(oldX, newX, (float)width);
		x = newX;

		if (x < 0) x = 0;
		if (x > LEVEL1_WIDTH - width) x = LEVEL1_WIDTH - width;
	}

	// Steps x one frame closer to playerX (from either side), at
	// ENEMY_CHASE_SPEED, without overshooting/jittering.
	void moveTowardPlayer()
	{
		float oldX = x;
		float newX = x;
		float dx = playerX - x;
		faceTowardPlayer();

		if (dx > ENEMY_CHASE_SPEED)       newX = x + ENEMY_CHASE_SPEED;
		else if (dx < -ENEMY_CHASE_SPEED) newX = x - ENEMY_CHASE_SPEED;
		else                               newX = playerX;

		// blockEnemyAtAllObstacles() (enemy_collision.h) stops the step
		// right at the obstacle's edge instead of crossing through it.
		newX = blockEnemyAtAllObstacles(oldX, newX, (float)width);
		x = newX;

		// Keep the soldier from wandering off the edges of the level.
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
				if (currentFrame >= ENEMY_FIGHT_FRAMES)
				{
					// Attack animation finished -- apply damage if the
					// player is still in range at the moment of impact.
					if (inAttackRange)
					{
						health -= damage;   // global player health from globals.h
						if (health < 0) health = 0;
					}
					currentFrame = 0;
					attackCooldownTimer = ENEMY_ATTACK_COOLDOWN;

					if (inChaseRange)
						state = ENEMY_CHASE;
					else if (hasPassedPlayer)
						state = ENEMY_PATROL;
					else
						state = ENEMY_STANDING;
				}
			}
			return;
		}

		if (state == ENEMY_CHASE)
		{
			if (attackCooldownTimer > 0)
				attackCooldownTimer--;

			if (frameTimer >= ENEMY_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame = (currentFrame + 1) % ENEMY_WALK_FRAMES;
			}

			if (inAttackRange && attackCooldownTimer == 0)
			{
				faceTowardPlayer();
				state = ENEMY_ATTACKING;
				currentFrame = 0;
				frameTimer = 0;
			}
			else if (!inChaseRange)
			{
				// Player got away -- back to pacing if the hero is
				// already past, otherwise back to standing guard.
				state = hasPassedPlayer ? ENEMY_PATROL : ENEMY_STANDING;
			}
			else
			{
				moveTowardPlayer();
			}
			return;
		}

		/* ---- STANDING / PATROL: a nearby hero always takes priority ---- */
		if (attackCooldownTimer > 0)
			attackCooldownTimer--;

		if (inAttackRange && attackCooldownTimer == 0)
		{
			faceTowardPlayer();
			state = ENEMY_ATTACKING;
			currentFrame = 0;
			frameTimer = 0;
			return;
		}
		if (inChaseRange)
		{
			state = ENEMY_CHASE;
			return;
		}

		// Once the hero has walked far enough past this soldier's post
		// it starts shadowing him. Only ever flips on, never back off.
		if (!hasPassedPlayer && (playerX - x) > ENEMY_PASS_MARGIN)
			hasPassedPlayer = true;

		if (state == ENEMY_STANDING)
		{
			currentFrame = 0; // idle pose
			faceTowardPlayer();  // watches the hero go by

			if (hasPassedPlayer)
				state = ENEMY_PATROL;
			return;
		}

		// ENEMY_PATROL
		{
			bool moving = fabs(playerX - x) > ENEMY_PATROL_DEADZONE;
			trackPlayerSide();

			if (moving && frameTimer >= ENEMY_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame = (currentFrame + 1) % ENEMY_WALK_FRAMES;
			}
			else if (!moving)
			{
				currentFrame = 0; // standing pose while level with the hero
			}
		}
	}

	void draw()
	{
		if (!alive) return;

		// World position -> screen position (level scrolls, see background.h).
		float screenX = x - cameraX;

		unsigned int tex;
		if (state == ENEMY_ATTACKING)
		{
			tex = (facing == ENEMY_FACING_LEFT)
				? enemyFightLeftTex[currentFrame]
				: enemyFightRightTex[currentFrame];
		}
		else if (state == ENEMY_STANDING ||
			(state == ENEMY_PATROL && fabs(playerX - x) <= ENEMY_PATROL_DEADZONE))
		{
			tex = (facing == ENEMY_FACING_LEFT) ? enemyStandLeftTex : enemyStandRightTex;
		}
		else // PATROL-moving or CHASE
		{
			tex = (facing == ENEMY_FACING_LEFT)
				? enemyWalkLeftTex[currentFrame]
				: enemyWalkRightTex[currentFrame];
		}

		drawEnemyTexture((int)screenX, (int)y, width, height, tex);

		// Health bar over the soldier's head (same proportions as before).
		int barW = (int)(width * 0.55f);
		int barH = 6;
		int bx = (int)screenX + (width - barW) / 2;
		int by = (int)y + height - 6 + 6;

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
		enemies[i].spawn(configX(i), ENEMY_GROUND_Y, configHp(i), configDamage(i),
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
		initEnemyTextures();
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
