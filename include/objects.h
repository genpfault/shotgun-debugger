/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * objects.h - header file for object routines
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
 *
 * MD2 model loader by DigiBen, www.gametutorials.com. Adapted from his code.
 * The MD2 model format is copyright id software, inc.
 */
 
#ifndef _OBJECTS_H_
#define _OBJECTS_H_

#include "sdb.h"

// Types of all objects
enum OBJECT_TYPES {

ENT_PLAYER, ENT_BOX, ENT_BARREL, ENT_STEEL_CRATE, ENT_MISC1, ENT_MISC2, ENT_MISC3, ENT_MISC4, ENT_MISC5,
ENT_UNARMED_GUARD, ENT_MIB, ENT_ARMED_DRONE, ENT_HUNTER, ENT_GLADIATOR,
ENT_TURRET1, ENT_SLAVE_TURRET, ENT_EMISC1,
ENT_EMISC2, ENT_EMISC3, ENT_EMISC4, ENT_EMISC5,

POWERUP_HEALTH, POWERUP_SHOTGUN_AMMO, POWERUP_ENERGY_CLIP,
POWERUP_HEPA_CLIP, POWERUP_LASER_CELL, POWERUP_GRENADE, POWERUP_ESG_BATTERY,
POWERUP_KEY1, POWERUP_KEY2, POWERUP_KEY3, WEAPON_RIFLE,
WEAPON_SHOTGUN, WEAPON_HEPA, WEAPON_RED_LASER, WEAPON_ESG,

EXPLODER, BLT_LASER,

PART_PARTICLE, PART_PARTICLE_BLOOD, PART_PARTICLE_SLIME, PART_PARTICLE_SPARK, PART_PARTICLE_ENERGY,
PART_PARTICLE_WOOD, PART_PARTICLE_METAL, PART_PARTICLE_GLOW, PART_HEPA_TRAIL, PART_HEPA_EXPLOSION, PART_HEPA_SPARK, PART_GRENADE_TRAIL,
PART_LASER_SPARK1, PART_LASER_SPARK2, PART_ESG_SHOCKWAVE,
PART_BIT,
PART_MF_RIFLE, PART_MF_SHOTGUN,
PART_RIFLE_SHELL, PART_SHOTGUN_SHELL,
PART_EXPLOSION, PART_EXPLOSIONPART,
PART_SMOKE,


BLT_RIFLE, BLT_SHOTGUN, BLT_HEPA, BLT_GRENADE, BLT_SHRAPNEL, BLT_BLASTER,

ESG_EXPLODER,
NUM_OBJ_TYPES};
#define MAX_ENT     38
//#define MAX_POWERUP 24
#define MAX_PART    7

#define IS_ENEMY(x)   (x >= ENT_UNARMED_GUARD && x <= ENT_SLAVE_TURRET)
#define IS_POWERUP(x) (x >= POWERUP_HEALTH   && x <= WEAPON_ESG)
#define IS_WEAPON(x)  (x >= WEAPON_RIFLE && x <= WEAPON_ESG)
#define IS_BULLET(x)  (x >= BLT_RIFLE        && x <= BLT_BLASTER)
#define POWERUP_HEALTH_AMOUNT   25

// Extra type definitions for particles
enum PARTICLE_TYPES {
P_PART_PARTICLE, P_PART_PARTICLE_BLOOD, P_PART_PARTICLE_SLIME, P_PART_PARTICLE_SPARK, P_PART_PARTICLE_ENERGY,
P_PART_PARTICLE_WOOD, P_PART_PARTICLE_METAL, P_PART_PARTICLE_GLOW, P_PART_HEPA_TRAIL,P_PART_HEPA_EXPLOSION,P_PART_HEPA_SPARK,P_PART_GRENADE_TRAIL,
P_PART_LASER_SPARK1, P_PART_LASER_SPARK2, P_PART_ESG_SHOCKWAVE,
P_PART_BIT,
P_PART_MF_RIFLE, P_PART_MF_SHOTGUN,
P_PART_RIFLE_SHELL, P_PART_SHOTGUN_SHELL,
P_PART_EXPLOSION, P_PART_EXPLOSIONPART,
P_PART_SMOKE,

P_BLT_RIFLE, P_BLT_SHOTGUN, P_BLT_HEPA, P_BLT_GRENADE, P_BLT_SHRAPNEL, P_BLT_BLASTER,
P_ESG_EXPLODER,
NUM_PART_TYPES};

