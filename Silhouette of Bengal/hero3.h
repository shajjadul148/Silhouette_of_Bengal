#pragma once
/* =========================================================
   hero3.h
   Single-header Hero module for LEVEL 3 of "Silhouette of
   Bengal". Same shape and behaviour as hero.h / hero2.h, but
   this one plays the Hero3 sword art (Images\Hero3\...)
   instead, and is only ever driven while
   currentState == LEVEL3_PLAYING.

   Header-only, like hero.h/hero2.h, so it drops straight
   into the existing project (no .cpp to add) -- just
   #include it from iMain.cpp alongside the other hero
   headers (order relative to them doesn't matter, only that
   all of them end up in the same translation unit).

   Targets Visual Studio 2013 (no C++14/17 features used).

   WHY A SEPARATE FILE INSTEAD OF REUSING Hero / Hero2:
   Shamsher switches weapons again for Level 3 (sword instead
   of axe), and the Level 3 art set has a different frame
   layout -- notably two mid-air frames instead of one, and
   only a single attack frame -- so this stays its own
   self-contained class (Hero3 / hero3 / initHero3() /
   updateHero3() / drawHero3()), named so it can never
   collide with hero.h's HERO_* / hero2.h's HERO2_* macros or
   classes even though all three headers are included in the
   same .cpp.

   ART ASSETS (Images\Hero3\...), same flat-white-background
   convention as Images\Hero\... / Images\Hero2\... -- see
   hero.h's long comment about loadHeroTexture() for why a
   plain iLoadImage() isn't used. loadHero3Texture() below
   does the same near-white -> transparent trick, kept as its
   own copy (not shared with hero.h/hero2.h) so this file
   stays fully self-contained.

   FRAME MAPPING NOTES:
   - Images\Hero3 has no dedicated "guard/block" frame, same
     situation as Hero2 -- HERO3_GUARDING reuses the idle
     (ReadyStanceSword) texture, kept as its own distinct
     state internally in case a real guard frame is added
     later.
   - Images\Hero3 has TWO attack frames: StabwithSword (a brief
     windup) followed by AttackSlashSword (the full swing, with
     its own motion-trail effect baked into the art), which holds
     for most of HERO3_ATTACK_DURATION -- see
     HERO3_ATTACK_STAB_HOLD and Hero3::updateAnimation() below.
   - Images\Hero3 has TWO mid-air frames (MidAirStrike1Sword /
     MidAirStrike2Sword) instead of hero.h/hero2.h's one --
     these alternate the same way the walk cycle does while
     Shamsher is airborne, giving the jump a bit of extra
     motion instead of a single static jump pose.
   ========================================================= */

#include "globals.h"
#include "iGraphics.h"
#include <math.h>

/* playerX / playerY are DEFINED in enemy.h; cameraX is DEFINED
   in background.h. Declared extern here (rather than by
   #include-ing those headers) so this file doesn't care what
   order it's included in relative to them -- only that all
   three end up in the same iMain.cpp translation unit, which
   they already do (exactly the same approach hero.h/hero2.h
   already use). getActiveLevelWidth() (background.h) is used
   directly below the same way hero2.h uses it -- again relying
   on the shared translation unit rather than an #include. */
extern float playerX, playerY;
extern float cameraX;

/* -------- tunables (all suffixed "3" so nothing here ever
   collides with hero.h's HERO_* or hero2.h's HERO2_* macros) -------- */
#define HERO3_WALK_FRAMES      4
#define HERO3_ATTACK_FRAMES    2
#define HERO3_JUMP_FRAMES      2
#define HERO3_ANIM_SPEED       6     // frames between sprite changes (lower = faster)
#define HERO3_ATTACK_DURATION  18    // total frames the attack animation plays for
#define HERO3_ATTACK_STAB_HOLD 4     // frames the initial stab pose holds before the full slash takes over
#define HERO3_HURT_DURATION    20    // total frames the hurt/backstep reaction plays for

#define HERO3_MOVE_SPEED       4.0f
#define HERO3_JUMP_STRENGTH    14.0f
#define HERO3_GRAVITY          0.8f
#define HERO3_GROUND_Y         40.0f  // same ground line hero.h/hero2.h spawn on
#define HERO3_KNOCKBACK        15.0f  // small push-back applied when hurt

