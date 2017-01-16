/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * sdb.h - main header file
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _SDB_H_
#define _SDB_H_

#include <cstdlib>
#include <cmath>
#include <vector>
#include <fstream>
#include <iostream>

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#ifdef _WIN32
// TODO: verify validity
#ifndef snprintf
#include <cstdio>
#define snprintf _snprintf
#endif

#include <direct.h>
inline void mkdir( const char* dir, int )
{
    _mkdir( dir );
}
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#define MD2_SUFFIX ".md2"

using namespace std;

#ifndef CALLBACK
#define CALLBACK
#endif

//#define GAME_TITLE   PACKAGE_NAME
//#define GAME_VERSION PACKAGE_VERSION

#define PACKAGE_AUTHOR  "Matt Sarnoff"

#ifndef PACKAGE_NAME
#define PACKAGE_NAME "Shotgun Debugger"
#endif

#define PACKAGE_VERSION "1.1"

#define PACKAGE_RELEASE_DATE "June 15, 2005"

// shared data file location
#ifndef PKGDATADIR
#ifdef _WIN32
#define PKGDATADIR "./"
#elif __MACOSX__
#define PKGDATADIR "./Shotgun Debugger.app/Contents/Resources/"
#else
#define PKGDATADIR "/usr/local/share/shotgun-debugger"
#endif
#endif

// per-user settings location
#ifdef _WIN32
#define LOCALSTATEDIR "/.sdb/"
#elif __MACOSX__
#define LOCALSTATEDIR "/Library/Application Support/Shotgun Debugger/" // relative to home dir
#else
#define LOCALSTATEDIR "/.sdb/"
#endif

#define CONFIG_FILE "preferences.txt"
#define PLAYER_DATA_FILE "player.dat"


//#ifndef __MACOSX__
#define ENTER_KEY "enter"
#define ENTER_KEY_CAP "Enter"
//#else
//#define ENTER_KEY "return"
//#define ENTER_KEY_CAP "Return"
//#endif

#define NEXT_TOK  nextToken(line, start, end)
#define S2F(x)    atof(x.c_str())
#define S2I(x)    atol(x.c_str())

#define X  0
#define Y  1
#define Z  2

#define USE_ALPHA -3.0

#define PI        3.14159265359
#define PIOVER180 0.01745329252
#define I80OVERPI 57.2957795131
#define PIOVER2   1.57079632679
#define SQRT2     1.41421356237
#define torad(x)  x*PIOVER180
#define todeg(x)  x*I80OVERPI
#define sign(x)   ((x != 0) ? ((x < 0) ? -1.0 : 1.0) : 0.0)
#define MAX(x,y)  ((x > y) ? x : y)
#define MIN(x,y)  ((x < y) ? x : y)

#define ACCEL_GRAVITY 32.1740485564 // feet per second squared

#define SCREEN_QUAKE_DECAY      10.0
#define SCREEN_BLUR_DECAY       1.0
#define MESSAGE_DELAY_TIME      2.0

#define PLAYER_OBJECT currLevelObjs[0]

#define NUM_PARTICLES 2000
#define NUM_BULLETS 2000
#define NUM_LASERS 50
#define NUM_FORCES 500
#define NUM_SHOCKWAVES 200
#define NUM_TRIGGERS 64
#define NUM_BLUR_TEXTURES 8

#define BLUR_TEXTURE_WIDTH  128
#define BLUR_TEXTURE_HEIGHT 128
#define BLUR_DELAY          0.1

#define LEVEL_PREFIX       PKGDATADIR "levels/level"
#define LEVEL_INTRO_PREFIX PKGDATADIR "levels/leveltext"
#define NUM_LEVELS 8

#define LEVEL_QUIT 0
#define LEVEL_LOSE 1
#define LEVEL_WIN  2

#define LAUNCH_MULTI_PARTICLES(num,tp,x,y,ht,ang,al,sp) for (int pt = 0; pt < num; pt++) \
  launchParticle(tp, x, y, ht, ang, al, sp)

#define LAUNCH_MULTI_PARTICLES_C(num,tp,x,y,ht,ang,al,sp,cr,cg,cb) for (int pt = 0; pt < num; pt++) \
  launchParticle(tp, x, y, ht, ang, al, sp, cr, cg, cb)

