#pragma once
/* =========================================================
   hero2.h
   Single-header Hero module for LEVEL 2 of "Silhouette of
   Bengal". Same shape and behaviour as hero.h (Level 1's
   knife-wielding Shamsher), but this one plays the Hero2
   axe art (Images\Hero2\...) instead, and is only ever
   driven while currentState == LEVEL2_PLAYING.

   Header-only, like hero.h/enemy2.h, so it drops straight
   into the existing project (no .cpp to add, no project-file
   changes) -- just #include it from iMain.cpp, after hero.h
   (order relative to hero.h doesn't actually matter, only
   that both end up in the same translation unit).

   Targets Visual Studio 2013 (no C++14/17 features used).

   WHY A SEPARATE FILE INSTEAD OF REUSING Hero FROM hero.h:
   Shamsher switches weapons for Level 2 (axe instead of
   knife), and the Level 2 art set is a different, smaller
   collection of frames (no dedicated "guard" pose, for
   instance) -- so rather than overload hero.h's Hero class
   with level-specific branches, this file is its own
   self-contained class (Hero2 / hero2 / initHero2() /
   updateHero2() / drawHero2()), named so it can never
   collide with hero.h's HERO_* macros or Hero class even
   though both headers are included in the same .cpp.

   ART ASSETS (Images\Hero2\...), same flat-white-background
   convention as Images\Hero\... -- see hero.h's long comment
   about loadHeroTexture() for why a plain iLoadImage() isn't
   used. loadHero2Texture() below does the same near-white ->
   transparent trick, kept as its own copy (not shared with
   hero.h) so this file stays fully self-contained.

   NOTE: Images\Hero2 has no dedicated "guard/block" frame
   (hero.h's Level 1 art has HeldDownWithKnife.png; Level 2's
   axe set doesn't). HERO2_GUARDING therefore just reuses the
   idle (ReadyStanceAxe) texture -- still a distinct state
   internally (in case a real guard frame is dropped in
   later), it simply looks the same as idle for now.
   ========================================================= */

#include "globals.h"
#include "StairMechanics.h"
#include "iGraphics.h"
#include <math.h>

/* playerX / playerY are DEFINED in enemy.h; cameraX is DEFINED
   in background.h. Declared extern here (rather than by
   #include-ing those headers) so this file doesn't care what
   order it's included in relative to them -- only that all
   three end up in the same iMain.cpp translation unit, which
   they already do (exactly the same approach hero.h/enemy2.h
   already use). */
extern float playerX, playerY;
extern float cameraX;

/* -------- tunables (all suffixed "2" so nothing here ever
   collides with hero.h's HERO_* macros) -------- */
#define HERO2_WALK_FRAMES      4
#define HERO2_ATTACK_FRAMES    2
#define HERO2_ANIM_SPEED       6     // frames between sprite changes (lower = faster)
#define HERO2_ATTACK_DURATION  18    // total frames the attack animation plays for
#define HERO2_HURT_DURATION    20    // total frames the hurt/backstep reaction plays for

#define HERO2_MOVE_SPEED       4.0f
#define HERO2_JUMP_STRENGTH    14.0f
#define HERO2_GRAVITY          0.8f
#define HERO2_GROUND_Y         40.0f  // same ground line enemy2.h spawns knights on
#define HERO2_KNOCKBACK        15.0f  // small push-back applied when hurt

// Matched to enemy2.h's knight sprites, which always draw at a
// fixed 100x100 box. Hero2 art is likewise roughly square, so
// using the same box keeps both characters the same height on
// screen with no stretching.
#define HERO2_DRAW_WIDTH       100
#define HERO2_DRAW_HEIGHT      100

#define HERO2_COLORKEY_TOLERANCE 24   // how close to pure white still counts as "background"

enum Hero2State
{
	HERO2_IDLE, HERO2_WALKING, HERO2_JUMPING,
	HERO2_ATTACKING, HERO2_HURT, HERO2_GUARDING
};

enum Hero2Facing { HERO2_FACING_LEFT = -1, HERO2_FACING_RIGHT = 1 };

