

#include "globals.h"
#include "iGraphics.h"
#include "collision2.h"
#include <math.h>
#include <stdlib.h>

/* playerX / playerY are DEFINED in enemy.h; cameraX is DEFINED
   in background.h. Declared extern here (rather than by
   #include-ing those headers) so this file doesn't care what
   order it's included in relative to them -- only that all three
   end up in the same iMain.cpp translation unit, which they
   already do. */
extern float playerX, playerY;
extern float cameraX;

/* -------- tunables -------- */
#define MAX_ENEMIES2            10
#define ENEMY2_WALK_FRAMES       3
#define ENEMY2_FIGHT_FRAMES      5
#define ENEMY2_ANIM_SPEED        6        // frames between sprite changes (lower = faster)
#define ENEMY2_ATTACK_COOLDOWN  50         // frames before a knight can attack again
#define ENEMY2_COLORKEY_TOLERANCE 24       // how close to pure white still counts as "background"

#define ENEMY2_DRAW_WIDTH       100
#define ENEMY2_DRAW_HEIGHT      100
#define ENEMY2_GROUND_Y          40.0f     // same ground line as hero.h's HERO_GROUND_Y

#define PLAYER2_ATTACK_RANGE    20.0f      // reach of the player's 'J' attack
#define PLAYER2_ATTACK_DAMAGE   15

#define ENEMY2_RESPAWN_DELAY     45        // frames of pause after a knight dies before the next one appears
#define ENEMY2_GATE_RESPAWN_DELAY 10      // shorter pause used once the last ENEMY2_LAST_GATE_COUNT (gate) knights start spawning -- keeps that final stretch feeling faster-paced

#define ENEMY2_PATROL_SPEED       1.2f     // px/frame once a knight starts shadowing the hero (post-pass, outside detection range)
#define ENEMY2_PATROL_DEADZONE    6.0f     // how close to the hero's x counts as "level with him" (stand still, don't jitter)
#define ENEMY2_CHASE_RANGE      260.0f     // "detection range" -- player must be this close before a knight breaks off and engages
#define ENEMY2_CHASE_SPEED        2.0f     // px/frame while chasing or walking in from the gate (hero walks at 4.0f)

/* How far the hero has to have walked past a knight's post (in the
   +x direction) before that knight is considered "passed" and
   starts shadowing him (trackPlayerSide()) instead of just standing
   guard. Tuned a bit wider than the knight's own draw width so the
   hero visibly has to get by it first. */
#define ENEMY2_PASS_MARGIN       80.0f

/* The last ENEMY2_LAST_GATE_COUNT knights in the encounter order
   spawn from "inside the gate" and walk out to their fighting-ground
   post instead of just appearing there.

   ENEMY2_GATE_X is tied to the gate background art (see Level2.h /
   Images\Level2\level2_bg_3.png, which is drawn as the 3rd
   background segment -- world x [2*SCREEN_WIDTH, 3*SCREEN_WIDTH)).
   The dark doorway gap in that art (the ajar wooden door, with the
   unlit interior visible beside it) sits roughly
   ENEMY2_GATE_DOOR_OFFSET_X px into that segment -- checked directly
   against the art's pixels -- so that's where these knights first
   appear. Retune ENEMY2_GATE_DOOR_OFFSET_X (and the
   ENEMY2_GATE_OVERLAY_* block right below) together if the
   background art ever changes. */
#define ENEMY2_GATE_SEGMENT_WORLD_X (2.0f * SCREEN_WIDTH)
#define ENEMY2_GATE_DOOR_OFFSET_X   500.0f
#define ENEMY2_GATE_X               (ENEMY2_GATE_SEGMENT_WORLD_X + ENEMY2_GATE_DOOR_OFFSET_X)
#define ENEMY2_LAST_GATE_COUNT   4

