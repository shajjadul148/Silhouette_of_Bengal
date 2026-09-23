#pragma once

#include "globals.h"
#include "iGraphics.h"
#include "Level3Obstacle.h"   // level3SafeEnemySpawnX() -- keeps spawns off the rocks
#include <math.h>
#include <stdlib.h>

/* External game variables */
extern float playerX, playerY;
extern float cameraX;
extern int score;
extern int health;
extern GameState currentState;

/* =========================================================
SOLDIER & ENEMY3 TUNABLES
========================================================= */
#define SOLDIER_FIRE_RATE         75       // Frames between gunshots
#define SOLDIER_BULLET_SPEED      8.5f     // Projectile speed (px/frame)
#define SOLDIER_BULLET_DAMAGE     15       // Damage dealt to hero per bullet hit
#define SOLDIER_KEEP_DIST         80.0f   // Distance soldier tries to keep from hero
#define SOLDIER_DIST_TOLERANCE    50.0f    // Margin around target distance before moving
#define SOLDIER_RUN_SPEED         3.2f     // Speed when retreating (backing off)
#define SOLDIER_WALK_SPEED        1.8f     // Speed when closing distance

#define BLUE_WALK_SPEED           2.2f
#define BLUE_ATTACK_RANGE         45.0f
#define BLUE_ATTACK_DAMAGE        12
#define BLUE_ATTACK_COOLDOWN      45

/* Boss (Level 3, Wave 3 -- spawns after Wave 2 is cleared).
   Placed at the far end of the level, just past where Wave 2's
   last enemies fight (x up to ~3950) -- previously this sat at
   x=2950 with escorts as far back as x=2350, i.e. BEHIND where
   the player ends up after clearing Wave 2, off-screen to the
   left with nothing else happening. That made the Wave 2 -> Wave 3
   transition look like the game had frozen. Anchored to
   LEVEL3_WIDTH so it stays inside the level regardless of screen
   size changes. */
#define BOSS_HP                   300
#define BOSS_WIDTH                100
#define BOSS_HEIGHT               100
#define BOSS_WALK_SPEED           1.6f
#define BOSS_ATTACK_RANGE         65.0f
#define BOSS_ATTACK_DAMAGE        25
#define BOSS_ATTACK_COOLDOWN      55
#define BOSS_SCORE_VALUE          200
#define BOSS_WALK_FRAMES          5
#define BOSS_FIGHT_FRAMES         7
#define BOSS_SPAWN_X              ((float)LEVEL3_WIDTH - 150.0f)
#define BOSS_SPAWN_Y              40.0f  // spawns on the ground, same as the other enemies
/* Wave 3 escorts: 4 normal (Blue) enemies, placed to the left of
   the boss so the player meets them before reaching it. */
#define BOSS_ESCORT_BLUE1_X       (BOSS_SPAWN_X - 600.0f)
#define BOSS_ESCORT_BLUE2_X       (BOSS_SPAWN_X - 450.0f)
#define BOSS_ESCORT_BLUE3_X       (BOSS_SPAWN_X - 300.0f)
#define BOSS_ESCORT_BLUE4_X       (BOSS_SPAWN_X - 150.0f)

#define ENEMY3_COLORKEY_TOLERANCE 24
#define ENEMY3_GROUND_Y           40.0f
#define MAX_BULLETS_PRO           30
#define ENEMY3_ANIM_SPEED         6
#define ENEMY3_RESPAWN_DELAY      40

/* Sprite-sheet frame counts (must match the files actually shipped
   under Images\Enemy_3\..., see the file-path tables below) */
#define BLUE_WALK_FRAMES          3
#define BLUE_FIGHT_FRAMES         5
#define SOLDIER_WALK_FRAMES       6
#define SOLDIER_RUN_FRAMES        8
#define SOLDIER_FIGHT_FRAMES      6
#define BULLET_PRO_FRAMES         2
#define BULLET_ANIM_SPEED         5

/* Wave composition constants */
#define WAVE1_BLUE_COUNT          4
#define WAVE1_SOLDIER_COUNT       2
#define WAVE2_BLUE_COUNT          5
#define WAVE2_SOLDIER_COUNT       2
/* Wave 3 boss fight: 4 escorts (2 Soldiers + 2 Blues) fight first,
   then the Boss spawns once all four are dead. */
#define MAX_ACTIVE_ENEMIES3       5
#define BOSS_ESCORT_COUNT         4

#define PLAYER3_ATTACK_RANGE      40.0f
#define PLAYER3_ATTACK_DAMAGE     20

/* =========================================================
HITBOX INSETS (precise collision)
-----------------------------------------------------------
How far in from each edge of a sprite's full 100x100 (or, for
the Boss, BOSS_WIDTH x BOSS_HEIGHT) canvas the real, visible
character is. Used by Enemy3::playerInRange() below so melee/
attack-range checks -- and whether a hit actually lands -- are
based on the sprites' real silhouettes overlapping, not a
coarse anchor-point distance. collision3.h includes this file
(for the Enemy3 type) rather than the other way around, so
these constants live here and collision3.h's body-collision
hitbox helpers reuse them, keeping a single source of truth. */
#define HERO_INSET_LEFT           22.0f
#define HERO_INSET_RIGHT          22.0f
#define HERO_INSET_BOTTOM         13.0f
#define HERO_INSET_TOP             9.0f

#define ENEMY3_BLUE_INSET_X       20.0f
#define ENEMY3_BLUE_INSET_Y       10.0f

#define SOLDIER_INSET_X           18.0f
#define SOLDIER_INSET_Y           12.0f

#define BOSS_INSET_X              30.0f
#define BOSS_INSET_Y              18.0f

/* Healing pickup (dropped when the last Soldier of a wave dies) */
#define MAX_HEALING_PICKUPS       2
#define HEALING_PICKUP_WIDTH      50
#define HEALING_PICKUP_HEIGHT     50
#define HEALING_PICKUP_AMOUNT     35      // HP restored to the hero on pickup
#define HEALING_PICKUP_BOB_SPEED  0.04f   // idle floating-bob animation speed
#define HEALING_PICKUP_BOB_RANGE  6.0f    // pixels the pickup bobs up/down

/* =========================================================
ENUMS
========================================================= */
enum Enemy3Type { TYPE_BLUE, TYPE_SOLDIER, TYPE_BOSS };
enum Enemy3Facing { ENEMY3_FACING_LEFT = -1, ENEMY3_FACING_RIGHT = 1 };
enum Enemy3State { ENEMY3_STANDING, ENEMY3_WALKING, ENEMY3_RUNNING, ENEMY3_FIGHTING, ENEMY3_HURT, ENEMY3_DEAD };
enum Level3Wave { L3_WAVE1_BLUES, L3_WAVE1_SOLDIERS, L3_WAVE2_BLUES, L3_WAVE2_SOLDIERS, L3_WAVE3_BOSS, L3_COMPLETE };