// Matched to hero2.h's 100x100 draw box, so Shamsher stays the
// same on-screen size across Level 2 -> Level 3.
#define HERO3_DRAW_WIDTH       100
#define HERO3_DRAW_HEIGHT      100

#define HERO3_COLORKEY_TOLERANCE 24   // how close to pure white still counts as "background"

enum Hero3State
{
	HERO3_IDLE, HERO3_WALKING, HERO3_JUMPING,
	HERO3_ATTACKING, HERO3_HURT, HERO3_GUARDING
};

enum Hero3Facing { HERO3_FACING_LEFT = -1, HERO3_FACING_RIGHT = 1 };

/* -------- sprite filenames -------- */
static char hero3WalkFrameFiles[HERO3_WALK_FRAMES][100] = {
	"Images\\Hero3\\WalkingFrame1Sword.png",
	"Images\\Hero3\\WalkingFrame2Sword.jpeg",
	"Images\\Hero3\\WalkingFrame3Sword.jpeg",
	"Images\\Hero3\\WalkingFrame4Sword.jpeg"
};
static char hero3AttackFrameFiles[HERO3_ATTACK_FRAMES][100] = {
	"Images\\Hero3\\StabwithSword.png",
	"Images\\Hero3\\AttackSlashSword.png"
};
static char hero3JumpFrameFiles[HERO3_JUMP_FRAMES][100] = {
	"Images\\Hero3\\MidAirStrike1Sword.png",
	"Images\\Hero3\\MidAirStrike2Sword.png"
};
static char hero3IdleFrameFile[100] = "Images\\Hero3\\ReadyStanceSword.png";
static char hero3HurtFrameFile[100] = "Images\\Hero3\\Backstep.jpeg";

/* -------- loaded textures (filled in by initHero3Textures()) -------- */
static unsigned int hero3WalkTex[HERO3_WALK_FRAMES];
static unsigned int hero3AttackTex[HERO3_ATTACK_FRAMES];
static unsigned int hero3JumpTex[HERO3_JUMP_FRAMES];
static unsigned int hero3IdleTex;
static unsigned int hero3HurtTex;

/* Set to true the moment ANY Hero3 texture fails to load, so
   initHero3Textures() can print one clear summary instead of
   the caller having to notice nine separate silent failures. */
static bool hero3AnyTextureFailed = false;


/* =========================================================
   logHero3LoadFailure -- writes the exact filename that failed
   both to the Visual Studio "Output" window (OutputDebugStringA,
   visible while debugging) and to a plain text file next to the
   .exe (Hero3_TextureLoad_Errors.txt), so a missing/misnamed
   file in Images\Hero3 is easy to diagnose even outside the
   debugger. This is purely diagnostic -- it never changes what
   gets drawn.
   ========================================================= */
inline void logHero3LoadFailure(const char *filename)
{
	char msg[256];
	sprintf_s(msg, "[Hero3] FAILED TO LOAD TEXTURE: %s\r\n", filename);

	OutputDebugStringA(msg);

	FILE *logFile = NULL;
	fopen_s(&logFile, "Hero3_TextureLoad_Errors.txt", "a");
	if (logFile)
	{
		fputs(msg, logFile);
		fclose(logFile);
	}
}


/* =========================================================
   loadHero3Texture -- like hero.h/hero2.h's loadHeroTexture():
   any pixel close enough to pure white is made fully
   transparent first, so these flat-white-background frames
   draw over the Level 3 art with no white box around them.

   ROBUSTNESS: if the exact filename given doesn't load (wrong
   case, or the art was actually saved with a different
   extension than the one hard-coded above), this now retries
   the same base name against every extension in
   HERO3_FALLBACK_EXTENSIONS before giving up, so a
   ".png" vs ".jpg" vs ".jpeg" mismatch between this file and
   whatever is actually sitting in Images\Hero3 no longer
   silently fails.
   ========================================================= */