/* "Duplicate gate" occlusion patch -- makes gate-spawned knights look
   like they're stepping OUT of the dark doorway instead of just
   appearing in front of it.

   Images\level2\level2_gate_overlay.png is a straight pixel crop of
   Images\level2\level2_bg_3.png (same doorway/door/pillar chunk, same
   segment), so it lines up perfectly with the real background behind
   it and is invisible on its own. EnemyManager2::drawAll() redraws it
   every frame AFTER the active knight, so any part of that knight's
   sprite currently under this patch gets painted over and hidden --
   i.e. while a gate knight is still near ENEMY2_GATE_X it's masked by
   this patch, and as it walks (in x) toward its post it slides out
   from under the patch and becomes visible, looking exactly like it
   stepped out from inside the gate. Level2.h's drawLevel2Background()
   -> EnemyManager2::drawAll() -> drawHero() ordering in iMain.cpp
   means the hero (drawn last, always) is never hidden by this patch.

   The four numbers below are that crop's position/size: LOCAL_X/Y
   place its bottom-left corner within segment 3 (X measured the same
   way as ENEMY2_GATE_DOOR_OFFSET_X above; Y in the same bottom-up
   coordinate space enemy/hero drawing already uses, 0 = screen
   bottom). W/H are the crop's pixel size. Retune all four -- and
   re-crop the PNG -- if the background art changes. */
#define ENEMY2_GATE_OVERLAY_FILE    "Images\\level2\\level2_gate_overlay.png"
#define ENEMY2_GATE_OVERLAY_LOCAL_X 410.0f
#define ENEMY2_GATE_OVERLAY_Y        30.0f
#define ENEMY2_GATE_OVERLAY_W       280
#define ENEMY2_GATE_OVERLAY_H       270

/* Level-end trigger: once all MAX_ENEMIES2 knights are defeated, the
   level is complete right away (see EnemyManager2::updateAll()).

   NOTE: this used to instead wait for the hero to walk past
   ENEMY2_GATE_X (into the gate opening) before completing the level,
   but that x sits inside the world-space range that
   ENEMY2_GATE_OVERLAY_* redraws every frame (drawAll() paints that
   patch AFTER the active knight but is itself still drawn UNDER
   drawHero2() every frame up until the level actually completes) --
   so the hero had to visibly walk through/under that gate patch
   first, which read as a glitch rather than "entering the gate".
   Ending the level immediately on the last kill avoids that
   walk-through entirely. */

enum Enemy2Facing { ENEMY2_FACING_LEFT = -1, ENEMY2_FACING_RIGHT = 1 };
enum Enemy2State { ENEMY2_ENTERING, ENEMY2_STANDING, ENEMY2_PATROL, ENEMY2_CHASE, ENEMY2_ATTACKING, ENEMY2_DEAD };

/* -------- sprite filenames -------- */
static char enemy2WalkLeftFiles[ENEMY2_WALK_FRAMES][100] = {
	"Images\\Enemy2\\Walking\\walking_left_1.png",
	"Images\\Enemy2\\Walking\\walking_left_2.png",
	"Images\\Enemy2\\Walking\\walking_left_3.png"
};
static char enemy2WalkRightFiles[ENEMY2_WALK_FRAMES][100] = {
	"Images\\Enemy2\\Walking\\walking_right_1.png",
	"Images\\Enemy2\\Walking\\walking_right_2.png",
	"Images\\Enemy2\\Walking\\walking_right_3.png"
};
static char enemy2FightLeftFiles[ENEMY2_FIGHT_FRAMES][100] = {
	"Images\\Enemy2\\Fighting\\fighting_left_1.png",
	"Images\\Enemy2\\Fighting\\fighting_left_2.png",
	"Images\\Enemy2\\Fighting\\fighting_left_3.png",
	"Images\\Enemy2\\Fighting\\fighting_left_4.png",
	"Images\\Enemy2\\Fighting\\fighting_left_5.png"
};
static char enemy2FightRightFiles[ENEMY2_FIGHT_FRAMES][100] = {
	"Images\\Enemy2\\Fighting\\fighting_right_1.png",
	"Images\\Enemy2\\Fighting\\fighting_right_2.png",
	"Images\\Enemy2\\Fighting\\fighting_right_3.png",
	"Images\\Enemy2\\Fighting\\fighting_right_4.png",
	"Images\\Enemy2\\Fighting\\fighting_right_5.png"
};
static char enemy2StandLeftFile[100]  = "Images\\Enemy2\\Standing\\standing_left.png";
static char enemy2StandRightFile[100] = "Images\\Enemy2\\Standing\\standing_right.png";

