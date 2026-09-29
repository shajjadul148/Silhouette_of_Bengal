#pragma once

#include "globals.h"
#include "iGraphics.h"
#include "Level4Obstacle.h"   // isLevel4CoverBetween / Level4EnemyMovementBlocked
#include <math.h>
#include <stdlib.h>

/* External game variables */
extern float playerX, playerY;
extern float cameraX;
extern int score;
extern int health;
extern GameState currentState;

/* =========================================================
LEVEL 4 -- SOLDIER4 & ENEMY4 TUNABLES
(Independent copy of the Level 3 tuning -- no shared enemy
logic with enemy3.h / collision3.h. Level 4 uses its own
Images\Enemy_4\... asset pack.)
========================================================= */
#define SOLDIER4_FIRE_RATE         75       // Frames between gunshots
#define SOLDIER4_BULLET_SPEED      8.5f     // Projectile speed (px/frame)
#define SOLDIER4_BULLET_DAMAGE     20       // Damage dealt to hero per bullet hit (100 HP / 20 = 5 bullets to kill hero)
#define SOLDIER4_KEEP_DIST         80.0f    // Distance soldier tries to keep from hero
#define SOLDIER4_DIST_TOLERANCE    50.0f    // Margin around target distance before moving
#define SOLDIER4_RUN_SPEED         3.2f     // Speed when retreating (backing off)
#define SOLDIER4_WALK_SPEED        1.8f     // Speed when closing distance

#define BLUE4_WALK_SPEED           2.2f
#define BLUE4_ATTACK_RANGE         45.0f
#define BLUE4_ATTACK_DAMAGE        12
#define BLUE4_ATTACK_COOLDOWN      45

/* Boss (Level 4, Wave 3 -- spawns after Wave 2 is cleared).
   Moved back from the old BOSS4_SPAWN_X = 3950.0f to 3550.0f so
   Wave 4 (Boss_2) has room to occupy the tail end of the world
   without extending LEVEL4_WIDTH. All earlier waves were compacted
   to match -- see getWave1BlueX/getWave2BlueX and the Soldier
   spawn coordinates in spawnNextInWave() below. */
#define BOSS4_HP                   300
#define BOSS4_WIDTH                100
#define BOSS4_HEIGHT               100
#define BOSS4_WALK_SPEED           1.6f
#define BOSS4_ATTACK_RANGE         65.0f
#define BOSS4_ATTACK_DAMAGE        25
#define BOSS4_ATTACK_COOLDOWN      55
#define BOSS4_SCORE_VALUE          200
#define BOSS4_WALK_FRAMES          5
#define BOSS4_FIGHT_FRAMES         7
#define BOSS4_SPAWN_X              3550.0f
#define BOSS4_SPAWN_Y              70.0f
/* Wave 3 escorts (2 Soldiers + 2 Blues) are placed to the left of
   the boss so the player meets them before reaching it. */
#define BOSS4_ESCORT_BLUE1_X       (BOSS4_SPAWN_X - 600.0f)
#define BOSS4_ESCORT_BLUE2_X       (BOSS4_SPAWN_X - 450.0f)
#define BOSS4_ESCORT_SOLDIER1_X    (BOSS4_SPAWN_X - 300.0f)
#define BOSS4_ESCORT_SOLDIER2_X    (BOSS4_SPAWN_X - 150.0f)

/* Boss_2 (Level 4, Wave 4 -- final encounter, spawns after Wave 3's
   Boss1 is cleared). Uses the previously-unused Images\Enemy_4\Boss_2
   sprite set. Deliberately stronger than Boss1 since it's the level's
   last fight, and escorted by 6 bodyguards (3 Blue + 3 Soldier)
   instead of Boss1's 4. Placed right at the tail of the 4096px-wide
   world so the final confrontation happens at the world's edge. */
#define BOSS2_HP                   450
#define BOSS2_WIDTH                100
#define BOSS2_HEIGHT               100
#define BOSS2_WALK_SPEED           1.8f
#define BOSS2_ATTACK_RANGE         70.0f
#define BOSS2_ATTACK_DAMAGE        32
#define BOSS2_ATTACK_COOLDOWN      45
#define BOSS2_SCORE_VALUE          350
#define BOSS2_WALK_FRAMES          4
#define BOSS2_FIGHT_FRAMES         6
#define BOSS2_SPAWN_X              4000.0f
#define BOSS2_SPAWN_Y              70.0f
/* Wave 4 escorts (3 Blues + 3 Soldiers) spaced 150px apart behind
   the boss, same convention as Boss1's escort offsets. */
#define BOSS2_ESCORT_BLUE1_X       (BOSS2_SPAWN_X - 900.0f)
#define BOSS2_ESCORT_BLUE2_X       (BOSS2_SPAWN_X - 750.0f)
#define BOSS2_ESCORT_BLUE3_X       (BOSS2_SPAWN_X - 600.0f)
#define BOSS2_ESCORT_SOLDIER1_X    (BOSS2_SPAWN_X - 450.0f)
#define BOSS2_ESCORT_SOLDIER2_X    (BOSS2_SPAWN_X - 300.0f)
#define BOSS2_ESCORT_SOLDIER3_X    (BOSS2_SPAWN_X - 150.0f)
/* Escort toughness: a notch above Boss1's escorts (55/90 HP),
   matching the "final wave" difficulty bump. */
#define BOSS2_ESCORT_BLUE_HP       60
#define BOSS2_ESCORT_SOLDIER_HP    100
#define BOSS2_ESCORT_BLUE_SCORE    40
#define BOSS2_ESCORT_SOLDIER_SCORE 70

#define ENEMY4_COLORKEY_TOLERANCE  24
#define ENEMY4_GROUND_Y            70.0f

/* Visual-only ground correction, in pixels, applied at DRAW time
   only (never to the real `y` used by AI/collision/spawn-margin
   logic). Each enemy type's sprite sheet has a different amount of
   empty transparent margin baked in below the actual character art
   (measured from the standing frames: Blue ~0%, Soldier ~3%,
   Boss1 ~4%, Boss2 ~12% of the 100px draw height), so drawing every
   type at the same raw `y` made some enemies look like they were
   floating above the ground line the obstacles/hero sit on instead
   of standing flush with it. See Enemy4::draw(). */
#define ENEMY4_BLUE_VISUAL_Y_OFFSET    0.0f
#define ENEMY4_SOLDIER_VISUAL_Y_OFFSET 3.0f
#define ENEMY4_BOSS_VISUAL_Y_OFFSET    4.0f
#define ENEMY4_BOSS2_VISUAL_Y_OFFSET   12.0f

#define MAX_BULLETS_PRO4           30
#define ENEMY4_ANIM_SPEED          6
#define ENEMY4_RESPAWN_DELAY       40

/* Sprite-sheet frame counts (must match the files actually shipped
   under Images\Enemy_4\..., see the file-path tables below) */
#define BLUE4_WALK_FRAMES          3
#define BLUE4_FIGHT_FRAMES         5
#define SOLDIER4_WALK_FRAMES       6
#define SOLDIER4_RUN_FRAMES        8
#define SOLDIER4_FIGHT_FRAMES      6
#define BULLET4_PRO_FRAMES         2
#define BULLET4_ANIM_SPEED         5

/* Ambush waves (extra mixed groups that spawn AROUND the hero, just
   off-screen, so the fights last longer and come from both sides).
   Order in the level:
     W1 blues -> W1 soldiers -> AMBUSH 1 -> W2 blues -> AMBUSH 2 ->
     W2 soldiers -> AMBUSH 3 -> Boss1 wave -> AMBUSH 4 -> Boss2 wave
   Tune the counts below to make the level longer or shorter.
   Keep (blues + soldiers) <= MAX_ACTIVE_ENEMIES4 (7). */
#define LEVEL4_AMBUSH_COUNT        4
#define AMBUSH4_SPAWN_BASE_OFFSET  560.0f   // px from hero (just off-screen)
#define AMBUSH4_SPAWN_STEP         90.0f    // extra px between group members
#define AMBUSH4_MIN_X              120.0f
#define AMBUSH4_MAX_X              (LEVEL4_WIDTH - 220.0f)

/* Wave composition constants */
#define WAVE1_BLUE4_COUNT          4
#define WAVE1_SOLDIER4_COUNT       2
#define WAVE2_BLUE4_COUNT          5
#define WAVE2_SOLDIER4_COUNT       2
/* Wave 3 boss fight: 4 escorts (2 Soldiers + 2 Blues) fight first,
   then the Boss spawns once all four are dead. */
#define BOSS4_ESCORT_COUNT         4
/* Wave 4 boss fight: 6 escorts (3 Soldiers + 3 Blues) fight first,
   then Boss_2 spawns once all six are dead. This is the largest
   simultaneous encounter (6 escorts + boss = 7), so it sizes the
   active-enemy pool below. */
#define BOSS2_ESCORT_COUNT         6
#define MAX_ACTIVE_ENEMIES4        (BOSS2_ESCORT_COUNT + 1)

/* Level 4: number of hero bullets needed to kill each enemy type. */
#define L4_BULLETS_TO_KILL_NORMAL   3    // Blue / normal enemy
#define L4_BULLETS_TO_KILL_SOLDIER  5
#define L4_BULLETS_TO_KILL_BOSS1    7
#define L4_BULLETS_TO_KILL_BOSS2    10

#define PLAYER4_ATTACK_RANGE       40.0f
#define PLAYER4_ATTACK_DAMAGE      20

/* Healing pickup (dropped when the last Soldier of a wave dies) */
#define MAX_HEALING_PICKUPS4       2
#define HEALING4_PICKUP_WIDTH      50
#define HEALING4_PICKUP_HEIGHT     50
#define HEALING4_PICKUP_AMOUNT     35      // HP restored to the hero on pickup
#define HEALING4_PICKUP_BOB_SPEED  0.04f   // idle floating-bob animation speed
#define HEALING4_PICKUP_BOB_RANGE  6.0f    // pixels the pickup bobs up/down