/* -------- sprite filenames -------- */
static char hero2WalkFrameFiles[HERO2_WALK_FRAMES][100] = {
	"Images\\Hero2\\walkingFrame1Axe.jpeg",
	"Images\\Hero2\\walkingFrame2Axe.png",
	"Images\\Hero2\\walkingFrame3Axe.png",
	"Images\\Hero2\\walkingFrame4Axe.png"
};
static char hero2AttackFrameFiles[HERO2_ATTACK_FRAMES][100] = {
	"Images\\Hero2\\AttackSlashAxe.png",
	"Images\\Hero2\\StabAxe.png"
};
static char hero2IdleFrameFile[100]  = "Images\\Hero2\\ReadyStanceAxe.jpeg";
static char hero2JumpFrameFile[100]  = "Images\\Hero2\\MidAirStrikeAxe.png";
static char hero2HurtFrameFile[100]  = "Images\\Hero2\\Backstep.png";

/* -------- loaded textures (filled in by initHero2Textures()) -------- */
static unsigned int hero2WalkTex[HERO2_WALK_FRAMES];
static unsigned int hero2AttackTex[HERO2_ATTACK_FRAMES];
static unsigned int hero2IdleTex;
static unsigned int hero2JumpTex;
static unsigned int hero2HurtTex;


/* =========================================================
   loadHero2Texture -- like hero.h's loadHeroTexture(): any
   pixel close enough to pure white is made fully transparent
   first, so these flat-white-background frames draw over the
   Level 2 art with no white box around them.
   ========================================================= */
inline unsigned int loadHero2Texture(char filename[])
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
			p[0] >= 255 - HERO2_COLORKEY_TOLERANCE &&
			p[1] >= 255 - HERO2_COLORKEY_TOLERANCE &&
			p[2] >= 255 - HERO2_COLORKEY_TOLERANCE;
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

inline void initHero2Textures()
{
	for (int i = 0; i < HERO2_WALK_FRAMES; i++)
		hero2WalkTex[i] = loadHero2Texture(hero2WalkFrameFiles[i]);
	for (int i = 0; i < HERO2_ATTACK_FRAMES; i++)
		hero2AttackTex[i] = loadHero2Texture(hero2AttackFrameFiles[i]);
	hero2IdleTex = loadHero2Texture(hero2IdleFrameFile);
	hero2JumpTex = loadHero2Texture(hero2JumpFrameFile);
	hero2HurtTex = loadHero2Texture(hero2HurtFrameFile);
}

/* =========================================================
   drawHero2Texture -- same textured quad as hero.h's
   drawHeroTexture(): mirrors the sprite horizontally
   (facing == HERO2_FACING_LEFT) by swapping the U texture
   coordinates. All the axe art faces right, so this is what
   lets Shamsher actually turn around when walking left.
   ========================================================= */
inline void drawHero2Texture(int x, int y, int width, int height,
	unsigned int texture, int facing)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	float uLeft  = (facing == HERO2_FACING_RIGHT) ? 0.0f : 1.0f;
	float uRight = (facing == HERO2_FACING_RIGHT) ? 1.0f : 0.0f;

	glBegin(GL_QUADS);
		glTexCoord2f(uLeft, 0);   glVertex2f(x, y);
		glTexCoord2f(uRight, 0);  glVertex2f(x + width, y);
		glTexCoord2f(uRight, -1); glVertex2f(x + width, y + height);
		glTexCoord2f(uLeft, -1);  glVertex2f(x, y + height);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}


/* =========================================================
   Hero2 -- the player character, Level 2 (axe) version
   ========================================================= */
class Hero2
{
public:
	float x, y;
	float velY;
	int width, height;
	int facing;              // HERO2_FACING_LEFT / HERO2_FACING_RIGHT
	bool grounded;

	Hero2State state;
	int currentFrame;
	int frameTimer;
	int stateTimer;          // frames left in a timed state (attack / hurt)