/* -------- loaded textures (filled in by initEnemy2Textures()) -------- */
static unsigned int enemy2WalkLeftTex[ENEMY2_WALK_FRAMES];
static unsigned int enemy2WalkRightTex[ENEMY2_WALK_FRAMES];
static unsigned int enemy2FightLeftTex[ENEMY2_FIGHT_FRAMES];
static unsigned int enemy2FightRightTex[ENEMY2_FIGHT_FRAMES];
static unsigned int enemy2StandLeftTex;
static unsigned int enemy2StandRightTex;
static unsigned int enemy2GateOverlayTex;  // "duplicate gate" occlusion patch, see ENEMY2_GATE_OVERLAY_* above

/* =========================================================
   loadEnemy2Texture -- like hero.h's loadHeroTexture(): any
   pixel close enough to pure white is made fully transparent
   first. The Images\Enemy2\... frames are flat-white-background
   PNGs (no alpha baked in), same as the hero's art, so the same
   trick is needed to draw them without a white box around them.
   ========================================================= */
inline unsigned int loadEnemy2Texture(char filename[])
{
	int width, height, channels;
	unsigned char *data = stbi_load(filename, &width, &height, &channels, 4);
	if (!data)
		return 0; // missing/bad file -- draws nothing rather than crashing

	int nPixels = width * height;
	for (int i = 0; i < nPixels; i++)
	{
		unsigned char *p = data + i * 4;
		bool nearWhite =
			p[0] >= 255 - ENEMY2_COLORKEY_TOLERANCE &&
			p[1] >= 255 - ENEMY2_COLORKEY_TOLERANCE &&
			p[2] >= 255 - ENEMY2_COLORKEY_TOLERANCE;
		p[3] = nearWhite ? 0 : 255;
	}

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, data);

	stbi_image_free(data);
	return texture;
}

inline void initEnemy2Textures()
{
	for (int i = 0; i < ENEMY2_WALK_FRAMES; i++)
	{
		enemy2WalkLeftTex[i]  = loadEnemy2Texture(enemy2WalkLeftFiles[i]);
		enemy2WalkRightTex[i] = loadEnemy2Texture(enemy2WalkRightFiles[i]);
	}
	for (int i = 0; i < ENEMY2_FIGHT_FRAMES; i++)
	{
		enemy2FightLeftTex[i]  = loadEnemy2Texture(enemy2FightLeftFiles[i]);
		enemy2FightRightTex[i] = loadEnemy2Texture(enemy2FightRightFiles[i]);
	}
	enemy2StandLeftTex  = loadEnemy2Texture(enemy2StandLeftFile);
	enemy2StandRightTex = loadEnemy2Texture(enemy2StandRightFile);

	// Plain opaque duplicate of the background art -- no white-colorkey
	// pass needed (that trick is only for the character sprites, which
	// are flat-white-background PNGs; this is a straight crop of the
	// already-final background image).
	enemy2GateOverlayTex = iLoadImage(ENEMY2_GATE_OVERLAY_FILE);
}

/* =========================================================
   drawEnemy2Texture -- same textured quad as hero.h's
   drawHeroTexture(), minus the flip logic: Level 2's art
   already has separate left/right frames, so the caller just
   picks the right texture up front instead of mirroring one.
   ========================================================= */
