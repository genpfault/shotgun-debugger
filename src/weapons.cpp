/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * weapons.cpp - weapon functions
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

#include "sdb.h"

void Weapon::set( int typ, int ra, int own )
{
    type = typ;
    owner = own;
    reset();
    set( ra );
    charge = 0;
    triggerTime = 0.0;
}

void Weapon::set( int ra )
{
    if( ra == MAX_AMMO )
        addAmmo( wpnType[type].maxAmmo );
    else if( ra <= -1 )
    {
        for( int i = ra; i < 0; i++ )
            addClipAmt();
    }
    else
    {
        addAmmo( ra );
        addAmmoToClip( wpnType[type].clipSize );
    }

    triggerTime = 0.0;
}

void Weapon::addAmmo( int amt )
{
    reserveAmmo = ( amt + reserveAmmo < wpnType[type].maxAmmo ) ? amt + reserveAmmo : wpnType[type].maxAmmo;
}

void Weapon::addClip()
{
    addAmmo( wpnType[type].clipSize );
}

int Weapon::addAmmoAmt( int amt )
{
    int prevAmmo = reserveAmmo;
    addAmmo( amt );
    return reserveAmmo - prevAmmo;
}

int Weapon::addClipAmt()
{
    return addAmmoAmt( wpnType[type].clipSize );
}

void Weapon::update()
{
    if( ammoInClip && !reloading )
    {
        if( !readyToFire )
        {
            rofTimer += timer.dT();
            if( rofTimer >= 1.0 / fabs( wpnType[type].rateOfFire ) )
            {
                rofTimer = 0;
                readyToFire = true;
            }
        }
    }
    else
        readyToFire = false;

    if( ammoInClip == 0 && !reloading )
        reload();

    if( reloading )
    {
        reloadTimer += timer.dT();
        if( reloadTimer >= wpnType[type].reloadTime )
        {
            reloadTimer = 0;
            readyToFire = true;
            reloading = false;
            addAmmoToClip( wpnType[type].clipSize - ammoInClip );
        }
    }
}

void Weapon::addAmmoToClip( int amt )
{
    if( reserveAmmo < amt ) amt = reserveAmmo;
    ammoInClip += amt;
    reserveAmmo -= amt;
}

// returns true if a bullet was fired
bool Weapon::fire( float x, float y, float head, float h )
{
    if( !reloading )
        triggerTime += timer.dT();

    if( readyToFire && !semiAutoLock )
    {
        if( wpnType[type].bulletType != NO_BULLET )
        {
            if( type != WPN_LASER )
                shoot( x, y, head, h, FIRE_PRIMARY );
            else
            {
                if( !playChargeSound && ammoInClip == wpnType[type].clipSize && triggerTime > 0.1 )
                {
                    playSound( SND_LASER_CHARGE, 3 );
                    playChargeSound = true;
                }

                if( !playChargeLoop && ammoInClip == wpnType[type].clipSize && triggerTime > 1.0 )
                {
                    playSoundLooped( SND_LASER_CHARGE_LOOP, 3 );
                    playChargeLoop = true;
                }
            }

            return true;
        }
    }

    return false;
}

void Weapon::releaseTrigger( float x, float y, float head, float h )
{
    if( type == WPN_LASER && readyToFire && !semiAutoLock )
    {
        if( triggerTime > 0 )
        {
            if( triggerTime > 1.0 && ammoInClip == wpnType[type].clipSize )
                shoot( x, y, head, h, FIRE_SECONDARY );
            else
                shoot( x, y, head, h, FIRE_PRIMARY );

            playChargeSound = playChargeLoop = false;
        }
    }

    triggerTime = 0.0;
    if( isSemiautomatic() && semiAutoLock )
        semiAutoLock = false;
}