#define HAS_KEY(x,y)    (x & 1 << (y-1))
#define HAS_WEAPON(x,y) (x & 1 << y)

#define DEFAULT_SURFACE_FRICTION  0.3
#define DEFAULT_SURFACE_HEALTH    0
#define DEFAULT_ARCH_HEIGHT       6.5
#define COLLIDE_OBJECT  1
#define COLLIDE_BULLET  2
#define RICOCHET_BULLET 4

#define OBJECT_CD_THRESHOLD   1600
#define OBJECT_DRAW_THRESHOLD 3600

#define MESSAGE_LENGTH 128

#define SCREENSHOT_CHECK if (keystate[SDLK_F5]) { \
  if (!screenshotKeyPressed) \
  { takeScreenshot(); screenshotKeyPressed = true; } \
} else screenshotKeyPressed = false;

// Cheats
#define CHEAT_INVUL   "invul"
#define CHEAT_WEAPONS "weapons"
#define CHEAT_AMMO    "ammo"
#define CHEAT_KEYS    "keys"
#define CHEAT_NOAI    "noai"
#define CHEAT_NOMASKS "nomasks"
#define CHEAT_ZOOM    "zoom"
#define CHEAT_SKIP    "levelskip"
#define CHEAT_ALL     "all"

#define NO_KEY 0
#define IS_MOUSE_AXIS(x)   (x <= -100)
#define IS_MOUSE_BUTTON(x) (x <= -1 && x >= -5)
#define MOUSE_X_POS      -101
#define MOUSE_X_NEG      -100
#define MOUSE_Y_POS      -201
#define MOUSE_Y_NEG      -200
#define MOUSE_BUTTON_1   -1
#define MOUSE_BUTTON_2   -2
#define MOUSE_BUTTON_3   -3

typedef unsigned char byte;

enum TEXTURE_NAMES
{
    // Fonts
    TEX_FONT,

    // Effects
    TEX_SHADOW, TEX_LIGHT, TEX_SCANLINES, TEX_MF_RIFLE, TEX_MF_SHOTGUN,
    TEX_EXPLOSION1, TEX_EXPLOSION2, TEX_EXPLOSION3, TEX_EXPLOSION4,
    TEX_SMOKE, TEX_BEAM1, TEX_BEAM2, TEX_LASER_GLOW, TEX_ESG_CHARGE,
    TEX_BORDER, TEX_TERMINAL_BG, TEX_BIT,

    // Box
    TEX_BOX_NORMAL, TEX_BARREL_NORMAL, TEX_STEELBOX_NORMAL,

    // Particles
    TEX_PARTICLE, TEX_PARTICLE_BLOOD, TEX_PARTICLE_SLIME, TEX_PARTICLE_SPARK, TEX_PARTICLE_ENERGY,
    TEX_PARTICLE_WOOD, TEX_PARTICLE_METAL, TEX_PARTICLE_GLOW, TEX_HEPA_SPARK, TEX_LASER_SPARK1, TEX_LASER_SPARK2,
    TEX_ESG_SHOCKWAVE,
    TEX_EXPLOSIONPART, TEX_RIFLE_SHELL, TEX_SHOTGUN_SHELL,

    // Powerups
    TEX_POWERUP_HEALTH, TEX_POWERUP_ENERGY_CLIP, TEX_POWERUP_SHOTGUN_AMMO, TEX_POWERUP_HEPA_CLIP,
    TEX_POWERUP_LASER_CELL, TEX_POWERUP_ESG_BATTERY,
    TEX_POWERUP_KEY1, TEX_POWERUP_KEY2, TEX_POWERUP_KEY3,
    TEX_WEAPON_RIFLE, TEX_WEAPON_SHOTGUN, TEX_WEAPON_HEPA, TEX_WEAPON_LASER, TEX_WEAPON_ESG,

    // Bullets
    TEX_BLT_RIFLE, TEX_BLT_SHOTGUN, TEX_BLT_GRENADE, TEX_BLT_SHRAPNEL, TEX_BLT_BLASTER,

    // Weapons
    TEX_WPN_RIFLE, TEX_WPN_SHOTGUN, TEX_WPN_HEPA, TEX_WPN_GRENADE, TEX_WPN_LASER, TEX_WPN_ESG,