/* =========================================================
ENUMS
========================================================= */
enum Enemy4Type { TYPE4_BLUE, TYPE4_SOLDIER, TYPE4_BOSS, TYPE4_BOSS2 };
enum Enemy4Facing { ENEMY4_FACING_LEFT = -1, ENEMY4_FACING_RIGHT = 1 };
enum Enemy4State { ENEMY4_STANDING, ENEMY4_WALKING, ENEMY4_RUNNING, ENEMY4_FIGHTING, ENEMY4_HURT, ENEMY4_DEAD };
enum Level4Wave { L4_WAVE1_BLUES, L4_WAVE1_SOLDIERS, L4_WAVE2_BLUES, L4_WAVE2_SOLDIERS, L4_AMBUSH, L4_WAVE3_BOSS, L4_WAVE4_BOSS2, L4_COMPLETE };

/* =========================================================
BULLET PROJECTILE STRUCT
========================================================= */
struct Bullet_pro4
{
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
struct HealingPickup4
{
	float x, y;
	bool active;
	float bobPhase;
};

/* =========================================================
SPRITE FILE PATHS
(paths verified against the shipped Images\Enemy_4\... assets)
========================================================= */
/* Enemy3_Blue (100x100) -- folder name kept exactly as shipped */
static char enemy4BlueStandLeftFile[100] = "Images\\Enemy_4\\Enemy3_Blue\\Enemy2\\Standing\\standing_left.png";
static char enemy4BlueStandRightFile[100] = "Images\\Enemy_4\\Enemy3_Blue\\Enemy2\\Standing\\standing_right.png";

static char enemy4BlueWalkLeftFiles[BLUE4_WALK_FRAMES][100] = {
	"Images\\Enemy_4\\Enemy3_Blue\\Enemy2\\Walking\\walking_left_1.png",
	"Images\\Enemy_4\\Enemy3_Blue\\Enemy2\\Walking\\walking_left_2.png",
	"Images\\Enemy_4\\Enemy3_Blue\\Enemy2\\Walking\\walking_left_3.png"
};
static char enemy4BlueWalkRightFiles[BLUE4_WALK_FRAMES][100] = {
	"Images\\Enemy_4\\Enemy3_Blue\\Enemy2\\Walking\\walking_right_1.png",
	"Images\\Enemy_4\\Enemy3_Blue\\Enemy2\\Walking\\walking_right_2.png",
	"Images\\Enemy_4\\Enemy3_Blue\\Enemy2\\Walking\\walking_right_3.png"
};

static char enemy4BlueFightLeftFiles[BLUE4_FIGHT_FRAMES][100] = {
	"Images\\Enemy_4\\Enemy3_Blue\\Enemy2\\Fighting\\fighting_left_1.png",
	"Images\\Enemy_4\\Enemy3_Blue\\Enemy2\\Fighting\\fighting_left_2.png",
	"Images\\Enemy_4\\Enemy3_Blue\\Enemy2\\Fighting\\fighting_left_3.png",
	"Images\\Enemy_4\\Enemy3_Blue\\Enemy2\\Fighting\\fighting_left_4.png",
	"Images\\Enemy_4\\Enemy3_Blue\\Enemy2\\Fighting\\fighting_left_5.png"
};
static char enemy4BlueFightRightFiles[BLUE4_FIGHT_FRAMES][100] = {
	"Images\\Enemy_4\\Enemy3_Blue\\Enemy2\\Fighting\\fighting_right_1.png",
	"Images\\Enemy_4\\Enemy3_Blue\\Enemy2\\Fighting\\fighting_right_2.png",
	"Images\\Enemy_4\\Enemy3_Blue\\Enemy2\\Fighting\\fighting_right_3.png",
	"Images\\Enemy_4\\Enemy3_Blue\\Enemy2\\Fighting\\fighting_right_4.png",
	"Images\\Enemy_4\\Enemy3_Blue\\Enemy2\\Fighting\\fighting_right_5.png"
};

/* Soldier (Standing/Walking/Running/Fighting all 100x100) */
static char soldier4StandLeftFile[100] = "Images\\Enemy_4\\Soldier\\Standing\\Enemy3_standing_left.png";
static char soldier4StandRightFile[100] = "Images\\Enemy_4\\Soldier\\Standing\\Enemy3_standing_right.png";

static char soldier4WalkLeftFiles[SOLDIER4_WALK_FRAMES][100] = {
	"Images\\Enemy_4\\Soldier\\Walking\\walking_left_1.png",
	"Images\\Enemy_4\\Soldier\\Walking\\walking_left_2.png",
	"Images\\Enemy_4\\Soldier\\Walking\\walking_left_3.png",
	"Images\\Enemy_4\\Soldier\\Walking\\walking_left_4.png",
	"Images\\Enemy_4\\Soldier\\Walking\\walking_left_5.png",
	"Images\\Enemy_4\\Soldier\\Walking\\walking_left_6.png"
};
static char soldier4WalkRightFiles[SOLDIER4_WALK_FRAMES][100] = {
	"Images\\Enemy_4\\Soldier\\Walking\\walking_right_1.png",
	"Images\\Enemy_4\\Soldier\\Walking\\walking_right_2.png",
	"Images\\Enemy_4\\Soldier\\Walking\\walking_right_3.png",
	"Images\\Enemy_4\\Soldier\\Walking\\walking_right_4.png",
	"Images\\Enemy_4\\Soldier\\Walking\\walking_right_5.png",
	"Images\\Enemy_4\\Soldier\\Walking\\walking_right_6.png"
};

static char soldier4RunLeftFiles[SOLDIER4_RUN_FRAMES][100] = {
	"Images\\Enemy_4\\Soldier\\Running\\running_left_1.png",
	"Images\\Enemy_4\\Soldier\\Running\\running_left_2.png",
	"Images\\Enemy_4\\Soldier\\Running\\running_left_3.png",
	"Images\\Enemy_4\\Soldier\\Running\\running_left_4.png",
	"Images\\Enemy_4\\Soldier\\Running\\running_left_5.png",
	"Images\\Enemy_4\\Soldier\\Running\\running_left_6.png",
	"Images\\Enemy_4\\Soldier\\Running\\running_left_7.png",
	"Images\\Enemy_4\\Soldier\\Running\\running_left_8.png"
};
static char soldier4RunRightFiles[SOLDIER4_RUN_FRAMES][100] = {
	"Images\\Enemy_4\\Soldier\\Running\\running_right_1.png",
	"Images\\Enemy_4\\Soldier\\Running\\running_right_2.png",
	"Images\\Enemy_4\\Soldier\\Running\\running_right_3.png",
	"Images\\Enemy_4\\Soldier\\Running\\running_right_4.png",
	"Images\\Enemy_4\\Soldier\\Running\\running_right_5.png",
	"Images\\Enemy_4\\Soldier\\Running\\running_right_6.png",
	"Images\\Enemy_4\\Soldier\\Running\\running_right_7.png",
	"Images\\Enemy_4\\Soldier\\Running\\running_right_8.png"
};

/* NOTE: the shipped filenames really do say "fightinig" (typo baked
   into the asset files themselves) -- kept verbatim so the paths
   resolve on disk. */
static char soldier4FightLeftFiles[SOLDIER4_FIGHT_FRAMES][100] = {
	"Images\\Enemy_4\\Soldier\\Fighting\\fightinig_left_1.png",
	"Images\\Enemy_4\\Soldier\\Fighting\\fightinig_left_2.png",
	"Images\\Enemy_4\\Soldier\\Fighting\\fightinig_left_3.png",
	"Images\\Enemy_4\\Soldier\\Fighting\\fightinig_left_4.png",
	"Images\\Enemy_4\\Soldier\\Fighting\\fightinig_left_5.png",
	"Images\\Enemy_4\\Soldier\\Fighting\\fightinig_left_6.png"
};
static char soldier4FightRightFiles[SOLDIER4_FIGHT_FRAMES][100] = {
	"Images\\Enemy_4\\Soldier\\Fighting\\fightinig_right_1.png",
	"Images\\Enemy_4\\Soldier\\Fighting\\fightinig_right_2.png",
	"Images\\Enemy_4\\Soldier\\Fighting\\fightinig_right_3.png",
	"Images\\Enemy_4\\Soldier\\Fighting\\fightinig_right_4.png",
	"Images\\Enemy_4\\Soldier\\Fighting\\fightinig_right_5.png",
	"Images\\Enemy_4\\Soldier\\Fighting\\fightinig_right_6.png"
};

/* Boss1 (100x100 source assets, drawn at native 100x100 -- see
   BOSS4_WIDTH/HEIGHT). Used for Wave 3. */
static char boss4StandLeftFile[100] = "Images\\Enemy_4\\Boss\\Boss1_standing\\Boss1_standing_left.png";
static char boss4StandRightFile[100] = "Images\\Enemy_4\\Boss\\Boss1_standing\\Boss1_standing_right.png";

static char boss4WalkLeftFiles[BOSS4_WALK_FRAMES][100] = {
	"Images\\Enemy_4\\Boss\\Boss1_walking\\Boss1_walking_left_1.png",
	"Images\\Enemy_4\\Boss\\Boss1_walking\\Boss1_walking_left_2.png",
	"Images\\Enemy_4\\Boss\\Boss1_walking\\Boss1_walking_left_3.png",
	"Images\\Enemy_4\\Boss\\Boss1_walking\\Boss1_walking_left_4.png",
	"Images\\Enemy_4\\Boss\\Boss1_walking\\Boss1_walking_left_5.png"
};
static char boss4WalkRightFiles[BOSS4_WALK_FRAMES][100] = {
	"Images\\Enemy_4\\Boss\\Boss1_walking\\Boss1_walking_right_1.png",
	"Images\\Enemy_4\\Boss\\Boss1_walking\\Boss1_walking_right_2.png",
	"Images\\Enemy_4\\Boss\\Boss1_walking\\Boss1_walking_right_3.png",
	"Images\\Enemy_4\\Boss\\Boss1_walking\\Boss1_walking_right_4.png",
	"Images\\Enemy_4\\Boss\\Boss1_walking\\Boss1_walking_right_5.png"
};

static char boss4FightLeftFiles[BOSS4_FIGHT_FRAMES][100] = {
	"Images\\Enemy_4\\Boss\\Boss1_fighting\\Boss1_fighting_left_1.png",
	"Images\\Enemy_4\\Boss\\Boss1_fighting\\Boss1_fighting_left_2.png",
	"Images\\Enemy_4\\Boss\\Boss1_fighting\\Boss1_fighting_left_3.png",
	"Images\\Enemy_4\\Boss\\Boss1_fighting\\Boss1_fighting_left_4.png",
	"Images\\Enemy_4\\Boss\\Boss1_fighting\\Boss1_fighting_left_5.png",
	"Images\\Enemy_4\\Boss\\Boss1_fighting\\Boss1_fighting_left_6.png",
	"Images\\Enemy_4\\Boss\\Boss1_fighting\\Boss1_fighting_left_7.png"
};
static char boss4FightRightFiles[BOSS4_FIGHT_FRAMES][100] = {
	"Images\\Enemy_4\\Boss\\Boss1_fighting\\Boss1_fighting_right_1.png",
	"Images\\Enemy_4\\Boss\\Boss1_fighting\\Boss1_fighting_right_2.png",
	"Images\\Enemy_4\\Boss\\Boss1_fighting\\Boss1_fighting_right_3.png",
	"Images\\Enemy_4\\Boss\\Boss1_fighting\\Boss1_fighting_right_4.png",
	"Images\\Enemy_4\\Boss\\Boss1_fighting\\Boss1_fighting_right_5.png",
	"Images\\Enemy_4\\Boss\\Boss1_fighting\\Boss1_fighting_right_6.png",
	"Images\\Enemy_4\\Boss\\Boss1_fighting\\Boss1_fighting_right_7.png"
};

/* Boss_2 (100x100 source assets, drawn at native 100x100 -- see
   BOSS2_WIDTH/HEIGHT). Used for Wave 4, the level's final fight.
   Note: the pack also ships a Boss2_falling animation (5 frames/dir)
   that isn't wired in here -- left unused for now, same as how
   Boss1's set has no extra states beyond stand/walk/fight. */
static char boss2StandLeftFile[100] = "Images\\Enemy_4\\Boss_2\\Boss2_standing\\Boss2_standing_left.png";
static char boss2StandRightFile[100] = "Images\\Enemy_4\\Boss_2\\Boss2_standing\\Boss2_standing_right.png";

static char boss2WalkLeftFiles[BOSS2_WALK_FRAMES][100] = {
	"Images\\Enemy_4\\Boss_2\\Boss2_walking\\Boss2_walking_left_1.png",
	"Images\\Enemy_4\\Boss_2\\Boss2_walking\\Boss2_walking_left_2.png",
	"Images\\Enemy_4\\Boss_2\\Boss2_walking\\Boss2_walking_left_3.png",
	"Images\\Enemy_4\\Boss_2\\Boss2_walking\\Boss2_walking_left_4.png"
};
static char boss2WalkRightFiles[BOSS2_WALK_FRAMES][100] = {
	"Images\\Enemy_4\\Boss_2\\Boss2_walking\\Boss2_walking_right_1.png",
	"Images\\Enemy_4\\Boss_2\\Boss2_walking\\Boss2_walking_right_2.png",
	"Images\\Enemy_4\\Boss_2\\Boss2_walking\\Boss2_walking_right_3.png",
	"Images\\Enemy_4\\Boss_2\\Boss2_walking\\Boss2_walking_right_4.png"
};

static char boss2FightLeftFiles[BOSS2_FIGHT_FRAMES][100] = {
	"Images\\Enemy_4\\Boss_2\\Boss2_fighting\\Boss2_fighting_left_1.png",
	"Images\\Enemy_4\\Boss_2\\Boss2_fighting\\Boss2_fighting_left_2.png",
	"Images\\Enemy_4\\Boss_2\\Boss2_fighting\\Boss2_fighting_left_3.png",
	"Images\\Enemy_4\\Boss_2\\Boss2_fighting\\Boss2_fighting_left_4.png",
	"Images\\Enemy_4\\Boss_2\\Boss2_fighting\\Boss2_fighting_left_5.png",
	"Images\\Enemy_4\\Boss_2\\Boss2_fighting\\Boss2_fighting_left_6.png"
};
static char boss2FightRightFiles[BOSS2_FIGHT_FRAMES][100] = {
	"Images\\Enemy_4\\Boss_2\\Boss2_fighting\\Boss2_fighting_right_1.png",
	"Images\\Enemy_4\\Boss_2\\Boss2_fighting\\Boss2_fighting_right_2.png",
	"Images\\Enemy_4\\Boss_2\\Boss2_fighting\\Boss2_fighting_right_3.png",
	"Images\\Enemy_4\\Boss_2\\Boss2_fighting\\Boss2_fighting_right_4.png",
	"Images\\Enemy_4\\Boss_2\\Boss2_fighting\\Boss2_fighting_right_5.png",
	"Images\\Enemy_4\\Boss_2\\Boss2_fighting\\Boss2_fighting_right_6.png"
};

/* Bullet_pro projectile (100x100 source, drawn scaled down) */
static char bulletPro4LeftFiles[BULLET4_PRO_FRAMES][100] = {
	"Images\\Enemy_4\\Bullet\\Bullet_pro_left_1.png",
	"Images\\Enemy_4\\Bullet\\Bullet_pro_left_2.png"
};
static char bulletPro4RightFiles[BULLET4_PRO_FRAMES][100] = {
	"Images\\Enemy_4\\Bullet\\Bullet_pro_right_1.png",
	"Images\\Enemy_4\\Bullet\\Bullet_pro_right_2.png"
};

/* Healing pickup (50x50) */
static char healing4PickupFile[100] = "Images\\Enemy_4\\Healing.png";

/* =========================================================
TEXTURE STORAGE
========================================================= */
static unsigned int enemy4BlueStandLeftTex;
static unsigned int enemy4BlueStandRightTex;
static unsigned int enemy4BlueWalkLeftTex[BLUE4_WALK_FRAMES];
static unsigned int enemy4BlueWalkRightTex[BLUE4_WALK_FRAMES];
static unsigned int enemy4BlueFightLeftTex[BLUE4_FIGHT_FRAMES];
static unsigned int enemy4BlueFightRightTex[BLUE4_FIGHT_FRAMES];

static unsigned int soldier4StandLeftTex;
static unsigned int soldier4StandRightTex;
static unsigned int soldier4WalkLeftTex[SOLDIER4_WALK_FRAMES];
static unsigned int soldier4WalkRightTex[SOLDIER4_WALK_FRAMES];
static unsigned int soldier4RunLeftTex[SOLDIER4_RUN_FRAMES];
static unsigned int soldier4RunRightTex[SOLDIER4_RUN_FRAMES];
static unsigned int soldier4FightLeftTex[SOLDIER4_FIGHT_FRAMES];
static unsigned int soldier4FightRightTex[SOLDIER4_FIGHT_FRAMES];

static unsigned int boss4StandLeftTex;
static unsigned int boss4StandRightTex;
static unsigned int boss4WalkLeftTex[BOSS4_WALK_FRAMES];
static unsigned int boss4WalkRightTex[BOSS4_WALK_FRAMES];
static unsigned int boss4FightLeftTex[BOSS4_FIGHT_FRAMES];
static unsigned int boss4FightRightTex[BOSS4_FIGHT_FRAMES];

static unsigned int boss2StandLeftTex;
static unsigned int boss2StandRightTex;
static unsigned int boss2WalkLeftTex[BOSS2_WALK_FRAMES];
static unsigned int boss2WalkRightTex[BOSS2_WALK_FRAMES];
static unsigned int boss2FightLeftTex[BOSS2_FIGHT_FRAMES];
static unsigned int boss2FightRightTex[BOSS2_FIGHT_FRAMES];

static unsigned int bulletPro4LeftTex[BULLET4_PRO_FRAMES];
static unsigned int bulletPro4RightTex[BULLET4_PRO_FRAMES];

static unsigned int healing4PickupTex;

/* =========================================================
TEXTURE LOADING & RENDERING HELPERS
========================================================= */
inline unsigned int loadEnemy4Texture(const char filename[])
{
	int width, height, channels;
	unsigned char *data = stbi_load(filename, &width, &height, &channels, 4);
	if (!data) return 0;

	int nPixels = width * height;
	for (int i = 0; i < nPixels; i++)
	{
		unsigned char *p = data + i * 4;
		bool nearWhite =
			p[0] >= 255 - ENEMY4_COLORKEY_TOLERANCE &&
			p[1] >= 255 - ENEMY4_COLORKEY_TOLERANCE &&
			p[2] >= 255 - ENEMY4_COLORKEY_TOLERANCE;
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

inline void initEnemy4Textures()
{
	static bool texturesLoaded = false;
	if (texturesLoaded) return;

	enemy4BlueStandLeftTex = loadEnemy4Texture(enemy4BlueStandLeftFile);
	enemy4BlueStandRightTex = loadEnemy4Texture(enemy4BlueStandRightFile);
	for (int i = 0; i < BLUE4_WALK_FRAMES; i++)
	{
		enemy4BlueWalkLeftTex[i] = loadEnemy4Texture(enemy4BlueWalkLeftFiles[i]);
		enemy4BlueWalkRightTex[i] = loadEnemy4Texture(enemy4BlueWalkRightFiles[i]);
	}
	for (int i = 0; i < BLUE4_FIGHT_FRAMES; i++)
	{
		enemy4BlueFightLeftTex[i] = loadEnemy4Texture(enemy4BlueFightLeftFiles[i]);
		enemy4BlueFightRightTex[i] = loadEnemy4Texture(enemy4BlueFightRightFiles[i]);
	}

	soldier4StandLeftTex = loadEnemy4Texture(soldier4StandLeftFile);
	soldier4StandRightTex = loadEnemy4Texture(soldier4StandRightFile);
	for (int i = 0; i < SOLDIER4_WALK_FRAMES; i++)
	{
		soldier4WalkLeftTex[i] = loadEnemy4Texture(soldier4WalkLeftFiles[i]);
		soldier4WalkRightTex[i] = loadEnemy4Texture(soldier4WalkRightFiles[i]);
	}
	for (int i = 0; i < SOLDIER4_RUN_FRAMES; i++)
	{
		soldier4RunLeftTex[i] = loadEnemy4Texture(soldier4RunLeftFiles[i]);
		soldier4RunRightTex[i] = loadEnemy4Texture(soldier4RunRightFiles[i]);
	}
	for (int i = 0; i < SOLDIER4_FIGHT_FRAMES; i++)
	{
		soldier4FightLeftTex[i] = loadEnemy4Texture(soldier4FightLeftFiles[i]);
		soldier4FightRightTex[i] = loadEnemy4Texture(soldier4FightRightFiles[i]);
	}

	boss4StandLeftTex = loadEnemy4Texture(boss4StandLeftFile);
	boss4StandRightTex = loadEnemy4Texture(boss4StandRightFile);
	for (int i = 0; i < BOSS4_WALK_FRAMES; i++)
	{
		boss4WalkLeftTex[i] = loadEnemy4Texture(boss4WalkLeftFiles[i]);
		boss4WalkRightTex[i] = loadEnemy4Texture(boss4WalkRightFiles[i]);
	}
	for (int i = 0; i < BOSS4_FIGHT_FRAMES; i++)
	{
		boss4FightLeftTex[i] = loadEnemy4Texture(boss4FightLeftFiles[i]);
		boss4FightRightTex[i] = loadEnemy4Texture(boss4FightRightFiles[i]);
	}

	boss2StandLeftTex = loadEnemy4Texture(boss2StandLeftFile);
	boss2StandRightTex = loadEnemy4Texture(boss2StandRightFile);
	for (int i = 0; i < BOSS2_WALK_FRAMES; i++)
	{
		boss2WalkLeftTex[i] = loadEnemy4Texture(boss2WalkLeftFiles[i]);
		boss2WalkRightTex[i] = loadEnemy4Texture(boss2WalkRightFiles[i]);
	}
	for (int i = 0; i < BOSS2_FIGHT_FRAMES; i++)
	{
		boss2FightLeftTex[i] = loadEnemy4Texture(boss2FightLeftFiles[i]);
		boss2FightRightTex[i] = loadEnemy4Texture(boss2FightRightFiles[i]);
	}

	for (int i = 0; i < BULLET4_PRO_FRAMES; i++)
	{
		bulletPro4LeftTex[i] = loadEnemy4Texture(bulletPro4LeftFiles[i]);
		bulletPro4RightTex[i] = loadEnemy4Texture(bulletPro4RightFiles[i]);
	}

	healing4PickupTex = loadEnemy4Texture(healing4PickupFile);

	texturesLoaded = true;
}

inline void drawEnemy4Texture(int x, int y, int width, int height, unsigned int texture)
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

class EnemyManager4;

/* =========================================================
ENEMY4 CLASS
========================================================= */
class Enemy4
{
public:
	Enemy4Type type;
	float x, y;
	int width, height;
	int hp, maxHp;
	int damage;
	float attackRange;
	bool alive;
	int scoreValue;

	Enemy4Facing facing;
	Enemy4State state;
	int currentFrame;
	int frameTimer;
	int attackCooldownTimer;
	int shootTimer;

	Enemy4() { alive = false; }

	void spawn(Enemy4Type t, float px, float py, int hpVal, int dmgVal, float rangeVal, int scoreVal, int w = 100, int h = 100)
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

		facing = (playerX < x) ? ENEMY4_FACING_LEFT : ENEMY4_FACING_RIGHT;
		state = ENEMY4_STANDING;
		currentFrame = 0;
		frameTimer = 0;
		attackCooldownTimer = 0;
		shootTimer = rand() % 20;
	}

	void faceTowardPlayer()
	{
		float dx = playerX - x;
		if (dx > 6.0f) facing = ENEMY4_FACING_RIGHT;
		else if (dx < -6.0f) facing = ENEMY4_FACING_LEFT;
	}

	bool playerInRange(float range) const
	{
		float dx = playerX - x;
		if (dx < 0) dx = -dx;
		float dy = playerY - y;
		if (dy < 0) dy = -dy;
		// Enemies can fight only when Hero is on the same floor.
		return dx <= range && dy <= 55.0f;
	}

	/* One hero bullet hit. Damage is scaled from this enemy's max HP so it
	   dies after exactly the required number of bullets (also keeps the
	   health bar draining evenly). */
	void takeBulletHit()
	{
		int need;
		switch (type)
		{
		case TYPE4_BLUE:    need = L4_BULLETS_TO_KILL_NORMAL;  break;
		case TYPE4_SOLDIER: need = L4_BULLETS_TO_KILL_SOLDIER; break;
		case TYPE4_BOSS:    need = L4_BULLETS_TO_KILL_BOSS1;   break;
		default:            need = L4_BULLETS_TO_KILL_BOSS2;   break;
		}
		int dmg = (maxHp + need - 1) / need;   // ceil(maxHp / need)
		if (dmg < 1) dmg = 1;
		takeDamage(dmg);
	}

	void takeDamage(int amount)
	{
		if (!alive) return;
		hp -= amount;
		if (hp <= 0)
		{
			hp = 0;
			alive = false;
			state = ENEMY4_DEAD;
			score += scoreValue;
		}
		else
		{
			state = ENEMY4_HURT;
		}
	}

	void update(EnemyManager4* mgr);
	void draw();
};

/* =========================================================
ENEMY MANAGER CLASS (LEVEL 4)
Same wave-spawn pattern as EnemyManager3 (Level 3): two waves of
Blues -> Soldiers, then a Wave 3 boss fight with 4 escorts that
shield the Boss until all of them are dead. Fully independent
class/state -- nothing here is shared with enemy3.h.
========================================================= */
class EnemyManager4
{
public:
	Enemy4 activeEnemies[MAX_ACTIVE_ENEMIES4];
	Bullet_pro4 bullets[MAX_BULLETS_PRO4];
	HealingPickup4 healingPickups[MAX_HEALING_PICKUPS4];

	Level4Wave waveState;
	int waveBlueDefeated;
	int waveSoldierDefeated;
	bool waveBossDefeated;
	int waveEscortsDefeated;
	bool waveBoss2Defeated;
	int waveEscorts2Defeated;
	int ambushIndex;
	int ambushDefeated;
	bool waitingToRespawn;
	int respawnTimer;

	EnemyManager4()
	{
		waveState = L4_WAVE1_BLUES;
		waveBlueDefeated = 0;
		waveSoldierDefeated = 0;
		waveBossDefeated = false;
		waveEscortsDefeated = 0;
		waveBoss2Defeated = false;
		waveEscorts2Defeated = 0;
		ambushIndex = 0;
		ambushDefeated = 0;
		waitingToRespawn = false;
		respawnTimer = 0;
	}

	/* All Level 4 enemies spawn on the ground floor. */
	static float enemyFloorY(int index)
	{
		return ENEMY4_GROUND_Y;
	}

	static float getWave1BlueX(int idx)
	{
		static const float x4[WAVE1_BLUE4_COUNT] = { 700.0f, 950.0f, 1250.0f, 1550.0f };
		return x4[idx];
	}

	static float getWave2BlueX(int idx)
	{
		static const float x4[WAVE2_BLUE4_COUNT] = { 2100.0f, 2300.0f, 2500.0f, 2700.0f, 2900.0f };
		return x4[idx];
	}

	void init()
	{
		initEnemy4Textures();
		reset();
	}

	void reset()
	{
		waveState = L4_WAVE1_BLUES;
		waveBlueDefeated = 0;
		waveSoldierDefeated = 0;
		waveBossDefeated = false;
		waveEscortsDefeated = 0;
		waveBoss2Defeated = false;
		waveEscorts2Defeated = 0;
		ambushIndex = 0;
		ambushDefeated = 0;
		waitingToRespawn = false;
		respawnTimer = 0;

		for (int i = 0; i < MAX_ACTIVE_ENEMIES4; i++)
			activeEnemies[i].alive = false;

		for (int i = 0; i < MAX_BULLETS_PRO4; i++)
			bullets[i].active = false;

		for (int i = 0; i < MAX_HEALING_PICKUPS4; i++)
			healingPickups[i].active = false;

		spawnNextInWave();
	}

	void spawnBullet(float bx, float by, float dir)
	{
		for (int i = 0; i < MAX_BULLETS_PRO4; i++)
		{
			if (!bullets[i].active)
			{
				bullets[i].x = bx;
				bullets[i].y = by;
				bullets[i].dirX = dir;
				bullets[i].active = true;
				bullets[i].damage = SOLDIER4_BULLET_DAMAGE;
				bullets[i].width = 34;
				bullets[i].height = 20;
				bullets[i].frame = 0;
				bullets[i].frameTimer = 0;
				break;
			}
		}
	}

	/* Drops one Healing.png pickup at the given world position. Used
	   for the "last Soldier of the wave" drop rule (waves 1/2) and
	   for Boss1's death (wave 3) -- see updateAll()'s death-handling
	   branch below. */
	void spawnHealingPickup(float hx, float hy)
	{
		for (int i = 0; i < MAX_HEALING_PICKUPS4; i++)
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

	static int ambushBlueCount(int idx)
	{
		static const int n[LEVEL4_AMBUSH_COUNT] = { 3, 2, 3, 3 };
		return n[idx];
	}

	static int ambushSoldierCount(int idx)
	{
		static const int n[LEVEL4_AMBUSH_COUNT] = { 2, 3, 3, 3 };
		return n[idx];
	}

	static int ambushTotal(int idx)
	{
		return ambushBlueCount(idx) + ambushSoldierCount(idx);
	}

	/* Picks a spawn X around the hero for group member k. Prefers a spot
	   just off-screen (AMBUSH4_SPAWN_BASE_OFFSET + k*STEP away), alternating
	   right/left, and searches nearer/farther offsets until it finds one
	   that (a) is inside the world, (b) is clear of rocks, (c) has NO rock
	   between it and the hero (an enemy stuck behind a rock could not be
	   hit or hit back, which would soft-lock the wave), and (d) is at
	   least 90 px from the members already placed (used[0..nUsed-1]). */
	static float ambushSpawnX(int k, const float* used, int nUsed)
	{
		float pref = AMBUSH4_SPAWN_BASE_OFFSET + k * AMBUSH4_SPAWN_STEP;
		float dir = (k % 2 == 0) ? 1.0f : -1.0f;

		/* Pass 0: preferred rules. Pass 1: looser rules for cramped spots
		   (e.g. the gap between two rocks) -- closer to the hero and
		   closer to each other. */
		for (int pass = 0; pass < 2; pass++)
		{
			float minOff = (pass == 0) ? 250.0f : 150.0f;
			float minGap = (pass == 0) ? 90.0f : 45.0f;

			for (float delta = 0.0f; delta <= 950.0f; delta += 45.0f)
			{
				for (int t = 0; t < 2; t++)
				{
					if (t == 1 && delta == 0.0f) continue;
					float off = (t == 0) ? (pref - delta) : (pref + delta);
					if (off < minOff || off > 1100.0f) continue;

					for (int side = 0; side < 2; side++)
					{
						float d = (side == 0) ? dir : -dir;
						float cx = playerX + d * off;
						if (cx < AMBUSH4_MIN_X || cx > AMBUSH4_MAX_X) continue;
						float sx = level4SafeEnemySpawnX(cx, ENEMY4_GROUND_Y);
						if (sx < AMBUSH4_MIN_X || sx > AMBUSH4_MAX_X) continue;
						if (isLevel4CoverBetween(playerX, playerY, sx, ENEMY4_GROUND_Y)) continue;

						bool tooClose = false;
						for (int u = 0; u < nUsed; u++)
							if (fabsf(sx - used[u]) < minGap) { tooClose = true; break; }
						if (tooClose) continue;

						return sx;
					}
				}
			}
		}

		/* Last resort (very rare): 200 px from the hero on whichever side has
		   no rock in the way; members may overlap here. */
		for (int side = 0; side < 2; side++)
		{
			float d = (side == 0) ? dir : -dir;
			float fx = playerX + d * 200.0f;
			if (fx < AMBUSH4_MIN_X) fx = AMBUSH4_MIN_X;
			if (fx > AMBUSH4_MAX_X) fx = AMBUSH4_MAX_X;
			if (!isLevel4CoverBetween(playerX, playerY, fx, ENEMY4_GROUND_Y))
				return fx;
		}
		float fx = playerX + dir * 200.0f;
		if (fx < AMBUSH4_MIN_X) fx = AMBUSH4_MIN_X;
		if (fx > AMBUSH4_MAX_X) fx = AMBUSH4_MAX_X;
		return fx;
	}

	/* Spawns ambush group `idx` (0..3) all at once. Enemies get a bit
	   tougher (melee) and worth more score with each ambush. */
	void startAmbush(int idx)
	{
		waveState = L4_AMBUSH;
		ambushIndex = idx;
		ambushDefeated = 0;

		int tier = idx;
		int slot = 0;
		float used[MAX_ACTIVE_ENEMIES4];
		int nb = ambushBlueCount(idx);
		int ns = ambushSoldierCount(idx);

		for (int b = 0; b < nb && slot < MAX_ACTIVE_ENEMIES4; b++, slot++)
		{
			float bx = ambushSpawnX(slot, used, slot); used[slot] = bx;
			activeEnemies[slot].spawn(TYPE4_BLUE, bx, ENEMY4_GROUND_Y,
				45 + tier * 5, BLUE4_ATTACK_DAMAGE + 1 + tier * 2, BLUE4_ATTACK_RANGE, 28 + tier * 4);
		}
		for (int q = 0; q < ns && slot < MAX_ACTIVE_ENEMIES4; q++, slot++)
		{
			float sx = ambushSpawnX(slot, used, slot); used[slot] = sx;
			activeEnemies[slot].spawn(TYPE4_SOLDIER, sx, ENEMY4_GROUND_Y,
				70 + tier * 10, SOLDIER4_BULLET_DAMAGE, SOLDIER4_KEEP_DIST, 55 + tier * 5);
		}
	}

	/* Wave 2's two soldiers (spawned once Wave 2's blues AND Ambush 2 are
	   cleared). */
	void spawnWave2Soldiers()
	{
		waveState = L4_WAVE2_SOLDIERS;
		waveSoldierDefeated = 0;
		float s0x = level4SafeEnemySpawnX(3000.0f, ENEMY4_GROUND_Y);
		float s1x = level4SafeEnemySpawnX(3150.0f, ENEMY4_GROUND_Y);
		activeEnemies[0].spawn(TYPE4_SOLDIER, s0x, ENEMY4_GROUND_Y, 80, SOLDIER4_BULLET_DAMAGE + 5, SOLDIER4_KEEP_DIST, 60);
		activeEnemies[1].spawn(TYPE4_SOLDIER, s1x, ENEMY4_GROUND_Y, 80, SOLDIER4_BULLET_DAMAGE + 5, SOLDIER4_KEEP_DIST, 60);
	}

	/* Called when an ambush group is fully cleared: moves on to whatever
	   comes next in the level. */
	void advanceAfterAmbush()
	{
		switch (ambushIndex)
		{
		case 0:   /* after W1 soldiers -> Wave 2 blues */
			waveState = L4_WAVE2_BLUES;
			waveBlueDefeated = 0;
			waitingToRespawn = true;
			respawnTimer = ENEMY4_RESPAWN_DELAY;
			break;
		case 1:   /* after W2 blues -> Ambush 3 */
			startAmbush(2);
			break;
		case 2:   /* after Ambush 3 -> Wave 2 soldiers */
			spawnWave2Soldiers();
			break;
		default:  /* after Boss1 -> Boss2 wave */
			spawnBoss2Wave();
			break;
		}
	}

	void spawnNextInWave()
	{
		if (waveState == L4_WAVE1_BLUES)
		{
			if (waveBlueDefeated < WAVE1_BLUE4_COUNT)
			{
				float bx = level4SafeEnemySpawnX(getWave1BlueX(waveBlueDefeated), enemyFloorY(waveBlueDefeated));
				activeEnemies[0].spawn(TYPE4_BLUE, bx, enemyFloorY(waveBlueDefeated), 40, BLUE4_ATTACK_DAMAGE, BLUE4_ATTACK_RANGE, 25);
			}
			else
			{
				/* Last fight of wave 1: exactly WAVE1_SOLDIER4_COUNT (2)
				   Soldiers spawn together. */
				waveState = L4_WAVE1_SOLDIERS;
				waveSoldierDefeated = 0;
				float s0x = level4SafeEnemySpawnX(1750.0f, ENEMY4_GROUND_Y);
				float s1x = level4SafeEnemySpawnX(1950.0f, ENEMY4_GROUND_Y);
				activeEnemies[0].spawn(TYPE4_SOLDIER, s0x, ENEMY4_GROUND_Y, 65, SOLDIER4_BULLET_DAMAGE, SOLDIER4_KEEP_DIST, 50);
				activeEnemies[1].spawn(TYPE4_SOLDIER, s1x, ENEMY4_GROUND_Y, 65, SOLDIER4_BULLET_DAMAGE, SOLDIER4_KEEP_DIST, 50);
			}
		}
		else if (waveState == L4_WAVE2_BLUES)
		{
			if (waveBlueDefeated < WAVE2_BLUE4_COUNT)
			{
				float bx = level4SafeEnemySpawnX(getWave2BlueX(waveBlueDefeated), enemyFloorY(waveBlueDefeated));
				activeEnemies[0].spawn(TYPE4_BLUE, bx, enemyFloorY(waveBlueDefeated), 50, BLUE4_ATTACK_DAMAGE + 3, BLUE4_ATTACK_RANGE, 30);
			}
			else
			{
				/* Wave 2's blues are done: ambush group 2 comes next; the
				   two Wave 2 soldiers follow after it (see
				   advanceAfterAmbush()). */
				startAmbush(1);
			}
		}
	}

	/* Wave 3: all 5 (4 escorts + Boss) spawn together, so the Boss is
	   visible from the start -- but the Boss stays passive (won't
	   move, attack, or take damage; see Enemy4::update() and
	   playerAttack()) until all 4 escorts are dead. Same pattern as
	   Level 3's boss fight, independently implemented here. */
	void spawnBossWave()
	{
		waveState = L4_WAVE3_BOSS;
		waveBossDefeated = false;
		waveEscortsDefeated = 0;

		activeEnemies[0].spawn(TYPE4_BLUE, level4SafeEnemySpawnX(BOSS4_ESCORT_BLUE1_X, ENEMY4_GROUND_Y), ENEMY4_GROUND_Y, 55, BLUE4_ATTACK_DAMAGE + 5, BLUE4_ATTACK_RANGE, 35);
		activeEnemies[1].spawn(TYPE4_BLUE, level4SafeEnemySpawnX(BOSS4_ESCORT_BLUE2_X, ENEMY4_GROUND_Y), ENEMY4_GROUND_Y, 55, BLUE4_ATTACK_DAMAGE + 5, BLUE4_ATTACK_RANGE, 35);
		activeEnemies[2].spawn(TYPE4_SOLDIER, level4SafeEnemySpawnX(BOSS4_ESCORT_SOLDIER1_X, ENEMY4_GROUND_Y), ENEMY4_GROUND_Y, 90, SOLDIER4_BULLET_DAMAGE + 5, SOLDIER4_KEEP_DIST, 65);
		activeEnemies[3].spawn(TYPE4_SOLDIER, level4SafeEnemySpawnX(BOSS4_ESCORT_SOLDIER2_X, ENEMY4_GROUND_Y), ENEMY4_GROUND_Y, 90, SOLDIER4_BULLET_DAMAGE + 5, SOLDIER4_KEEP_DIST, 65);
		activeEnemies[4].spawn(TYPE4_BOSS, level4SafeEnemySpawnX(BOSS4_SPAWN_X, BOSS4_SPAWN_Y, BOSS4_WIDTH), BOSS4_SPAWN_Y, BOSS4_HP, BOSS4_ATTACK_DAMAGE, BOSS4_ATTACK_RANGE, BOSS4_SCORE_VALUE, BOSS4_WIDTH, BOSS4_HEIGHT);
	}

	/* Wave 4: the level's final fight. Same shielded-boss pattern as
	   Wave 3, but with 6 escorts (3 Blues + 3 Soldiers) instead of 4,
	   and Boss_2's own sprite set / stats. All 7 actors spawn together
	   so Boss_2 is visible immediately; it stays passive and
	   undamageable (see Enemy4::update() and playerAttack()) until all
	   6 escorts are dead. */
	void spawnBoss2Wave()
	{
		waveState = L4_WAVE4_BOSS2;
		waveBoss2Defeated = false;
		waveEscorts2Defeated = 0;

		activeEnemies[0].spawn(TYPE4_BLUE, level4SafeEnemySpawnX(BOSS2_ESCORT_BLUE1_X, ENEMY4_GROUND_Y), ENEMY4_GROUND_Y, BOSS2_ESCORT_BLUE_HP, BLUE4_ATTACK_DAMAGE + 8, BLUE4_ATTACK_RANGE, BOSS2_ESCORT_BLUE_SCORE);
		activeEnemies[1].spawn(TYPE4_BLUE, level4SafeEnemySpawnX(BOSS2_ESCORT_BLUE2_X, ENEMY4_GROUND_Y), ENEMY4_GROUND_Y, BOSS2_ESCORT_BLUE_HP, BLUE4_ATTACK_DAMAGE + 8, BLUE4_ATTACK_RANGE, BOSS2_ESCORT_BLUE_SCORE);
		activeEnemies[2].spawn(TYPE4_BLUE, level4SafeEnemySpawnX(BOSS2_ESCORT_BLUE3_X, ENEMY4_GROUND_Y), ENEMY4_GROUND_Y, BOSS2_ESCORT_BLUE_HP, BLUE4_ATTACK_DAMAGE + 8, BLUE4_ATTACK_RANGE, BOSS2_ESCORT_BLUE_SCORE);
		activeEnemies[3].spawn(TYPE4_SOLDIER, level4SafeEnemySpawnX(BOSS2_ESCORT_SOLDIER1_X, ENEMY4_GROUND_Y), ENEMY4_GROUND_Y, BOSS2_ESCORT_SOLDIER_HP, SOLDIER4_BULLET_DAMAGE + 8, SOLDIER4_KEEP_DIST, BOSS2_ESCORT_SOLDIER_SCORE);
		activeEnemies[4].spawn(TYPE4_SOLDIER, level4SafeEnemySpawnX(BOSS2_ESCORT_SOLDIER2_X, ENEMY4_GROUND_Y), ENEMY4_GROUND_Y, BOSS2_ESCORT_SOLDIER_HP, SOLDIER4_BULLET_DAMAGE + 8, SOLDIER4_KEEP_DIST, BOSS2_ESCORT_SOLDIER_SCORE);
		activeEnemies[5].spawn(TYPE4_SOLDIER, level4SafeEnemySpawnX(BOSS2_ESCORT_SOLDIER3_X, ENEMY4_GROUND_Y), ENEMY4_GROUND_Y, BOSS2_ESCORT_SOLDIER_HP, SOLDIER4_BULLET_DAMAGE + 8, SOLDIER4_KEEP_DIST, BOSS2_ESCORT_SOLDIER_SCORE);
		activeEnemies[6].spawn(TYPE4_BOSS2, level4SafeEnemySpawnX(BOSS2_SPAWN_X, BOSS2_SPAWN_Y, BOSS2_WIDTH), BOSS2_SPAWN_Y, BOSS2_HP, BOSS2_ATTACK_DAMAGE, BOSS2_ATTACK_RANGE, BOSS2_SCORE_VALUE, BOSS2_WIDTH, BOSS2_HEIGHT);
	}

	void updateBullets()
	{
		for (int i = 0; i < MAX_BULLETS_PRO4; i++)
		{
			if (!bullets[i].active) continue;

			bullets[i].x += bullets[i].dirX * SOLDIER4_BULLET_SPEED;

			bullets[i].frameTimer++;
			if (bullets[i].frameTimer >= BULLET4_ANIM_SPEED)
			{
				bullets[i].frameTimer = 0;
				bullets[i].frame = (bullets[i].frame + 1) % BULLET4_PRO_FRAMES;
			}

			float pxMin = playerX + 22.0f;
			float pxMax = playerX + 100.0f - 22.0f;
			float pyMin = playerY + 13.0f;
			float pyMax = playerY + 100.0f - 9.0f;

			bool hitX = (bullets[i].x + bullets[i].width >= pxMin) && (bullets[i].x <= pxMax);
			bool hitY = (bullets[i].y + bullets[i].height >= pyMin) && (bullets[i].y <= pyMax);

			if (hitX && hitY)
			{
				bool blocked = isLevel4CoverBetween(playerX, playerY, bullets[i].x, bullets[i].y);
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
	   collision4.h (handleLevel4HealingPickupCollisions), which is
	   called every frame from updateLevel4Collisions(). */
	void updateHealingPickups()
	{
		for (int i = 0; i < MAX_HEALING_PICKUPS4; i++)
		{
			if (!healingPickups[i].active) continue;
			healingPickups[i].bobPhase += HEALING4_PICKUP_BOB_SPEED;
		}
	}

	void drawBullets()
	{
		for (int i = 0; i < MAX_BULLETS_PRO4; i++)
		{
			if (!bullets[i].active) continue;

			float screenX = bullets[i].x - cameraX;
			unsigned int tex = (bullets[i].dirX < 0)
				? bulletPro4LeftTex[bullets[i].frame]
				: bulletPro4RightTex[bullets[i].frame];

			if (tex != 0)
			{
				drawEnemy4Texture((int)screenX, (int)bullets[i].y, bullets[i].width, bullets[i].height, tex);
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
		for (int i = 0; i < MAX_HEALING_PICKUPS4; i++)
		{
			if (!healingPickups[i].active) continue;

			float screenX = healingPickups[i].x - cameraX;
			float bobY = sinf(healingPickups[i].bobPhase) * HEALING4_PICKUP_BOB_RANGE;

			if (healing4PickupTex != 0)
			{
				drawEnemy4Texture((int)screenX, (int)(healingPickups[i].y + bobY),
					HEALING4_PICKUP_WIDTH, HEALING4_PICKUP_HEIGHT, healing4PickupTex);
			}
			else
			{
				iSetColor(60, 200, 90);
				iFilledRectangle((int)screenX, (int)(healingPickups[i].y + bobY),
					HEALING4_PICKUP_WIDTH, HEALING4_PICKUP_HEIGHT);
			}
		}
	}

	void updateAll()
	{
		int currentActiveCount = 0;

		for (int i = 0; i < MAX_ACTIVE_ENEMIES4; i++)
		{
			if (activeEnemies[i].alive)
			{
				activeEnemies[i].update(this);
				currentActiveCount++;
			}
			else if (activeEnemies[i].state == ENEMY4_DEAD)
			{
				activeEnemies[i].state = ENEMY4_STANDING;
				if (waveState == L4_WAVE1_BLUES || waveState == L4_WAVE2_BLUES)
				{
					waveBlueDefeated++;
					waitingToRespawn = true;
					respawnTimer = ENEMY4_RESPAWN_DELAY;
				}
				else if (waveState == L4_WAVE1_SOLDIERS || waveState == L4_WAVE2_SOLDIERS)
				{
					waveSoldierDefeated++;

					/* Healing kit rule: the LAST Soldier of the wave
					   drops one Healing.png pickup right where it
					   fell. */
					bool isLastSoldierOfWave =
						(waveState == L4_WAVE1_SOLDIERS && waveSoldierDefeated == WAVE1_SOLDIER4_COUNT) ||
						(waveState == L4_WAVE2_SOLDIERS && waveSoldierDefeated == WAVE2_SOLDIER4_COUNT);

					if (isLastSoldierOfWave)
					{
						spawnHealingPickup(activeEnemies[i].x, activeEnemies[i].y);
					}
				}
				else if (waveState == L4_AMBUSH)
				{
					ambushDefeated++;

					/* Same healing-kit rule as the soldier waves: the LAST
					   enemy of the ambush drops a pickup where it fell. */
					if (ambushDefeated == ambushTotal(ambushIndex))
						spawnHealingPickup(activeEnemies[i].x, activeEnemies[i].y);
				}
				else if (waveState == L4_WAVE3_BOSS)
				{
					if (activeEnemies[i].type == TYPE4_BOSS)
					{
						waveBossDefeated = true;

						/* Boss1 is the last enemy of wave 3 -- drop a
						   Healing.png pickup here too (same rule as
						   the last Soldier of waves 1/2), so the
						   player gets a heal before Boss_2's tougher
						   wave 4 fight starts. */
						spawnHealingPickup(activeEnemies[i].x, activeEnemies[i].y);
					}
					else
					{
						/* One of the 4 escorts just died -- once all of
						   them are gone, the Boss stops being passive
						   (see Enemy4::update() and playerAttack()). */
						waveEscortsDefeated++;
					}
				}
				else if (waveState == L4_WAVE4_BOSS2)
				{
					/* No healing kit drop for Boss_2/wave 4 -- it's the
					   level's final fight, nothing comes after it to
					   heal up for. */
					if (activeEnemies[i].type == TYPE4_BOSS2)
					{
						waveBoss2Defeated = true;
					}
					else
					{
						/* One of the 6 escorts just died -- once all of
						   them are gone, Boss_2 stops being passive
						   (see Enemy4::update() and playerAttack()). */
						waveEscorts2Defeated++;
					}
				}
			}
		}

		updateBullets();
		updateHealingPickups();

		/* Gate: the next wave's enemies only ever come into existence
		   once the current wave is fully cleared (currentActiveCount
		   == 0 and every kill in the wave accounted for), so the
		   player simply cannot progress the encounter until then. */
		if (waveState == L4_WAVE1_SOLDIERS && waveSoldierDefeated >= WAVE1_SOLDIER4_COUNT && currentActiveCount == 0)
		{
			/* Wave 1 cleared -> Ambush 1 (then Wave 2 blues). */
			startAmbush(0);
		}
		else if (waveState == L4_AMBUSH && ambushDefeated >= ambushTotal(ambushIndex) && currentActiveCount == 0)
		{
			advanceAfterAmbush();
		}
		else if (waveState == L4_WAVE2_SOLDIERS && waveSoldierDefeated >= WAVE2_SOLDIER4_COUNT && currentActiveCount == 0)
		{
			/* Wave 2 cleared -> Boss1 wave. */
			spawnBossWave();
		}
		else if (waveState == L4_WAVE3_BOSS && waveBossDefeated && currentActiveCount == 0)
		{
			/* Boss1 down -> Ambush 4, then the Boss2 wave. */
			startAmbush(3);
		}
		else if (waveState == L4_WAVE4_BOSS2 && waveBoss2Defeated && currentActiveCount == 0)
		{
			waveState = L4_COMPLETE;
			currentState = LEVEL_COMPLETE;
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
		for (int i = 0; i < MAX_ACTIVE_ENEMIES4; i++)
		{
			if (activeEnemies[i].alive)
				activeEnemies[i].draw();
		}
		drawBullets();
		drawHealingPickups();
	}

	void playerAttack()
	{
		for (int i = 0; i < MAX_ACTIVE_ENEMIES4; i++)
		{
			if (!activeEnemies[i].alive) continue;
			/* Boss is shielded by its escorts -- can't be hurt until
			   all 4 are dead (matches its passive state in
			   Enemy4::update()). */
			if (activeEnemies[i].type == TYPE4_BOSS && waveEscortsDefeated < BOSS4_ESCORT_COUNT) continue;
			/* Same shield pattern for Boss_2 and its 6 escorts. */
			if (activeEnemies[i].type == TYPE4_BOSS2 && waveEscorts2Defeated < BOSS2_ESCORT_COUNT) continue;
			if (activeEnemies[i].playerInRange(PLAYER4_ATTACK_RANGE))
			{
				activeEnemies[i].takeDamage(PLAYER4_ATTACK_DAMAGE);
			}
		}
	}
};

inline void Enemy4::update(EnemyManager4* mgr)
{
	if (!alive) return;

	frameTimer++;
	faceTowardPlayer();

	if (state == ENEMY4_HURT)
	{
		if (frameTimer >= ENEMY4_ANIM_SPEED)
		{
			frameTimer = 0;
			/* Fight/run animations are longer than the walk ones (e.g.
			   Blue: 5 fight vs 3 walk frames). Restart at frame 0 so the
			   old frame index can't run off the end of a walk array. */
			currentFrame = 0;
			state = ENEMY4_WALKING;
		}
		return;
	}

	if (type == TYPE4_BLUE)
	{
		if (attackCooldownTimer > 0) attackCooldownTimer--;

		bool inAttack = playerInRange(attackRange);

		if (state == ENEMY4_FIGHTING)
		{
			if (frameTimer >= ENEMY4_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame++;
				if (currentFrame >= BLUE4_FIGHT_FRAMES)
				{
					if (inAttack)
					{
						bool blocked = isLevel4CoverBetween(playerX, playerY, x, y);
						if (blocked) { currentFrame = 0; attackCooldownTimer = BLUE4_ATTACK_COOLDOWN; state = ENEMY4_STANDING; return; }
						health -= damage;
						if (health < 0) health = 0;
					}
					currentFrame = 0;
					attackCooldownTimer = BLUE4_ATTACK_COOLDOWN;
					state = ENEMY4_WALKING;
				}
			}
		}
		else
		{
			if (frameTimer >= ENEMY4_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame = (currentFrame + 1) % BLUE4_WALK_FRAMES;
			}

			if (inAttack && attackCooldownTimer == 0)
			{
				state = ENEMY4_FIGHTING;
				currentFrame = 0;
				frameTimer = 0;
			}
			else
			{
				state = ENEMY4_WALKING;
				float oldX = x;
				float nextX = x + ((facing == ENEMY4_FACING_RIGHT) ? BLUE4_WALK_SPEED : -BLUE4_WALK_SPEED);
				if (Level4EnemyMovementBlocked(oldX, nextX, y))
					state = ENEMY4_STANDING;
				else
					x = nextX;
			}
		}
	}
	else if (type == TYPE4_SOLDIER)
	{
		/* Soldiers never close in like Enemy4_Blue -- they hold
		   SOLDIER4_KEEP_DIST, backing off (RUNNING) if the hero gets
		   too close and closing in (WALKING) if the hero drifts too
		   far, then periodically stop to fire a Bullet_pro4 shot. */
		shootTimer++;
		float dist = (float)fabs(playerX - x);

		if (state == ENEMY4_FIGHTING)
		{
			if (frameTimer >= ENEMY4_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame++;
				if (currentFrame >= SOLDIER4_FIGHT_FRAMES)
				{
					float bulletSpawnX = (facing == ENEMY4_FACING_RIGHT) ? (x + width * 0.6f) : (x - 10.0f);
					float bulletSpawnY = y + (height * 0.55f);
					mgr->spawnBullet(bulletSpawnX, bulletSpawnY, (float)facing);

					currentFrame = 0;
					state = ENEMY4_STANDING;
				}
			}
		}
		else if (shootTimer >= SOLDIER4_FIRE_RATE)
		{
			shootTimer = 0;
			state = ENEMY4_FIGHTING;
			currentFrame = 0;
			frameTimer = 0;
		}
		else if (dist < (SOLDIER4_KEEP_DIST - SOLDIER4_DIST_TOLERANCE))
		{
			if (state != ENEMY4_RUNNING) { state = ENEMY4_RUNNING; currentFrame = 0; frameTimer = 0; }

			if (frameTimer >= ENEMY4_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame = (currentFrame + 1) % SOLDIER4_RUN_FRAMES;
			}

			float oldX = x;
			float nextX;
			if (playerX < x)
			{
				nextX = x + SOLDIER4_RUN_SPEED;
				facing = ENEMY4_FACING_LEFT;
			}
			else
			{
				nextX = x - SOLDIER4_RUN_SPEED;
				facing = ENEMY4_FACING_RIGHT;
			}
			if (!Level4EnemyMovementBlocked(oldX, nextX, y)) x = nextX;
		}
		else if (dist > (SOLDIER4_KEEP_DIST + SOLDIER4_DIST_TOLERANCE))
		{
			if (state != ENEMY4_WALKING) { state = ENEMY4_WALKING; currentFrame = 0; frameTimer = 0; }

			if (frameTimer >= ENEMY4_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame = (currentFrame + 1) % SOLDIER4_WALK_FRAMES;
			}

			float oldX = x;
			float nextX = (playerX > x) ? x + SOLDIER4_WALK_SPEED : x - SOLDIER4_WALK_SPEED;
			if (!Level4EnemyMovementBlocked(oldX, nextX, y)) x = nextX;
		}
		else
		{
			state = ENEMY4_STANDING;
			currentFrame = 0;
		}
	}
	else if (type == TYPE4_BOSS)
	{
		/* Stays passive (just stands there, won't move or attack)
		   while any of its 4 escorts are still alive -- see
		   waveEscortsDefeated in EnemyManager4. It's also shielded
		   from player damage during this time (see playerAttack()). */
		if (mgr->waveEscortsDefeated < BOSS4_ESCORT_COUNT)
		{
			state = ENEMY4_STANDING;
			return;
		}

		/* Melee fighter like TYPE4_BLUE, but bigger, tankier, hits
		   harder, and on its own frame counts/cooldown. */
		if (attackCooldownTimer > 0) attackCooldownTimer--;

		bool inAttack = playerInRange(attackRange);

		if (state == ENEMY4_FIGHTING)
		{
			if (frameTimer >= ENEMY4_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame++;
				if (currentFrame >= BOSS4_FIGHT_FRAMES)
				{
					if (inAttack)
					{
						bool blocked = isLevel4CoverBetween(playerX, playerY, x, y);
						if (blocked) { currentFrame = 0; attackCooldownTimer = BOSS4_ATTACK_COOLDOWN; state = ENEMY4_STANDING; return; }
						health -= damage;
						if (health < 0) health = 0;
					}
					currentFrame = 0;
					attackCooldownTimer = BOSS4_ATTACK_COOLDOWN;
					state = ENEMY4_WALKING;
				}
			}
		}
		else
		{
			if (frameTimer >= ENEMY4_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame = (currentFrame + 1) % BOSS4_WALK_FRAMES;
			}

			if (inAttack && attackCooldownTimer == 0)
			{
				state = ENEMY4_FIGHTING;
				currentFrame = 0;
				frameTimer = 0;
			}
			else
			{
				state = ENEMY4_WALKING;
				float oldX = x;
				float nextX = x + ((facing == ENEMY4_FACING_RIGHT) ? BOSS4_WALK_SPEED : -BOSS4_WALK_SPEED);
				if (Level4EnemyMovementBlocked(oldX, nextX, y))
					state = ENEMY4_STANDING;
				else
					x = nextX;
			}
		}
	}
	else if (type == TYPE4_BOSS2)
	{
		/* Stays passive (just stands there, won't move or attack)
		   while any of its 6 escorts are still alive -- see
		   waveEscorts2Defeated in EnemyManager4. It's also shielded
		   from player damage during this time (see playerAttack()). */
		if (mgr->waveEscorts2Defeated < BOSS2_ESCORT_COUNT)
		{
			state = ENEMY4_STANDING;
			return;
		}

		/* Melee fighter, same shape as TYPE4_BOSS but on its own
		   frame counts, speed and cooldown (Boss_2 is the tougher,
		   final-wave version). */
		if (attackCooldownTimer > 0) attackCooldownTimer--;

		bool inAttack = playerInRange(attackRange);

		if (state == ENEMY4_FIGHTING)
		{
			if (frameTimer >= ENEMY4_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame++;
				if (currentFrame >= BOSS2_FIGHT_FRAMES)
				{
					if (inAttack)
					{
						bool blocked = isLevel4CoverBetween(playerX, playerY, x, y);
						if (blocked) { currentFrame = 0; attackCooldownTimer = BOSS2_ATTACK_COOLDOWN; state = ENEMY4_STANDING; return; }
						health -= damage;
						if (health < 0) health = 0;
					}
					currentFrame = 0;
					attackCooldownTimer = BOSS2_ATTACK_COOLDOWN;
					state = ENEMY4_WALKING;
				}
			}
		}
		else
		{
			if (frameTimer >= ENEMY4_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame = (currentFrame + 1) % BOSS2_WALK_FRAMES;
			}

			if (inAttack && attackCooldownTimer == 0)
			{
				state = ENEMY4_FIGHTING;
				currentFrame = 0;
				frameTimer = 0;
			}
			else
			{
				state = ENEMY4_WALKING;
				float oldX = x;
				float nextX = x + ((facing == ENEMY4_FACING_RIGHT) ? BOSS2_WALK_SPEED : -BOSS2_WALK_SPEED);
				if (Level4EnemyMovementBlocked(oldX, nextX, y))
					state = ENEMY4_STANDING;
				else
					x = nextX;
			}
		}
	}
}

/* Picks the visual-only ground correction for a given enemy type.
   See the ENEMY4_*_VISUAL_Y_OFFSET comment block above. */
inline float enemy4VisualYOffset(Enemy4Type t)
{
	switch (t)
	{
		case TYPE4_SOLDIER: return ENEMY4_SOLDIER_VISUAL_Y_OFFSET;
		case TYPE4_BOSS:    return ENEMY4_BOSS_VISUAL_Y_OFFSET;
		case TYPE4_BOSS2:   return ENEMY4_BOSS2_VISUAL_Y_OFFSET;
		case TYPE4_BLUE:
		default:            return ENEMY4_BLUE_VISUAL_Y_OFFSET;
	}
}

/* Clamp a frame index into [0, count) so a stale index left over from a
   longer animation can never read past the end of a texture array. */
inline int enemy4SafeFrame(int f, int count)
{
	return (f < 0 || f >= count) ? 0 : f;
}

inline void Enemy4::draw()
{
	if (!alive) return;

	float screenX = x - cameraX;
	/* Drawn position only -- `y` itself (used by AI, collisions and
	   level4SafeEnemySpawnX's obstacle-distance checks) is untouched. */
	float drawY = y + enemy4VisualYOffset(type);
	unsigned int tex = 0;

	if (type == TYPE4_BLUE)
	{
		if (state == ENEMY4_FIGHTING)
			tex = (facing == ENEMY4_FACING_LEFT) ? enemy4BlueFightLeftTex[enemy4SafeFrame(currentFrame, BLUE4_FIGHT_FRAMES)] : enemy4BlueFightRightTex[enemy4SafeFrame(currentFrame, BLUE4_FIGHT_FRAMES)];
		else if (state == ENEMY4_WALKING)
			tex = (facing == ENEMY4_FACING_LEFT) ? enemy4BlueWalkLeftTex[enemy4SafeFrame(currentFrame, BLUE4_WALK_FRAMES)] : enemy4BlueWalkRightTex[enemy4SafeFrame(currentFrame, BLUE4_WALK_FRAMES)];
		else
			tex = (facing == ENEMY4_FACING_LEFT) ? enemy4BlueStandLeftTex : enemy4BlueStandRightTex;
	}
	else if (type == TYPE4_SOLDIER)
	{
		if (state == ENEMY4_FIGHTING)
			tex = (facing == ENEMY4_FACING_LEFT) ? soldier4FightLeftTex[enemy4SafeFrame(currentFrame, SOLDIER4_FIGHT_FRAMES)] : soldier4FightRightTex[enemy4SafeFrame(currentFrame, SOLDIER4_FIGHT_FRAMES)];
		else if (state == ENEMY4_RUNNING)
			tex = (facing == ENEMY4_FACING_LEFT) ? soldier4RunLeftTex[enemy4SafeFrame(currentFrame, SOLDIER4_RUN_FRAMES)] : soldier4RunRightTex[enemy4SafeFrame(currentFrame, SOLDIER4_RUN_FRAMES)];
		else if (state == ENEMY4_WALKING)
			tex = (facing == ENEMY4_FACING_LEFT) ? soldier4WalkLeftTex[enemy4SafeFrame(currentFrame, SOLDIER4_WALK_FRAMES)] : soldier4WalkRightTex[enemy4SafeFrame(currentFrame, SOLDIER4_WALK_FRAMES)];
		else
			tex = (facing == ENEMY4_FACING_LEFT) ? soldier4StandLeftTex : soldier4StandRightTex;
	}
	else if (type == TYPE4_BOSS)
	{
		if (state == ENEMY4_FIGHTING)
			tex = (facing == ENEMY4_FACING_LEFT) ? boss4FightLeftTex[enemy4SafeFrame(currentFrame, BOSS4_FIGHT_FRAMES)] : boss4FightRightTex[enemy4SafeFrame(currentFrame, BOSS4_FIGHT_FRAMES)];
		else if (state == ENEMY4_WALKING)
			tex = (facing == ENEMY4_FACING_LEFT) ? boss4WalkLeftTex[enemy4SafeFrame(currentFrame, BOSS4_WALK_FRAMES)] : boss4WalkRightTex[enemy4SafeFrame(currentFrame, BOSS4_WALK_FRAMES)];
		else
			tex = (facing == ENEMY4_FACING_LEFT) ? boss4StandLeftTex : boss4StandRightTex;
	}
	else if (type == TYPE4_BOSS2)
	{
		if (state == ENEMY4_FIGHTING)
			tex = (facing == ENEMY4_FACING_LEFT) ? boss2FightLeftTex[enemy4SafeFrame(currentFrame, BOSS2_FIGHT_FRAMES)] : boss2FightRightTex[enemy4SafeFrame(currentFrame, BOSS2_FIGHT_FRAMES)];
		else if (state == ENEMY4_WALKING)
			tex = (facing == ENEMY4_FACING_LEFT) ? boss2WalkLeftTex[enemy4SafeFrame(currentFrame, BOSS2_WALK_FRAMES)] : boss2WalkRightTex[enemy4SafeFrame(currentFrame, BOSS2_WALK_FRAMES)];
		else
			tex = (facing == ENEMY4_FACING_LEFT) ? boss2StandLeftTex : boss2StandRightTex;
	}

	drawEnemy4Texture((int)screenX, (int)drawY, width, height, tex);

	int barW = (int)(width * 0.55f);
	int barH = 6;
	int bx = (int)screenX + (width - barW) / 2;
	int by = (int)drawY + height + 4;

	iSetColor(60, 20, 20);
	iFilledRectangle(bx, by, barW, barH);
	iSetColor(180, 40, 40);
	iFilledRectangle(bx, by, barW * hp / maxHp, barH);
	iSetColor(230, 210, 170);
	iRectangle(bx, by, barW, barH);
}

/* Global Instance & External Wrapper Functions */
static EnemyManager4 enemyManager4;

inline void initEnemies4()         { enemyManager4.init(); }
inline void resetEnemies4()        { enemyManager4.reset(); }
inline void updateEnemies4()       { enemyManager4.updateAll(); }
inline void drawEnemies4()         { enemyManager4.drawAll(); }
inline void playerAttackEnemies4() { enemyManager4.playerAttack(); }