void Weapon::shoot( float x, float y, float head, float h, bool mode )
{
    // adjust the coordinates so the bullet is fired from the tip of the weapon
    if( wpnType[type].bulletType == BLT_LASER )
    {
        launchLaser( ( mode == FIRE_PRIMARY ) ? L_STANDARD : L_CHARGED,
                     x + ( wpnType[type].spriteSize.c[X] / 2 * cos( head ) ),
                     y + ( wpnType[type].spriteSize.c[X] / 2 * sin( head ) ),
                     h, head + ( frand()*wpnType[type].jitter * 2 - wpnType[type].jitter ) );
        stopSound( 3 );
    }
    else
    {
        float xoff = wpnType[type].spriteSize.c[X] / 2 + objType[wpnType[type].bulletType].boundRadius + 0.1;
        for( int i = 0; i < wpnType[type].bulletsPerShot; i++ )
            launchBullet( owner, wpnType[type].bulletType,
                          x + ( xoff * cos( head ) ),
                          y + ( xoff * sin( head ) ),
                          h, head + ( frand()*wpnType[type].jitter * 2 - wpnType[type].jitter ) );
    }

    // muzzle flash
    int mf = 0;
    switch( type )
    {
    case WPN_RIFLE:
        mf = PART_MF_RIFLE;
        break;

    case WPN_SHOTGUN:
        mf = PART_MF_SHOTGUN;
        break;
    }

    if( mf )
        launchParticle( mf,
                        x + ( ( wpnType[type].spriteSize.c[X] + objType[mf].spriteSize.c[X] ) / 2 * cos( head ) ),
                        y + ( ( wpnType[type].spriteSize.c[X] + objType[mf].spriteSize.c[X] ) / 2 * sin( head ) ), h, head, 1.0, 0 );

    // magazine casing
    if( wpnType[type].shellType >= 0 )
        launchParticle( wpnType[type].shellType, x,
                        y, 0, head - PIOVER2, 1.0, 0 );

    // recoil
    if( wpnType[type].recoil != 0.0 && owner > -1 )
        currLevelObjs[owner]->changeForceVel(
            Vector2D( cos( head ) * -wpnType[type].recoil, sin( head ) * -wpnType[type].recoil ) );

    if( owner == 0 )
        P.shots++;

    if( owner == 0 && config.cheat_ammo ) {}
    else
        ammoInClip = ( mode == FIRE_PRIMARY ) ? ammoInClip - 1 : 0;

    readyToFire = false;
    if( isSemiautomatic() )
        semiAutoLock = true;

    playSound( wpnType[type].sound + ( ( mode == FIRE_PRIMARY ) ? 0 : 1 ), 0 );
}


void Weapon::reload()
{
    if( !reloading && ammoInClip < wpnType[type].clipSize && reserveAmmo > 0 )
    {
        reloading = true;
        reloadTimer = 0;
        readyToFire = false;
        triggerTime = 0.0;

        if( type != WPN_GRENADE )
            playSound( SND_RELOAD, 7 );
    }
}

// Translate and rotate to the weapon's position and heading
// before drawing
void Weapon::draw()
{
    if( wpnType[type].sprite != NO_SPRITE )
    {
        glEnable( GL_TEXTURE_2D );
        glColor4f( 1.0, 1.0, 1.0, 1.0 );
        glPushMatrix();
        glBindTexture( GL_TEXTURE_2D, textures[wpnType[type].sprite] );
        glBegin( GL_QUADS );
        glTexCoord2f( 0.0, 0.0 );
        glVertex2f( -wpnType[type].spriteSize.c[X] / 2, wpnType[type].spriteSize.c[Y] / 2 );
        glTexCoord2f( 1.0, 0.0 );
        glVertex2f( wpnType[type].spriteSize.c[X] / 2, wpnType[type].spriteSize.c[Y] / 2 );
        glTexCoord2f( 1.0, 1.0 );
        glVertex2f( wpnType[type].spriteSize.c[X] / 2, -wpnType[type].spriteSize.c[Y] / 2 );
        glTexCoord2f( 0.0, 1.0 );
        glVertex2f( -wpnType[type].spriteSize.c[X] / 2, -wpnType[type].spriteSize.c[Y] / 2 );
        glEnd();

        if( type == WPN_LASER && ammoInClip == wpnType[type].clipSize )
        {
            glBindTexture( GL_TEXTURE_2D, textures[TEX_LASER_GLOW] );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE );
            glColor4f( 1.0, 1.0, 1.0, MIN( triggerTime, 1.0 ) );
            glTranslatef( 0, 0, 0.1 );
            glBegin( GL_QUADS );
            glTexCoord2f( 0.0, 0.0 );
            glVertex2f( -wpnType[type].spriteSize.c[X] / 2, wpnType[type].spriteSize.c[Y] / 2 );
            glTexCoord2f( 1.0, 0.0 );
            glVertex2f( wpnType[type].spriteSize.c[X] / 2, wpnType[type].spriteSize.c[Y] / 2 );
            glTexCoord2f( 1.0, 1.0 );
            glVertex2f( wpnType[type].spriteSize.c[X] / 2, -wpnType[type].spriteSize.c[Y] / 2 );
            glTexCoord2f( 0.0, 1.0 );
            glVertex2f( -wpnType[type].spriteSize.c[X] / 2, -wpnType[type].spriteSize.c[Y] / 2 );
            glEnd();
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        }

        glPopMatrix();
        glDisable( GL_TEXTURE_2D );
    }
}