// Extra type definitions for bullets
enum BULLET_TYPES {
B_BLT_RIFLE, B_BLT_SHOTGUN, B_BLT_HEPA, B_BLT_GRENADE, B_BLT_SHRAPNEL, B_BLT_BLASTER,
NUM_BLT_TYPES};

// Laser types
enum LASER_TYPES {
L_STANDARD, L_CHARGED,
NUM_LASER_TYPES};

// Death types
enum DEATH_TYPES { DEATH_NONE, DEATH_PLAYER, DEATH_SMALL_EXPLOSION, DEATH_EXPLOSION,
DEATH_HEPA_EXPLOSION, DEATH_GRENADE_EXPLOSION, DEATH_WOOD, DEATH_ROBOT, DEATH_GLADIATOR, DEATH_POWERUP, DEATH_ESG, NUM_DEATH_TYPES };

enum ALERT_TYPES { ALERT_NONE, ALERT_TOUCH, ALERT_SHOT, ALERT_SHOCKWAVE };

#define MAX_OBJ_SPRITES  32
#define NO_SPRITE        -1
#define NO_MODEL         -1
#define NO_ALERT         -1000
#define INF              -1
#define ANIM_FWD         1
#define ANIM_REV         -1
#define FORCE_RECT       true
#define FORCE_POLAR      false
#define HIT_NOTHING      0
#define HIT_DIE          1
#define HIT_STOP         2
#define HIT_BOUNCE       3
#define ANIM_BASE        false
#define ANIM_TRANSIENT   true
#define DAMAGE_SHOCKWAVE true
#define DAMAGE_BULLET    false
#define SHOCK_ESG        -10000.0

#define NUM_KEYS         5

#define JUMP_PAUSE_TIME          0.5
#define FALL_KILL_DELAY          2.0
#define PUSH_STRENGTH            0.5
#define TERMINAL_VELOCITY        100.0
#define WALL_COLLISION_THRESHOLD 20.0
#define WALL_DAMAGE_THRESHOLD    35.0
#define IMPACT_DAMAGE_FACTOR     10.0
#define IMPACT_STUN_FACTOR       33.0
#define GIB_THRESHOLD            -30.0
#define AUG_DAMAGE_SUPPRESSION   0.25
#define AUG_SPEED_INCREASE       1.4



#define texQuad(texnum) glBindTexture(GL_TEXTURE_2D, textures[texnum]); \
    glBegin(GL_QUADS); \
    glTexCoord2f(0.0, 0.0); \
    glVertex2f(-objType[type].spriteSize.c[X]/2, objType[type].spriteSize.c[Y]/2); \
    glTexCoord2f(1.0, 0.0); \
    glVertex2f(objType[type].spriteSize.c[X]/2, objType[type].spriteSize.c[Y]/2); \
    glTexCoord2f(1.0, 1.0); \
    glVertex2f(objType[type].spriteSize.c[X]/2, -objType[type].spriteSize.c[Y]/2); \
    glTexCoord2f(0.0, 1.0); \
    glVertex2f(-objType[type].spriteSize.c[X]/2, -objType[type].spriteSize.c[Y]/2); \
    glEnd(); \

