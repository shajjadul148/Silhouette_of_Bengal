#pragma once
/* =========================================================
   hero4.h
   LEVEL 4 Hero module for "Silhouette of Bengal".

   Level 4 uses the gun-based Hero4 artwork in:
       Images\Hero4\

   This header is completely separate from hero.h, hero2.h and
   hero3.h. The player uses:
       A / D or LEFT / RIGHT  -> move
       W / UP                 -> jump
       S / DOWN               -> guard pose
       J                     -> fire the gun

   The gun attack creates a real world-space projectile. The
   projectile is checked by collision4.h against Enemy4 objects.
   A successful projectile hit kills the enemy immediately.
   Enemy bullets are handled by collision4.h and reduce the
   global health value.
   ========================================================= */

#include "globals.h"
#include "iGraphics.h"
#include <math.h>
#include <string.h>

/* playerX / playerY are shared with enemy4.h/background.h. */
extern float playerX, playerY;
extern float cameraX;

/* ---------------------------------------------------------
   LEVEL 4 HERO TUNABLES
   --------------------------------------------------------- */
#define HERO4_WALK_FRAMES          4
#define HERO4_ATTACK_FRAMES        1
#define HERO4_JUMP_FRAMES          2
#define HERO4_ANIM_SPEED           6
#define HERO4_ATTACK_DURATION      18
/* Minimum frames between two shots (counted from the moment a shot is
   fired). The firing pose lasts HERO4_ATTACK_DURATION frames, so the
   extra wait is (HERO4_SHOT_COOLDOWN - HERO4_ATTACK_DURATION) frames. */
#define HERO4_SHOT_COOLDOWN        36
#define HERO4_HURT_DURATION        20

#define HERO4_MOVE_SPEED            4.0f
#define HERO4_JUMP_STRENGTH        14.0f
#define HERO4_GRAVITY               0.8f
#define HERO4_GROUND_Y             80.0f
#define HERO4_KNOCKBACK             15.0f

/* The artwork has a few pixels of white/transparent margin below the
   feet. playerY is the real floor coordinate (60), while the sprite
   is drawn slightly lower so the visible feet sit exactly on that line. */
#define HERO4_SPRITE_ANCHOR_GROUND_Y 80.0f

/* Small visual correction: move the rendered sprite a few pixels downward
   while keeping playerY aligned with the Level 4 collision/ground coordinate (60). */
#define HERO4_GROUND_VISUAL_ADJUST 4.0f

#define HERO4_DRAW_WIDTH           100
#define HERO4_DRAW_HEIGHT          100
#define HERO4_COLORKEY_TOLERANCE   24

/* Gun projectile.  It is intentionally small on screen even
   though the supplied Hero4 firing image is a full 1254x1254
   character frame. */
#define HERO4_MAX_BULLETS           40
#define HERO4_BULLET_SPEED          13.0f
#define HERO4_BULLET_DAMAGE         9999
#define HERO4_BULLET_WIDTH          24
#define HERO4_BULLET_HEIGHT          8
#define HERO4_BULLET_DRAW_SIZE       48
#define HERO4_BULLET_ANIM_SPEED      3

enum Hero4State
{
    HERO4_IDLE,
    HERO4_WALKING,
    HERO4_JUMPING,
    HERO4_ATTACKING,
    HERO4_HURT,
    HERO4_GUARDING
};

enum Hero4Facing
{
    HERO4_FACING_LEFT  = -1,
    HERO4_FACING_RIGHT = 1
};

struct Hero4Bullet
{
    float x;
    float y;
    float dirX;
    bool active;
    int width;
    int height;
    int frame;
    int frameTimer;
};

/* ---------------------------------------------------------
   SPRITE FILES
   --------------------------------------------------------- */
static char hero4WalkFrameFiles[HERO4_WALK_FRAMES][120] =
{
    "Images\\Hero4\\WalkingFrame1Gun.png",
    "Images\\Hero4\\WalkingFrame2Gun.png",
    "Images\\Hero4\\WalkingFrame3Gun.jpeg",
    "Images\\Hero4\\WalkingFrame4Gun.jpeg"
};

static char hero4AttackFrameFile[120] =
    "Images\\Hero4\\BulletComingFromGun.png";

static char hero4JumpFrameFiles[HERO4_JUMP_FRAMES][120] =
{
    "Images\\Hero4\\MidAirGunStrike1.png",
    "Images\\Hero4\\MidAirGunStrike2.png"
};

static char hero4IdleFrameFile[120] =
    "Images\\Hero4\\ReadyStanceGun.png";