    // Others
    TEX_TITLE_BG, TEX_BG2, TEX_HEX1, TEX_HEX2,


    // Model textures
    MTEX_PLAYER,
    MTEX_PLAYER2,
    MTEX_BARREL,
    MTEX_TURRET,
    MTEX_MIB,
    MTEX_UNARMED_GUARD,
    MTEX_ARMED_DRONE,
    MTEX_HUNTER,
    MTEX_GLADIATOR,
    MTEX_FADE,
    NUM_TEXTURES
};

enum MODEL_NAMES
{
    MDL_PLAYER_LEGS,
    MDL_PLAYER_TORSO,
    MDL_PLAYER_TORSO2,
    MDL_BOX,
    MDL_BARREL,
    MDL_STEELBOX,
    MDL_TURRET,
    MDL_MIB,
    MDL_UNARMED_GUARD,
    MDL_ARMED_DRONE,
    MDL_HUNTER,
    MDL_GLADIATOR,
    MDL_ZERO,
    MDL_ONE,
    NUM_MODELS
};


enum SOUND_NAMES
{
    // Weapons
    SND_WPN_RIFLE, SND_WPN_SHOTGUN, SND_WPN_HEPA, SND_WPN_LASER, SND_WPN_LASER2, SND_WPN_ESG, SND_GRENADE_BOUNCE,

    // Environment
    SND_DOOR_OPEN, SND_DOOR_CLOSE, SND_DOOR_UNLOCK,
    SND_EXPLOSION, SND_HEPA_EXPLOSION, SND_GRENADE_EXPLOSION, SND_BULLET_HIT_HARD, SND_BULLET_HIT_SOFT,
    SND_PICKUP_HEALTH, SND_PICKUP_ITEM,

    // Player
    SND_JUMP, SND_LAND, SND_RELOAD, SND_CHANGE_WEAPON,
    SND_EXPLODE, SND_FALL, SND_HEAL, SND_HURT1, SND_HURT2, SND_HURTSURFACE, SND_KILLED,

    // Enemies
    SND_ROBOT_SEES, SND_ROBOT_DEATH, SND_ROBOT_PARALYZE, SND_ROBOT_ALERT,

    SND_TERM_MOVE, SND_TERM_SELECT, SND_TERM_CLEAR, SND_LAUNCH, SND_LAUNCH2, SND_BUTTON, SND_ALARM,
    SND_LASER_CHARGE, SND_LASER_CHARGE_LOOP,
    NUM_SOUNDS
};


enum SONG_NAMES
{
    MUS_TITLE, MUS_DD2, MUS_END,
    NUM_SONGS
};

enum TRIGGER_TYPES { TRIG_NORMAL = 0, TRIG_EVERY = 1, TRIG_ONCE = 2, TRIG_ADD = 4, TRIG_SOUND1 = 8, TRIG_SOUND2 = 16, TRIG_WHENSHOT = 32, MAX_TRIGTYPE = 52 };

enum MA_TYPES { MA_IDLE, MA_STAND, MA_WALK, MA_RUN, MA_SHORT, MA_LONG, MA_FLINCH, MA_KNOCK, MA_DIE, MA_DEAD, MA_DIEBACK, MA_DEADBACK, MA_SPECIAL1, MA_SPECIAL2, MA_SPECIAL3, NUM_MA_TYPES };

//enum MA_TYPES { MA_STAND, MA_RUN, MA_ATTACK, MA_PAIN1, MA_PAIN2, MA_DIE1, MA_DIE2, MA_DIE3, MA_SPECIAL1, MA_SPECIAL2, MA_SPECIAL3, NUM_MA_TYPES };

enum MA_PLAYER_LEG_TYPES { MA_PL_STAND, MA_PL_FWD, MA_PL_BACK, MA_PL_RIGHT, MA_PL_LEFT, MA_PL_JUMP, MA_PL_DIESOFT, MA_PL_DEADSOFT, MA_PL_DIEHARD, MA_PL_DEADHARD, MA_PL_FWDRIGHT, MA_PL_FWDLEFT, NUM_MA_PLAYER_LEG_TYPES };