#define texQuadSize(texnum,w,h) glBindTexture(GL_TEXTURE_2D, textures[texnum]); \
    glBegin(GL_QUADS); \
    glTexCoord2f(0.0, 0.0); \
    glVertex2f(-w/2, h/2); \
    glTexCoord2f(1.0, 0.0); \
    glVertex2f(w/2, h/2); \
    glTexCoord2f(1.0, 1.0); \
    glVertex2f(w/2, -h/2); \
    glTexCoord2f(0.0, 1.0); \
    glVertex2f(-w/2, -h/2); \
    glEnd(); \

#define texQuadClip(texnum,width,offset) glBindTexture(GL_TEXTURE_2D, textures[texnum]); \
    glBegin(GL_QUADS); \
    glTexCoord2f(width*offset, 0.0); \
    glVertex2f(-objType[type].spriteSize.c[X]/2, objType[type].spriteSize.c[Y]/2); \
    glTexCoord2f(width*offset+offset, 0.0); \
    glVertex2f(objType[type].spriteSize.c[X]/2, objType[type].spriteSize.c[Y]/2); \
    glTexCoord2f(width*offset+offset, 1.0); \
    glVertex2f(objType[type].spriteSize.c[X]/2, -objType[type].spriteSize.c[Y]/2); \
    glTexCoord2f(width*offset, 1.0); \
    glVertex2f(-objType[type].spriteSize.c[X]/2, -objType[type].spriteSize.c[Y]/2); \
    glEnd(); \

#define addWeapon(wepn,name,full) \
    if (augmented) \
      pickedUp = false; \
    else if (wpn[wepn].addClipAmt() > 0 || !(weapons & (1 << wepn))) \
      setMessage(name); \
    else \
    { \
      setMessage(full); \
      pickedUp = false; \
    } \
    weapons |= 1 << wepn; \

#define addWeaponSwitch(wepn,name,full) \
    if (augmented) \
      pickedUp = false; \
    else if (wpn[wepn].addClipAmt() > 0 || !(weapons & (1 << wepn))) \
    { \
      setMessage(name); \
      currWeapon = wepn; \
    } \
    else \
    { \
      setMessage(full); \
      pickedUp = false; \
    } \
    weapons |= 1 << wepn; \

#define addAmmoPowerup(wepn,str,full) \
    if (augmented) \
      pickedUp = false; \
    else if ((iAmt = wpn[wepn].addClipAmt()) > 0) \
      setMessage(str, iAmt); \
    else \
    { \
      setMessage(full); \
      pickedUp = false; \
    } \
    
extern int oID;
    
// Data for animation control
class Animation
{
  public:
    Animation() { reset(); }
    Animation(int nf, float dl, int lp) { set(nf, dl, lp); }
    void define(int nf, float dl, int lp) { numFrames = nf; delay = dl; loop = lp; }
    void set(int nf, float dl, int lp) { define(nf, dl, lp); reset(); }
    void reset() { done = false; run = true; animTime = frame = numLoops = 0; stopAtFrame = -1; direction = ANIM_FWD; }
    void update();
    
    int NumFrames() { return numFrames; }
    int Loops() { return loop; }
    bool isDone() { return done; }
    int Frame() { return frame; }
    float Delay() { return delay; }
    float Time() { return animTime; }
    
    void setDelay(float dl) { delay = dl; }
    void setDirection(int dir) { direction = dir; }
    void freezeAtFrame(int f) { stopAtFrame = f; }
    
  private:
    int numFrames;
    float delay;
    int loop;
    bool run;
    bool done;
    
    float animTime;
    int frame;
    int numLoops;
    int direction;
    int stopAtFrame;
};

class ModelAnimation
{
  public:
    ModelAnimation() { reset(); }
    ModelAnimation(int mdl) { set(mdl); }
    void reset() { model = baseAnim = transientAnim = 0; currAnim = false; currFrame = 0; nextFrame = 1; animTime = 0.0; speed = ANIMATION_SPEED; }
    void set(int mdl) { reset(); model = mdl; }
    void setSpeed(float s) { speed = s; }
    void update();
    void setAnimation(int val, bool loop);
    void inc() { currFrame++; nextFrame++; }
    void draw();
  //private:
    int model;
    int currFrame, nextFrame;
    int baseAnim, transientAnim;
    bool currAnim;
    float animTime;
    float speed;
};