static char hero4HurtFrameFile[120] =
    "Images\\Hero4\\BackstepGun.png";

/* Standalone projectile art supplied in the new Hero4 pack.
   Frame 1 has the stronger muzzle/smoke trail; frame 2 is the
   cleaner travelling projectile. */
static char hero4BulletRightFiles[2][120] =
{
    "Images\\Hero4\\Bullet\\Bullet_pro_right_1.png",
    "Images\\Hero4\\Bullet\\Bullet_pro_right_2.png"
};

static char hero4BulletLeftFiles[2][120] =
{
    "Images\\Hero4\\Bullet\\Bullet_pro_left_1.png",
    "Images\\Hero4\\Bullet\\Bullet_pro_left_2.png"
};

/* Loaded OpenGL textures. */
static unsigned int hero4WalkTex[HERO4_WALK_FRAMES];
static unsigned int hero4AttackTex;
static unsigned int hero4JumpTex[HERO4_JUMP_FRAMES];
static unsigned int hero4IdleTex;
static unsigned int hero4HurtTex;
static unsigned int hero4BulletRightTex[2];
static unsigned int hero4BulletLeftTex[2];

/* Level 4 hero bullets. */
static Hero4Bullet hero4Bullets[HERO4_MAX_BULLETS];

/* ---------------------------------------------------------
   IMAGE LOADING
   --------------------------------------------------------- */
inline unsigned char* hero4TryLoad(
    const char filename[],
    int* width,
    int* height,
    int* channels)
{
    return stbi_load(
        filename,
        width,
        height,
        channels,
        4
        );
}

static const char* HERO4_FALLBACK_EXTENSIONS[] =
{
    ".png", ".PNG",
    ".jpg", ".JPG",
    ".jpeg", ".JPEG",
    ".bmp", ".BMP"
};

#define HERO4_FALLBACK_EXT_COUNT 8

inline unsigned int loadHero4Texture(char filename[])
{
    int width, height, channels;

    unsigned char* data =
        hero4TryLoad(
            filename,
            &width,
            &height,
            &channels
            );

    /* If the exact extension is wrong, try the same filename
       with the common image extensions. */
    if (!data)
    {
        char base[140];
        strcpy_s(
            base,
            filename
            );

        char* dot =
            strrchr(
                base,
                '.'
                );

        if (dot)
            *dot = '\0';

        char candidate[160];

        for (
            int i = 0;
            i < HERO4_FALLBACK_EXT_COUNT && !data;
            i++
            )
        {
            sprintf_s(
                candidate,
                "%s%s",
                base,
                HERO4_FALLBACK_EXTENSIONS[i]
                );

            data =
                hero4TryLoad(
                    candidate,
                    &width,
                    &height,
                    &channels
                    );
        }
    }

    if (!data)
        return 0;

    /* All supplied Hero4 art has a white background. Make
       near-white pixels transparent. */
    int nPixels =
        width * height;

    for (
        int i = 0;
        i < nPixels;
        i++
        )
    {
        unsigned char* p =
            data + i * 4;

        bool nearWhite =
            p[0] >= 255 - HERO4_COLORKEY_TOLERANCE &&
            p[1] >= 255 - HERO4_COLORKEY_TOLERANCE &&
            p[2] >= 255 - HERO4_COLORKEY_TOLERANCE;

        p[3] =
            nearWhite ? 0 : 255;
    }

    unsigned int texture;

    glGenTextures(
        1,
        &texture
        );

    glBindTexture(
        GL_TEXTURE_2D,
        texture
        );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR
        );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
        );

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data
        );

    stbi_image_free(data);

    return texture;
}

inline void initHero4Textures()
{
    for (
        int i = 0;
        i < HERO4_WALK_FRAMES;
        i++
        )
    {
        hero4WalkTex[i] =
            loadHero4Texture(
                hero4WalkFrameFiles[i]
                );
    }

    hero4AttackTex =
        loadHero4Texture(
            hero4AttackFrameFile
            );

    for (
        int i = 0;
        i < HERO4_JUMP_FRAMES;
        i++
        )
    {
        hero4JumpTex[i] =
            loadHero4Texture(
                hero4JumpFrameFiles[i]
                );
    }

    hero4IdleTex =
        loadHero4Texture(
            hero4IdleFrameFile
            );

    hero4HurtTex =
        loadHero4Texture(
            hero4HurtFrameFile
            );

    for (int i = 0; i < 2; i++)
    {
        hero4BulletRightTex[i] =
            loadHero4Texture(hero4BulletRightFiles[i]);
        hero4BulletLeftTex[i] =
            loadHero4Texture(hero4BulletLeftFiles[i]);
    }

    for (
        int i = 0;
        i < HERO4_MAX_BULLETS;
        i++
        )
    {
        hero4Bullets[i].active = false;
        hero4Bullets[i].width = HERO4_BULLET_WIDTH;
        hero4Bullets[i].height = HERO4_BULLET_HEIGHT;
    }
}