/* =========================================================
BULLET PROJECTILE STRUCT
========================================================= */
struct Bullet_pro {
	float x, y;
	float dirX;      // -1.0f for left, 1.0f for right
	bool active;
	int damage;
	int width, height;
	int frame;
	int frameTimer;
};

/* =========================================================
HEALING PICKUP STRUCT
========================================================= */
struct HealingPickup {
	float x, y;
	bool active;
	float bobPhase;
};

/* =========================================================
SPRITE FILE PATHS
(paths verified against the shipped Images\Enemy_3\... assets)
========================================================= */
/* Enemy3_Blue (100x100) */
static char enemy3BlueStandLeftFile[100] = "Images\\Enemy_3\\Enemy3_Blue\\Enemy2\\Standing\\standing_left.png";
static char enemy3BlueStandRightFile[100] = "Images\\Enemy_3\\Enemy3_Blue\\Enemy2\\Standing\\standing_right.png";

static char enemy3BlueWalkLeftFiles[BLUE_WALK_FRAMES][100] = {
	"Images\\Enemy_3\\Enemy3_Blue\\Enemy2\\Walking\\walking_left_1.png",
	"Images\\Enemy_3\\Enemy3_Blue\\Enemy2\\Walking\\walking_left_2.png",
	"Images\\Enemy_3\\Enemy3_Blue\\Enemy2\\Walking\\walking_left_3.png"
};
static char enemy3BlueWalkRightFiles[BLUE_WALK_FRAMES][100] = {
	"Images\\Enemy_3\\Enemy3_Blue\\Enemy2\\Walking\\walking_right_1.png",
	"Images\\Enemy_3\\Enemy3_Blue\\Enemy2\\Walking\\walking_right_2.png",
	"Images\\Enemy_3\\Enemy3_Blue\\Enemy2\\Walking\\walking_right_3.png"
};

static char enemy3BlueFightLeftFiles[BLUE_FIGHT_FRAMES][100] = {
	"Images\\Enemy_3\\Enemy3_Blue\\Enemy2\\Fighting\\fighting_left_1.png",
	"Images\\Enemy_3\\Enemy3_Blue\\Enemy2\\Fighting\\fighting_left_2.png",
	"Images\\Enemy_3\\Enemy3_Blue\\Enemy2\\Fighting\\fighting_left_3.png",
	"Images\\Enemy_3\\Enemy3_Blue\\Enemy2\\Fighting\\fighting_left_4.png",
	"Images\\Enemy_3\\Enemy3_Blue\\Enemy2\\Fighting\\fighting_left_5.png"
};
static char enemy3BlueFightRightFiles[BLUE_FIGHT_FRAMES][100] = {
	"Images\\Enemy_3\\Enemy3_Blue\\Enemy2\\Fighting\\fighting_right_1.png",
	"Images\\Enemy_3\\Enemy3_Blue\\Enemy2\\Fighting\\fighting_right_2.png",
	"Images\\Enemy_3\\Enemy3_Blue\\Enemy2\\Fighting\\fighting_right_3.png",
	"Images\\Enemy_3\\Enemy3_Blue\\Enemy2\\Fighting\\fighting_right_4.png",
	"Images\\Enemy_3\\Enemy3_Blue\\Enemy2\\Fighting\\fighting_right_5.png"
};

/* Soldier (Standing/Walking/Running/Fighting all 100x100) */
static char soldierStandLeftFile[100] = "Images\\Enemy_3\\Soldier\\Standing\\Enemy3_standing_left.png";
static char soldierStandRightFile[100] = "Images\\Enemy_3\\Soldier\\Standing\\Enemy3_standing_right.png";

static char soldierWalkLeftFiles[SOLDIER_WALK_FRAMES][100] = {
	"Images\\Enemy_3\\Soldier\\Walking\\walking_left_1.png",
	"Images\\Enemy_3\\Soldier\\Walking\\walking_left_2.png",
	"Images\\Enemy_3\\Soldier\\Walking\\walking_left_3.png",
	"Images\\Enemy_3\\Soldier\\Walking\\walking_left_4.png",
	"Images\\Enemy_3\\Soldier\\Walking\\walking_left_5.png",
	"Images\\Enemy_3\\Soldier\\Walking\\walking_left_6.png"
};
static char soldierWalkRightFiles[SOLDIER_WALK_FRAMES][100] = {
	"Images\\Enemy_3\\Soldier\\Walking\\walking_right_1.png",
	"Images\\Enemy_3\\Soldier\\Walking\\walking_right_2.png",
	"Images\\Enemy_3\\Soldier\\Walking\\walking_right_3.png",
	"Images\\Enemy_3\\Soldier\\Walking\\walking_right_4.png",
	"Images\\Enemy_3\\Soldier\\Walking\\walking_right_5.png",
	"Images\\Enemy_3\\Soldier\\Walking\\walking_right_6.png"
};

static char soldierRunLeftFiles[SOLDIER_RUN_FRAMES][100] = {
	"Images\\Enemy_3\\Soldier\\Running\\running_left_1.png",
	"Images\\Enemy_3\\Soldier\\Running\\running_left_2.png",
	"Images\\Enemy_3\\Soldier\\Running\\running_left_3.png",
	"Images\\Enemy_3\\Soldier\\Running\\running_left_4.png",
	"Images\\Enemy_3\\Soldier\\Running\\running_left_5.png",
	"Images\\Enemy_3\\Soldier\\Running\\running_left_6.png",
	"Images\\Enemy_3\\Soldier\\Running\\running_left_7.png",
	"Images\\Enemy_3\\Soldier\\Running\\running_left_8.png"
};
static char soldierRunRightFiles[SOLDIER_RUN_FRAMES][100] = {
	"Images\\Enemy_3\\Soldier\\Running\\running_right_1.png",
	"Images\\Enemy_3\\Soldier\\Running\\running_right_2.png",
	"Images\\Enemy_3\\Soldier\\Running\\running_right_3.png",
	"Images\\Enemy_3\\Soldier\\Running\\running_right_4.png",
	"Images\\Enemy_3\\Soldier\\Running\\running_right_5.png",
	"Images\\Enemy_3\\Soldier\\Running\\running_right_6.png",
	"Images\\Enemy_3\\Soldier\\Running\\running_right_7.png",
	"Images\\Enemy_3\\Soldier\\Running\\running_right_8.png"
};

/* NOTE: the shipped filenames really do say "fightinig" (typo baked
   into the asset files themselves) -- kept verbatim so the paths
   resolve on disk. */