// A force vector, applied to the specified object for the specified
// length of time. Measured in my force units (essentially the newton
// in US units, because the game uses US units)
class Force
{
  public:
    Force() { reset(); }
    Force(int obj, float dur, float a, float b, bool mode) { set(obj,dur,a,b,mode); }
    void set(int obj, float dur, float a, float b, bool mode);
    void reset() { forceTime = 0; active = false; destroyMe = false; }
    void apply();
    void setActive(bool val) {active = val;}
    bool toBeDestroyed() { return destroyMe; }
  private:
    int target;
    float duration;
    Vector2D force;
    float forceTime;
    bool active;
    bool destroyMe;
};

// An explosion shockwave.
class Shockwave
{
  public:
    Shockwave() { reset(); }
    
    // So much for Kate's syntax highlighting. I originally named "float otr" "float or"
    // (for "outer radius") and was getting the weirdest syntax errors ("parse error before "||").
    // I nearly screwed myself until I finally remembered that "or" is a C++ reserved word.
    // That's why I changed it to "otr".    
    Shockwave(float x, float y, float ir, float irs, float otr, float ors, float dur, float frc, float hlth, bool ag)
    { set(x, y, ir, irs, otr, ors, dur, frc, hlth, ag); }
    void set(float x, float y, float ir, float irs, float otr, float ors, float dur, float frc, float hlth, bool ag);
    void reset();
    void update();
    void draw();
    bool detect(Vector2D p, float *resultForce, float *resultHealth);
    float PosX() { return pos.c[X]; }
    float PosY() { return pos.c[Y]; }
    void setActive(bool val) { active = val; }
    bool isActive() { return active; }
    bool toBeDestroyed() { return destroyMe; }
    bool AffectGladiators() { return affectGladiators; }
  private:
    Vector2D pos;
    float innerRad, outerRad;
    float innerRadSpeed, outerRadSpeed;
    float duration;
    float force;
    float health;
    bool affectGladiators;
    float shockTime;
    bool active;
    bool destroyMe;
};

// A set of event-trigger flags.
// Value is 0 (do not take the action), 1 or -1 (take the action),
// or anywhere in between (analog input is allowed by some functions)
class ObjectEventTriggers
{
  public:
    ObjectEventTriggers() { reset(); }
    void reset() { LR = FB = strafe = 0.0; jump = fire = grenade = reload = switchWpn = 0; }
    void setLR(float val) { LR = val; }
    void setFB(float val) { FB = val; }
    void setStrafe(float val) { strafe = val; }
    void setJump(int val) { jump = val; }
    void setFire(int val) { fire = val; }
    void setGrenade(int val) { grenade = val; }
    void setReload(int val) { reload = val; }
    void setSwitchWpn(int val) { switchWpn = val; }
    float getLR() { return LR; }
    float getFB() { return FB; }
    float getStrafe() { return strafe; }
    int getJump() { return jump; }
    int getFire() { return fire; }
    int getGrenade() { return grenade; }
    int getReload() { return reload; }
    int getSwitchWpn() { return switchWpn; }
    
  private:
    float LR;      // turn left (-1) or right (+1)
    float FB;      // move forward (+1) or backward (-1)
    float strafe;  // strafe left (-1) or right (+1)
    int jump;      // jump
    int fire;      // fire current weapon
    int grenade;   // throw grenade
    int reload;    // reload current weapon
    int switchWpn; // switch weapon
};

