#pragma once
/* =========================================================
hero.h
Single-header Hero (player character) module for
"Silhouette of Bengal". Header-only, like enemy.h and
background.h, so it drops straight into the existing
project (no .cpp to add, no project-file changes) — just
#include it from iMain.cpp, after enemy.h and background.h.

Targets Visual Studio 2013 (no C++14/17 features used).

WHAT THIS FILE OWNS:
- Player movement (A/D or LEFT/RIGHT, held keys)
- Jumping (W or UP, held-then-released style single jump)
- Attack animation (J) — the actual damage-dealing to
enemies is still done by enemyManager.playerAttack()
in enemy.h (unchanged); this file only drives the
matching visual animation so a J press both hits and
looks right.
- A simple "hurt" reaction (Backstep.png) that triggers
automatically whenever the global `health` drops, so
getting hit by a knight actually shows on screen.
- Publishing playerX / playerY every frame — those two
globals are DEFINED in enemy.h (see the TODO note at
the top of that file); this is the file that finally
"syncs them with the real player position" as asked
for there.

ART ASSETS (Images\Hero\...), 1254x1254 (a couple are
1024x1024), all plain RGB with a flat white background —
there is no alpha channel baked in. Because of that this
file does NOT use iLoadImage()/iShowImage() directly (that
pair has no way to punch a hole in a fully-opaque texture).
Instead it loads each frame with loadHeroTexture() below,
which treats near-white pixels as transparent (alpha = 0)
before uploading the texture — the same idea as enemy.h's
IGNORE_WHITE trick for BMPs, just done for PNG/JPEG frames
loaded as GL textures. iGraphics.h already turns on
GL_ALPHA_TEST (glAlphaFunc(GL_GREATER, 0.0f)) in
iInitialize(), so an alpha of 0 is enough to make those
pixels disappear — no extra project/library changes needed.

NOTE: the two .jpeg frames (walkingframe3.jpeg,
MidAirStrike.jpeg) may show a faint white halo around the
silhouette because JPEG compression slightly blurs the
white background near the edges. If that bothers you,
swap them for a re-exported .png with a clean white bg and
nothing else needs to change here — same filename, same
loader.
========================================================= */

#include "globals.h"
#include "iGraphics.h"
#include <math.h>

/* playerX / playerY are DEFINED in enemy.h (placeholder there
said "TODO: sync with real player position" — this file is
that sync). cameraX is DEFINED in background.h. Both are
declared extern here rather than by #include-ing those
headers, so hero.h doesn't care what order it's included in
relative to them (only that all three end up in the same
iMain.cpp translation unit, which they already do). */
extern float playerX, playerY;
extern float cameraX;

/* -------- tunables -------- */
#define HERO_WALK_FRAMES      4
#define HERO_ATTACK_FRAMES    2
#define HERO_ANIM_SPEED       6     // frames between sprite changes (lower = faster)
#define HERO_ATTACK_DURATION  18    // total frames the attack animation plays for
#define HERO_HURT_DURATION    20    // total frames the hurt/backstep reaction plays for

#define HERO_MOVE_SPEED       5.0f
#define HERO_JUMP_STRENGTH    15.0f
#define HERO_GRAVITY          0.75f
#define HERO_GROUND_Y         40.0f  // same ground line enemy.h spawns knights on
#define HERO_KNOCKBACK        15.0f  // small push-back applied when hurt

// Matched to enemy.h's knight sprites, which always draw at a
// fixed 100x100 box (see Enemy::init() -> width = 100; height = 100;).
// Hero art is likewise roughly square, so using the same box here
// keeps both characters the same height on screen with no stretching.
#define HERO_DRAW_WIDTH       100
#define HERO_DRAW_HEIGHT      100

#define HERO_COLORKEY_TOLERANCE 24   // how close to pure white still counts as "background"

enum HeroState
{
	HERO_IDLE, HERO_WALKING, HERO_JUMPING,
	HERO_ATTACKING, HERO_HURT, HERO_GUARDING
};

enum HeroFacing { HERO_FACING_LEFT = -1, HERO_FACING_RIGHT = 1 };

/* -------- sprite filenames -------- */
static char heroWalkFrameFiles[HERO_WALK_FRAMES][100] = {
	"Images\\Hero\\WalkingFrame1.png",
	"Images\\Hero\\walkingframe2.png",
	"Images\\Hero\\walkingframe3.jpeg",
	"Images\\Hero\\Walkingframe4.png"
};
static char heroAttackFrameFiles[HERO_ATTACK_FRAMES][100] = {
	"Images\\Hero\\AttackSlash.png",
	"Images\\Hero\\Stab.png"
};
static char heroIdleFrameFile[100] = "Images\\Hero\\Hero_ReadyWithKnife.png";
static char heroJumpFrameFile[100] = "Images\\Hero\\MidAirStrike.jpeg";
static char heroHurtFrameFile[100] = "Images\\Hero\\Backstep.png";
/* Not wired to a key yet — reserved for whoever wants to add a
block/guard move later (see Hero::state == HERO_GUARDING). */
static char heroGuardFrameFile[100] = "Images\\Hero\\HeldDownWithKnife.png";