	int lastHealth;          // used to detect "just got hit" without enemy2.h calling back in
	bool prevAttackKeyHeld;  // edge-detect so holding J doesn't spam the attack

	void init()
	{
		// Same spawn x enemy2.h's own comments assume for the
		// hero (its first knight posts start at x = 650, clear
		// of this spawn point) -- see EnemyManager2::configX().
		x = 500.0f;
		y = HERO2_GROUND_Y;
		velY = 0.0f;
		facing = HERO2_FACING_RIGHT;
		grounded = true;

		state = HERO2_IDLE;
		currentFrame = 0;
		frameTimer = 0;
		stateTimer = 0;

		lastHealth = health;
		prevAttackKeyHeld = false;
	}

	/* Call this if/when other code wants to force a hurt
	   reaction directly instead of relying on the health-drop
	   auto-detect below. Not required for the current game to
	   work. */
	void takeHit()
	{
		state = HERO2_HURT;
		stateTimer = HERO2_HURT_DURATION;
		currentFrame = 0;
		x -= facing * HERO2_KNOCKBACK;
		clampToLevel();
	}

	void clampToLevel()
	{
		if (x < 0)
			x = 0;
		if (x > getActiveLevelWidth() - width)
			x = getActiveLevelWidth() - width;
	}

	/* Detects "health just went down" so getting hit by a
	   knight (which edits the global `health` directly in
	   enemy2.h) still plays a reaction here, with zero changes
	   needed in enemy2.h. */
	void checkForDamage()
	{
		if (health < lastHealth && state != HERO2_HURT)
		{
			state = HERO2_HURT;
			stateTimer = HERO2_HURT_DURATION;
			currentFrame = 0;
			x -= facing * HERO2_KNOCKBACK;
			clampToLevel();
		}
		lastHealth = health;
	}

	void handleInput()
	{
		if (state == HERO2_HURT)
			return; // can't move / attack / guard while staggering

		bool left  = isKeyPressed('a') || isKeyPressed('A') || isSpecialKeyPressed(GLUT_KEY_LEFT);
		bool right = isKeyPressed('d') || isKeyPressed('D') || isSpecialKeyPressed(GLUT_KEY_RIGHT);
		bool upKey = isKeyPressed('w') || isKeyPressed('W') || isSpecialKeyPressed(GLUT_KEY_UP);
		bool downKey = isKeyPressed('s') || isKeyPressed('S') || isSpecialKeyPressed(GLUT_KEY_DOWN);
		bool jump = upKey;
		bool guard = downKey;
		bool attackKeyHeld = isKeyPressed('j') || isKeyPressed('J');

		/* Level 3/4 use player-controlled stairs instead of jumping.
		   UP/W moves only along Y; DOWN/S moves only along Y. */
		if (currentState == LEVEL3_PLAYING || currentState == LEVEL4_PLAYING)
		{
			bool verticalMoved = false;
			if (upKey && !downKey) verticalMoved = movePlayerUpOnStair(x, y);
			else if (downKey && !upKey) verticalMoved = movePlayerDownOnStair(x, y);
			if (verticalMoved) grounded = true;
		}

		/* Attack: edge-triggered, so a held J doesn't restart
		   the swing every frame. The actual damage is still
		   applied by enemyManager2.playerAttack() where 'J' is
		   handled in iMain.cpp's iKeyboard() -- this only drives
		   the matching visual (axe swing instead of knife). */
		if (attackKeyHeld && !prevAttackKeyHeld)
			attack();
		prevAttackKeyHeld = attackKeyHeld;

		if (state == HERO2_ATTACKING)
			return; // don't let movement interrupt an active swing

		bool moved = false;
		if (left && !right)
		{
			x -= HERO2_MOVE_SPEED;
			facing = HERO2_FACING_LEFT;
			moved = true;
		}
		else if (right && !left)
		{
			x += HERO2_MOVE_SPEED;
			facing = HERO2_FACING_RIGHT;
			moved = true;
		}
		clampToLevel();

		if ((currentState != LEVEL3_PLAYING && currentState != LEVEL4_PLAYING) && jump && grounded)
		{
			velY = HERO2_JUMP_STRENGTH;
			grounded = false;
		}

		if (!grounded)
			state = HERO2_JUMPING;
		else if (guard)
			state = HERO2_GUARDING;
		else if (moved)
			state = HERO2_WALKING;
		else
			state = HERO2_IDLE;
	}