/* ---------------------------------------------------------
   TEXTURED QUAD
   --------------------------------------------------------- */
inline void drawHero4Texture(
    int x,
    int y,
    int width,
    int height,
    unsigned int texture,
    int facing)
{
    if (texture == 0)
        return;

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBindTexture(
        GL_TEXTURE_2D,
        texture
        );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR
        );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
        );

    float uLeft =
        (facing == HERO4_FACING_RIGHT)
        ? 0.0f
        : 1.0f;

    float uRight =
        (facing == HERO4_FACING_RIGHT)
        ? 1.0f
        : 0.0f;

    glBegin(GL_QUADS);

        glTexCoord2f(uLeft, 0);
        glVertex2f(
            x,
            y
            );

        glTexCoord2f(uRight, 0);
        glVertex2f(
            x + width,
            y
            );

        glTexCoord2f(uRight, -1);
        glVertex2f(
            x + width,
            y + height
            );

        glTexCoord2f(uLeft, -1);
        glVertex2f(
            x,
            y + height
            );

    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

/* ---------------------------------------------------------
   HERO4 CLASS
   --------------------------------------------------------- */
class Hero4
{
public:

    float x;
    float y;
    float velY;

    int width;
    int height;

    int facing;
    bool grounded;

    Hero4State state;

    int currentFrame;
    int frameTimer;
    int stateTimer;

    int lastHealth;

    bool prevAttackKeyHeld;
    int shotCooldown;

    void init()
    {
        x = 500.0f;
        y = HERO4_GROUND_Y;

        velY = 0.0f;

        facing =
            HERO4_FACING_RIGHT;

        grounded = true;

        state =
            HERO4_IDLE;

        currentFrame = 0;
        frameTimer = 0;
        stateTimer = 0;

        lastHealth =
            health;

        prevAttackKeyHeld = false;
        shotCooldown = 0;

        resetBullets();
    }

    void resetBullets()
    {
        for (
            int i = 0;
            i < HERO4_MAX_BULLETS;
            i++
            )
        {
            hero4Bullets[i].active = false;
            hero4Bullets[i].width =
                HERO4_BULLET_WIDTH;
            hero4Bullets[i].height =
                HERO4_BULLET_HEIGHT;
            hero4Bullets[i].frame = 0;
            hero4Bullets[i].frameTimer = 0;
        }
    }

    void clampToLevel()
    {
        if (x < 0.0f)
            x = 0.0f;

        float maxX =
            (float)getActiveLevelWidth()
            - width;

        if (x > maxX)
            x = maxX;
    }

    void takeHit()
    {
        state =
            HERO4_HURT;

        stateTimer =
            HERO4_HURT_DURATION;

        currentFrame = 0;

        x -=
            facing *
            HERO4_KNOCKBACK;

        clampToLevel();
    }

    void checkForDamage()
    {
        if (
            health < lastHealth &&
            state != HERO4_HURT
            )
        {
            takeHit();
        }

        lastHealth =
            health;
    }

    void spawnBullet()
    {
        for (
            int i = 0;
            i < HERO4_MAX_BULLETS;
            i++
            )
        {
            if (!hero4Bullets[i].active)
            {
                Hero4Bullet& b =
                    hero4Bullets[i];

                b.active = true;

                b.dirX =
                    (float)facing;

                /* Start just outside the gun barrel. */
                if (facing == HERO4_FACING_RIGHT)
                {
                    b.x =
                        x + width - 2.0f;
                }
                else
                {
                    b.x =
                        x - HERO4_BULLET_WIDTH + 2.0f;
                }

                /* The gun muzzle is about 65 px above the physical
                   ground anchor in the 100x100 firing frame. */
                b.y =
                    y + 61.0f;

                b.width =
                    HERO4_BULLET_WIDTH;

                b.height =
                    HERO4_BULLET_HEIGHT;
                b.frame = 0;
                b.frameTimer = 0;

                return;
            }
        }
    }