enum MA_PLAYER_TORSO_TYPES { MA_PT_STAND, MA_PT_RUN, MA_PT_PALMSHOOT, MA_PT_GRENADE, MA_PT_PISTOL, MA_PT_PISTOLRUN, MA_PT_PISTOLSHOOT, MA_PT_PISTOLGRENADE, MA_PT_PISTOLRELOAD, MA_PT_RIFLE, MA_PT_RIFLERUN, MA_PT_RIFLESHOOT, MA_PT_RIFLEGRENADE, MA_PT_RIFLERELOAD, MA_PT_HEPA, MA_PT_HEPARUN, MA_PT_HEPASHOOT, MA_PT_HEPARELOAD, MA_PT_DIESOFT, MA_PT_DEADSOFT, MA_PT_DIEHARD, MA_PT_DEADHARD, NUM_MA_PLAYER_TORSO_TYPES };

float frand();
float lerp( float a0, float a1, float t );

class Configuration
{
public:
    Configuration()
    {
        defaults();
    }
    void defaults()
    {
        set( 1024, 768, 32, true, true, true, true, true, true, 0, 20, true, false, 0, "", false, false, false, false, false, false, false, false, false, false, false,  false );
        cheat_all = false;
        defaultKeys();
    }
    void defaultKeys()
    {
        setKeymapping( SDLK_LEFT, SDLK_RIGHT, SDLK_w, SDLK_s, SDLK_a, SDLK_d, SDLK_SPACE, SDLK_LCTRL, SDLK_g, SDLK_r, SDLK_TAB, SDLK_v, SDLK_z );
    }
    void loadFromFile( const char* filename );
    void writeToFile( const char* filename );
    void set( int x, int y, int d, bool f, bool a, bool bl, bool snd, bool mus, bool fx, int mt, int ms, bool mg, bool fp, int fc, string dl, bool n, bool p, bool i, bool c, bool ci, bool cw, bool ca, bool ck, bool cna, bool cnm, bool cls, bool cz )
    {
        xres = x;
        yres = y;
        depth = d;
        fullscreen = f;
        antialias = a;
        blur = bl;
        use_sound = snd;
        music = mus;
        sfx = fx;
        mthresh = mt;
        msense = ms;
        mgrab = mg;
        fps = fp;
        framecap = fc;
        defaultLevel = dl;
        normals = n;
        paths = p;
        info = i;
        culling = c;
        cheat_invul = ci;
        cheat_weapons = cw;
        cheat_ammo = ca;
        cheat_keys = ck;
        cheat_noai = cna;
        cheat_nomasks = cnm;
        cheat_skip = cls;
        cheat_zoom = cz;
    }
    void setKeymapping( SDLKey nkleft, SDLKey nkright, SDLKey nkfwd, SDLKey nkback, SDLKey nkstrl,
                        SDLKey nkstrr, SDLKey nkjump, SDLKey nkfire, SDLKey nkgren, SDLKey nkrel,
                        SDLKey nksww, SDLKey nkswv, SDLKey nkzoom )
    {
        keys[0] = nkleft;
        keys[1] = nkright;
        keys[2] = nkfwd;
        keys[3] = nkback;
        keys[4] = nkstrl;
        keys[5] = nkstrr;
        keys[6] = nkjump;
        keys[7] = nkfire;
        keys[8] = nkgren;
        keys[9] = nkrel;
        keys[10] = nksww;
        keys[11] = nkswv;
        keys[12] = nkzoom;
    }
    void setDefaultLevel( char* level )
    {
        defaultLevel = level;
    }
    bool cheating()
    {
        return cheat_invul || cheat_weapons || cheat_ammo || cheat_keys || cheat_skip || cheat_zoom;
    }
    int xres;
    int yres;
    int depth;
    bool fullscreen;
    bool antialias;
    bool blur;
    bool use_sound;
    bool music;
    bool sfx;
    int mthresh;
    int msense;
    bool mgrab;
    bool fps;
    int framecap;
    string defaultLevel;
    bool normals;
    bool paths;
    bool info;
    bool culling;

    SDLKey keys[13];

    bool cheat_all;
    bool cheat_invul;
    bool cheat_weapons;
    bool cheat_ammo;
    bool cheat_keys;
    bool cheat_noai;
    bool cheat_nomasks;
    bool cheat_skip;
    bool cheat_zoom;
};

