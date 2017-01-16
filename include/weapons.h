/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * weapons.h - header file for weapon functions
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

#ifndef _WEAPONS_H_
#define _WEAPONS_H_

#include "sdb.h"

#define MAX_AMMO  -9999
#define SEMI_AUTO -1
#define NO_BULLET -1
#define NO_SOUND  -1

#define FIRE_PRIMARY   false
#define FIRE_SECONDARY true

// Weapon types
enum WEAPON_TYPES
{
    WPN_NONE,
    WPN_RIFLE,
    WPN_SHOTGUN,
    WPN_HEPA,
    WPN_GRENADE,
    WPN_LASER,
    WPN_ESG,
    WPN_BLASTER,
    WPN_TURRET_GUN,
    NUM_WPN_TYPES
};

#define NUM_PLAYER_WEAPONS 7

// A type of weapon.
class WeaponType
{
public:
    WeaponType() {}
    WeaponType( const char* nm, int btype, int spr, int stype, int snd, float sx, float sy, int cs, int ma, int bps, float rof, float rt, float j, float r )
    {
        set( nm, btype, spr, stype, snd, sx, sy, cs, ma, bps, rof, rt, j, r );
    }
    void set( const char* nm, int btype, int spr, int stype, int snd, float sx, float sy, int cs, int ma, int bps, float rof, float rt, float j, float r )
    {
        strncpy( name, nm, 17 );
        sound = snd;
        bulletType = btype;
        sprite = spr;
        shellType = stype;
        spriteSize.set( sx, sy );
        clipSize = cs;
        maxAmmo = ma;
        bulletsPerShot = bps;
        rateOfFire = rof;
        reloadTime = rt;
        jitter = j;
        recoil = r;
    }

    char name[17];

    int bulletType;     // Type of projectiles it shoots
    int sprite;         // Sprite
    int sound;          // Shot sound
    int shellType;      // Type of particle ejected from gun
    Vector2D spriteSize;
    int clipSize;
    int maxAmmo;
    int bulletsPerShot; // number of projectiles per cartridge

    // Rate of fire. If this number is negative, the gun is semi-automatic,
    // and the trigger must be released and pulled again before another round
    // is fired.
    float rateOfFire;
    float reloadTime;
    float jitter;     // max. angle deviance (plus or minus)
    float recoil;
};

// A weapon (projectile launcher)
class Weapon
{
public:
    Weapon() {}
    Weapon( int typ, int own )
    {
        type = typ;
        owner = own;
        reset();
    }
    Weapon( int typ, int ra, int own )
    {
        set( typ, ra, own );
    }
    void set( int typ, int ra, int own );
    void set( int ra );
    void reset()
    {
        rofTimer = reloadTimer = ammoInClip = reserveAmmo = 0;
        reloading = semiAutoLock = playChargeSound = playChargeLoop = false;
    }
    void draw();
    void update();

    void addAmmo( int amt );
    void addAmmoToClip( int amt );
    void addClip();

    int addAmmoAmt( int amt );
    int addClipAmt();

    bool fire( float x, float y, float head, float h );
    void releaseTrigger( float x, float y, float head, float h );
    void shoot( float x, float y, float head, float h, bool mode );
    void reload();

    int Type()
    {
        return type;
    }
    int AmmoInClip()
    {
        return ammoInClip;
    }
    int ReserveAmmo()
    {
        return reserveAmmo;
    }
    bool Ready()
    {
        return readyToFire && !semiAutoLock;
    }
    float RofTimer()
    {
        return rofTimer;
    }
    float ReloadTimer()
    {
        return reloadTimer;
    }
    bool isReloading()
    {
        return reloading;
    }
    bool isSemiautomatic();
    float getLaserCharge();
private:
    int owner;

    int type;
    int ammoInClip;
    int reserveAmmo;
    bool readyToFire;
    bool semiAutoLock;

    bool reloading;
    float rofTimer;
    float reloadTimer;

    float triggerTime;

    // Special for charged weapons
    int charge;
    bool playChargeSound;
    bool playChargeLoop;
};

extern WeaponType wpnType[NUM_WPN_TYPES];
void initWeaponTypes();

#endif