    void updateBullets()
    {
        for (int i = 0; i < HERO4_MAX_BULLETS; i++)
        {
            if (!hero4Bullets[i].active)
                continue;

            Hero4Bullet &b = hero4Bullets[i];

            /* Real world-space projectile movement. Previously the
               projectile was only drawn at the muzzle, so the yellow
               debug rectangle appeared to stay attached to the gun. */
            b.x += b.dirX * HERO4_BULLET_SPEED;

            b.frameTimer++;
            if (b.frameTimer >= HERO4_BULLET_ANIM_SPEED)
            {
                b.frameTimer = 0;
                b.frame = (b.frame + 1) % 2;
            }

            if (b.x < -100.0f ||
                b.x > (float)getActiveLevelWidth() + 100.0f)
            {
                b.active = false;
            }
        }
    }

    void handleInput()
    {
        if (state == HERO4_HURT)
            return;

        bool left =
            isKeyPressed('a') ||
            isKeyPressed('A') ||
            isSpecialKeyPressed(GLUT_KEY_LEFT);

        bool right =
            isKeyPressed('d') ||
            isKeyPressed('D') ||
            isSpecialKeyPressed(GLUT_KEY_RIGHT);

        bool jump =
            isKeyPressed('w') ||
            isKeyPressed('W') ||
            isSpecialKeyPressed(GLUT_KEY_UP);

        bool guard =
            isKeyPressed('s') ||
            isKeyPressed('S') ||
            isSpecialKeyPressed(GLUT_KEY_DOWN);

        bool attackKeyHeld =
            isKeyPressed('j') ||
            isKeyPressed('J');

        if (
            attackKeyHeld &&
            !prevAttackKeyHeld &&
            grounded
            )
        {
            attack();
        }

        prevAttackKeyHeld =
            attackKeyHeld;

        if (state == HERO4_ATTACKING)
            return;

        bool moved = false;

        if (left && !right)
        {
            x -= HERO4_MOVE_SPEED;
            facing =
                HERO4_FACING_LEFT;
            moved = true;
        }
        else if (right && !left)
        {
            x += HERO4_MOVE_SPEED;
            facing =
                HERO4_FACING_RIGHT;
            moved = true;
        }

        clampToLevel();

        if (
            jump &&
            grounded
            )
        {
            velY =
                HERO4_JUMP_STRENGTH;

            grounded = false;
        }

        Hero4State newState;
        if (!grounded)
            newState = HERO4_JUMPING;
        else if (guard)
            newState = HERO4_GUARDING;
        else if (moved)
            newState = HERO4_WALKING;
        else
            newState = HERO4_IDLE;

        /* Each animation has a different frame count (walk = 4,
           jump = 2). Restart the frame counter on every state change,
           otherwise a walk frame index of 2 or 3 leaks into the jump
           animation and indexes past the end of hero4JumpTex[]. */
        if (newState != state)
        {
            currentFrame = 0;
            frameTimer = 0;
        }
        state = newState;
    }

    void applyPhysics()
    {
        if (!grounded)
        {
            velY -=
                HERO4_GRAVITY;

            y += velY;

            if (y <= HERO4_GROUND_Y)
            {
                y =
                    HERO4_GROUND_Y;

                velY =
                    0.0f;

                grounded =
                    true;
            }
        }
    }

    void updateAnimation()
    {
        frameTimer++;

        if (state == HERO4_WALKING)
        {
            if (
                frameTimer >=
                HERO4_ANIM_SPEED
                )
            {
                frameTimer = 0;

                currentFrame =
                    (currentFrame + 1) %
                    HERO4_WALK_FRAMES;
            }
        }
        else if (state == HERO4_JUMPING)
        {
            if (
                frameTimer >=
                HERO4_ANIM_SPEED
                )
            {
                frameTimer = 0;

                currentFrame =
                    (currentFrame + 1) %
                    HERO4_JUMP_FRAMES;
            }
        }
        else if (state == HERO4_ATTACKING)
        {
            /* The supplied firing artwork is one complete firing
               pose, so hold it for the whole attack duration. */
            currentFrame = 0;

            stateTimer--;

            if (stateTimer <= 0)
            {
                state =
                    HERO4_IDLE;

                currentFrame = 0;
            }
        }
        else if (state == HERO4_HURT)
        {
            stateTimer--;

            if (stateTimer <= 0)
                state =
                    HERO4_IDLE;
        }
        else
        {
            currentFrame = 0;
        }
    }

    void update()
    {
        if (shotCooldown > 0)
            shotCooldown--;

        checkForDamage();

        handleInput();

        applyPhysics();

        updateBullets();
        updateAnimation();

        /* Keep the common targeting/camera coordinates synchronized. */
        playerX = x;
        playerY = y;
    }