float Weapon::getLaserCharge()
{
    if( type != WPN_LASER || ammoInClip != wpnType[type].clipSize )
        return 0.0;
    else
        return MIN( triggerTime, 1.0 );
}

bool Weapon::isSemiautomatic()
{
    return wpnType[type].rateOfFire < 0;
}

void initWeaponTypes()
{
    for( int i = 0; i < NUM_WPN_TYPES; i++ )
        switch( i )
        {
        // char *nm, int btype, int spr, int stype, int snd, float sx, float sy, int cs, int ma, int bps, float rof, float rt, float j, float r
        case WPN_NONE:
            wpnType[i].set( "None", NO_BULLET, NO_SPRITE, NO_SPRITE, NO_SOUND, 0, 0, 0, 0, 1, 0, 0, 0, 0 );
            break;
        case WPN_RIFLE:
            wpnType[i].set( "Rifle", BLT_RIFLE, TEX_WPN_RIFLE, PART_RIFLE_SHELL, SND_WPN_RIFLE, 2.0, 0.3, 40, 320, 1, 10, 1, torad( 2 ), 160 );
            break;
        case WPN_SHOTGUN:
            wpnType[i].set( "Shotgun", BLT_SHOTGUN, TEX_WPN_SHOTGUN, PART_SHOTGUN_SHELL, SND_WPN_SHOTGUN, 2.0, 0.25, 6, 42, 10, 1 * SEMI_AUTO, 2.5, torad( 8 ), 4800 );
            break;
        case WPN_HEPA:
            wpnType[i].set( "H.E.P.A.", BLT_HEPA, TEX_WPN_HEPA, NO_SPRITE, SND_WPN_HEPA, 2.0, 1.0, 4, 30, 1, 0.75 * SEMI_AUTO, 2.5, 0, 6000 );
            break;
        case WPN_GRENADE:
            wpnType[i].set( "Grenade", BLT_GRENADE, NO_SPRITE, NO_SPRITE, NO_SOUND, 0.3, 0.3, 1, 9, 1, 0.5 * SEMI_AUTO, 2.5, 0, 0 );
            break;
        case WPN_LASER:
            wpnType[i].set( "Laser", BLT_LASER, TEX_WPN_LASER, NO_SPRITE, SND_WPN_LASER, 2.0, 1.0, 6, 30, 1, 4 * SEMI_AUTO, 2.0, 0, 6000 );
            break;
        case WPN_ESG:
            wpnType[i].set( "EM Shock Gen", ESG_EXPLODER, TEX_WPN_ESG, NO_SPRITE, SND_WPN_ESG, 2.0, 2.0, 1, 5, 1, 1.5 * SEMI_AUTO, 3.5, 0, 0 );
            break;
        case WPN_BLASTER:
            wpnType[i].set( "Subdermal Blaster", BLT_BLASTER, NO_SPRITE, NO_SPRITE, SND_WPN_HEPA, 0, 0, 100, 1000, 1, 10 * SEMI_AUTO, 0, 0, 0 );
            break;
        case WPN_TURRET_GUN:
            wpnType[i].set( "Turret gun", BLT_RIFLE, TEX_WPN_RIFLE, PART_RIFLE_SHELL, SND_WPN_RIFLE, 2.0, 0, 10, 999999, 1, 10, 1, torad( 5 ), 0 );
            break;

        }
}