// Specifications for an object. Different types of object
// (particle, etc.) may have an additional, more specific specification.
// ObjectType specifies basic information about movement, life, appearance, etc.
class ObjectType
{
  public:
    ObjectType() {}
    ObjectType(float im,float iw,float ih,float ir,float imv,float ijv,float imtv,float ia,float ita,float hlth,int dth,bool cd,bool grav,bool shad)
    { set(im, iw, ih, ir, imv, ijv, imtv, ia, ita, hlth, dth, grav, shad, cd); }
    void set(float im,float iw,float ih,float ir,float imv,float ijv,float imtv,float ia,float ita,float hlth,int dth,bool cd,bool grav,bool shad);
    void clearSprites() { for (int i=0;i<MAX_OBJ_SPRITES;i++) sprites[i]=NO_SPRITE; }
    void setSprites(int num, ...);
    void setModels(int mod1, int mod2) { model1 = mod1; numModels = 1; if (mod2 != NO_MODEL) { model2 = mod2; numModels++; } }
    
    float mass;         // object mass (pounds)
    Vector2D spriteSize;// sprite height and width (note that this may, and probably
                        // will, exceed the dimensions of the actual object. An object's
                        // real size is determined by the bounding-box parameter.
    float boundRadius;  // bounding circle radius
   
    float maxVel;       // maximum velocity (feet per second)
    float jumpVel;      // jump velocity (feet per second)
    float maxTurnvel;   // maximum turn speed (radians per second)
  
    float accel;        // lateral acceleration rate (feet per second squared)
    float turnAccel;    // turning acceleration rate (radians per second squared)
   
    bool collide;       // collision detection on/off?
    bool gravAffect;    // is the object affected by gravity?
    bool hasShadow;     // does the object have a shadow?
    
    float maxHealth;    // maximum health (-1 == indestructible)
   
    int deathType;      // death type
    
    // Possible appearances of this object.
    // Each object type uses and controls these sprites in its own way.
    int sprites[MAX_OBJ_SPRITES];
    
    int model1, model2;
    int numModels;
};

class ParticleType
{
  public:
    ParticleType() {}
    ParticleType(float mrl, float mrs, float mrjs, float rv, float mrrv, float fs, float mrfs, float sf, float ss, float sst, int hb, bool gl,
             float acc, int anf, float ad, float mrad, int al)
             { set(mrl, mrs, mrjs, rv, mrrv, fs, mrfs, sf, ss, sst, hb, gl, acc, anf, ad, mrad, al); }
    void set(float mrl, float mrs, float mrjs, float rv, float mrrv, float fs, float mrfs, float sf, float ss, float sst, int hb, bool gl,
             float acc, int anf, float ad, float mrad, int al);

    float maxRandAddedToLife;   // life, speed, and jump speed are both defined in
    float maxRandAddedToSpeed;  // the ObjectType.
    float maxRandAddedToJumpSpeed;
    float minRotVel;            // minimum rotational velocity
    float maxRandAddedToRotVel;
    float minFadeSpeed;
    float maxRandAddedToFadeSpeed;
    float startFade;
    float scaleSpeed;
    float scaleStop;
    int hitBehavior;
    bool glow;
    float accel;
    
    int animNumFrames;
    float animDelay;
    float maxRandAddedToAnimDelay;
    int animLoop;
};

class BulletType
{
  public:
    BulletType() {}
    BulletType(float dmg, float f) { set(dmg, f); }
    void set(float dmg, float f) { damage = dmg; impactForce = f; }
    
    float damage;
    float impactForce;
};

class LaserType
{
  public:
    LaserType() {}
    LaserType(int spr, float w, float mr, int mb, float dmg, float fs) { set(spr, w, mr, mb, dmg, fs); }
    void set(int spr, float w, float mr, int mb, float dmg, float fs)
    { sprite = spr; width = w; maxRange = mr; maxBounces = mb; damage = dmg; fadeSpeed = fs; }
    
    int sprite;
    float width;
    float maxRange;
    int maxBounces;
    float damage;
    float fadeSpeed;
}; 