    void attack()
    {
        if (
            state == HERO4_ATTACKING ||
            state == HERO4_HURT ||
            shotCooldown > 0
            )
            return;

        shotCooldown = HERO4_SHOT_COOLDOWN;

        state =
            HERO4_ATTACKING;

        currentFrame = 0;
        frameTimer = 0;

        stateTimer =
            HERO4_ATTACK_DURATION;

        /* One J press = one projectile. */
        spawnBullet();
    }

    void drawBullets()
    {
        for (int i = 0; i < HERO4_MAX_BULLETS; i++)
        {
            if (!hero4Bullets[i].active)
                continue;

            Hero4Bullet &b = hero4Bullets[i];
            float screenX = b.x - cameraX;

            unsigned int bulletTex;
            if (b.dirX >= 0.0f)
                bulletTex = hero4BulletRightTex[b.frame];
            else
                bulletTex = hero4BulletLeftTex[b.frame];

            if (bulletTex == 0)
                continue;

            /* The source projectile images are 100x100 with the actual
               projectile centered inside them. Draw the full image at
               48x48; the white background is removed by loadHero4Texture().
               The collision box remains the small 24x8 box above. */
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBindTexture(GL_TEXTURE_2D, bulletTex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

            int drawX = (int)(screenX - 12.0f);
            int drawY = (int)(b.y - 20.0f);
            int drawSize = HERO4_BULLET_DRAW_SIZE;

            glBegin(GL_QUADS);
                glTexCoord2f(0, 0);
                glVertex2f(drawX, drawY);
                glTexCoord2f(1, 0);
                glVertex2f(drawX + drawSize, drawY);
                glTexCoord2f(1, -1);
                glVertex2f(drawX + drawSize, drawY + drawSize);
                glTexCoord2f(0, -1);
                glVertex2f(drawX, drawY + drawSize);
            glEnd();

            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
        }
    }

    float getSpriteBottomOffset() const
    {
        /* These offsets are the transparent/white bottom margins of the
           supplied 1254/1024 px source images, scaled into the 100x100
           game sprite. This keeps every visible foot/toe on the same
           physical ground line even though the source images have
           different amounts of whitespace. */
        if (state == HERO4_IDLE || state == HERO4_GUARDING)
            return 6.6f;
        if (state == HERO4_WALKING)
        {
            if (currentFrame == 0 || currentFrame == 1) return 8.5f;
            if (currentFrame == 2) return 8.8f;
            return 8.2f;
        }
        if (state == HERO4_ATTACKING)
            return 10.4f;
        if (state == HERO4_JUMPING)
            return (currentFrame == 0) ? 11.5f : 10.9f;
        if (state == HERO4_HURT)
            return 14.4f;
        return 8.0f;
    }

    void draw()
    {
        float screenX =
            x - cameraX;

        unsigned int tex = 0;

        /* Never index a texture array with a stale/out-of-range frame. */
        int walkFrame = currentFrame;
        if (walkFrame < 0 || walkFrame >= HERO4_WALK_FRAMES) walkFrame = 0;
        int jumpFrame = currentFrame;
        if (jumpFrame < 0 || jumpFrame >= HERO4_JUMP_FRAMES) jumpFrame = 0;

        if (state == HERO4_WALKING)
        {
            tex =
                hero4WalkTex[
                    walkFrame
                    ];
        }
        else if (state == HERO4_JUMPING)
        {
            tex =
                hero4JumpTex[
                    jumpFrame
                    ];
        }
        else if (state == HERO4_ATTACKING)
        {
            tex =
                hero4AttackTex;
        }
        else if (state == HERO4_HURT)
        {
            tex =
                hero4HurtTex;
        }
        else if (state == HERO4_GUARDING)
        {
            /* No separate guard frame was supplied. */
            tex =
                hero4IdleTex;
        }
        else
        {
            tex =
                hero4IdleTex;
        }

        drawHero4Texture(
            (int)screenX,
            (int)(y - getSpriteBottomOffset() - HERO4_GROUND_VISUAL_ADJUST),
            width,
            height,
            tex,
            facing
            );

        drawBullets();
    }
};

/* ---------------------------------------------------------
   GLOBAL INSTANCE + WRAPPERS
   --------------------------------------------------------- */
static Hero4 hero4;

inline void initHero4()
{
    initHero4Textures();

    hero4.width =
        HERO4_DRAW_WIDTH;

    hero4.height =
        HERO4_DRAW_HEIGHT;

    hero4.init();
}

inline void resetHero4()
{
    hero4.init();

    playerX =
        hero4.x;

    playerY =
        hero4.y;
}

inline void updateHero4()
{
    hero4.update();
}

inline void drawHero4()
{
    hero4.draw();
}

inline void hero4Attack()
{
    hero4.attack();
}