static const char *HERO3_FALLBACK_EXTENSIONS[] =
{
	".png", ".PNG",
	".jpg", ".JPG",
	".jpeg", ".JPEG",
	".bmp", ".BMP"
};
#define HERO3_FALLBACK_EXT_COUNT 8

inline unsigned char *hero3TryLoad(const char filename[],
	int *width, int *height, int *channels)
{
	return stbi_load(filename, width, height, channels, 4);
}

inline unsigned int loadHero3Texture(char filename[])
{
	int width, height, channels;
	unsigned char *data = hero3TryLoad(filename, &width, &height, &channels);

	/* Exact filename failed -- try swapping just the extension,
	   in case the actual file in Images\Hero3 uses a different
	   one than what's hard-coded above (this is the single most
	   common cause of an OpenGL "white box": the load fails,
	   loadHero3Texture returns 0, and drawing texture 0 shows a
	   solid box instead of the intended sprite). */
	if (!data)
	{
		char base[100];
		strcpy_s(base, filename);

		char *dot = strrchr(base, '.');
		if (dot) *dot = '\0'; // trim the extension, keep the path+name

		char candidate[110];
		for (int i = 0; i < HERO3_FALLBACK_EXT_COUNT && !data; i++)
		{
			sprintf_s(candidate, "%s%s", base, HERO3_FALLBACK_EXTENSIONS[i]);
			data = hero3TryLoad(candidate, &width, &height, &channels);
		}
	}

	if (!data)
	{
		// missing/bad file (under every extension we tried) --
		// log it and draw nothing rather than a white box.
		logHero3LoadFailure(filename);
		hero3AnyTextureFailed = true;
		return 0;
	}

	int nPixels = width * height;
	for (int i = 0; i < nPixels; i++)
	{
		unsigned char *p = data + i * 4;
		bool nearWhite =
			p[0] >= 255 - HERO3_COLORKEY_TOLERANCE &&
			p[1] >= 255 - HERO3_COLORKEY_TOLERANCE &&
			p[2] >= 255 - HERO3_COLORKEY_TOLERANCE;
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

inline void initHero3Textures()
{
	// Fresh log each run, so it only ever reflects the current state.
	remove("Hero3_TextureLoad_Errors.txt");
	hero3AnyTextureFailed = false;

	for (int i = 0; i < HERO3_WALK_FRAMES; i++)
		hero3WalkTex[i] = loadHero3Texture(hero3WalkFrameFiles[i]);
	for (int i = 0; i < HERO3_ATTACK_FRAMES; i++)
		hero3AttackTex[i] = loadHero3Texture(hero3AttackFrameFiles[i]);
	for (int i = 0; i < HERO3_JUMP_FRAMES; i++)
		hero3JumpTex[i] = loadHero3Texture(hero3JumpFrameFiles[i]);
	hero3IdleTex = loadHero3Texture(hero3IdleFrameFile);
	hero3HurtTex = loadHero3Texture(hero3HurtFrameFile);

	if (hero3AnyTextureFailed)
	{
		OutputDebugStringA(
			"[Hero3] One or more Level 3 hero sprites failed to load -- "
			"see Hero3_TextureLoad_Errors.txt next to the .exe for exactly "
			"which files. Shamsher will be invisible in Level 3 (nothing is "
			"drawn instead of a white box) until Images\\Hero3\\ contains "
			"those exact files.\r\n"
			);
	}
}

/* =========================================================
   drawHero3Texture -- same textured quad as hero.h/hero2.h's
   draw*Texture(): mirrors the sprite horizontally
   (facing == HERO3_FACING_LEFT) by swapping the U texture
   coordinates. All the sword art faces right, so this is what
   lets Shamsher actually turn around when walking left.

   texture == 0 means loadHero3Texture() couldn't find/read the
   file for this frame -- binding texture 0 and drawing it is
   exactly what was producing the solid white box, so this now
   bails out before touching GL at all and draws nothing for
   that frame instead.
   ========================================================= */
inline void drawHero3Texture(int x, int y, int width, int height,
	unsigned int texture, int facing)
{
	if (texture == 0)
		return; // missing texture -- draw nothing, not a white box

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	float uLeft  = (facing == HERO3_FACING_RIGHT) ? 0.0f : 1.0f;
	float uRight = (facing == HERO3_FACING_RIGHT) ? 1.0f : 0.0f;

	glBegin(GL_QUADS);
		glTexCoord2f(uLeft, 0);   glVertex2f(x, y);
		glTexCoord2f(uRight, 0);  glVertex2f(x + width, y);
		glTexCoord2f(uRight, -1); glVertex2f(x + width, y + height);
		glTexCoord2f(uLeft, -1);  glVertex2f(x, y + height);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}


/* =========================================================
   Hero3 -- the player character, Level 3 (sword) version
   ========================================================= */
class Hero3
{
public:
	float x, y;
	float velY;
	int width, height;
	int facing;              // HERO3_FACING_LEFT / HERO3_FACING_RIGHT
	bool grounded;

	Hero3State state;
	int currentFrame;
	int frameTimer;
	int stateTimer;          // frames left in a timed state (attack / hurt)

	int lastHealth;          // used to detect "just got hit" without an enemy module calling back in
	bool prevAttackKeyHeld;  // edge-detect so holding J doesn't spam the attack

	void init()
	{
		// Same spawn-x convention hero2.h uses for Level 2:
		// clear of wherever this level's enemy module starts
		// posting its own spawns.
		x = 500.0f;
		y = HERO3_GROUND_Y;
		velY = 0.0f;
		facing = HERO3_FACING_RIGHT;
		grounded = true;

		state = HERO3_IDLE;
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
		state = HERO3_HURT;
		stateTimer = HERO3_HURT_DURATION;
		currentFrame = 0;
		x -= facing * HERO3_KNOCKBACK;
		clampToLevel();
	}

	void clampToLevel()
	{
		if (x < 0)
			x = 0;
		if (x > getActiveLevelWidth() - width)
			x = getActiveLevelWidth() - width;
	}

	/* Detects "health just went down" so getting hit still
	   plays a reaction here, with zero changes needed in
	   whatever Level 3 enemy module edits `health` directly
	   (same pattern as hero2.h/enemy2.h). */
	void checkForDamage()
	{
		if (health < lastHealth && state != HERO3_HURT)
		{
			state = HERO3_HURT;
			stateTimer = HERO3_HURT_DURATION;
			currentFrame = 0;
			x -= facing * HERO3_KNOCKBACK;
			clampToLevel();
		}
		lastHealth = health;
	}

	void handleInput()
	{
		if (state == HERO3_HURT)
			return; // can't move / attack / guard while staggering

		bool left  = isKeyPressed('a') || isKeyPressed('A') || isSpecialKeyPressed(GLUT_KEY_LEFT);
		bool right = isKeyPressed('d') || isKeyPressed('D') || isSpecialKeyPressed(GLUT_KEY_RIGHT);
		bool jump  = isKeyPressed('w') || isKeyPressed('W') || isSpecialKeyPressed(GLUT_KEY_UP);
		bool guard = isKeyPressed('s') || isKeyPressed('S') || isSpecialKeyPressed(GLUT_KEY_DOWN);
		bool attackKeyHeld = isKeyPressed('j') || isKeyPressed('J');

		/* Attack: edge-triggered, so a held J doesn't restart
		   the swing every frame. The actual damage is still
		   applied wherever Level 3's enemy manager handles 'J'
		   in iMain.cpp's iKeyboard() -- this only drives the
		   matching visual (sword stab). */
		if (attackKeyHeld && !prevAttackKeyHeld)
			attack();
		prevAttackKeyHeld = attackKeyHeld;

		if (state == HERO3_ATTACKING)
			return; // don't let movement interrupt an active swing

		bool moved = false;
		if (left && !right)
		{
			x -= HERO3_MOVE_SPEED;
			facing = HERO3_FACING_LEFT;
			moved = true;
		}
		else if (right && !left)
		{
			x += HERO3_MOVE_SPEED;
			facing = HERO3_FACING_RIGHT;
			moved = true;
		}
		clampToLevel();

		if (jump && grounded)
		{
			velY = HERO3_JUMP_STRENGTH;
			grounded = false;
		}

		if (!grounded)
			state = HERO3_JUMPING;
		else if (guard)
			state = HERO3_GUARDING;
		else if (moved)
			state = HERO3_WALKING;
		else
			state = HERO3_IDLE;
	}

	void applyPhysics()
	{
		if (!grounded)
		{
			velY -= HERO3_GRAVITY;
			y += velY;
			if (y <= HERO3_GROUND_Y)
			{
				y = HERO3_GROUND_Y;
				velY = 0.0f;
				grounded = true;
			}
		}
	}

	void updateAnimation()
	{
		frameTimer++;

		if (state == HERO3_WALKING)
		{
			if (frameTimer >= HERO3_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame = (currentFrame + 1) % HERO3_WALK_FRAMES;
			}
		}
		else if (state == HERO3_JUMPING)
		{
			// Alternates the two mid-air frames the same way the
			// walk cycle alternates its four, instead of holding
			// one static jump pose like hero.h/hero2.h do.
			if (frameTimer >= HERO3_ANIM_SPEED)
			{
				frameTimer = 0;
				currentFrame = (currentFrame + 1) % HERO3_JUMP_FRAMES;
			}
		}
		else if (state == HERO3_ATTACKING)
		{
			// StabwithSword plays only as a brief windup; the full
			// AttackSlashSword swing art (with its own motion-trail
			// effect already baked into the art) then takes over and
			// holds for the remainder of the attack -- that's the
			// pose the player actually sees for most of the swing.
			if (currentFrame == 0 && frameTimer >= HERO3_ATTACK_STAB_HOLD)
			{
				currentFrame = 1;
			}
			stateTimer--;
			if (stateTimer <= 0)
			{
				state = HERO3_IDLE;
				currentFrame = 0;
			}
		}
		else if (state == HERO3_HURT)
		{
			stateTimer--;
			if (stateTimer <= 0)
				state = HERO3_IDLE;
		}
		else // HERO3_IDLE / HERO3_GUARDING -- single-frame poses
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

		// Same "sync" job hero.h/hero2.h do: whatever Level 3's
		// enemy module (targeting) and background.h's camera
		// both read these every frame.
		playerX = x;
		playerY = y;
	}

	void attack()
	{
		if (state == HERO3_ATTACKING || state == HERO3_HURT)
			return;
		state = HERO3_ATTACKING;
		currentFrame = 0;
		frameTimer = 0;
		stateTimer = HERO3_ATTACK_DURATION;
	}

	void draw()
	{
		float screenX = x - cameraX;

		unsigned int tex;
		switch (state)
		{
		case HERO3_WALKING:   tex = hero3WalkTex[currentFrame];   break;
		case HERO3_JUMPING:   tex = hero3JumpTex[currentFrame];   break;
		case HERO3_ATTACKING: tex = hero3AttackTex[currentFrame]; break;
		case HERO3_HURT:      tex = hero3HurtTex;                 break;
		case HERO3_GUARDING:  tex = hero3IdleTex;                 break; // no dedicated guard frame in Images\Hero3 -- see file header note
		case HERO3_IDLE:
		default:              tex = hero3IdleTex;                 break;
		}

		drawHero3Texture((int)screenX, (int)y, width, height, tex, facing);
	}
};

/* Single global instance -- this is what iMain.cpp talks to
   during LEVEL3_PLAYING. */
Hero3 hero3;

/* Thin wrapper functions so the call sites read the same as
   hero.h's initHero()/updateHero()/... and hero2.h's
   initHero2()/updateHero2()/... */
inline void initHero3()
{
	initHero3Textures();
	hero3.width = HERO3_DRAW_WIDTH;
	hero3.height = HERO3_DRAW_HEIGHT;
	hero3.init();
}

inline void resetHero3()
{
	hero3.init();
	playerX = hero3.x;
	playerY = hero3.y;
}

inline void updateHero3() { hero3.update(); }
inline void drawHero3()   { hero3.draw(); }

/* Optional explicit hook -- not required (attack() already
   self-triggers off 'J' inside Hero3::handleInput()), but
   available if a teammate would rather call it directly from
   iMain.cpp's iKeyboard() next to a Level 3 enemy-attack call. Safe
   to call even if the animation is already playing. */
inline void hero3Attack() { hero3.attack(); }