class Trigger
{
public:
    Trigger()
    {
        hit = false;
        hitsRequired = hits = 0;
    }
    bool hit;
    int hitsRequired;
    int hits;
};

class RGBAColor
{
public:
    RGBAColor( float nr, float ng, float nb, float na ) :
        r( nr ), g( ng ), b( nb ), a( na ) {}
    RGBAColor()
    {
        zero();
    }
    void set( float nr, float ng, float nb, float na )
    {
        set( nr, ng, nb );
        a = na;
    }
    void set( float nr, float ng, float nb )
    {
        r = nr;
        g = ng;
        b = nb;
    }
    void blend( RGBAColor c, float amountOfNew );
    void print()
    {
        printf( "[R%0.3f G%0.3f B%0.3f A%0.3f]\n", r, g, b, a );
    }
    void zero()
    {
        set( 0, 0, 0, 0 );
    }
    void white()
    {
        set( 1, 1, 1, 1 );
    }
    void apply()
    {
        glColor4f( r, g, b, a );
    }

    float r, g, b, a;
};

// A 2D vector, backed by an array for convenience. Z coordinate is a dummy, because some OpenGL functions
// require ordered triples.
class Vector2D
{
public:
    Vector2D()
    {
        set( 0.0, 0.0 );
    }
    //Vector2D(Vector2D v) { set(v.c[X], v.c[Y]); }
    Vector2D( float nx, float ny )
    {
        set( nx, ny );
    }
    void set( float nx, float ny )
    {
        c[X] = nx;
        c[Y] = ny;
        c[Z] = 0;
    }
    void apply()
    {
        glVertex3fv( c );
    }
    void print()
    {
        printf( "(%f, %f)\n", c[X], c[Y] );
    }
    Vector2D& operator = ( Vector2D v )
    {
        c[X] = v.c[X];
        c[Y] = v.c[Y];
        return *this;
    }
    void operator += ( Vector2D v )
    {
        c[X] += v.c[X];
        c[Y] += v.c[Y];
    }
    void operator -= ( Vector2D v )
    {
        c[X] -= v.c[X];
        c[Y] -= v.c[Y];
    }
    void operator += ( float s )
    {
        c[X] += s;
        c[Y] += s;
    }
    void operator -= ( float s )
    {
        c[X] -= s;
        c[Y] -= s;
    }
    bool operator == ( Vector2D v )
    {
        return c[X] == v.c[X] && c[Y] == v.c[Y];
    }

    Vector2D operator + ( Vector2D v )
    {
        return Vector2D( c[X] + v.c[X], c[Y] + v.c[Y] );
    }
    Vector2D operator + ( float s )
    {
        return Vector2D( c[X] + s, c[Y] + s );
    }
    Vector2D operator - ( Vector2D v )
    {
        return Vector2D( c[X] - v.c[X], c[Y] - v.c[Y] );
    }

    // dot product
    float operator * ( Vector2D v )
    {
        return c[X] * v.c[X] + c[Y] * v.c[Y];
    }

    void operator *= ( float s )
    {
        c[X] *= s;
        c[Y] *= s;
    }
    void operator /= ( float s )
    {
        c[X] /= s;
        c[Y] /= s;
    }
    Vector2D operator * ( float s )
    {
        return Vector2D( c[X] * s, c[Y] * s );
    }
    Vector2D operator / ( float s )
    {
        return Vector2D( c[X] / s, c[Y] / s );
    }

    // interpolate between two vectors
    Vector2D interpolate( Vector2D v, float t )
    {
        return Vector2D( lerp( c[X], v.c[X], t ), lerp( c[Y], v.c[Y], t ) );
    }

    float mag()
    {
        return sqrt( ( c[X] * c[X] ) + ( c[Y] * c[Y] ) );
    }
    void normalize()
    {
        if( mag() != 0 )
        {
            *this /= mag();
        }
        else set( 0, 0 );
    }
    void neg()
    {
        c[X] = -c[X];
        c[Y] = -c[Y];
    }

    float c[3];
};