/* -------- loaded textures (filled in by initHeroTextures()) -------- */
static unsigned int heroWalkTex[HERO_WALK_FRAMES];
static unsigned int heroAttackTex[HERO_ATTACK_FRAMES];
static unsigned int heroIdleTex;
static unsigned int heroJumpTex;
static unsigned int heroHurtTex;
static unsigned int heroGuardTex;


/* =========================================================
loadHeroTexture — like iGraphics.h's iLoadImage(), but any
pixel close enough to pure white is made fully transparent
first. That's what lets these flat-white-background frames
draw over the level art without a white box around them.
========================================================= */
inline unsigned int loadHeroTexture(char filename[])
{
	int width, height, channels;
	unsigned char *data = stbi_load(filename, &width, &height, &channels, 4);
	if (!data)
		return 0; // missing/bad file — draws nothing rather than crashing

	int nPixels = width * height;
	for (int i = 0; i < nPixels; i++)
	{
		unsigned char *p = data + i * 4;
		bool nearWhite =
			p[0] >= 255 - HERO_COLORKEY_TOLERANCE &&
			p[1] >= 255 - HERO_COLORKEY_TOLERANCE &&
			p[2] >= 255 - HERO_COLORKEY_TOLERANCE;
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

inline void initHeroTextures()
{
	for (int i = 0; i < HERO_WALK_FRAMES; i++)
		heroWalkTex[i] = loadHeroTexture(heroWalkFrameFiles[i]);
	for (int i = 0; i < HERO_ATTACK_FRAMES; i++)
		heroAttackTex[i] = loadHeroTexture(heroAttackFrameFiles[i]);
	heroIdleTex = loadHeroTexture(heroIdleFrameFile);
	heroJumpTex = loadHeroTexture(heroJumpFrameFile);
	heroHurtTex = loadHeroTexture(heroHurtFrameFile);
	heroGuardTex = loadHeroTexture(heroGuardFrameFile);
}

/* =========================================================
drawHeroTexture — same textured quad as iGraphics.h's
iShowImage(), except it can mirror the sprite horizontally
(facing == HERO_FACING_LEFT) by swapping the U texture
coordinates. All the art faces right, so this is what lets
the hero actually turn around when walking left.
========================================================= */
inline void drawHeroTexture(int x, int y, int width, int height,
	unsigned int texture, int facing)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	float uLeft = (facing == HERO_FACING_RIGHT) ? 0.0f : 1.0f;
	float uRight = (facing == HERO_FACING_RIGHT) ? 1.0f : 0.0f;

	glBegin(GL_QUADS);
	glTexCoord2f(uLeft, 0);   glVertex2f(x, y);
	glTexCoord2f(uRight, 0);  glVertex2f(x + width, y);
	glTexCoord2f(uRight, -1); glVertex2f(x + width, y + height);
	glTexCoord2f(uLeft, -1);  glVertex2f(x, y + height);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}


/* =========================================================
Hero — the player character
========================================================= */
class Hero
{
public:
	float x, y;
	float velY;
	int width, height;
	int facing;              // HERO_FACING_LEFT / HERO_FACING_RIGHT
	bool grounded;

	HeroState state;
	int currentFrame;
	int frameTimer;
	int stateTimer;          // frames left in a timed state (attack / hurt)

	int lastHealth;          // used to detect "just got hit" without enemy.h calling back in
	bool prevAttackKeyHeld;  // edge-detect so holding J doesn't spam the attack

	void init()
	{
		x = 500.0f;
		y = HERO_GROUND_Y;
		velY = 0.0f;
		facing = HERO_FACING_RIGHT;
		grounded = true;

		state = HERO_IDLE;
		currentFrame = 0;
		frameTimer = 0;
		stateTimer = 0;

		lastHealth = health;
		prevAttackKeyHeld = false;
	}

	/* Call this if/when other code (e.g. a future enemy.h
	change) wants to force a hurt reaction directly instead
	of relying on the health-drop auto-detect below. Not
	required for the current game to work. */
	void takeHit()
	{
		state = HERO_HURT;
		stateTimer = HERO_HURT_DURATION;
		currentFrame = 0;
		x -= facing * HERO_KNOCKBACK;
		clampToLevel();
	}

	void clampToLevel()
	{
		if (x < 0)
			x = 0;
		if (x > LEVEL1_WIDTH - width)
			x = LEVEL1_WIDTH - width;
	}

	/* Detects "health just went down" so getting hit by a
	knight (which edits the global `health` directly in
	enemy.h) still plays a reaction here, with zero changes
	needed in enemy.h. */
	void checkForDamage()
	{
		if (health < lastHealth && state != HERO_HURT)
		{
			state = HERO_HURT;
			stateTimer = HERO_HURT_DURATION;
			currentFrame = 0;
			x -= facing * HERO_KNOCKBACK;
			clampToLevel();
		}
		lastHealth = health;
	}

	void handleInput()
	{
		if (state == HERO_HURT)
			return; // can't move / attack / guard while staggering

		bool left = isKeyPressed('a') || isKeyPressed('A') || isSpecialKeyPressed(GLUT_KEY_LEFT);
		bool right = isKeyPressed('d') || isKeyPressed('D') || isSpecialKeyPressed(GLUT_KEY_RIGHT);
		bool jump = isKeyPressed('w') || isKeyPressed('W') || isSpecialKeyPressed(GLUT_KEY_UP);
		bool guard = isKeyPressed('s') || isKeyPressed('S') || isSpecialKeyPressed(GLUT_KEY_DOWN);
		bool attackKeyHeld = isKeyPressed('j') || isKeyPressed('J');

		/* Attack: edge-triggered, so a held J doesn't restart
		the swing every frame. The actual damage is still
		applied by enemyManager.playerAttack() where 'J' is
		handled in iMain.cpp's iKeyboard() — this only drives
		the matching visual. */
		if (attackKeyHeld && !prevAttackKeyHeld)
			attack();
		prevAttackKeyHeld = attackKeyHeld;

		if (state == HERO_ATTACKING)
			return; // don't let movement interrupt an active swing

		bool moved = false;
		if (left && !right)
		{
			x -= HERO_MOVE_SPEED;
			facing = HERO_FACING_LEFT;
			moved = true;
		}
		else if (right && !left)
		{
			x += HERO_MOVE_SPEED;
			facing = HERO_FACING_RIGHT;
			moved = true;
		}
		clampToLevel();

		if (jump && grounded)
		{
			velY = HERO_JUMP_STRENGTH;
			grounded = false;
		}

		if (!grounded)
			state = HERO_JUMPING;
		else if (guard)
			state = HERO_GUARDING;
		else if (moved)
			state = HERO_WALKING;
		else
			state = HERO_IDLE;
	}

	void applyPhysics()
	{
		if (!grounded)
		{
			velY -= HERO_GRAVITY;
			y += velY;
			if (y <= HERO_GROUND_Y)
			{
				y = HERO_GROUND_Y;
				velY = 0.0f;
				grounded = true;
			}
		}
	}

	void updateAnimation()
	{
		frameTimer++;

		if (state == HERO_WALKING)
		{
			if (frameTimer >= HERO_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame = (currentFrame + 1) % HERO_WALK_FRAMES;
			}
		}
		else if (state == HERO_ATTACKING)
		{
			if (frameTimer >= HERO_ANIM_SPEED)
			{
				frameTimer = 0;
				if (currentFrame < HERO_ATTACK_FRAMES - 1)
					currentFrame++;
			}
			stateTimer--;
			if (stateTimer <= 0)
			{
				state = HERO_IDLE;
				currentFrame = 0;
			}
		}
		else if (state == HERO_HURT)
		{
			stateTimer--;
			if (stateTimer <= 0)
				state = HERO_IDLE;
		}
		else // HERO_IDLE / HERO_JUMPING / HERO_GUARDING — single-frame poses
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

		// This is the actual "sync" enemy.h's placeholder asked for:
		// enemies (and the camera, via background.h) read these
		// every frame to know where the player really is.
		playerX = x;
		playerY = y;
	}

	void attack()
	{
		if (state == HERO_ATTACKING || state == HERO_HURT)
			return;
		state = HERO_ATTACKING;
		currentFrame = 0;
		frameTimer = 0;
		stateTimer = HERO_ATTACK_DURATION;
	}

	void draw()
	{
		float screenX = x - cameraX;

		unsigned int tex;
		switch (state)
		{
		case HERO_WALKING:   tex = heroWalkTex[currentFrame];   break;
		case HERO_ATTACKING: tex = heroAttackTex[currentFrame]; break;
		case HERO_JUMPING:   tex = heroJumpTex;                 break;
		case HERO_HURT:      tex = heroHurtTex;                 break;
		case HERO_GUARDING:  tex = heroGuardTex;                break;
		case HERO_IDLE:
		default:             tex = heroIdleTex;                 break;
		}

		drawHeroTexture((int)screenX, (int)y, width, height, tex, facing);
	}
};

/* Single global instance — this is what iMain.cpp talks to */
Hero hero;

/* Thin wrapper functions so the call sites read the same as
enemy.h's initEnemies()/updateEnemies()/... */
inline void initHero()
{
	initHeroTextures();
	hero.width = HERO_DRAW_WIDTH;
	hero.height = HERO_DRAW_HEIGHT;
	hero.init();
	playerX = hero.x;
	playerY = hero.y;
}

inline void resetHero()
{
	hero.init();
	playerX = hero.x;
	playerY = hero.y;
}

inline void updateHero() { hero.update(); }
inline void drawHero()   { hero.draw(); }

/* Optional explicit hook — not required (attack() already
self-triggers off 'J' inside Hero::handleInput()), but
available if a teammate would rather call it directly from
iMain.cpp's iKeyboard() next to playerAttackEnemies(). Safe
to call even if the animation is already playing. */
inline void heroAttack() { hero.attack(); }