// Base class for objects.
class Object : public LevelObject
{
  public:
    Object() { type = 0; pos.set(0,0); vel.set(0,0); reset(); respondTo1 = respondTo2 = 0; setID(); }
    Object(int typ) { type = typ; reset(); setID(); }
    Object(int typ, float x, float y, float head, int rt1, int rt2, int tr, int tt) { setID(); set(typ, x, y, head, rt1, rt2, tr, tt); }
    Object(int typ, float x, float y, float height, float head, int rt1, int rt2, int tr, int tt) { setID(); set(typ, x, y, height, head, rt1, rt2, tr, tt); }
    void set(int typ, float x, float y, float head, int rt1, int rt2, int tr, int tt)
    { type = typ; reset(); setStartPos(x,y); heading = origHeading = head; trigInit(tr, tt); respondTo1 = rt1; respondTo2 = rt2; fixed = false; augmented = false;  spr1 = spr2 = NO_SPRITE; }
    void set(int typ, float x, float y, float height, float head, int rt1, int rt2, int tr, int tt)
    { type = typ; reset(); setStartPos(x,y); h = height; heading = origHeading = head; trigInit(tr, tt); respondTo1 = rt1; respondTo2 = rt2; fixed = false; augmented = false; spr1 = spr2 = NO_SPRITE; }
    
    virtual void reset();    
    virtual void draw();
    virtual void drawShadow();
    virtual void update();
      
    static void resetIDCounter();
    void setID();
    void setPos(float x, float y) { pos.set(x, y); }
    void setStartPos(float x, float y) { setPos(x, y); origPos.set(x,y); }
    void addPos(Vector2D v) { pos += v; }
    void setVel(float x, float y) { vel.set(x, y); }
    void setHeight(float nh) { h = nh; }
    void setHeading(float nhead) { heading = nhead; }
    void setLookAngle(float nangle) { lookAngle = nangle; }
    void turn(float amt);
    void setSpeed(float nspeed) { speed = nspeed; }
    void setHVel(float nhvel) { hvel = nhvel; }
    void setTurnVel(float ntvel) { turnvel = ntvel; }
    void setActive(bool val) { active = val; }
    void setHealth(int val) { health = val; }
    void setAnimation(int val, bool anim);
    void changeVel(Vector2D v);
    void changeForceVel(Vector2D v);
    void changeForceVelNoMass(Vector2D v);
    
    void setDirectionAlert(float ang, int typ) { dirAlert = ang; dirAlertType = typ; }
    void changeHealth(float amt);
	void makeInvulnerable() { invul = true; }
    float changeHealthAmt(float amt);
    void damage(float amt, bool by);
    void stun(float amt);
    void dyingCheck();
    
    void kill(bool death);
    void die(float delay) { dying = true; dyingTimer = delay; }
    void doDeath();
    
    int Type() { return type; }
    Vector2D Pos() { return pos; }
    Vector2D Vel() { return vel; }
    
    int ID() { return id; }
    float PosX() { return pos.c[X]; }
    float PosY() { return pos.c[Y]; }
    float Heading() { return heading; }
    float LookAngle() { return lookAngle; }
    float VelX() { return vel.c[X]; }
    float VelY() { return vel.c[Y]; }
    float Speed() { return vel.mag(); }
    float Height() { return h; }
    float HVel() { return hvel; }
    float Turnvel() { return turnvel; }
    float Health() { return health; }
    float StunTimer() { return stunTimer; }
    bool Stunned() { return stunTimer > 0; }
    bool Dying() { return dying; }
    bool Fixed() { return fixed; }
    int Keys() { return keys; }
    bool isActive() { return active; }
    bool toBeDestroyed() { return destroyMe; }
    bool Falling() { return fall; }
    int RespondTo1() { return respondTo1; }
    int RespondTo2() { return respondTo2; }
    float DirectionAlert() { return dirAlert; }
    float DirectionAlertType() { return dirAlertType; }
    int State() { return state; }
    float LastDamageAmount() { return lastDamageAmount; }
    bool Augmented() { return augmented; }
    void Augment() { model[1].set(MDL_PLAYER_TORSO2); augmented = true; }