// A 2D vector backed by an array of doubles.
// we need this for floor polygons because OpenGL absolutely refuses
// to properly tessellate a concave polygon if its vertices are not doubles.
// (We want to keep everything else as floats, though.)
class Vector2Dd
{
public:
    Vector2Dd()
    {
        set( 0.0, 0.0 );
    }
    Vector2Dd( GLdouble nx, GLdouble ny )
    {
        set( nx, ny );
    }
    void set( GLdouble nx, GLdouble ny )
    {
        c[X] = nx;
        c[Y] = ny;
        c[Z] = 0;
    }
    void apply()
    {
        glVertex3dv( c );
    }
    Vector2D v2D()
    {
        return Vector2D( c[X], c[Y] );
    }
    GLdouble c[3];
};

class Line
{
public:
    Line() {}
    Line( Vector2D n1, Vector2D n2 )
    {
        p1 = n1;
        p2 = n2;
        normal = findNormal();
    }
    Line( float p1x, float p1y, float p2x, float p2y )
    {
        setCoords( p1x, p1y, p2x, p2y );
    }
    void setCoords( float p1x, float p1y, float p2x, float p2y )
    {
        p1.set( p1x, p1y );
        p2.set( p2x, p2y );
    }
    void set( Vector2D n1, Vector2D n2 )
    {
        p1 = n1;
        p2 = n2;
        normal = findNormal();
    }
    Vector2D midpoint()
    {
        return Vector2D( ( p1.c[X] + p2.c[X] ) / 2, ( p1.c[Y] + p2.c[Y] ) / 2 );
    }
    Vector2D findNormal()
    {
        Vector2D d = p2 - p1;
        return Vector2D( d.c[Y] / d.mag(), -d.c[X] / d.mag() );
    }
    bool intersection( Vector2D a, Vector2D b, Vector2D* p );
    Vector2D p1, p2, normal;
};

class BoundingBox
{
public:
    BoundingBox()
    {
        reset();
    }
    BoundingBox( vector<Vector2D> points )
    {
        calculate( points );
    }
    void reset()
    {
        pointsAdded = false;
    }
    void addPoint( Vector2D point );
    void set( Vector2D nmin, Vector2D nmax )
    {
        min = nmin;
        max = nmax;
        pointsAdded = true;
        center = ( min + max ) * 0.5;
    }
    void calculate( vector<Vector2D> points );
    void rotate( float angle );
    bool pointInBB( Vector2D point );
    bool intersectBB( BoundingBox bb );
    Vector2D min, max, center;
private:
    bool pointsAdded;
};

class Timer
{
public:
    Timer()
    {
        dt = 0.0;
        start = 0.0;
        fpsCache = 0.0;
        fpsCount = 0;
    }
    void reset()
    {
        start = timerFunc();
    }
    void update();
    double dT()
    {
        return dt;
    }
    int FPS()
    {
        return fps;
    }
    double dt;
    int fps;
private:
    double timerFunc();
    double start;
    double fpsCache;
    int fpsCount;
};

class BitmapFont
{
public:
    BitmapFont() {}
    BitmapFont( int index )
    {
        create( index );
    }
    void create( int index );
    void drawChar( char c );
    void print( float x, float y, float scaleX, float scaleY, float spacing, const char* text );
    void printf( float x, float y, float scaleX, float scaleY, const char* fmt, ... );
    void LEDprintf( float x, float y, float xscale, float yscale, const char* fmt, ... );
    void vprintf( float x, float y, float scaleX, float scaleY, const char* fmt, va_list ap );
private:
    SDL_Surface* charMap;
    int texIndex;
    float lastY;
    float lastYScale;
};

////

// game.cpp prototypes
float distSquared( Vector2D a, Vector2D b );
float dist( Vector2D a, Vector2D b );
bool hasLineOfSight( Vector2D start, Vector2D end, bool includeDoors );
float rayCircleIntersection( Vector2D rayOrigin, Vector2D rayDisplacement, Vector2D circleCtr, float circleRad );
bool circleSegmentIntersection( Vector2D c, float r, Vector2D p1, Vector2D p2, Vector2D* intersect );
string nextToken( string line, int& start, int& end );
string getLevelFileName( int levelNum );
string getLevelIntroFileName( int levelNum );
void gameWelcomeMessage();
void gameInitSDL();
void gameInitOpenGL();
void gameSetVideoMode( int width, int height, int depth, int fullscreen );
void gameSetAntialiasing( bool set );
void gameSetPerspectiveMode();
void gameSetOrthoMode( float r );
void gameInitTessellator();
void gameLoadSprites();
void gameLoadModels();
void gameLoadSounds();
void gameLoadMusic();
void gameDestroySprites();
void gameDestroySounds();
void gameDestroyMusic();
void gameShutdown();