inline void drawEnemy2Texture(int x, int y, int width, int height,
	unsigned int texture)
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
		glTexCoord2f(1, 0);  glVertex2f(x + width, y);
		glTexCoord2f(1, -1); glVertex2f(x + width, y + height);
		glTexCoord2f(0, -1); glVertex2f(x, y + height);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}


/* =========================================================
   Enemy2 -- a single Level 2 knight
   ========================================================= */
class Enemy2
{
public:
	float x, y;
	int width, height;
	int hp, maxHp;
	int damage;
	float attackRange;
	bool alive;
	int scoreValue;

	int facing;               // ENEMY2_FACING_LEFT / ENEMY2_FACING_RIGHT
	Enemy2State state;
	int currentFrame;
	int frameTimer;
	int attackCooldownTimer;

	/* -------- new bookkeeping for the standing-guard / gate-entry design -------- */
	float postX;               // this knight's fighting-ground x (patrol beat is centered on this)
	bool hasPassedPlayer;      // true once the hero has walked past this knight's post

	Enemy2() { alive = false; }

	/* spawnX      : where the knight actually appears (its post, or
	                 ENEMY2_GATE_X for the last few knights)
	   targetPostX : the fighting-ground x it belongs at (== spawnX
	                 unless enterFromGate)
	   enterFromGate: true for the last ENEMY2_LAST_GATE_COUNT knights --
	                 they walk in from spawnX to targetPostX instead of
	                 already being there. */
	void spawn(float spawnX, float py, int hpVal, int dmgVal,
		float range, int scoreVal, float targetPostX, bool enterFromGate)
	{
		x = spawnX; y = py;
		width = ENEMY2_DRAW_WIDTH; height = ENEMY2_DRAW_HEIGHT;
		hp = hpVal; maxHp = hpVal;
		damage = dmgVal;
		attackRange = range;
		scoreValue = scoreVal;
		alive = true;

		postX = targetPostX;
		hasPassedPlayer = false;

		currentFrame = 0;
		frameTimer = 0;
		attackCooldownTimer = 0;

		if (enterFromGate)
		{
			// Walking in from the gate -- face whichever way the
			// post actually is relative to the spawn point.
			facing = (postX < x) ? ENEMY2_FACING_LEFT : ENEMY2_FACING_RIGHT;
			state = ENEMY2_ENTERING;
		}
		else
		{
			facing = ENEMY2_FACING_RIGHT;
			state = ENEMY2_STANDING;
		}
	}

	bool playerInRange(float range) const
	{
		return playerInEnemy2Range(x, y, (float)width, (float)height, range);
	}

	void takeDamage(int amount)
	{
		if (!alive) return;
		hp -= amount;
		if (hp <= 0)
		{
			hp = 0;
			alive = false;
			state = ENEMY2_DEAD;
			score += scoreValue;   // global from globals.h
		}
	}

	/* Switches this knight into "shadow the hero" mode -- called the
	   first time the hero is judged to have passed it (from STANDING
	   or ENTERING), and again whenever it gives up a chase/fight with
	   the hero already behind it. From here on trackPlayerSide() below
	   decides which way it walks every frame. */
	void beginPatrol()
	{
		state = ENEMY2_PATROL;
	}

	/* Turns to face whichever side the hero is CURRENTLY on, using
	   ENEMY2_PATROL_DEADZONE as hysteresis: facing only flips when
	   the hero is clearly to one side, and is left alone while he's
	   within the deadzone. This is the single source of truth for
	   left/right facing outside of ENTERING (which faces its walk-in
	   direction instead, since it hasn't engaged the hero yet).

	   Without this hysteresis, anything that set facing from a bare
	   "playerX < x ? LEFT : RIGHT" comparison (no deadzone) would
	   flip sign almost every frame whenever the hero's x hovered
	   right around the knight's x (e.g. walking past a standing
	   guard, or the instant an attack begins) -- which is exactly
	   the "walking/fighting image flickers between left and right"
	   bug this fixes. Every caller below now goes through here so
	   walking AND fighting art always match the side the hero is
	   actually on. */
	void faceTowardPlayer()
	{
		float dx = playerX - x;
		if (dx > ENEMY2_PATROL_DEADZONE)
			facing = ENEMY2_FACING_RIGHT;
		else if (dx < -ENEMY2_PATROL_DEADZONE)
			facing = ENEMY2_FACING_LEFT;
		// else: hero roughly level with this knight -- keep current
		// facing rather than flip-flopping.
	}