static char soldierFightLeftFiles[SOLDIER_FIGHT_FRAMES][100] = {
	"Images\\Enemy_3\\Soldier\\Fighting\\fightinig_left_1.png",
	"Images\\Enemy_3\\Soldier\\Fighting\\fightinig_left_2.png",
	"Images\\Enemy_3\\Soldier\\Fighting\\fightinig_left_3.png",
	"Images\\Enemy_3\\Soldier\\Fighting\\fightinig_left_4.png",
	"Images\\Enemy_3\\Soldier\\Fighting\\fightinig_left_5.png",
	"Images\\Enemy_3\\Soldier\\Fighting\\fightinig_left_6.png"
};
static char soldierFightRightFiles[SOLDIER_FIGHT_FRAMES][100] = {
	"Images\\Enemy_3\\Soldier\\Fighting\\fightinig_right_1.png",
	"Images\\Enemy_3\\Soldier\\Fighting\\fightinig_right_2.png",
	"Images\\Enemy_3\\Soldier\\Fighting\\fightinig_right_3.png",
	"Images\\Enemy_3\\Soldier\\Fighting\\fightinig_right_4.png",
	"Images\\Enemy_3\\Soldier\\Fighting\\fightinig_right_5.png",
	"Images\\Enemy_3\\Soldier\\Fighting\\fightinig_right_6.png"
};

/* Boss1 (100x100 source assets, drawn at native 100x100 -- see BOSS_WIDTH/HEIGHT) */
static char bossStandLeftFile[100] = "Images\\Enemy_3\\Boss\\Boss1_standing\\Boss1_standing_left.png";
static char bossStandRightFile[100] = "Images\\Enemy_3\\Boss\\Boss1_standing\\Boss1_standing_right.png";

static char bossWalkLeftFiles[BOSS_WALK_FRAMES][100] = {
	"Images\\Enemy_3\\Boss\\Boss1_walking\\Boss1_walking_left_1.png",
	"Images\\Enemy_3\\Boss\\Boss1_walking\\Boss1_walking_left_2.png",
	"Images\\Enemy_3\\Boss\\Boss1_walking\\Boss1_walking_left_3.png",
	"Images\\Enemy_3\\Boss\\Boss1_walking\\Boss1_walking_left_4.png",
	"Images\\Enemy_3\\Boss\\Boss1_walking\\Boss1_walking_left_5.png"
};
static char bossWalkRightFiles[BOSS_WALK_FRAMES][100] = {
	"Images\\Enemy_3\\Boss\\Boss1_walking\\Boss1_walking_right_1.png",
	"Images\\Enemy_3\\Boss\\Boss1_walking\\Boss1_walking_right_2.png",
	"Images\\Enemy_3\\Boss\\Boss1_walking\\Boss1_walking_right_3.png",
	"Images\\Enemy_3\\Boss\\Boss1_walking\\Boss1_walking_right_4.png",
	"Images\\Enemy_3\\Boss\\Boss1_walking\\Boss1_walking_right_5.png"
};

static char bossFightLeftFiles[BOSS_FIGHT_FRAMES][100] = {
	"Images\\Enemy_3\\Boss\\Boss1_fighting\\Boss1_fighting_left_1.png",
	"Images\\Enemy_3\\Boss\\Boss1_fighting\\Boss1_fighting_left_2.png",
	"Images\\Enemy_3\\Boss\\Boss1_fighting\\Boss1_fighting_left_3.png",
	"Images\\Enemy_3\\Boss\\Boss1_fighting\\Boss1_fighting_left_4.png",
	"Images\\Enemy_3\\Boss\\Boss1_fighting\\Boss1_fighting_left_5.png",
	"Images\\Enemy_3\\Boss\\Boss1_fighting\\Boss1_fighting_left_6.png",
	"Images\\Enemy_3\\Boss\\Boss1_fighting\\Boss1_fighting_left_7.png"
};
static char bossFightRightFiles[BOSS_FIGHT_FRAMES][100] = {
	"Images\\Enemy_3\\Boss\\Boss1_fighting\\Boss1_fighting_right_1.png",
	"Images\\Enemy_3\\Boss\\Boss1_fighting\\Boss1_fighting_right_2.png",
	"Images\\Enemy_3\\Boss\\Boss1_fighting\\Boss1_fighting_right_3.png",
	"Images\\Enemy_3\\Boss\\Boss1_fighting\\Boss1_fighting_right_4.png",
	"Images\\Enemy_3\\Boss\\Boss1_fighting\\Boss1_fighting_right_5.png",
	"Images\\Enemy_3\\Boss\\Boss1_fighting\\Boss1_fighting_right_6.png",
	"Images\\Enemy_3\\Boss\\Boss1_fighting\\Boss1_fighting_right_7.png"
};

/* Bullet_pro projectile (100x100 source, drawn scaled down) */
static char bulletProLeftFiles[BULLET_PRO_FRAMES][100] = {
	"Images\\Enemy_3\\Bullet\\Bullet_pro_left_1.png",
	"Images\\Enemy_3\\Bullet\\Bullet_pro_left_2.png"
};
static char bulletProRightFiles[BULLET_PRO_FRAMES][100] = {
	"Images\\Enemy_3\\Bullet\\Bullet_pro_right_1.png",
	"Images\\Enemy_3\\Bullet\\Bullet_pro_right_2.png"
};

/* Healing pickup (50x50) */
static char healingPickupFile[100] = "Images\\Enemy_3\\Healing.png";

/* =========================================================
TEXTURE STORAGE
========================================================= */
static unsigned int enemy3BlueStandLeftTex;
static unsigned int enemy3BlueStandRightTex;
static unsigned int enemy3BlueWalkLeftTex[BLUE_WALK_FRAMES];
static unsigned int enemy3BlueWalkRightTex[BLUE_WALK_FRAMES];
static unsigned int enemy3BlueFightLeftTex[BLUE_FIGHT_FRAMES];
static unsigned int enemy3BlueFightRightTex[BLUE_FIGHT_FRAMES];

static unsigned int soldierStandLeftTex;
static unsigned int soldierStandRightTex;
static unsigned int soldierWalkLeftTex[SOLDIER_WALK_FRAMES];
static unsigned int soldierWalkRightTex[SOLDIER_WALK_FRAMES];
static unsigned int soldierRunLeftTex[SOLDIER_RUN_FRAMES];
static unsigned int soldierRunRightTex[SOLDIER_RUN_FRAMES];
static unsigned int soldierFightLeftTex[SOLDIER_FIGHT_FRAMES];
static unsigned int soldierFightRightTex[SOLDIER_FIGHT_FRAMES];

static unsigned int bossStandLeftTex;
static unsigned int bossStandRightTex;
static unsigned int bossWalkLeftTex[BOSS_WALK_FRAMES];
static unsigned int bossWalkRightTex[BOSS_WALK_FRAMES];
static unsigned int bossFightLeftTex[BOSS_FIGHT_FRAMES];
static unsigned int bossFightRightTex[BOSS_FIGHT_FRAMES];

static unsigned int bulletProLeftTex[BULLET_PRO_FRAMES];
static unsigned int bulletProRightTex[BULLET_PRO_FRAMES];

static unsigned int healingPickupTex;