    virtual Weapon* Wpn() { return NULL; }
    virtual int CurrWeapon() { return 0; }
    virtual void selectWeapon(int wp) {}
    virtual char weaponState(int wp) { return 0; }
    virtual char keyState(int wp) { return 0; }
    virtual Vector2D* WeaponPoint() { return NULL; }
    
    void giveKey(int key) { keys |= 1 << (key-1); }
    virtual void givePowerup(int idx) {}
    
    void resetCollided() { collided = -1; }
    void setCollided(int with) { collided = with; }
    int Collided() { return collided; }
    
    void resetShockCollisions() { shockCollisions = 0; }
    long long int ShockCollisions() { return shockCollisions; }
    
    ObjectEventTriggers ev;
        
    void nextAnimFrame() { model[0].inc(); model[1].inc(); }
    void CurrFrame() { printf("m1=(%d->%d) m2=(%d->%d)\n", model[0].currFrame+1, model[0].nextFrame+1, model[1].currFrame+1, model[1].nextFrame+1); }
    void setObjectCollisionDetectOnOff(bool s) { collideWithObjects = s; }
  protected:
    void drawStart();
    void drawEnd();
    
    void shockwaveCollisionDetect();
    void laserCollisionDetect();
    Wall* wallCollisionDetect(int cell, Vector2D *inter);
    bool floorCollisionDetect(int cell, float *fric);
    void surfaceCollisionDetect(int cell, float *fric, float *hlth);
    void objectCollisionDetect(int cell);
    bool CollideWithObjects() { return collideWithObjects; }
    
    void levelCollisionDetection();
    
    int id;
    
    int type;         // object type (for lookup in the global objType table)
    bool augmented;
    bool collideWithObjects;
    
    Vector2D pos;     // position
    float h;          // height
    
    Vector2D vel;     // x and y velocity components
    Vector2D moveVel;
    Vector2D forceVel;
    float speed;      // lateral (x-y) speed (feet per second)
    float jumpSpeed, jumpHeading, jumpStrafe, jumpPause;
    
    float hvel;       // height velocity
    
    float heading;    // heading
    float lookAngle;  // direction the player is looking in (can be different from movement heading)
    float turnvel;    // turning speed (radians per second)
    
    Vector2D origPos;
    float origHeading;
    
    // The object can be comprised of two independently-controlled sprites
    // displayed on top of each other. spr1 is the lower sprite, spr2 is the upper
    // sprite.
    int spr1, spr2;
    Animation spr1Anim, spr2Anim;
    
    ModelAnimation model[2];
    bool useModel;
    
    float health;       // health (dead at 0)
	bool invul;
    int keys;           // bitfield of keys this object owns
    
    // Whether an entity is fully alive or not
    bool active;
    bool dying;
    bool damagedBy;
    float lastDamageAmount;
    float dyingTimer;
    
    bool fixed;
    
    // Set this flag to tell the "higher powers" to deallocate this object.
    bool destroyMe;
    
    // Whether an object has just hit the ground
    bool hitGround;
    
    bool surfaceHealthSound;
    
    // Whether we should let this object fall to its DEATH
    bool fall;
    float fallTimer;
    
    float stunTimer;
    
    int collided;
    Uint64 shockCollisions; // bit-flags
    
    float dirAlert;
    int dirAlertType;
    
    int respondTo1;
    int respondTo2;
    
    float currSurfaceFriction;
    float currSurfaceHealth;
    
    int state;
};

// A static or moving entity (player, enemy, other object)
class Entity: public Object
{
  public:
    Entity() : Object() {}
    Entity(int typ) : Object(typ) {}
    Entity(int typ, float x, float y, float head, int st, bool fix, int rt1, int rt2, int tr, int tt, bool aug) : Object(typ, x, y, head, rt1, rt2, tr, tt) { fixed = fix; state = st; if (aug) Augment(); }
    void reset();
    void draw();
    void update();
    void drawShadow() { Object::drawShadow(); }
    
    void setPlayerAnimations();
        