	/* Moves one step toward whichever side the hero is currently on --
	   left if the hero is to this knight's left, right if the hero is
	   to its right -- instead of bouncing along a fixed beat. Holds
	   still (within ENEMY2_PATROL_DEADZONE) once roughly level with
	   the hero, so it doesn't jitter back and forth in place. */
	void trackPlayerSide()
	{
		float oldX = x;
		float newX = x;
		float dx = playerX - x;
		faceTowardPlayer();

		if (dx > ENEMY2_PATROL_DEADZONE)
			newX = x + ENEMY2_PATROL_SPEED;
		else if (dx < -ENEMY2_PATROL_DEADZONE)
			newX = x - ENEMY2_PATROL_SPEED;
		// else: roughly level with the hero -- hold position.

		// FIX: knights used to walk straight through the Level 2
		// obstacles while patrolling. blockEnemy2AtAllObstacles()
		// (collision2.h) stops the step right at the obstacle's
		// edge instead of letting it cross through.
		newX = blockEnemy2AtAllObstacles(oldX, newX, (float)width);
		x = newX;

		if (x < 0) x = 0;
		if (x > LEVEL1_WIDTH - width) x = LEVEL1_WIDTH - width;
	}

	/* Steps x one frame closer to playerX (from either side), at
	   ENEMY2_CHASE_SPEED, updating facing to match whichever side
	   the hero is actually on. */
	void moveTowardPlayer()
	{
		float oldX = x;
		float newX = x;
		float dx = playerX - x;
		faceTowardPlayer();

		if (dx > ENEMY2_CHASE_SPEED)
		{
			newX = x + ENEMY2_CHASE_SPEED;
		}
		else if (dx < -ENEMY2_CHASE_SPEED)
		{
			newX = x - ENEMY2_CHASE_SPEED;
		}
		else
		{
			newX = playerX;
		}

		// FIX: knights used to walk straight through the Level 2
		// obstacles while chasing the player. blockEnemy2AtAllObstacles()
		// (collision2.h) stops the step right at the obstacle's edge
		// instead of letting it cross through.
		newX = blockEnemy2AtAllObstacles(oldX, newX, (float)width);
		x = newX;

		if (x < 0) x = 0;
		if (x > LEVEL1_WIDTH - width) x = LEVEL1_WIDTH - width;
	}