void loadAI();
void destroyAI();

void launchParticle( int typ, float x, float y, float height, float head, float alph, int sprite );
void launchParticle( int typ, float x, float y, float height, float head, float alph, int sprite, float r, float g, float b );
void killParticle( int index );
void launchBullet( int own, int typ, float x, float y, float height, float head );
void killBullet( int index );
void launchLaser( int typ, float x, float y, float height, float head );
void killLaser( int index );
void addForce( int obj, float dur, float a, float b, bool mode );
void addInstantForce( int obj, float a, float b, bool mode );
void killForce( int index );
void addShockwave( float x, float y, float ir, float irs, float otr, float ors, float dur, float frc, float hlth, bool ag = true );
void killShockwave( int index );
void setScreenQuake( float amt );
void setBlur( float amt, float r, float g, float b );
void addBlur( float amt, float r, float g, float b );
void setTrigger( int t );
void incrementTrigger( int t );
void setMessage( const char* fmt, ... );

void playSound( int sound, int channel );
void playSoundLooped( int sound, int channel );
void stopSound( int channel );
void startMusic( int song );
void stopMusic();
void fadeOutMusic( int ms );
void fadeInMusic( int song, int ms );

int playCurrentLevel( bool playInSequence );
void game( string levelToPlay, bool playInSequence );
void drawScene( BoundingBox viewRegion );
void drawObjectShadows( int cMinX, int cMinY, int cMaxX, int cMaxY );
bool lineSegmentIntersection( Vector2D a, Vector2D b, Vector2D c, Vector2D d, Vector2D* p );

// misc. prototypes
void CALLBACK tcbBegin( GLenum prim );
void CALLBACK tcbEnd();
void CALLBACK tcbCombine( GLdouble c[3], void* d[4], GLfloat w[4], void** out );
void CALLBACK tcbError( GLenum errnum );

#include "bitmapfont.h"
#include "levelobjects.h"
#include "md2.h"
#include "weapons.h"
#include "objects.h"
#include "enemies.h"
#include "level.h"
#include "camera.h"
#include "input.h"
#include "player.h"
#include "interface.h"

extern SDL_Surface* texdata[NUM_TEXTURES];
extern GLuint textures[NUM_TEXTURES];

extern unsigned int* blurdata[NUM_BLUR_TEXTURES];
extern GLuint blurTextures[NUM_BLUR_TEXTURES];

extern Model models[NUM_MODELS];
extern Mix_Chunk* snddata[NUM_SOUNDS];
extern Mix_Music* musicdata[NUM_SONGS];

extern Configuration config;
extern bool GFXMODE_ISACTIVE;

extern float SCREEN_TOP;
extern float SCREEN_CENTER;
extern float SCREEN_ASPECT;

extern BitmapFont font;
extern GLUtesselator* tess;
extern vector<GLdouble*> tessAllocatedVertices;
extern MD2Loader modelLoader;
extern Timer timer;

extern SDL_Event event;
extern Uint8* keystate;
extern SDLMod modstate;
extern int mouse_dX;
extern int mouse_dY;
extern Uint8 mouseButtons;
// Global variables which store the state of the current
// level and its objects. The player object is always index 0
// in the currLevelObjs array.

extern char message[MESSAGE_LENGTH];
extern float messageTime;

extern bool winLevel;
extern Level currLevel;
extern Player P;
extern vector<Object*> currLevelObjs;
extern vector<AI*> currLevelEnemies;
extern Particle particles[NUM_PARTICLES];
extern int liveParticles;
extern Bullet bullets[NUM_BULLETS];
extern int liveBullets;
extern Laser lasers[NUM_LASERS];
extern int liveLasers;
extern Force forces[NUM_FORCES];
extern int liveForces;
extern Shockwave shockwaves[NUM_SHOCKWAVES];
extern int liveShockwaves;
extern Trigger triggers[NUM_TRIGGERS];

extern int screenshotCount;
extern bool screenshotKeyPressed;
#endif