    void selectWeapon(int wp)
    { if (HAS_WEAPON(weapons, wp)) currWeapon = wp; }
    void selectNextWeapon();
    
    void givePowerup(int idx);
    
    int CurrWeapon() { return currWeapon; }
    Weapon* Wpn() { return &wpn[currWeapon]; }
    Vector2D* WeaponPoint() { return &wpnPos[currWeapon]; }
    char weaponState(int wp);
    char keyState(int k);
  private:
    int weapons;
    float grenadeThrowTimer;
    bool throwGrenade;
    
    bool jumping;
    bool jumpReady;
    
    bool walkSound;
    
    Weapon wpn[NUM_PLAYER_WEAPONS];
    Vector2D wpnPos[NUM_PLAYER_WEAPONS];
    int currWeapon;
    bool switchReady;
};

// A particle
class Particle: public Object
{
  public:
    Particle() : Object() { color.white(); }
    Particle(int typ) : Object(typ) { initType(); }
    Particle(int typ, float x, float y, float head) : Object(typ, x, y, head, 0, 0, 0, 0) { initType(); }
    Particle(int typ, float x, float y, float height, float head) : Object(typ, x, y, height, head, 0, 0, 0, 0) { initType(); }
    void initType() { pType = type - PART_PARTICLE; color.white(); }
    void launch(int typ, float x, float y, float height, float head, float alph, int sprite);
    void launch(int typ, float x, float y, float height, float head, float alph, int sprite, float r, float g, float b);
    void setSpeed(float s) { speed = s; }
    void reset();
    void draw();
    void drawShadow();
    void update();
    void levelCollisionDetection();
  protected:
    int pType;
    int bounceCount;
    float rotation;
    float rotVel;
    float alpha;
    float fadeSpeed;
    float scale;
    
    float additionalSpeed;
    
    RGBAColor color;
    
    Animation anim;

};

// A bullet/projectile
class Bullet: public Particle
{
  public:
    Bullet() : Particle() {}
    Bullet(int own, int typ) : Particle(typ) { owner = own; bType = type - BLT_RIFLE; }
    Bullet(int own, int typ, float x, float y, float head) : Particle(typ, x, y, head) { owner = own; bType = type - BLT_RIFLE; }
    Bullet(int own, int typ, float x, float y, float height, float head) : Particle(typ, x, y, height, head) { owner = own; bType = type - BLT_RIFLE; }
    void launch(int own, int typ, float x, float y, float height, float head)
    { frameDelay = 2; owner = own; bType = typ - BLT_RIFLE; Particle::launch(typ, x, y, height, head, 1.0, 0); }
    
    void kill(bool death) { Object::kill(death); }
    
    int BType() { return bType; }
    
    void update();
    
    bool Live() { return frameDelay == 0; }
    int Owner() { return owner; }
  private:
    int bType;
    int owner;
    
    int frameDelay;
};

class LaserBeam
{
  public:
    LaserBeam() {}
    LaserBeam(Vector2D np1, Vector2D np2);
    
    Vector2D p1, p2;
    float angle;
    float length;
};

// LASER!
class Laser
{
  public:
    Laser() { active = destroyMe = false; live = 0; beams.clear(); } 
    void launch(int typ, float x, float y, float height, float head);
    void draw();
    void update();
    int Live() { return live; }
    bool toBeDestroyed() { return destroyMe; }
    int Type() { return type; }
    vector<LaserBeam> beams;
  private:
    Wall* wallCollisionDetect(Vector2D pos, float head, float distLeft, Vector2D *inter);
    
    bool active;
    int live;
    bool destroyMe;
    
    int type;
    float h;
    float alpha;
};

// Array of specification structures for all different types of objects.
extern ObjectType objType[NUM_OBJ_TYPES];
extern ParticleType partType[NUM_PART_TYPES];
extern BulletType bltType[NUM_BLT_TYPES];
extern LaserType lasType[NUM_LASER_TYPES];
void initObjectTypes();

#endif