	void update()
	{
		if (!alive) return;

		bool inAttackRange = playerInRange(attackRange);
		bool inChaseRange = playerInRange(ENEMY2_CHASE_RANGE);
		frameTimer++;

		if (state == ENEMY2_ATTACKING)
		{
			if (frameTimer >= ENEMY2_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame++;
				if (currentFrame >= ENEMY2_FIGHT_FRAMES)
				{
					// Attack animation finished -- apply damage if the
					// player is still in range at the moment of impact.
					if (inAttackRange)
					{
						health -= damage;   // global player health from globals.h
						if (health < 0) health = 0;
					}
					currentFrame = 0;
					attackCooldownTimer = ENEMY2_ATTACK_COOLDOWN;

					if (inChaseRange)
						state = ENEMY2_CHASE;
					else if (hasPassedPlayer)
						beginPatrol();
					else
						state = ENEMY2_STANDING;
				}
			}
			return;
		}

		if (state == ENEMY2_CHASE)
		{
			if (attackCooldownTimer > 0)
				attackCooldownTimer--;

			if (frameTimer >= ENEMY2_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame = (currentFrame + 1) % ENEMY2_WALK_FRAMES;
			}

			if (inAttackRange && attackCooldownTimer == 0)
			{
				faceTowardPlayer();
				state = ENEMY2_ATTACKING;
				currentFrame = 0;
				frameTimer = 0;
			}
			else if (!inChaseRange)
			{
				// Player got away -- resume whatever the knight was
				// doing before: pacing its beat if the hero's already
				// past it, otherwise back to standing guard.
				if (hasPassedPlayer) beginPatrol();
				else state = ENEMY2_STANDING;
			}
			else
			{
				moveTowardPlayer();
			}
			return;
		}

		/* ---- From here down: ENTERING, STANDING, PATROL ---- none
		   of these are mid-fight, so a nearby hero always takes
		   priority (this is the "detection range" -- it fires no
		   matter what passive thing the knight is currently doing). */
		if (attackCooldownTimer > 0)
			attackCooldownTimer--;

		if (inAttackRange && attackCooldownTimer == 0)
		{
			faceTowardPlayer();
			state = ENEMY2_ATTACKING;
			currentFrame = 0;
			frameTimer = 0;
			return;
		}
		if (inChaseRange)
		{
			state = ENEMY2_CHASE;
			return;
		}

		// Once the hero has walked far enough past this knight's post,
		// it stops just standing there and starts its patrol beat.
		// This only ever flips on, never back off, so a knight that's
		// already pacing doesn't freeze again if the hero backtracks.
		if (!hasPassedPlayer && (playerX - x) > ENEMY2_PASS_MARGIN)
			hasPassedPlayer = true;

		if (state == ENEMY2_ENTERING)
		{
			if (frameTimer >= ENEMY2_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame = (currentFrame + 1) % ENEMY2_WALK_FRAMES;
			}

			float oldX = x;
			float dx = postX - x;
			if (dx > ENEMY2_CHASE_SPEED)
			{
				float newX = blockEnemy2AtAllObstacles(oldX, x + ENEMY2_CHASE_SPEED, (float)width);
				x = newX;
				facing = ENEMY2_FACING_RIGHT;
			}
			else if (dx < -ENEMY2_CHASE_SPEED)
			{
				float newX = blockEnemy2AtAllObstacles(oldX, x - ENEMY2_CHASE_SPEED, (float)width);
				x = newX;
				facing = ENEMY2_FACING_LEFT;
			}
			else
			{
				// Arrived at the fighting-ground post -- settle into
				// standing guard (or straight into patrol, if the hero
				// already got past while this knight was still walking in).
				x = postX;
				currentFrame = 0;
				if (hasPassedPlayer) beginPatrol();
				else state = ENEMY2_STANDING;
			}
			return;
		}

		if (state == ENEMY2_STANDING)
		{
			currentFrame = 0; // idle pose

			// A stationary guard still turns to watch the hero go by
			// (with the same deadzone hysteresis as everything else,
			// so it doesn't flicker left/right while he's level with it).
			faceTowardPlayer();

			if (hasPassedPlayer)
				beginPatrol();
			return;
		}

		// ENEMY2_PATROL -- "shadow the hero": walks left when he's to
		// this knight's left, right when he's to its right, and holds
		// still when it's roughly level with him.
		{
			bool moving = fabs(playerX - x) > ENEMY2_PATROL_DEADZONE;
			trackPlayerSide();

			if (moving && frameTimer >= ENEMY2_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame = (currentFrame + 1) % ENEMY2_WALK_FRAMES;
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

		float screenX = x - cameraX;

		unsigned int tex;
		if (state == ENEMY2_ATTACKING)
		{
			tex = (facing == ENEMY2_FACING_LEFT)
				? enemy2FightLeftTex[currentFrame]
				: enemy2FightRightTex[currentFrame];
		}
		else if (state == ENEMY2_STANDING ||
			(state == ENEMY2_PATROL && fabs(playerX - x) <= ENEMY2_PATROL_DEADZONE))
		{
			tex = (facing == ENEMY2_FACING_LEFT) ? enemy2StandLeftTex : enemy2StandRightTex;
		}
		else // ENTERING, PATROL-moving, or CHASE
		{
			tex = (facing == ENEMY2_FACING_LEFT)
				? enemy2WalkLeftTex[currentFrame]
				: enemy2WalkRightTex[currentFrame];
		}

		drawEnemy2Texture((int)screenX, (int)y, width, height, tex);

		// Health bar over the knight's head, same proportions as
		// enemy.h's Level 1 knights.
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
   EnemyManager2 -- holds and drives every Level 2 Enemy2
   ========================================================= */
class EnemyManager2
{
public:
	// Only ONE of the MAX_ENEMIES2 (10) knights is alive at a time --
	// same "one at a time, next spawns after the current one dies"
	// shape as Level 1's EnemyManager (see enemy.h). All 10 configs
	// still live here so the encounter order and per-knight tuning
	// stays in one place.
	Enemy2 enemies[MAX_ENEMIES2];

	int defeatedCount;     // how many of the 10 have been killed so far
	int activeIndex;       // index into enemies[] currently alive, -1 = none
	bool waitingToRespawn; // true during the pause between a death and the next spawn
	int respawnTimer;

	EnemyManager2() { defeatedCount = 0; activeIndex = -1; waitingToRespawn = false; respawnTimer = 0; }

	static float configX(int i)
	{
		// Spread across the same 3-screen-wide world Level 1 uses
		// (LEVEL1_WIDTH = 3072), starting clear of the hero's own
		// spawn point (x = 500, see hero.h). This is each knight's
		// fighting-ground post -- for the last ENEMY2_LAST_GATE_COUNT
		// knights it's where they walk TO, not where they spawn.
		// NOTE: every post for the last ENEMY2_LAST_GATE_COUNT knights
		// (see spawnNext()) must stay LESS than ENEMY2_GATE_X (2548) --
		// those knights spawn at the gate and walk out to this post, so
		// if the post is past the gate they'd walk further IN (away
		// from the hero, the wrong direction) instead of out to meet
		// him. 2100/2220/2340/2448 all land comfortably before the
		// gate/pillar art.
		static const float x[MAX_ENEMIES2] =
		{
			650, 900, 1200, 1400, 1650,
			1800, 2100, 2220, 2340, 2448
		};
		return x[i];
	}
	static int configHp(int i)
	{
		static const int hp[MAX_ENEMIES2] =
		{
			30, 30, 35, 35, 40, 40, 40, 45, 45, 50
		};
		return hp[i];
	}
	static int configDamage(int i)
	{
		static const int dmg[MAX_ENEMIES2] =
		{
			8, 8, 10, 10, 12, 12, 14, 14, 16, 18
		};
		return dmg[i];
	}
	static int configScore(int i)
	{
		static const int score[MAX_ENEMIES2] =
		{
			15, 15, 18, 18, 20, 20, 22, 22, 25, 30
		};
		return score[i];
	}
	static const float configRange; // same attackRange for every knight

	// Spawns enemies[defeatedCount] as the new active knight, unless
	// all 10 have already been defeated. The last ENEMY2_LAST_GATE_COUNT
	// knights in the order spawn at the gate and walk in to their post
	// instead of appearing there directly.
	void spawnNext()
	{
		if (defeatedCount >= MAX_ENEMIES2)
		{
			activeIndex = -1;
			return;
		}

		int i = defeatedCount;
		bool fromGate = (i >= MAX_ENEMIES2 - ENEMY2_LAST_GATE_COUNT);
		float spawnX = fromGate ? ENEMY2_GATE_X : configX(i);

		enemies[i].spawn(spawnX, ENEMY2_GROUND_Y,
			configHp(i), configDamage(i), configRange, configScore(i),
			configX(i), fromGate);
		activeIndex = i;
	}

	// Called whenever a level/game (re)starts
	void reset()
	{
		defeatedCount = 0;
		activeIndex = -1;
		waitingToRespawn = false;
		respawnTimer = 0;

		for (int i = 0; i < MAX_ENEMIES2; i++)
			enemies[i].alive = false;

		spawnNext();   // first knight (index 0) appears immediately
	}

	void init()
	{
		initEnemy2Textures();
		reset();
	}

	void updateAll()
	{
		if (activeIndex != -1)
		{
			enemies[activeIndex].update();

			// The active knight just died -- start the short pause
			// before the next one (if any) is generated.
			if (!enemies[activeIndex].alive && !waitingToRespawn)
			{
				defeatedCount++;
				activeIndex = -1;
				waitingToRespawn = true;

				// defeatedCount is now the index of whichever knight
				// spawns next -- if THAT one is one of the last
				// ENEMY2_LAST_GATE_COUNT (gate) knights, use the
				// shorter gate respawn delay instead of the normal one.
				bool nextIsFromGate = (defeatedCount >= MAX_ENEMIES2 - ENEMY2_LAST_GATE_COUNT);
				respawnTimer = nextIsFromGate ? ENEMY2_GATE_RESPAWN_DELAY : ENEMY2_RESPAWN_DELAY;
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

		/* Level-end: all 10 knights dead. Completes immediately
		   rather than waiting for the hero to walk further in to a
		   fixed x -- see the ENEMY2_LEVEL_END_X removal note above. */
		if (defeatedCount >= MAX_ENEMIES2 &&
			!waitingToRespawn && activeIndex == -1 &&
			currentState == LEVEL2_PLAYING)
		{
			storyStart(STORY_LEVEL2_END);
		}
	}

	void drawAll()
	{
		if (activeIndex != -1)
			enemies[activeIndex].draw();

		// Redraw the "duplicate gate" patch on top of whatever was just
		// drawn -- see the big ENEMY2_GATE_OVERLAY_* comment above.
		// Cheap to always draw (it's a single quad, and it's a no-op
		// visually except where it happens to sit over a knight), so
		// no need to gate it on which knight/state is currently active.
		float overlayWorldX = ENEMY2_GATE_SEGMENT_WORLD_X + ENEMY2_GATE_OVERLAY_LOCAL_X;
		float overlayScreenX = overlayWorldX - cameraX;
		if (overlayScreenX + ENEMY2_GATE_OVERLAY_W >= 0 && overlayScreenX <= SCREEN_WIDTH)
		{
			iShowImage((int)overlayScreenX, (int)ENEMY2_GATE_OVERLAY_Y,
				ENEMY2_GATE_OVERLAY_W, ENEMY2_GATE_OVERLAY_H, enemy2GateOverlayTex);
		}
	}

	// Called when the player presses the attack key ('J') during
	// Level 2. Only the one active knight can ever be in reach now,
	// same as Level 1's playerAttack().
	void playerAttack()
	{
		if (activeIndex == -1) return; // no knight generated right now

		Enemy2 &e = enemies[activeIndex];
		if (!e.alive) return;

		if (e.playerInRange(PLAYER2_ATTACK_RANGE))
			e.takeDamage(PLAYER2_ATTACK_DAMAGE);
	}
};

const float EnemyManager2::configRange = 40.0f;

/* Single global instance -- this is what iMain.cpp talks to */
EnemyManager2 enemyManager2;

/* Thin wrapper functions so the call sites read the same as
   enemy.h's initEnemies()/updateEnemies()/... */
inline void initEnemies2()          { enemyManager2.init(); }
inline void resetEnemies2()         { enemyManager2.reset(); }
inline void updateEnemies2()        { enemyManager2.updateAll(); }
inline void drawEnemies2()          { enemyManager2.drawAll(); }
inline void playerAttackEnemies2()  { enemyManager2.playerAttack(); }