/* =========================================================
TEXTURE LOADING & RENDERING HELPERS
========================================================= */
inline unsigned int loadEnemy3Texture(const char filename[])
{
	int width, height, channels;
	unsigned char *data = stbi_load(filename, &width, &height, &channels, 4);
	if (!data) return 0;

	int nPixels = width * height;
	for (int i = 0; i < nPixels; i++)
	{
		unsigned char *p = data + i * 4;
		bool nearWhite =
			p[0] >= 255 - ENEMY3_COLORKEY_TOLERANCE &&
			p[1] >= 255 - ENEMY3_COLORKEY_TOLERANCE &&
			p[2] >= 255 - ENEMY3_COLORKEY_TOLERANCE;
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

inline void initEnemy3Textures()
{
	static bool texturesLoaded = false;
	if (texturesLoaded) return;

	enemy3BlueStandLeftTex = loadEnemy3Texture(enemy3BlueStandLeftFile);
	enemy3BlueStandRightTex = loadEnemy3Texture(enemy3BlueStandRightFile);
	for (int i = 0; i < BLUE_WALK_FRAMES; i++)
	{
		enemy3BlueWalkLeftTex[i] = loadEnemy3Texture(enemy3BlueWalkLeftFiles[i]);
		enemy3BlueWalkRightTex[i] = loadEnemy3Texture(enemy3BlueWalkRightFiles[i]);
	}
	for (int i = 0; i < BLUE_FIGHT_FRAMES; i++)
	{
		enemy3BlueFightLeftTex[i] = loadEnemy3Texture(enemy3BlueFightLeftFiles[i]);
		enemy3BlueFightRightTex[i] = loadEnemy3Texture(enemy3BlueFightRightFiles[i]);
	}

	soldierStandLeftTex = loadEnemy3Texture(soldierStandLeftFile);
	soldierStandRightTex = loadEnemy3Texture(soldierStandRightFile);
	for (int i = 0; i < SOLDIER_WALK_FRAMES; i++)
	{
		soldierWalkLeftTex[i] = loadEnemy3Texture(soldierWalkLeftFiles[i]);
		soldierWalkRightTex[i] = loadEnemy3Texture(soldierWalkRightFiles[i]);
	}
	for (int i = 0; i < SOLDIER_RUN_FRAMES; i++)
	{
		soldierRunLeftTex[i] = loadEnemy3Texture(soldierRunLeftFiles[i]);
		soldierRunRightTex[i] = loadEnemy3Texture(soldierRunRightFiles[i]);
	}
	for (int i = 0; i < SOLDIER_FIGHT_FRAMES; i++)
	{
		soldierFightLeftTex[i] = loadEnemy3Texture(soldierFightLeftFiles[i]);
		soldierFightRightTex[i] = loadEnemy3Texture(soldierFightRightFiles[i]);
	}

	bossStandLeftTex = loadEnemy3Texture(bossStandLeftFile);
	bossStandRightTex = loadEnemy3Texture(bossStandRightFile);
	for (int i = 0; i < BOSS_WALK_FRAMES; i++)
	{
		bossWalkLeftTex[i] = loadEnemy3Texture(bossWalkLeftFiles[i]);
		bossWalkRightTex[i] = loadEnemy3Texture(bossWalkRightFiles[i]);
	}
	for (int i = 0; i < BOSS_FIGHT_FRAMES; i++)
	{
		bossFightLeftTex[i] = loadEnemy3Texture(bossFightLeftFiles[i]);
		bossFightRightTex[i] = loadEnemy3Texture(bossFightRightFiles[i]);
	}

	for (int i = 0; i < BULLET_PRO_FRAMES; i++)
	{
		bulletProLeftTex[i] = loadEnemy3Texture(bulletProLeftFiles[i]);
		bulletProRightTex[i] = loadEnemy3Texture(bulletProRightFiles[i]);
	}

	healingPickupTex = loadEnemy3Texture(healingPickupFile);

	texturesLoaded = true;
}

inline void drawEnemy3Texture(int x, int y, int width, int height, unsigned int texture)
{
	if (texture == 0) return;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);  glVertex2f((float)x, (float)y);
	glTexCoord2f(1, 0);  glVertex2f((float)(x + width), (float)y);
	glTexCoord2f(1, -1); glVertex2f((float)(x + width), (float)(y + height));
	glTexCoord2f(0, -1); glVertex2f((float)x, (float)(y + height));
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

class EnemyManager3;

/* =========================================================
ENEMY3 CLASS
========================================================= */
class Enemy3
{
public:
	Enemy3Type type;
	float x, y;
	int width, height;
	int hp, maxHp;
	int damage;
	float attackRange;
	bool alive;
	int scoreValue;

	Enemy3Facing facing;
	Enemy3State state;
	int currentFrame;
	int frameTimer;
	int attackCooldownTimer;
	int shootTimer;

	Enemy3() { alive = false; }

	void spawn(Enemy3Type t, float px, float py, int hpVal, int dmgVal, float rangeVal, int scoreVal, int w = 100, int h = 100)
	{
		type = t;
		x = px;
		y = py;
		width = w;
		height = h;
		hp = hpVal;
		maxHp = hpVal;
		damage = dmgVal;
		attackRange = rangeVal;
		scoreValue = scoreVal;
		alive = true;

		facing = (playerX < x) ? ENEMY3_FACING_LEFT : ENEMY3_FACING_RIGHT;
		state = ENEMY3_STANDING;
		currentFrame = 0;
		frameTimer = 0;
		attackCooldownTimer = 0;
		shootTimer = rand() % 20;
	}

	void faceTowardPlayer()
	{
		float dx = playerX - x;
		if (dx > 6.0f) facing = ENEMY3_FACING_RIGHT;
		else if (dx < -6.0f) facing = ENEMY3_FACING_LEFT;
	}

	/* Precise, hitbox-based range check. Crops both this enemy's box
	   and the hero's box down to their real visible silhouettes
	   (see the HITBOX INSETS block above), expands the enemy's
	   cropped box outward by `range` pixels on every side, then
	   tests whether the hero's cropped box overlaps that expanded
	   box. Replaces the old raw-anchor-point distance check, which
	   ignored each sprite's actual size/type and let hits land (or
	   miss) even when the two silhouettes clearly weren't (or were)
	   touching on screen -- same approach as enemy.h/enemy2.h's
	   playerInRange(). */
	bool playerInRange(float range) const
	{
		float insetX, insetY;
		if (type == TYPE_BLUE)         { insetX = ENEMY3_BLUE_INSET_X; insetY = ENEMY3_BLUE_INSET_Y; }
		else if (type == TYPE_SOLDIER) { insetX = SOLDIER_INSET_X;     insetY = SOLDIER_INSET_Y; }
		else                            { insetX = BOSS_INSET_X;        insetY = BOSS_INSET_Y; }

		float ex = x + insetX;
		float ey = y + insetY;
		float ew = (float)width - 2.0f * insetX;
		float eh = (float)height - 2.0f * insetY;

		// Expand the enemy's cropped box outward by the reach being tested.
		ex -= range;
		ey -= range;
		ew += range * 2.0f;
		eh += range * 2.0f;

		float px = playerX + HERO_INSET_LEFT;
		float py = playerY + HERO_INSET_BOTTOM;
		float pw = 100.0f - (HERO_INSET_LEFT + HERO_INSET_RIGHT);
		float ph = 100.0f - (HERO_INSET_BOTTOM + HERO_INSET_TOP);

		return (ex < px + pw) && (ex + ew > px) &&
		       (ey < py + ph) && (ey + eh > py);
	}

	void takeDamage(int amount)
	{
		if (!alive) return;
		hp -= amount;
		if (hp <= 0)
		{
			hp = 0;
			alive = false;
			state = ENEMY3_DEAD;
			score += scoreValue;
		}
		else
		{
			state = ENEMY3_HURT;
		}
	}

	void update(EnemyManager3* mgr);
	void draw();
};

/* =========================================================
ENEMY MANAGER CLASS (LEVEL 3)
========================================================= */
class EnemyManager3
{
public:
	Enemy3 activeEnemies[MAX_ACTIVE_ENEMIES3];
	Bullet_pro bullets[MAX_BULLETS_PRO];
	HealingPickup healingPickups[MAX_HEALING_PICKUPS];

	Level3Wave waveState;
	int waveBlueDefeated;
	int waveSoldierDefeated;
	bool waveBossDefeated;
	int waveEscortsDefeated;
	bool waitingToRespawn;
	int respawnTimer;

	EnemyManager3()
	{
		waveState = L3_WAVE1_BLUES;
		waveBlueDefeated = 0;
		waveSoldierDefeated = 0;
		waveBossDefeated = false;
		waveEscortsDefeated = 0;
		waitingToRespawn = false;
		respawnTimer = 0;
	}

	static float enemyFloorY(int index)
	{
		// Level 3 spawns every regular enemy on the ground.
		return ENEMY3_GROUND_Y;
	}

	static float getWave1BlueX(int idx)
	{
		/* Every enemy is spawned to the RIGHT of the relevant cover. */
		static const float x3[WAVE1_BLUE_COUNT] = { 1050.0f, 1430.0f, 1930.0f, 2630.0f };
		return x3[idx];
	}

	static float getWave2BlueX(int idx)
	{
		static const float x3[WAVE2_BLUE_COUNT] = { 3050.0f, 3550.0f, 3750.0f, 3850.0f, 3950.0f };
		return x3[idx];
	}

	void init()
	{
		initEnemy3Textures();
		reset();
	}

	void reset()
	{
		waveState = L3_WAVE1_BLUES;
		waveBlueDefeated = 0;
		waveSoldierDefeated = 0;
		waveBossDefeated = false;
		waveEscortsDefeated = 0;
		waitingToRespawn = false;
		respawnTimer = 0;

		for (int i = 0; i < MAX_ACTIVE_ENEMIES3; i++)
			activeEnemies[i].alive = false;

		for (int i = 0; i < MAX_BULLETS_PRO; i++)
			bullets[i].active = false;

		for (int i = 0; i < MAX_HEALING_PICKUPS; i++)
			healingPickups[i].active = false;

		spawnNextInWave();
	}

	void spawnBullet(float bx, float by, float dir)
	{
		for (int i = 0; i < MAX_BULLETS_PRO; i++)
		{
			if (!bullets[i].active)
			{
				bullets[i].x = bx;
				bullets[i].y = by;
				bullets[i].dirX = dir;
				bullets[i].active = true;
				bullets[i].damage = SOLDIER_BULLET_DAMAGE;
				bullets[i].width = 34;
				bullets[i].height = 20;
				bullets[i].frame = 0;
				bullets[i].frameTimer = 0;
				break;
			}
		}
	}

	/* Drops one Healing.png pickup at the given world position. Used
	   for the "last Soldier of the wave" drop rule -- see
	   updateAll()'s death-handling branch below. */
	void spawnHealingPickup(float hx, float hy)
	{
		for (int i = 0; i < MAX_HEALING_PICKUPS; i++)
		{
			if (!healingPickups[i].active)
			{
				healingPickups[i].x = hx;
				healingPickups[i].y = hy;
				healingPickups[i].active = true;
				healingPickups[i].bobPhase = 0.0f;
				break;
			}
		}
	}

	void spawnNextInWave()
	{
		if (waveState == L3_WAVE1_BLUES)
		{
			if (waveBlueDefeated < WAVE1_BLUE_COUNT)
			{
				float bx = level3SafeEnemySpawnX(getWave1BlueX(waveBlueDefeated), enemyFloorY(waveBlueDefeated));
				activeEnemies[0].spawn(TYPE_BLUE, bx, enemyFloorY(waveBlueDefeated), 40, BLUE_ATTACK_DAMAGE, BLUE_ATTACK_RANGE, 25);
			}
			else
			{
				/* Last fight of wave 1: exactly WAVE1_SOLDIER_COUNT (2)
				   Soldiers spawn together. */
				waveState = L3_WAVE1_SOLDIERS;
				waveSoldierDefeated = 0;
				float s0x = level3SafeEnemySpawnX(2850.0f, ENEMY3_GROUND_Y);
				float s1x = level3SafeEnemySpawnX(3050.0f, ENEMY3_GROUND_Y);
				activeEnemies[0].spawn(TYPE_SOLDIER, s0x, ENEMY3_GROUND_Y, 65, SOLDIER_BULLET_DAMAGE, SOLDIER_KEEP_DIST, 50);
				activeEnemies[1].spawn(TYPE_SOLDIER, s1x, ENEMY3_GROUND_Y, 65, SOLDIER_BULLET_DAMAGE, SOLDIER_KEEP_DIST, 50);
			}
		}
		else if (waveState == L3_WAVE2_BLUES)
		{
			if (waveBlueDefeated < WAVE2_BLUE_COUNT)
			{
				float bx = level3SafeEnemySpawnX(getWave2BlueX(waveBlueDefeated), enemyFloorY(waveBlueDefeated));
				activeEnemies[0].spawn(TYPE_BLUE, bx, enemyFloorY(waveBlueDefeated), 50, BLUE_ATTACK_DAMAGE + 3, BLUE_ATTACK_RANGE, 30);
			}
			else
			{
				/* Last fight of wave 2: exactly WAVE2_SOLDIER_COUNT (2)
				   Soldiers spawn together. */
				waveState = L3_WAVE2_SOLDIERS;
				waveSoldierDefeated = 0;
				float s0x = level3SafeEnemySpawnX(3650.0f, ENEMY3_GROUND_Y);
				float s1x = level3SafeEnemySpawnX(3850.0f, ENEMY3_GROUND_Y);
				activeEnemies[0].spawn(TYPE_SOLDIER, s0x, ENEMY3_GROUND_Y, 80, SOLDIER_BULLET_DAMAGE + 5, SOLDIER_KEEP_DIST, 60);
				activeEnemies[1].spawn(TYPE_SOLDIER, s1x, ENEMY3_GROUND_Y, 80, SOLDIER_BULLET_DAMAGE + 5, SOLDIER_KEEP_DIST, 60);
			}
		}
	}

	/* Wave 3: all 5 (4 escorts + Boss) spawn together, so the Boss is
	   visible from the start -- but the Boss stays passive (won't
	   move, attack, or take damage; see Enemy3::update() and
	   playerAttack()) until all 4 escorts are dead. Level 3 only --
	   Level 4's ending is untouched. */
	void spawnBossWave()
	{
		waveState = L3_WAVE3_BOSS;
		waveBossDefeated = false;
		waveEscortsDefeated = 0;

		activeEnemies[0].spawn(TYPE_BLUE, level3SafeEnemySpawnX(BOSS_ESCORT_BLUE1_X, ENEMY3_GROUND_Y), ENEMY3_GROUND_Y, 55, BLUE_ATTACK_DAMAGE + 5, BLUE_ATTACK_RANGE, 35);
		activeEnemies[1].spawn(TYPE_BLUE, level3SafeEnemySpawnX(BOSS_ESCORT_BLUE2_X, ENEMY3_GROUND_Y), ENEMY3_GROUND_Y, 55, BLUE_ATTACK_DAMAGE + 5, BLUE_ATTACK_RANGE, 35);
		activeEnemies[2].spawn(TYPE_BLUE, level3SafeEnemySpawnX(BOSS_ESCORT_BLUE3_X, ENEMY3_GROUND_Y), ENEMY3_GROUND_Y, 55, BLUE_ATTACK_DAMAGE + 5, BLUE_ATTACK_RANGE, 35);
		activeEnemies[3].spawn(TYPE_BLUE, level3SafeEnemySpawnX(BOSS_ESCORT_BLUE4_X, ENEMY3_GROUND_Y), ENEMY3_GROUND_Y, 55, BLUE_ATTACK_DAMAGE + 5, BLUE_ATTACK_RANGE, 35);
		activeEnemies[4].spawn(TYPE_BOSS, level3SafeEnemySpawnX(BOSS_SPAWN_X, BOSS_SPAWN_Y, BOSS_WIDTH), BOSS_SPAWN_Y, BOSS_HP, BOSS_ATTACK_DAMAGE, BOSS_ATTACK_RANGE, BOSS_SCORE_VALUE, BOSS_WIDTH, BOSS_HEIGHT);
	}

	void updateBullets()
	{
		for (int i = 0; i < MAX_BULLETS_PRO; i++)
		{
			if (!bullets[i].active) continue;

			bullets[i].x += bullets[i].dirX * SOLDIER_BULLET_SPEED;

			bullets[i].frameTimer++;
			if (bullets[i].frameTimer >= BULLET_ANIM_SPEED)
			{
				bullets[i].frameTimer = 0;
				bullets[i].frame = (bullets[i].frame + 1) % BULLET_PRO_FRAMES;
			}

			float pxMin = playerX + HERO_INSET_LEFT;
			float pxMax = playerX + 100.0f - HERO_INSET_RIGHT;
			float pyMin = playerY + HERO_INSET_BOTTOM;
			float pyMax = playerY + 100.0f - HERO_INSET_TOP;

			bool hitX = (bullets[i].x + bullets[i].width >= pxMin) && (bullets[i].x <= pxMax);
			bool hitY = (bullets[i].y + bullets[i].height >= pyMin) && (bullets[i].y <= pyMax);

			if (hitX && hitY)
			{
				bool blocked = isLevel3CoverBetween(playerX, playerY, bullets[i].x, bullets[i].y);
				if (blocked) { bullets[i].active = false; continue; }
				health -= bullets[i].damage;
				if (health < 0) health = 0;
				bullets[i].active = false;
				continue;
			}

			float screenX = bullets[i].x - cameraX;
			if (screenX < -100 || screenX > SCREEN_WIDTH + 100)
			{
				bullets[i].active = false;
			}
		}
	}

	/* Idle bob animation for pickups still lying on the ground; the
	   actual "walk over it, heal, remove it" logic lives in
	   collision3.h (handleLevel3HealingPickupCollisions), which is
	   called every frame from updateLevel3Collisions(). */
	void updateHealingPickups()
	{
		for (int i = 0; i < MAX_HEALING_PICKUPS; i++)
		{
			if (!healingPickups[i].active) continue;
			healingPickups[i].bobPhase += HEALING_PICKUP_BOB_SPEED;
		}
	}

	void drawBullets()
	{
		for (int i = 0; i < MAX_BULLETS_PRO; i++)
		{
			if (!bullets[i].active) continue;

			float screenX = bullets[i].x - cameraX;
			unsigned int tex = (bullets[i].dirX < 0)
				? bulletProLeftTex[bullets[i].frame]
				: bulletProRightTex[bullets[i].frame];

			if (tex != 0)
			{
				drawEnemy3Texture((int)screenX, (int)bullets[i].y, bullets[i].width, bullets[i].height, tex);
			}
			else
			{
				/* Fallback if an asset failed to load, so a shot is
				   still visible instead of silently disappearing. */
				iSetColor(255, 215, 0);
				iFilledRectangle((int)screenX, (int)bullets[i].y, bullets[i].width, bullets[i].height);
				iSetColor(255, 255, 255);
				iRectangle((int)screenX, (int)bullets[i].y, bullets[i].width, bullets[i].height);
			}
		}
	}

	void drawHealingPickups()
	{
		for (int i = 0; i < MAX_HEALING_PICKUPS; i++)
		{
			if (!healingPickups[i].active) continue;

			float screenX = healingPickups[i].x - cameraX;
			float bobY = sinf(healingPickups[i].bobPhase) * HEALING_PICKUP_BOB_RANGE;

			if (healingPickupTex != 0)
			{
				drawEnemy3Texture((int)screenX, (int)(healingPickups[i].y + bobY),
					HEALING_PICKUP_WIDTH, HEALING_PICKUP_HEIGHT, healingPickupTex);
			}
			else
			{
				iSetColor(60, 200, 90);
				iFilledRectangle((int)screenX, (int)(healingPickups[i].y + bobY),
					HEALING_PICKUP_WIDTH, HEALING_PICKUP_HEIGHT);
			}
		}
	}

	void updateAll()
	{
		int currentActiveCount = 0;

		for (int i = 0; i < MAX_ACTIVE_ENEMIES3; i++)
		{
			if (activeEnemies[i].alive)
			{
				activeEnemies[i].update(this);
				currentActiveCount++;
			}
			else if (activeEnemies[i].state == ENEMY3_DEAD)
			{
				activeEnemies[i].state = ENEMY3_STANDING;
				if (waveState == L3_WAVE1_BLUES || waveState == L3_WAVE2_BLUES)
				{
					waveBlueDefeated++;
					waitingToRespawn = true;
					respawnTimer = ENEMY3_RESPAWN_DELAY;
				}
				else if (waveState == L3_WAVE1_SOLDIERS || waveState == L3_WAVE2_SOLDIERS)
				{
					waveSoldierDefeated++;

					/* Healing kit rule: the LAST Soldier of the wave
					   drops one Healing.png pickup right where it
					   fell. */
					bool isLastSoldierOfWave =
						(waveState == L3_WAVE1_SOLDIERS && waveSoldierDefeated == WAVE1_SOLDIER_COUNT) ||
						(waveState == L3_WAVE2_SOLDIERS && waveSoldierDefeated == WAVE2_SOLDIER_COUNT);

					if (isLastSoldierOfWave)
					{
						spawnHealingPickup(activeEnemies[i].x, activeEnemies[i].y);
					}
				}
				else if (waveState == L3_WAVE3_BOSS)
				{
					/* No healing kit drop in wave 3 (matches existing
					   design note). */
					if (activeEnemies[i].type == TYPE_BOSS)
					{
						waveBossDefeated = true;
					}
					else
					{
						/* One of the 4 escorts just died -- once all of
						   them are gone, the Boss stops being passive
						   (see Enemy3::update() and playerAttack()). */
						waveEscortsDefeated++;
					}
				}
			}
		}

		updateBullets();
		updateHealingPickups();

		/* Gate: exactly like Level 2's sequential spawn/lock -- the
		   next wave's enemies only ever come into existence once the
		   current wave is fully cleared (currentActiveCount == 0 and
		   every kill in the wave accounted for), so the player simply
		   cannot progress the encounter until then. */
		if (waveState == L3_WAVE1_SOLDIERS && waveSoldierDefeated >= WAVE1_SOLDIER_COUNT && currentActiveCount == 0)
		{
			waveState = L3_WAVE2_BLUES;
			waveBlueDefeated = 0;
			waitingToRespawn = true;
			respawnTimer = ENEMY3_RESPAWN_DELAY;
		}
		else if (waveState == L3_WAVE2_SOLDIERS && waveSoldierDefeated >= WAVE2_SOLDIER_COUNT && currentActiveCount == 0)
		{
			spawnBossWave();
		}
		else if (waveState == L3_WAVE3_BOSS && waveBossDefeated && currentActiveCount == 0)
		{
			waveState = L3_COMPLETE;
			storyStart(STORY_LEVEL3_END);
		}

		if (waitingToRespawn)
		{
			respawnTimer--;
			if (respawnTimer <= 0)
			{
				waitingToRespawn = false;
				spawnNextInWave();
			}
		}

		if (health <= 0)
		{
			health = 0;
			currentState = GAME_OVER;
		}
	}

	void drawAll()
	{
		for (int i = 0; i < MAX_ACTIVE_ENEMIES3; i++)
		{
			if (activeEnemies[i].alive)
				activeEnemies[i].draw();
		}
		drawBullets();
		drawHealingPickups();
	}

	void playerAttack()
	{
		for (int i = 0; i < MAX_ACTIVE_ENEMIES3; i++)
		{
			if (!activeEnemies[i].alive) continue;
			/* Boss is shielded by its escorts -- can't be hurt until
			   all 4 are dead (matches its passive state in
			   Enemy3::update()). */
			if (activeEnemies[i].type == TYPE_BOSS && waveEscortsDefeated < BOSS_ESCORT_COUNT) continue;
			if (activeEnemies[i].playerInRange(PLAYER3_ATTACK_RANGE))
			{
				activeEnemies[i].takeDamage(PLAYER3_ATTACK_DAMAGE);
			}
		}
	}
};

inline void Enemy3::update(EnemyManager3* mgr)
{
	if (!alive) return;

	frameTimer++;
	faceTowardPlayer();

	if (state == ENEMY3_HURT)
	{
		if (frameTimer >= ENEMY3_ANIM_SPEED)
		{
			frameTimer = 0;
			state = ENEMY3_WALKING;
		}
		return;
	}

	if (type == TYPE_BLUE)
	{
		if (attackCooldownTimer > 0) attackCooldownTimer--;

		bool inAttack = playerInRange(attackRange);

		if (state == ENEMY3_FIGHTING)
		{
			if (frameTimer >= ENEMY3_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame++;
				if (currentFrame >= BLUE_FIGHT_FRAMES)
				{
					if (inAttack)
					{
						bool blocked = isLevel3CoverBetween(playerX, playerY, x, y);
						if (blocked) { currentFrame = 0; attackCooldownTimer = BLUE_ATTACK_COOLDOWN; state = ENEMY3_STANDING; return; }
						health -= damage;
						if (health < 0) health = 0;
					}
					currentFrame = 0;
					attackCooldownTimer = BLUE_ATTACK_COOLDOWN;
					state = ENEMY3_WALKING;
				}
			}
		}
		else
		{
			if (frameTimer >= ENEMY3_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame = (currentFrame + 1) % BLUE_WALK_FRAMES;
			}

			if (inAttack && attackCooldownTimer == 0)
			{
				state = ENEMY3_FIGHTING;
				currentFrame = 0;
				frameTimer = 0;
			}
			else
			{
				state = ENEMY3_WALKING;
				float oldX = x;
				float nextX = x + ((facing == ENEMY3_FACING_RIGHT) ? BLUE_WALK_SPEED : -BLUE_WALK_SPEED);
				if (Level3EnemyMovementBlocked(oldX, nextX, y))
					state = ENEMY3_STANDING;
				else
					x = nextX;
			}
		}
	}
	else if (type == TYPE_SOLDIER)
	{
		/* Soldiers never close in like Enemy3_Blue -- they hold
		   SOLDIER_KEEP_DIST, backing off (RUNNING) if the hero gets
		   too close and closing in (WALKING) if the hero drifts too
		   far, then periodically stop to fire a Bullet_pro shot. */
		shootTimer++;
		float dist = (float)fabs(playerX - x);

		if (state == ENEMY3_FIGHTING)
		{
			if (frameTimer >= ENEMY3_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame++;
				if (currentFrame >= SOLDIER_FIGHT_FRAMES)
				{
					float bulletSpawnX = (facing == ENEMY3_FACING_RIGHT) ? (x + width * 0.6f) : (x - 10.0f);
					float bulletSpawnY = y + (height * 0.55f);
					mgr->spawnBullet(bulletSpawnX, bulletSpawnY, (float)facing);

					currentFrame = 0;
					state = ENEMY3_STANDING;
				}
			}
		}
		else if (shootTimer >= SOLDIER_FIRE_RATE)
		{
			shootTimer = 0;
			state = ENEMY3_FIGHTING;
			currentFrame = 0;
			frameTimer = 0;
		}
		else if (dist < (SOLDIER_KEEP_DIST - SOLDIER_DIST_TOLERANCE))
		{
			if (state != ENEMY3_RUNNING) { state = ENEMY3_RUNNING; currentFrame = 0; frameTimer = 0; }

			if (frameTimer >= ENEMY3_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame = (currentFrame + 1) % SOLDIER_RUN_FRAMES;
			}

			float oldX = x;
			float nextX;
			if (playerX < x)
			{
				nextX = x + SOLDIER_RUN_SPEED;
				facing = ENEMY3_FACING_LEFT;
			}
			else
			{
				nextX = x - SOLDIER_RUN_SPEED;
				facing = ENEMY3_FACING_RIGHT;
			}
			bool blocked = Level3EnemyMovementBlocked(oldX, nextX, y);
			if (!blocked) x = nextX;
		}
		else if (dist > (SOLDIER_KEEP_DIST + SOLDIER_DIST_TOLERANCE))
		{
			if (state != ENEMY3_WALKING) { state = ENEMY3_WALKING; currentFrame = 0; frameTimer = 0; }

			if (frameTimer >= ENEMY3_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame = (currentFrame + 1) % SOLDIER_WALK_FRAMES;
			}

			float oldX = x;
			float nextX = (playerX > x) ? x + SOLDIER_WALK_SPEED : x - SOLDIER_WALK_SPEED;
			bool blocked = Level3EnemyMovementBlocked(oldX, nextX, y);
			if (!blocked) x = nextX;
		}
		else
		{
			state = ENEMY3_STANDING;
			currentFrame = 0;
		}
	}
	else if (type == TYPE_BOSS)
	{
		/* Stays passive (just stands there, won't move or attack)
		   while any of its 4 escorts are still alive -- see
		   waveEscortsDefeated in EnemyManager3. It's also shielded
		   from player damage during this time (see playerAttack()). */
		if (mgr->waveEscortsDefeated < BOSS_ESCORT_COUNT)
		{
			state = ENEMY3_STANDING;
			return;
		}

		/* Melee fighter like TYPE_BLUE, but bigger, tankier, hits
		   harder, and on its own frame counts/cooldown. */
		if (attackCooldownTimer > 0) attackCooldownTimer--;

		bool inAttack = playerInRange(attackRange);

		if (state == ENEMY3_FIGHTING)
		{
			if (frameTimer >= ENEMY3_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame++;
				if (currentFrame >= BOSS_FIGHT_FRAMES)
				{
					if (inAttack)
					{
						bool blocked = isLevel3CoverBetween(playerX, playerY, x, y);
						if (blocked) { currentFrame = 0; attackCooldownTimer = BOSS_ATTACK_COOLDOWN; state = ENEMY3_STANDING; return; }
						health -= damage;
						if (health < 0) health = 0;
					}
					currentFrame = 0;
					attackCooldownTimer = BOSS_ATTACK_COOLDOWN;
					state = ENEMY3_WALKING;
				}
			}
		}
		else
		{
			if (frameTimer >= ENEMY3_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame = (currentFrame + 1) % BOSS_WALK_FRAMES;
			}

			if (inAttack && attackCooldownTimer == 0)
			{
				state = ENEMY3_FIGHTING;
				currentFrame = 0;
				frameTimer = 0;
			}
			else
			{
				state = ENEMY3_WALKING;
				float oldX = x;
				float nextX = x + ((facing == ENEMY3_FACING_RIGHT) ? BOSS_WALK_SPEED : -BOSS_WALK_SPEED);
				if (Level3EnemyMovementBlocked(oldX, nextX, y))
					state = ENEMY3_STANDING;
				else
					x = nextX;
			}
		}
	}
}

inline void Enemy3::draw()
{
	if (!alive) return;

	float screenX = x - cameraX;
	unsigned int tex = 0;

	if (type == TYPE_BLUE)
	{
		if (state == ENEMY3_FIGHTING)
			tex = (facing == ENEMY3_FACING_LEFT) ? enemy3BlueFightLeftTex[currentFrame] : enemy3BlueFightRightTex[currentFrame];
		else if (state == ENEMY3_WALKING)
			tex = (facing == ENEMY3_FACING_LEFT) ? enemy3BlueWalkLeftTex[currentFrame] : enemy3BlueWalkRightTex[currentFrame];
		else
			tex = (facing == ENEMY3_FACING_LEFT) ? enemy3BlueStandLeftTex : enemy3BlueStandRightTex;
	}
	else if (type == TYPE_SOLDIER)
	{
		if (state == ENEMY3_FIGHTING)
			tex = (facing == ENEMY3_FACING_LEFT) ? soldierFightLeftTex[currentFrame] : soldierFightRightTex[currentFrame];
		else if (state == ENEMY3_RUNNING)
			tex = (facing == ENEMY3_FACING_LEFT) ? soldierRunLeftTex[currentFrame] : soldierRunRightTex[currentFrame];
		else if (state == ENEMY3_WALKING)
			tex = (facing == ENEMY3_FACING_LEFT) ? soldierWalkLeftTex[currentFrame] : soldierWalkRightTex[currentFrame];
		else
			tex = (facing == ENEMY3_FACING_LEFT) ? soldierStandLeftTex : soldierStandRightTex;
	}
	else if (type == TYPE_BOSS)
	{
		if (state == ENEMY3_FIGHTING)
			tex = (facing == ENEMY3_FACING_LEFT) ? bossFightLeftTex[currentFrame] : bossFightRightTex[currentFrame];
		else if (state == ENEMY3_WALKING)
			tex = (facing == ENEMY3_FACING_LEFT) ? bossWalkLeftTex[currentFrame] : bossWalkRightTex[currentFrame];
		else
			tex = (facing == ENEMY3_FACING_LEFT) ? bossStandLeftTex : bossStandRightTex;
	}

	drawEnemy3Texture((int)screenX, (int)y, width, height, tex);

	int barW = (int)(width * 0.55f);
	int barH = 6;
	int bx = (int)screenX + (width - barW) / 2;
	int by = (int)y + height + 4;

	iSetColor(60, 20, 20);
	iFilledRectangle(bx, by, barW, barH);
	iSetColor(180, 40, 40);
	iFilledRectangle(bx, by, barW * hp / maxHp, barH);
	iSetColor(230, 210, 170);
	iRectangle(bx, by, barW, barH);
}

/* Global Instance & External Wrapper Functions */
static EnemyManager3 enemyManager3;

inline void initEnemies3()         { enemyManager3.init(); }
inline void resetEnemies3()        { enemyManager3.reset(); }
inline void updateEnemies3()       { enemyManager3.updateAll(); }
inline void drawEnemies3()         { enemyManager3.drawAll(); }
inline void playerAttackEnemies3() { enemyManager3.playerAttack(); }