	void applyPhysics()
	{
		if (currentState == LEVEL3_PLAYING || currentState == LEVEL4_PLAYING)
		{
			velY = 0.0f;
			grounded = true;
			return;
		}
		if (!grounded)
		{
			velY -= HERO2_GRAVITY;
			y += velY;
			if (y <= HERO2_GROUND_Y)
			{
				y = HERO2_GROUND_Y;
				velY = 0.0f;
				grounded = true;
			}
		}
	}

	void updateAnimation()
	{
		frameTimer++;

		if (state == HERO2_WALKING)
		{
			if (frameTimer >= HERO2_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame = (currentFrame + 1) % HERO2_WALK_FRAMES;
			}
		}
		else if (state == HERO2_ATTACKING)
		{
			if (frameTimer >= HERO2_ANIM_SPEED)
			{
				frameTimer = 0;
				if (currentFrame < HERO2_ATTACK_FRAMES - 1)
					currentFrame++;
			}
			stateTimer--;
			if (stateTimer <= 0)
			{
				state = HERO2_IDLE;
				currentFrame = 0;
			}
		}
		else if (state == HERO2_HURT)
		{
			stateTimer--;
			if (stateTimer <= 0)
				state = HERO2_IDLE;
		}
		else // HERO2_IDLE / HERO2_JUMPING / HERO2_GUARDING -- single-frame poses
		{
			currentFrame = 0;
		}
	}

	void update()
	{
		checkForDamage();
		handleInput();
		applyPhysics();
		updateAnimation();

		// Same "sync" job hero.h's Hero::update() does for
		// Level 1: enemy2.h (targeting) and background.h's
		// camera both read these every frame.
		playerX = x;
		playerY = y;
	}

	void attack()
	{
		if (state == HERO2_ATTACKING || state == HERO2_HURT)
			return;
		state = HERO2_ATTACKING;
		currentFrame = 0;
		frameTimer = 0;
		stateTimer = HERO2_ATTACK_DURATION;
	}

	void draw()
	{
		float screenX = x - cameraX;

		unsigned int tex;
		switch (state)
		{
		case HERO2_WALKING:   tex = hero2WalkTex[currentFrame];   break;
		case HERO2_ATTACKING: tex = hero2AttackTex[currentFrame]; break;
		case HERO2_JUMPING:   tex = hero2JumpTex;                 break;
		case HERO2_HURT:      tex = hero2HurtTex;                 break;
		case HERO2_GUARDING:  tex = hero2IdleTex;                 break; // no dedicated guard frame in Images\Hero2 -- see file header note
		case HERO2_IDLE:
		default:              tex = hero2IdleTex;                 break;
		}

		drawHero2Texture((int)screenX, (int)y, width, height, tex, facing);
	}
};

/* Single global instance -- this is what iMain.cpp talks to
   during LEVEL2_PLAYING. */
Hero2 hero2;

/* Thin wrapper functions so the call sites read the same as
   hero.h's initHero()/updateHero()/... */
inline void initHero2()
{
	initHero2Textures();
	hero2.width = HERO2_DRAW_WIDTH;
	hero2.height = HERO2_DRAW_HEIGHT;
	hero2.init();
}

inline void resetHero2()
{
	hero2.init();
	playerX = hero2.x;
	playerY = hero2.y;
}

inline void updateHero2() { hero2.update(); }
inline void drawHero2()   { hero2.draw(); }

/* Optional explicit hook -- not required (attack() already
   self-triggers off 'J' inside Hero2::handleInput()), but
   available if a teammate would rather call it directly from
   iMain.cpp's iKeyboard() next to playerAttackEnemies2(). Safe
   to call even if the animation is already playing. */
inline void hero2Attack() { hero2.attack(); }
