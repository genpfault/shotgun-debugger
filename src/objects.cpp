/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * objects.cpp - object (entity, particle, bullet, laser) routines
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
 */

#include "sdb.h"

//Level currLevel;
ObjectType objType[NUM_OBJ_TYPES];
ParticleType partType[NUM_PART_TYPES];
BulletType bltType[NUM_BLT_TYPES];
LaserType lasType[NUM_LASER_TYPES];
WeaponType wpnType[NUM_WPN_TYPES];
int oID;

// Update an animation
void Animation::update()
{
    if( numFrames > 1 && !done )
    {
        if( frame == stopAtFrame )
        {
            setDirection( 0 );
            stopAtFrame = -1;
        }

        animTime += timer.dT();
        if( animTime >= delay )
        {
            frame += direction;
            animTime = 0;

            if( direction > 0 )
            {
                if( frame >= numFrames )
                {
                    frame = 0;
                    numLoops++;
                }
            }
            else if( direction < 0 )
            {
                if( frame < 0 )
                {
                    frame = numFrames - 1;
                    numLoops++;
                }
            }

            if( loop > INF )
                if( numLoops >= loop )
                    done = true;
        }
    }
}

void ModelAnimation::update()
{
    if( model < 0 || models[model].pObject.size() <= 1 )
        return;

    tAnimationInfo* pAnim = &( models[model].pAnimations[
                                   ( currAnim == ANIM_BASE ) ? baseAnim : transientAnim] );

    nextFrame = ( currFrame + 1 ) % pAnim->endFrame;

    // Loop to the start if we're on the base animation. If not,
    // return to the base animation.
    if( nextFrame == 0 )
    {
        if( currAnim == ANIM_BASE )
        {
            nextFrame = pAnim->startFrame;
            //nextFrame = (currFrame+1) % pAnim->endFrame;
        }
        else
        {
            currAnim = ANIM_BASE;
            transientAnim = -1;
            tAnimationInfo* pAnim = &( models[model].pAnimations[baseAnim] );
            currFrame = pAnim->startFrame;
            nextFrame = ( currFrame + 1 ) % pAnim->endFrame;
        }
    }

    animTime += timer.dT();

    if( animTime >= 1.0 / speed )
    {
        currFrame = nextFrame;
        animTime = 0;
    }
}

void ModelAnimation::setAnimation( int val, bool anim )
{
    if( model >= 0 && models[model].pObject.size() > 1 )
    {
        //printf("%d\n", val);
        if( anim == ANIM_BASE && baseAnim != val )
        {
            baseAnim = val;
            if( currAnim == ANIM_BASE )
            {
                tAnimationInfo* pAnim = &( models[model].pAnimations[baseAnim] );
                currFrame = pAnim->startFrame;

                nextFrame = ( currFrame + 1 ) % pAnim->endFrame;
            }
        }
        // If we specify a transient ("one-shot") animation, we switch
        // to it automatically
        else if( anim == ANIM_TRANSIENT && transientAnim != val )
        {
            transientAnim = val;
            currAnim = ANIM_TRANSIENT;
            tAnimationInfo* pAnim = &( models[model].pAnimations[transientAnim] );
            currFrame = pAnim->startFrame;
            nextFrame = ( currFrame + 1 ) % pAnim->endFrame;
        }
    }
}

void ModelAnimation::draw()
{
    if( model >= 0 )
    {
        glBindTexture( GL_TEXTURE_2D, textures[models[model].texture] );
        models[model].draw( ( currAnim == ANIM_BASE ) ? baseAnim : transientAnim, currFrame, nextFrame, animTime, speed );
    }
}

// Set properties of a force.
// If mode is FORCE_RECT, a and b are the vector's x and y components
// If mode is FORCE_POLAR, a and b are the vector's magnitude and angle
void Force::set( int obj, float dur, float a, float b, bool mode )
{
    target = obj;
    duration = dur;
    ( mode == FORCE_RECT ) ? force.set( a, b ) : force.set( a * cos( b ), a * sin( b ) );
    reset();
}

// Applies a force to the target object
void Force::apply()
{
    if( active )
    {
        forceTime += timer.dT();
        currLevelObjs[target]->changeVel( force );
        if( forceTime >= duration ) destroyMe = true;
    }
}

void Shockwave::reset()
{
    shockTime = 0;
    active = false;
    destroyMe = false;
}

void Shockwave::set( float x, float y, float ir, float irs, float otr, float ors, float dur, float frc, float hlth, bool ag )
{
    pos.set( x, y );
    innerRad = ir;
    innerRadSpeed = irs;
    outerRad = otr;
    outerRadSpeed = ors;
    duration = dur;
    force = frc;
    health = hlth;
    affectGladiators = ag;
    active = false;
    destroyMe = false;
    shockTime = duration;
}

void Shockwave::update()
{
    if( active )
    {
        innerRad += innerRadSpeed * timer.dT();
        outerRad += outerRadSpeed * timer.dT();

        shockTime -= timer.dT();

        if( shockTime <= 0.0 )
        {
            active = false;
            destroyMe = true;
        }
    }
}

bool Shockwave::detect( Vector2D p, float* resultForce, float* resultHealth )
{
    if( active )
    {
        float distSq = distSquared( pos, p );
        if( distSq >= innerRad * innerRad && distSq <= outerRad * outerRad )
        {
            float factor = 1.0 - ( distSq / ( outerRad * outerRad ) );
            *resultHealth = ( health == SHOCK_ESG ) ? SHOCK_ESG : factor * health;

            *resultForce =  factor * force;
            return true;
        }
    }
    return false;
}

void Shockwave::draw()
{
    if( active )
    {
        glDisable( GL_TEXTURE_2D );
        glPushMatrix();
        glColor3f( 1.0, 1.0, 1.0 );
        glTranslatef( pos.c[X], pos.c[Y], 0 );
        glPointSize( 4 );
        for( int i = 0; i < 360; i += 15 )
        {
            glRotatef( 15.0, 0.0, 0.0, i );
            glBegin( GL_POINTS );
            glVertex3f( innerRad, 0.0, 0.5 );
            glVertex3f( outerRad, 0.0, 0.5 );
            glEnd();
        }
        glPopMatrix();
    }
}

// Set the properties of an object type
void ObjectType::set( float im, float iw, float ih, float ir, float imv, float ijv,
                      float imtv, float ia, float ita, float hlth, int dth, bool cd, bool grav, bool shad )
{
    spriteSize.set( iw, ih );
    boundRadius = ir;
    mass = im;
    maxVel = imv;
    jumpVel = ijv;
    maxTurnvel = imtv;
    accel = ia;
    turnAccel = ita;
    maxHealth = hlth;
    deathType = dth;
    collide = cd;
    gravAffect = grav;
    hasShadow = shad;
    model1 = model2 = 0;
    numModels = 0;
    clearSprites();
}

// Set the properties of a particle type
void ParticleType::set( float mrl, float mrs, float mrjs, float rv, float mrrv, float fs, float mrfs, float sf, float ss, float sst,
                        int hb, bool gl, float acc, int anf, float ad, float mrad, int al )
{
    maxRandAddedToLife = mrl;
    maxRandAddedToSpeed = mrs;
    maxRandAddedToJumpSpeed = mrjs;
    minRotVel = rv;
    maxRandAddedToRotVel = mrrv;
    minFadeSpeed = fs;
    maxRandAddedToFadeSpeed = mrfs;
    startFade = sf;
    scaleSpeed = ss;
    scaleStop = sst;
    hitBehavior = hb;
    glow = gl;
    accel = acc;

    animNumFrames = anf;
    animDelay = ad;
    maxRandAddedToAnimDelay = mrad;
    animLoop = al;
}

// Specify a list of texture IDs that makes up the object's sprite collection
void ObjectType::setSprites( int num, ... )
{
    if( num > 0 && num <= MAX_OBJ_SPRITES )
    {
        va_list ap;
        va_start( ap, num );
        for( int i = 0; i < num; i++ )
        {
            int sp = va_arg( ap, int );
            if( sp >= -1 && sp < NUM_TEXTURES )
                sprites[i] = sp;
        }
    }
}

void Object::resetIDCounter()
{
    oID = 0;
}

void Object::setID()
{
    id = oID;
    oID++;
}

// Initializes an object.
void Object::reset()
{
    h = speed = hvel = turnvel = 0;

    pos = origPos;
    heading = origHeading;

    lookAngle = heading;
    jumpSpeed = jumpHeading = jumpStrafe = jumpPause = 0;
    spr1Anim.reset();
    spr2Anim.reset();

    model[0].set( objType[type].model1 );
    model[1].set( ( !augmented ) ? objType[type].model2 : MDL_PLAYER_TORSO2 );

    ev.reset();
    health = objType[type].maxHealth;
    invul = false;
    active = false;
    dying = false;
    destroyMe = false;
    hitGround = false;
    collideWithObjects = false;
    fall = false;
    fallTimer = stunTimer = dyingTimer = 0;
    shockCollisions = 0;
    collided = -1;
    currSurfaceFriction = 0;
    currSurfaceHealth = 0;
    surfaceHealthSound = false;
    dirAlert = NO_ALERT;
    dirAlertType = ALERT_NONE;
    lastDamageAmount = 0.0;

    moveVel.set( 0, 0 );
    forceVel.set( 0, 0 );
}

void Object::changeVel( Vector2D v )
{
    vel += v;
}

void Object::changeForceVel( Vector2D v )
{
    forceVel += ( v / objType[type].mass );
}

void Object::changeForceVelNoMass( Vector2D v )
{
    forceVel += v;
}


void Object::changeHealth( float amt )
{
    if( invul )
        return;

    float prevHealth = health;
    if( objType[type].maxHealth != INF )
        health = ( health + amt < objType[type].maxHealth ) ? health + amt : objType[type].maxHealth;

    if( type == ENT_PLAYER )
    {
        if( health - prevHealth < 0 )
            addBlur( 0.1 * fabs( health - prevHealth ), 1.0, 0.0, 0.0 );
        else if( health - prevHealth > 0 )
            addBlur( 0.05 * fabs( health - prevHealth ), 0.0, 0.0, 1.0 );
    }
}

float Object::changeHealthAmt( float amt )
{
    float prevHealth = health;
    changeHealth( amt );
    return health - prevHealth;
}

// Widely-used drawing commands
void Object::drawStart()
{
    glPushMatrix();

    glTranslatef( pos.c[X], pos.c[Y], 0 );

    // uncomment to draw bounding circles
    /*glPushMatrix();
    glColor3f(1.0, 0.0, 0.0);
    for (int i = 0; i < 360; i+=15)
    {
      glRotatef(15.0, 0.0, 0.0, i);
      glBegin(GL_POINTS);
        glVertex2f(objType[type].boundRadius, 0.0);
      glEnd();
    }
    glPopMatrix();*/

    glRotatef( todeg( lookAngle ), 0.0, 0.0, 1.0 );

    glEnable( GL_TEXTURE_2D );
    glColor4f( 1.0, 1.0, 1.0, 1.0 );
}

void Object::drawShadow()
{
    if( active && objType[type].hasShadow && h >= 0 )
    {
        glPushMatrix();
        // translate ever so slightly above the ground to avoid being
        // screwed over by the depth test
        glTranslatef( pos.c[X], pos.c[Y], 0.01 );
        glRotatef( todeg( lookAngle ), 0.0, 0.0, 1.0 );
        glEnable( GL_TEXTURE_2D );
        glColor4f( 1.0, 1.0, 1.0, 1.0 );
        texQuadSize( TEX_SHADOW, objType[type].boundRadius * 2, objType[type].boundRadius * 2 );
        glDisable( GL_TEXTURE_2D );
        glPopMatrix();
    }
}

void Object::drawEnd()
{
    glDisable( GL_TEXTURE_2D );
    glPopMatrix();
}

// Typical way of drawing an object (shadow, spr1, spr2)
void Object::draw()
{
    if( active )
    {
        drawStart();

        if( !objType[type].numModels )
        {
            glTranslatef( 0.0, 0.0, h + 0.02 );
            if( spr1 != NO_SPRITE )
            {
                texQuad( objType[type].sprites[spr1] );
            }
            glTranslatef( 0.0, 0.0, 0.01 );
            if( spr2 != NO_SPRITE )
            {
                texQuad( objType[type].sprites[spr2] );
            }
        }
        else
        {
            glEnable( GL_TEXTURE_2D );
            glTranslatef( 0.0, 0.0, h + 0.02 );

            for( int i = 0; i < objType[type].numModels; i++ )
                model[i].draw();
        }
        drawEnd();
    }
}

// Returns a pointer to the closest wall intersected, or NULL.
Wall* Object::wallCollisionDetect( int cell, Vector2D* inter )
{
    // Don't do collision detection on static objects
    if( fabs( vel.mag() ) > 0.01 || type == ENT_PLAYER )
    {
        Vector2D intersect;
        int numWalls = currLevel.grid[cell].wall.size();
        Wall* currWall;

        Wall* closestHitWall = NULL;
        Vector2D closestHitIntersect;
        float closestHitDistance = 1e37; // big numbah!

        for( int i = 0; i < numWalls + 4 * currLevel.grid[cell].door.size(); i++ )
        {
            // A pointer allows us to combine collision detection with
            // walls and doors into one loop.
            if( i < numWalls )
                currWall = &currLevel.wall[currLevel.grid[cell].wall[i]];
            else
                currWall = &currLevel.door[currLevel.grid[cell].door[( i - numWalls ) / 4]].sides[( i - numWalls ) % 4];

            if( ( currWall->CollFlags() & COLLIDE_OBJECT && !IS_BULLET( type ) ) ||
                    ( currWall->CollFlags() & COLLIDE_BULLET && IS_BULLET( type ) ) )
            {
                // Open doors
                if( i >= numWalls )
                    currLevel.door[currLevel.grid[cell].door[( i - numWalls ) / 4]].detect( pos, ( type == ENT_PLAYER ) ? keys : -1 );

                Vector2D offset = vel;
                offset.normalize();
                offset *= objType[type].boundRadius + ( vel.mag() * timer.dT() );

                Vector2D back = vel;
                back.normalize();
                back *= objType[type].boundRadius;

                if( currWall->intersection( pos - back, pos + offset, &intersect ) )
                {
                    if( ( intersect - pos ).mag() < closestHitDistance )
                    {
                        closestHitDistance = ( intersect - pos ).mag();
                        closestHitWall = currWall;
                        closestHitIntersect = Vector2D( intersect.c[X], intersect.c[Y] );
                    }
                }
                else
                    currWall->renewTrigger();
            }
        }
        if( closestHitWall )
        {
            if( inter )
                inter->set( closestHitIntersect.c[X], closestHitIntersect.c[Y] );

            return closestHitWall;
        }
    }
    return NULL;
}

// Shockwave collision detection
void Object::shockwaveCollisionDetect()
{
    float swf, swh, ang;
    for( int i = 0; !IS_POWERUP( type ) && i < liveShockwaves; i++ )
    {
        if( shockwaves[i].isActive() )
        {
            if( shockwaves[i].detect( pos, &swf, &swh ) && !( shockCollisions & ( 1 << ( i % 64 ) ) ) )
            {
                if( swh == SHOCK_ESG )
                {
                    if( IS_ENEMY( type ) && !Stunned() )
                    {
                        stun( 15 );
                        LAUNCH_MULTI_PARTICLES( 25, PART_PARTICLE_SPARK,
                                                pos.c[X], pos.c[Y], 0.5, torad( rand() % 360 ), 1.0, 0 );

                        LAUNCH_MULTI_PARTICLES( 5, PART_SMOKE,
                                                pos.c[X], pos.c[Y], 0.5, torad( rand() % 360 ), 1.0, 0 );

                        playSound( SND_ROBOT_PARALYZE, 5 );
                    }
                }
                else
                {
                    if( type == ENT_GLADIATOR )
                    {
                        if( shockwaves[i].AffectGladiators() )
                            damage( swh, DAMAGE_SHOCKWAVE );
                    }
                    else
                        damage( swh, DAMAGE_SHOCKWAVE );
                }

                ang = atan2( pos.c[Y] - shockwaves[i].PosY(), pos.c[X] - shockwaves[X].PosX() );
                setDirectionAlert( ang + PI, ALERT_SHOCKWAVE );

                if( !fixed )
                    changeForceVel( Vector2D( cos( ang )*swf, sin( ang )*swf ) );

                // set bit flag
                shockCollisions |= 1 << ( i % 64 );

                if( type == ENT_PLAYER && health <= 0 )
                    playSound( SND_EXPLODE, 2 );
            }
        }
        else
        {
            if( shockCollisions & ( 1 << ( i % 64 ) ) )
                shockCollisions &= ~( 1 << ( i % 64 ) );
        }
    }

    if( liveShockwaves == 0 )
        shockCollisions = 0;
}

// Collision with laaaaaaaaaaaaaaasers.
void Object::laserCollisionDetect()
{
    for( int i = 0; !IS_POWERUP( type ) && i < liveLasers; i++ )
    {
        if( ( type == ENT_PLAYER && lasers[i].Live() > 0 ) ||
                ( type != ENT_PLAYER && lasers[i].Live() == 1 ) )
        {
            Vector2D intersection;
            for( int j = 0; j < lasers[i].beams.size(); j++ )
            {
                if( circleSegmentIntersection( pos, objType[type].boundRadius,
                                               lasers[i].beams[j].p1, lasers[i].beams[j].p2, &intersection ) )
                {
                    if( type != ENT_PLAYER )
                        damage( -lasType[lasers[i].Type()].damage, DAMAGE_BULLET );
                    else
                        damage( -lasType[lasers[i].Type()].damage * 0.3, DAMAGE_BULLET );

                    if( lasers[i].Type() == L_STANDARD )
                    {
                        LAUNCH_MULTI_PARTICLES( 15, PART_LASER_SPARK1,
                                                intersection.c[X], intersection.c[Y], h, torad( rand() % 360 ), 1.0, 0 );
                    }
                    else if( lasers[i].Type() == L_CHARGED )
                    {
                        LAUNCH_MULTI_PARTICLES( 30, PART_LASER_SPARK2,
                                                intersection.c[X], intersection.c[Y], h, torad( rand() % 360 ), 1.0, 0 );
                    }
                }
            }
        }
    }
}

// Collision with floors; returns true if on a floor.
bool Object::floorCollisionDetect( int cell, float* fric )
{
    if( h <= 0.2 )
    {
        int s;
        bool onFloor = false;
        for( int i = 0; i < currLevel.grid[cell].floor.size(); i++ )
        {
            s = currLevel.grid[cell].floor[i];
            if( currLevel.floor[s].CollFlags() & COLLIDE_OBJECT )
            {
                if( currLevel.floor[s].pointInBB( pos ) && currLevel.floor[s].pointInPolygon( pos ) )
                {
                    if( fric )
                        *fric = currLevel.floor[s].Friction();

                    currLevel.floor[s].handleTrigger( false, type == ENT_PLAYER );
                    onFloor = true;
                }
                else
                    currLevel.floor[s].renewTrigger();
            }
        }
        return onFloor;
    }
    return true;
}

// Collision with surfaces
void Object::surfaceCollisionDetect( int cell, float* fric, float* hlth )
{
    if( h <= 0.2 )
    {
        int s;
        for( int i = 0; i < currLevel.grid[cell].surf.size(); i++ )
        {
            s = currLevel.grid[cell].surf[i];
            if( currLevel.surf[s].pointInBB( pos ) && currLevel.surf[s].pointInPolygon( pos ) )
            {
                if( fric ) *fric = currLevel.surf[s].Friction();
                if( hlth ) *hlth = currLevel.surf[s].Health();

                if( vel.mag() > 1.0 || hvel != 0 )
                    currLevel.surf[s].spewParticles( pos );

                currLevel.surf[s].handleTrigger( false, type == ENT_PLAYER );
            }
            else
                currLevel.surf[s].renewTrigger();
        }
    }
}


void Object::objectCollisionDetect( int cell )
{
    if( collideWithObjects && active )
    {
        Object* obj;

        for( int j = 0; j < currLevelObjs.size(); j++ )
        {
            obj = currLevelObjs[j];

            if( obj->isActive() && obj->CollideWithObjects() )
            {
                if( id != obj->ID() &&
                        collided != obj->ID() &&
                        obj->Collided() != id )
                {
                    // Extend object's reach for powerups
                    if( IS_POWERUP( obj->Type() ) )
                    {
                        Vector2D offset = vel;
                        offset.normalize();
                        offset *= objType[type].boundRadius + ( vel.mag() * timer.dT() );
                        if( distSquared( pos + offset, obj->Pos() ) <=
                                ( objType[type].boundRadius +
                                  objType[obj->Type()].boundRadius ) *
                                ( objType[type].boundRadius +
                                  objType[obj->Type()].boundRadius ) )
                        {
                            if( type == ENT_PLAYER )
                            {
                                if( h == 0.0 )
                                    givePowerup( j );
                            }
                        }
                    }

                    if( distSquared( pos, obj->Pos() ) <=
                            ( objType[type].boundRadius +
                              objType[obj->Type()].boundRadius ) *
                            ( objType[type].boundRadius +
                              objType[obj->Type()].boundRadius ) )
                    {
                        if( ( IS_POWERUP( obj->Type() ) || IS_POWERUP( type ) ) && ( h != 0.0 || obj->Height() != 0.0 ) )
                            continue;
                        else
                        {
                            float m1 =  objType[type].mass;
                            float m2 =  objType[obj->Type()].mass;

                            float v1i = vel.mag();
                            float v2i = obj->Vel().mag();

                            float v1f = ( m1 * v1i - m2 * ( v1i - 2 * v2i ) ) / ( m1 + m2 );
                            float v2f = ( m1 * ( 2 * v1i - v2i ) + m2 * v2i ) / ( m1 + m2 );

                            float a1 = atan2( pos.c[Y] - obj->PosY(),
                                              pos.c[X] - obj->PosX() );
                            float a2 = atan2( obj->PosY() - pos.c[Y],
                                              obj->PosX() - pos.c[X] );


                            setCollided( obj->ID() );
                            obj->setCollided( id );

                            if( fabs( v1i ) > fabs( v2i ) )
                                pos += Vector2D( cos( a1 ), sin( a1 ) ) * ( ( vel * timer.dT() ).mag() );
                            else
                                obj->addPos( Vector2D( cos( a2 ), sin( a2 ) ) * ( ( obj->Vel()*timer.dT() ).mag() ) );

                            if( !fixed )
                                changeForceVelNoMass( Vector2D( cos( a1 ), sin( a1 ) )*fabs( v1f ) * 0.1 );

                            if( !obj->Fixed() )
                                obj->changeForceVelNoMass( Vector2D( cos( a2 ), sin( a2 ) )*fabs( v2f ) * 0.1 );

                            setDirectionAlert( a2, ALERT_TOUCH );
                            obj->setDirectionAlert( a1, ALERT_TOUCH );
                        }
                    }
                }
            }
        }
    }
}

// Handles collision detection with walls and surfaces.
void Object::levelCollisionDetection()
{
    if( currLevel.inGrid( pos ) )
    {
        int cell = currLevel.cellNumber( pos );

        shockwaveCollisionDetect();
        laserCollisionDetect();
        if( forceVel.mag() > TERMINAL_VELOCITY )
        {
            forceVel.normalize();
            forceVel *= TERMINAL_VELOCITY;
        }
        vel = moveVel + forceVel;

        objectCollisionDetect( cell );
        if( forceVel.mag() > TERMINAL_VELOCITY )
        {
            forceVel.normalize();
            forceVel *= TERMINAL_VELOCITY;
        }
        vel = moveVel + forceVel;

        Vector2D impactPoint;
        Wall* closestHitWall = wallCollisionDetect( cell, &impactPoint );

        // React to a collision, if there was one
        if( closestHitWall )
        {
            // Push the object back by his velocity
            // in the direction of the wall's normal
            if( vel.mag() < WALL_COLLISION_THRESHOLD )
                pos += closestHitWall->normal * ( ( vel * timer.dT() ).mag() + 0.1 );
            else
            {
                pos = impactPoint;
                pos += closestHitWall->normal * ( objType[type].boundRadius + 0.1 );
                moveVel.set( 0, 0 );
                forceVel.set( 0, 0 );
            }

            if( vel.mag() >= WALL_DAMAGE_THRESHOLD )
                // Lose health based on your impact velocity.
                damage( -vel.mag() / IMPACT_DAMAGE_FACTOR, false );

            if( closestHitWall->Health() < 0 )
                damage( closestHitWall->Health(), false );
            else if( closestHitWall->Health() > 0 )
                changeHealth( closestHitWall->Health() );

            if( closestHitWall->Force() )
                changeForceVel( closestHitWall->normal * closestHitWall->Force() );

            if( closestHitWall->Health() < 0 )
            {
                if( type == ENT_PLAYER )
                {
                    LAUNCH_MULTI_PARTICLES( 10, PART_PARTICLE_BLOOD,
                                            pos.c[X], pos.c[Y], 0, torad( rand() % 360 ), 1.0, 0 );
                }
            }
            closestHitWall->handleTrigger( false, type == ENT_PLAYER );
        }

        currSurfaceFriction = DEFAULT_SURFACE_FRICTION;
        currSurfaceHealth = 0;

        if( !floorCollisionDetect( cell, &currSurfaceFriction ) && !fall )
        {
            fall = true;
            currSurfaceFriction = 0;
            if( type == ENT_PLAYER )
            {
                setMessage( "Oh crap" );
                playSound( SND_FALL, 2 );
            }
        }

        surfaceCollisionDetect( cell, &currSurfaceFriction, &currSurfaceHealth );
    }
}

void Object::turn( float amt )
{
    turnvel = amt;
}

void Object::setAnimation( int val, bool anim )
{
    for( int i = 0; i < objType[type].numModels; i++ )
        model[i].setAnimation( val, anim );
}

// Basic framework for updating an object's position, velocity, heading.
// States, jumping, sprite representation is per-subtype.
void Object::update()
{
    if( active && timer.dT() > 0 )
    {
        if( !currLevel.inGrid( pos ) )
        {
            active = false;
            return;
        }

        for( int i = 0; i < objType[type].numModels; i++ )
            model[i].update();

        if( objType[type].maxHealth != INF )
        {
            if( type == ENT_PLAYER && config.cheat_invul && currSurfaceHealth < 0 ) {}
            else
                changeHealth( currSurfaceHealth * timer.dT() );
        }

        dyingCheck();

        if( type == ENT_PLAYER )
        {
            if( currSurfaceHealth > 0 )
            {
                if( surfaceHealthSound )
                {
                    playSound( SND_HEAL, 2 );
                    surfaceHealthSound = false;
                }
            }
            else if( currSurfaceHealth < 0 && !config.cheat_invul )
            {
                if( surfaceHealthSound )
                {
                    playSound( SND_HURTSURFACE, 2 );
                    surfaceHealthSound = false;
                }
            }
            else
                surfaceHealthSound = true;
        }

        // Kill the object's ability to do anything if we're stunned
        if( stunTimer > 0 )
        {
            ev.reset();
            stunTimer -= timer.dT();
        }
        else
            stunTimer = 0;

        turnvel += objType[type].turnAccel * -ev.getLR();

        //float friction = turnvel * -currSurfaceFriction;
        turnvel += ( turnvel * -currSurfaceFriction ); //friction;

        if( turnvel > objType[type].maxTurnvel * fabs( ev.getLR() ) )
            turnvel = objType[type].maxTurnvel * fabs( ev.getLR() );
        if( turnvel < -objType[type].maxTurnvel * fabs( ev.getLR() ) )
            turnvel = -objType[type].maxTurnvel * fabs( ev.getLR() );
        lookAngle += turnvel * timer.dT();

        if( h == 0 )
        {
            heading = lookAngle;

            if( fall == true )
                h -= ACCEL_GRAVITY * timer.dT();
        }

        float maxVel = objType[type].maxVel;
        if( augmented ) maxVel *= AUG_SPEED_INCREASE;

        if( moveVel.mag() <= maxVel )
        {
            if( h == 0 && jumpPause == 0.0 )
            {
                moveVel.c[X] += cos( heading ) * objType[type].accel * ev.getFB();
                moveVel.c[Y] += sin( heading ) * objType[type].accel * ev.getFB();

                moveVel.c[X] += cos( heading - PIOVER2 ) * objType[type].accel * ev.getStrafe();
                moveVel.c[Y] += sin( heading - PIOVER2 ) * objType[type].accel * ev.getStrafe();

            }
            else
            {
                moveVel.c[X] += cos( jumpHeading ) * jumpSpeed;
                moveVel.c[Y] += sin( jumpHeading ) * jumpSpeed;
                moveVel.c[X] += cos( jumpHeading - PIOVER2 ) * jumpStrafe;
                moveVel.c[Y] += sin( jumpHeading - PIOVER2 ) * jumpStrafe;
            }
        }

        if( moveVel.mag() > maxVel )
        {
            moveVel.normalize();
            moveVel *= maxVel;
        }

        if( jumpPause > 0 )
            moveVel.set( 0, 0 );

        if( h == 0 )
            moveVel += moveVel * -currSurfaceFriction;
        else
            moveVel += moveVel * -0.1;

        if( moveVel.mag() > TERMINAL_VELOCITY )
        {
            moveVel.normalize();
            moveVel *= TERMINAL_VELOCITY;
        }

        forceVel += ( forceVel * -currSurfaceFriction );
        if( forceVel.mag() < 0.01 )
            forceVel.set( 0, 0 );
        if( forceVel.mag() > TERMINAL_VELOCITY )
        {
            forceVel.normalize();
            forceVel *= TERMINAL_VELOCITY;
        }

        vel = moveVel + forceVel;
        // Collision detection
        levelCollisionDetection();

        vel = moveVel + forceVel;
        if( vel.mag() > TERMINAL_VELOCITY )
        {
            vel.normalize();
            vel *= TERMINAL_VELOCITY;
        }

        pos += ( vel ) * timer.dT();

        // Object is affected by gravity if it is not on the ground
        if( objType[type].gravAffect )
        {
            if( h != 0 )
                hvel -= ACCEL_GRAVITY * timer.dT();
            if( h < 0 && !fall )
                hitGround = true; // response is per-object-type
        }

        h += hvel * timer.dT();

        if( jumpPause > 0 )
            jumpPause -= timer.dT();
        else if( jumpPause < 0 )
            jumpPause = 0;

        if( jumpPause == 0 )
        {
            spr1Anim.update();
            spr2Anim.update();
        }

        // Respond to triggers
        if( triggers[respondTo2].hit )
        {
            kill( true );
        }

        if( type == ENT_PLAYER )
        {
            if( health < 25 )
                setBlur( 1.0, 1.0, 0.0, 0.0 );
        }

        if( health != INF )
        {
            if( health < 0 ) health = 0;
            if( health > objType[type].maxHealth ) health = objType[type].maxHealth;
        }

        if( dying )
        {
            dyingTimer -= timer.dT();
            if( dyingTimer <= 0 )
            {
                dying = false;
                kill( true );
            }
        }
    }

    if( fall )
    {
        fallTimer += timer.dT();
        if( fallTimer >= FALL_KILL_DELAY )
        {
            kill( false );
            fallTimer = 0;
            fall = false;
            if( type == ENT_PLAYER )
                playSound( SND_EXPLODE, 2 );
        }
    }
}

// by = true for explosions, false for bullets/etc.
void Object::damage( float amt, bool by )
{
    if( type == ENT_PLAYER )
    {
        if( amt < 0 && !config.cheat_invul )
        {
            changeHealth( amt * ( ( augmented ) ? AUG_DAMAGE_SUPPRESSION : 1.0 ) );
            P.hits++;
        }
    }
    else
        changeHealth( amt );

    if( !( Stunned() || Dying() ) )
    {
        if( type != ENT_PLAYER )
            setAnimation( MA_FLINCH, ANIM_TRANSIENT );

        if( type == ENT_PLAYER )
        {
            if( !config.cheat_invul ) playSound( SND_HURT1 + rand() % 2, 2 );
        }
        else
            playSound( SND_BULLET_HIT_HARD, 1 );

        damagedBy = by;
        lastDamageAmount = amt;
    }
}

void Object::stun( float amt )
{
    stunTimer += amt;

    if( type != ENT_PLAYER && !Dying() )
        setAnimation( MA_IDLE, ANIM_BASE );
}

void Object::dyingCheck()
{
    if( !dying && health <= 0 && health != INF )
    {
        switch( type )
        {
        case ENT_PLAYER:
            if( damagedBy == DAMAGE_BULLET )
            {
                model[0].setAnimation( MA_PL_DIESOFT, ANIM_TRANSIENT );
                model[0].setAnimation( MA_PL_DEADSOFT, ANIM_BASE );
                model[1].setAnimation( MA_PT_DIESOFT, ANIM_TRANSIENT );
                model[1].setAnimation( MA_PT_DEADSOFT, ANIM_BASE );
            }
            else
            {
                model[0].setAnimation( MA_PL_DIEHARD, ANIM_TRANSIENT );
                model[0].setAnimation( MA_PL_DEADHARD, ANIM_BASE );
                model[1].setAnimation( MA_PT_DIEHARD, ANIM_TRANSIENT );
                model[1].setAnimation( MA_PT_DEADHARD, ANIM_BASE );
            }
            die( 2 );
            stun( 99999 );
            playSound( SND_EXPLODE, 2 );
            break;
        case ENT_UNARMED_GUARD:
        case ENT_MIB:
        case ENT_ARMED_DRONE:
        case ENT_HUNTER:
        case ENT_GLADIATOR:
            if( lastDamageAmount > GIB_THRESHOLD )
            {
                setAnimation( MA_DIE, ANIM_TRANSIENT );
                setAnimation( MA_DEAD, ANIM_BASE );
                die( 1 );
                stun( 10 );
            }
            else
                die( 0 );
            break;
        default:
            die( 0 );
        }
    }
}

void Object::kill( bool death )
{
    health = 0;
    active = false;
    selectWeapon( 0 );

    if( type == ENT_PLAYER )
        setBlur( 1.0, 1.0, 0.0, 0.0 );
    else if( IS_ENEMY( type ) )
        P.kills++;

    if( death )
        doDeath();
}

void Object::doDeath()
{
    float distToPlayer, randAng, randRad, px, py, ph, phead;
    switch( objType[type].deathType )
    {
    case DEATH_NONE:
        break;
    case DEATH_PLAYER:
        break;
    case DEATH_WOOD:
        LAUNCH_MULTI_PARTICLES( 20, PART_PARTICLE_WOOD,
                                pos.c[X] + frand() * 3.0 - 1.5, pos.c[Y] + frand() * 3.0 - 1.5, 0, torad( rand() % 360 ), 1.0, 0 );
        break;
    case DEATH_SMALL_EXPLOSION:
        LAUNCH_MULTI_PARTICLES( 4, PART_EXPLOSION,
                                pos.c[X] + frand() * 7.0 - 3.5, pos.c[Y] + frand() * 7.0 - 3.5, 0.5, torad( rand() % 360 ), 1.0, rand() % 4 );

        LAUNCH_MULTI_PARTICLES( 25, PART_EXPLOSIONPART,
                                pos.c[X], pos.c[Y], 0.5, torad( rand() % 360 ), 1.0, 0 );

        addShockwave( pos.c[X], pos.c[Y], 0, 0, 9, 0, 0.5, 48000, -50 );

        distToPlayer = dist( pos, PLAYER_OBJECT->Pos() );
        if( distToPlayer < 20 )
            setScreenQuake( lerp( 5, 0, distToPlayer / 20 ) );

        playSound( SND_EXPLOSION, 5 );
        break;


    case DEATH_GRENADE_EXPLOSION:
        LAUNCH_MULTI_PARTICLES( 4, PART_EXPLOSION,
                                pos.c[X] + frand() * 7.0 - 3.5, pos.c[Y] + frand() * 7.0 - 3.5, 0.5, torad( rand() % 360 ), 1.0, rand() % 4 );

        addShockwave( pos.c[X], pos.c[Y], 0, 0, 16, 0, 0.5, 40000, -70 );

        distToPlayer = dist( pos, PLAYER_OBJECT->Pos() );
        if( distToPlayer < 20 )
            setScreenQuake( lerp( 5, 0, distToPlayer / 20 ) );

        playSound( SND_GRENADE_EXPLOSION, 5 );

        for( int i = 0; i < 25; i++ )
            launchBullet( -1, BLT_SHRAPNEL, pos.c[X], pos.c[Y], 0.5, torad( rand() % 360 ) );

        break;

    case DEATH_EXPLOSION:
        LAUNCH_MULTI_PARTICLES( 4, PART_EXPLOSION,
                                pos.c[X] + frand() * 7.0 - 3.5, pos.c[Y] + frand() * 7.0 - 3.5, 0.5, torad( rand() % 360 ), 1.0, rand() % 4 );

        LAUNCH_MULTI_PARTICLES( 25, PART_EXPLOSIONPART,
                                pos.c[X], pos.c[Y], 0.5, torad( rand() % 360 ), 1.0, 0 );

        LAUNCH_MULTI_PARTICLES( 5, PART_SMOKE,
                                pos.c[X], pos.c[Y], 0.5, torad( rand() % 360 ), 1.0, 0 );

        addShockwave( pos.c[X], pos.c[Y], 0, 0, 10, 0, 0.5, 48000, -65 );

        distToPlayer = dist( pos, PLAYER_OBJECT->Pos() );
        if( distToPlayer < 20 )
            setScreenQuake( lerp( 5, 0, distToPlayer / 20 ) );

        playSound( SND_EXPLOSION, 5 );
        break;


    case DEATH_HEPA_EXPLOSION:
        LAUNCH_MULTI_PARTICLES( 4, PART_HEPA_EXPLOSION,
                                pos.c[X] + frand() * 14.0 - 7, pos.c[Y] + frand() * 14.0 - 7, 0.5, torad( rand() % 360 ), 1.0, 0 );
        LAUNCH_MULTI_PARTICLES( 50, PART_HEPA_SPARK,
                                pos.c[X], pos.c[Y], 0.5, torad( rand() % 360 ), 1.0, 0 );

        addShockwave( pos.c[X], pos.c[Y], 0, 0, 20, 0, 0.5, 0, -200 );

        distToPlayer = dist( pos, PLAYER_OBJECT->Pos() );
        if( distToPlayer < 80 )
            setScreenQuake( lerp( 5, 0, distToPlayer / 80 ) );

        playSound( SND_HEPA_EXPLOSION, 5 );
        break;
    case DEATH_POWERUP:
        for( int i = 0; i < 15; i++ )
        {
            randAng = frand() * PI * 2;
            randRad = frand() * 0.25;
            launchParticle( PART_PARTICLE_GLOW,
                            pos.c[X] + ( cos( randAng )*randRad ), pos.c[Y] + ( sin( randAng )*randRad ), 0.5, randAng, 1.0, 0 );
        }

        break;
    case DEATH_ROBOT:
        LAUNCH_MULTI_PARTICLES( 25, PART_PARTICLE_METAL,
                                pos.c[X], pos.c[Y], 0, torad( rand() % 360 ), 1.0, 0 );
        LAUNCH_MULTI_PARTICLES( 25, PART_EXPLOSIONPART,
                                pos.c[X], pos.c[Y], 0.5, torad( rand() % 360 ), 1.0, 0 );

        playSound( SND_ROBOT_DEATH, 5 );

        break;
    case DEATH_GLADIATOR:
        LAUNCH_MULTI_PARTICLES( 4, PART_HEPA_EXPLOSION,
                                pos.c[X] + frand() * 14.0 - 7, pos.c[Y] + frand() * 14.0 - 7, 0.5, torad( rand() % 360 ), 1.0, 0 );
        LAUNCH_MULTI_PARTICLES( 50, PART_HEPA_SPARK,
                                pos.c[X], pos.c[Y], 0.5, torad( rand() % 360 ), 1.0, 0 );

        addShockwave( pos.c[X], pos.c[Y], 0, 0, 20, 0, 0.5, 0, -200, false );

        distToPlayer = dist( pos, PLAYER_OBJECT->Pos() );
        if( distToPlayer < 80 )
            setScreenQuake( lerp( 5, 0, distToPlayer / 80 ) );

        playSound( SND_HEPA_EXPLOSION, 5 );
        break;
    case DEATH_ESG:
        px = PLAYER_OBJECT->PosX();
        py = PLAYER_OBJECT->PosY();
        ph = PLAYER_OBJECT->Height();
        phead = PLAYER_OBJECT->LookAngle();

        launchParticle( PART_ESG_SHOCKWAVE,
                        px + ( wpnType[WPN_ESG].spriteSize.c[X] / 3.5 * cos( phead ) ),
                        py + ( wpnType[WPN_ESG].spriteSize.c[X] / 3.5 * sin( phead ) ),
                        ph, phead, 1.0, 0 );
        setBlur( 2.0, 1.0, 0.5, 1.0 );
        setScreenQuake( 7.0 );
        addShockwave( px + ( wpnType[WPN_ESG].spriteSize.c[X] / 3.5 * cos( phead ) ),
                      py + ( wpnType[WPN_ESG].spriteSize.c[X] / 3.5 * sin( phead ) ),
                      0, 0, 0, 100.0, 0.5, 0, SHOCK_ESG );
        break;
    }

    handleTrigger( false, type == ENT_PLAYER );
}

void Entity::reset()
{
    Object::reset();
    jumping = false;
    currWeapon = 0;
    keys = 0;
    weapons = 1;
    walkSound = false;
    grenadeThrowTimer = 0;
    throwGrenade = false;

    switch( type )
    {
    case ENT_PLAYER:

        if( !augmented )
        {
            wpn[0].set( WPN_NONE, MAX_AMMO * ( int )config.cheat_weapons, 0 );
            wpnPos[0].set( 0, 0 );
            wpn[1].set( WPN_RIFLE, MAX_AMMO * ( int )config.cheat_weapons, 0 );
            wpnPos[1].set( -0.25, 1.1 );
            wpn[2].set( WPN_SHOTGUN, MAX_AMMO * ( int )config.cheat_weapons, 0 );
            wpnPos[2].set( -0.20, 1.1 );
            wpn[3].set( WPN_HEPA, MAX_AMMO * ( int )config.cheat_weapons, 0 );
            wpnPos[3].set( -0.35, 0.9 );
            wpn[4].set( WPN_GRENADE, MAX_AMMO * ( int )config.cheat_weapons, 0 );
            wpnPos[4].set( -0.45, 0.9 );
            wpn[5].set( WPN_LASER, MAX_AMMO * ( int )config.cheat_weapons, 0 );
            wpnPos[5].set( -0.30, 0.9 );
            wpn[6].set( WPN_ESG, MAX_AMMO * ( int )config.cheat_weapons, 0 );
            wpnPos[6].set( -0.30, 0.9 );
        }
        else
            wpn[0].set( WPN_BLASTER, MAX_AMMO, 0 );
        wpnPos[0].set( -0.30, 0.9 );

        break;
    case ENT_TURRET1:
    case ENT_SLAVE_TURRET:
        wpn[0].set( WPN_TURRET_GUN, MAX_AMMO, -1 );
        wpnPos[0].set( 0, 2.2 );
        break;
    case ENT_ARMED_DRONE:
        wpn[0].set( WPN_RIFLE, MAX_AMMO, -1 );
        wpnPos[0].set( -1.6, 1.5 );
        break;
    case ENT_HUNTER:
        wpn[0].set( WPN_RIFLE, MAX_AMMO, -1 );
        wpnPos[0].set( -2.5, 0.4 );
        wpn[1].set( WPN_RIFLE, MAX_AMMO, -1 );
        wpnPos[1].set( 2.5, 0.4 );
        break;
    case ENT_GLADIATOR:
        wpn[0].set( WPN_SHOTGUN, MAX_AMMO, -1 );
        wpnPos[0].set( 1.6, 1.9 );
        break;

    }

    if( type == ENT_PLAYER )
    {
        weapons = ( !config.cheat_weapons || augmented ) ? 1 : ( 1 << NUM_PLAYER_WEAPONS ) - 1;
        keys = ( !config.cheat_keys ) ? 0 : ( 1 << NUM_KEYS ) - 1;
    }
}

// Draws an Entity.
void Entity::draw()
{
    if( active )
    {
        if( type == ENT_PLAYER )
        {
            drawStart();
            glEnable( GL_TEXTURE_2D );
            glTranslatef( 0.0, 0.0, h + 0.02 );

            for( int i = 0; i < objType[type].numModels; i++ )
                model[i].draw();

            if( !Dying() )
            {
                glTranslatef( wpnPos[currWeapon].c[Y], -wpnPos[currWeapon].c[X], 4.6 );
                wpn[currWeapon].draw();
            }

            drawEnd();
        }
        else
            Object::draw();
    }
}

void Entity::setPlayerAnimations()
{
    if( type == ENT_PLAYER && !dying )
    {
        // Torso animation
        if( ev.getFB() != 0 || ev.getStrafe() != 0 )
        {
            switch( currWeapon )
            {
            case WPN_NONE:
            case WPN_GRENADE:
                model[1].setAnimation( MA_PT_RUN, ANIM_BASE );
                break;
            case WPN_RIFLE:
            case WPN_SHOTGUN:
            case WPN_LASER:
                model[1].setAnimation( MA_PT_RIFLERUN, ANIM_BASE );
                break;
            case WPN_HEPA:
            case WPN_ESG:
                model[1].setAnimation( MA_PT_HEPARUN, ANIM_BASE );
                break;
            }
        }
        else
        {
            switch( currWeapon )
            {
            case WPN_NONE:
            case WPN_GRENADE:
                model[1].setAnimation( MA_PT_STAND, ANIM_BASE );
                break;
            case WPN_RIFLE:
            case WPN_SHOTGUN:
            case WPN_LASER:
                model[1].setAnimation( MA_PT_RIFLE, ANIM_BASE );
                break;
            case WPN_HEPA:
            case WPN_ESG:
                model[1].setAnimation( MA_PT_HEPA, ANIM_BASE );
                break;
            }
        }

        // Leg animation
        if( ev.getFB() > 0 )
            model[0].setAnimation( MA_PL_FWD, ANIM_BASE );
        else if( ev.getFB() < 0 )
            model[0].setAnimation( MA_PL_BACK, ANIM_BASE );
        else if( ev.getStrafe() > 0 )
            model[0].setAnimation( MA_PL_FWDRIGHT, ANIM_BASE );
        else if( ev.getStrafe() < 0 )
            model[0].setAnimation( MA_PL_FWDLEFT, ANIM_BASE );
        else
            model[0].setAnimation( MA_PL_STAND, ANIM_BASE );
    }
}


// Updates an Entity: controls sprite appearance based on sprite,
// and handles jump event.
void Entity::update()
{
    spr1 = 0;
    spr2 = NO_SPRITE;
    Object::update();

    if( type == ENT_PLAYER )
    {
        wpn[currWeapon].update();

        if( !augmented )
        {
            if( currWeapon != WPN_GRENADE ) wpn[WPN_GRENADE].update();
        }

        if( currWeapon > 0 )
        {
            spr2 = 4;
            spr2Anim.set( 1, 0, 0 );
        }
    }

    if( type == ENT_TURRET1 || type == ENT_SLAVE_TURRET || type == ENT_ARMED_DRONE || type == ENT_HUNTER || type == ENT_GLADIATOR )
        wpn[0].update();

    if( type == ENT_HUNTER )
        wpn[1].update();

    if( active && !fall )
    {
        if( type == ENT_PLAYER )
            setPlayerAnimations();
        else
        {
            if( ev.getFB() > 0 || ev.getStrafe() != 0 )
            {
                if( objType[type].numModels )
                {
                    if( !( Stunned() || Dying() ) ) setAnimation( ( type != ENT_MIB ) ? MA_WALK : MA_RUN, ANIM_BASE );
                }
                else
                {
                    spr1Anim.setDirection( ANIM_FWD );
                    spr2Anim.setDirection( ANIM_FWD );
                }
            }
            else if( ev.getFB() < 0 || ev.getStrafe() != 0 )
            {
                spr1Anim.setDirection( ANIM_REV );
                spr2Anim.setDirection( ANIM_REV );
            }
            else
            {
                if( objType[type].numModels )
                {
                    if( !( Stunned() || Dying() ) ) setAnimation( MA_STAND, ANIM_BASE );
                }
                else
                {
                    spr1Anim.freezeAtFrame( 0 );
                    spr2Anim.freezeAtFrame( 0 );
                }
            }
        }

        if( !ev.getJump() && jumpPause == 0 )
            jumpReady = true;

        if( h == 0 && !jumping && jumpReady && ev.getJump() && jumpPause == 0 )
        {
            hvel = objType[type].jumpVel;
            jumping = true;
            jumpReady = false;
            jumpSpeed = vel.mag() * 0.75 * ev.getFB();
            jumpStrafe = vel.mag() * 0.75 * ev.getStrafe();
            jumpHeading = heading;
            playSound( SND_JUMP, 4 );

            if( type == ENT_PLAYER )
            {
                if( currWeapon == WPN_NONE || currWeapon == WPN_GRENADE )
                    model[1].setAnimation( MA_PT_RUN, ANIM_TRANSIENT );

                model[0].setAnimation( MA_PL_JUMP, ANIM_TRANSIENT );
            }
        }
        if( hitGround )
        {
            jumpPause = JUMP_PAUSE_TIME;
            hvel = 0;
            h = 0;
            jumping = false;
            hitGround = false;
            playSound( SND_LAND, 4 );
        }

        if( ev.getSwitchWpn() && !throwGrenade )
        {
            if( switchReady )
            {
                selectNextWeapon();
                switchReady = false;
            }
        }
        else
            switchReady = true;

        if( type == ENT_PLAYER )
        {
            if( ev.getFire() && !Dying() )
            {
                if( currWeapon == WPN_GRENADE )
                {
                    if( wpn[currWeapon].Ready() && !throwGrenade )
                    {
                        throwGrenade = true;
                        grenadeThrowTimer = 0.5;
                        model[1].setAnimation( MA_PT_GRENADE, ANIM_TRANSIENT );
                    }
                }
                else
                {
                    if( wpn[currWeapon].fire( pos.c[X] + ( wpnPos[currWeapon].c[X]*cos( lookAngle - PIOVER2 ) ) +
                                              ( wpnPos[currWeapon].c[Y]*cos( lookAngle ) ),
                                              pos.c[Y] + ( wpnPos[currWeapon].c[X]*sin( lookAngle - PIOVER2 ) ) +
                                              ( wpnPos[currWeapon].c[Y]*sin( lookAngle ) ), lookAngle, 4.5 ) )
                    {
                        switch( currWeapon )
                        {
                        case WPN_RIFLE:
                        case WPN_SHOTGUN:
                            model[1].setAnimation( MA_PT_RIFLESHOOT, ANIM_TRANSIENT );
                            break;
                        case WPN_HEPA:
                            model[1].setAnimation( MA_PT_HEPASHOOT, ANIM_TRANSIENT );
                            break;
                        case WPN_NONE:
                            if( augmented )
                                model[1].setAnimation( MA_PT_PALMSHOOT, ANIM_TRANSIENT );
                            break;

                        }
                    }
                }
            }
            else
                wpn[currWeapon].releaseTrigger( pos.c[X] + ( wpnPos[currWeapon].c[X]*cos( lookAngle - PIOVER2 ) ) +
                                                ( wpnPos[currWeapon].c[Y]*cos( lookAngle ) ),
                                                pos.c[Y] + ( wpnPos[currWeapon].c[X]*sin( lookAngle - PIOVER2 ) ) +
                                                ( wpnPos[currWeapon].c[Y]*sin( lookAngle ) ), lookAngle, 4.5 );

            if( !augmented )
            {
                if( ev.getGrenade() && !Dying() && currWeapon != WPN_HEPA && currWeapon != WPN_NONE && currWeapon != WPN_ESG )
                {
                    if( wpn[WPN_GRENADE].Ready() && !throwGrenade )
                    {
                        throwGrenade = true;
                        grenadeThrowTimer = 0.5;

                        switch( currWeapon )
                        {
                        case WPN_RIFLE:
                        case WPN_SHOTGUN:
                        case WPN_LASER:
                            model[1].setAnimation( MA_PT_RIFLEGRENADE, ANIM_TRANSIENT );
                            break;
                        default:
                            model[1].setAnimation( MA_PT_GRENADE, ANIM_TRANSIENT );
                        }
                    }
                }
                else
                    wpn[WPN_GRENADE].releaseTrigger( 0, 0, 0, 0 );

                if( throwGrenade )
                {
                    grenadeThrowTimer -= timer.dT();
                    if( grenadeThrowTimer <= 0 )
                    {
                        int hand = ( currWeapon == WPN_GRENADE ) ? 1 : -1;
                        wpn[WPN_GRENADE].fire( pos.c[X] + ( wpnPos[WPN_GRENADE].c[X]*hand * cos( lookAngle - PIOVER2 ) ) +
                                               ( wpnPos[WPN_GRENADE].c[Y]*cos( lookAngle ) ),
                                               pos.c[Y] + ( wpnPos[WPN_GRENADE].c[X]*hand * sin( lookAngle - PIOVER2 ) ) +
                                               ( wpnPos[WPN_GRENADE].c[Y]*sin( lookAngle ) ), lookAngle, 4.5 );
                        grenadeThrowTimer = 0;
                        throwGrenade = false;
                        setMessage( "Grenades: %d", wpn[WPN_GRENADE].ReserveAmmo() );
                    }
                }
            }

            if( ev.getReload() )
                wpn[currWeapon].reload();

            if( wpn[currWeapon].isReloading() && !( Dying() || Stunned() ) )
            {
                switch( currWeapon )
                {
                case WPN_RIFLE:
                case WPN_SHOTGUN:
                case WPN_LASER:
                    model[1].setAnimation( MA_PT_RIFLERELOAD, ANIM_TRANSIENT );
                    break;
                case WPN_HEPA:
                case WPN_ESG:
                    model[1].setAnimation( MA_PT_HEPARELOAD, ANIM_TRANSIENT );
                    break;
                }
            }
        }

        if( type == ENT_TURRET1 || type == ENT_SLAVE_TURRET || type == ENT_ARMED_DRONE || type == ENT_HUNTER || type == ENT_GLADIATOR )
        {
            if( ev.getFire() )
            {
                wpn[0].fire( pos.c[X] + ( wpnPos[0].c[X]*cos( lookAngle - PIOVER2 ) ) +
                             ( wpnPos[0].c[Y]*cos( lookAngle ) ),
                             pos.c[Y] + ( wpnPos[0].c[X]*sin( lookAngle - PIOVER2 ) ) +
                             ( wpnPos[0].c[Y]*sin( lookAngle ) ), lookAngle, 0.5 );
            }
            else
                wpn[0].releaseTrigger( 0, 0, 0, 0 );

            if( ev.getReload() )
                wpn[0].reload();
        }

        if( type == ENT_HUNTER )
        {
            if( ev.getFire() )
            {
                wpn[1].fire( pos.c[X] + ( wpnPos[1].c[X]*cos( lookAngle - PIOVER2 ) ) +
                             ( wpnPos[1].c[Y]*cos( lookAngle ) ),
                             pos.c[Y] + ( wpnPos[1].c[X]*sin( lookAngle - PIOVER2 ) ) +
                             ( wpnPos[1].c[Y]*sin( lookAngle ) ), lookAngle, 0.5 );
            }
            else
                wpn[1].releaseTrigger( 0, 0, 0, 0 );

            if( ev.getReload() )
                wpn[1].reload();
        }
    }
}

void Entity::selectNextWeapon()
{
    currWeapon++;
    if( currWeapon >= NUM_PLAYER_WEAPONS )
        currWeapon = 0;
    else if( !HAS_WEAPON( weapons, currWeapon ) )
        selectNextWeapon();
    else
        playSound( SND_CHANGE_WEAPON, 7 );
}

void Entity::givePowerup( int idx )
{
    bool pickedUp = true;
    int iAmt;
    float fAmt;
    int snd = 1;
    switch( currLevelObjs[idx]->Type() )
    {
    case POWERUP_HEALTH:
        if( ( fAmt = changeHealthAmt( POWERUP_HEALTH_AMOUNT ) ) > 0 )
        {
            setMessage( "Health +%0.0f", fAmt );
            snd = 2;
        }
        else
        {
            setMessage( "Health full" );
            pickedUp = false;
        }
        break;
    case POWERUP_ENERGY_CLIP:
        addAmmoPowerup( WPN_RIFLE, "Rifle ammo +%d", "Rifle ammo full" );
        break;
    case POWERUP_SHOTGUN_AMMO:
        addAmmoPowerup( WPN_SHOTGUN, "Shotgun ammo +%d", "Shotgun ammo full" );
        break;
    case POWERUP_HEPA_CLIP:
        addAmmoPowerup( WPN_HEPA, "Positron cells +%d", "H.E.P.A. ammo full" );
        break;
    case POWERUP_LASER_CELL:
        addAmmoPowerup( WPN_LASER, "Laser cells +%d", "Laser ammo full" );
        break;
    case POWERUP_ESG_BATTERY:
        addAmmoPowerup( WPN_ESG, "ESG battery +%d", "ESG ammo full" );
        break;

    case POWERUP_KEY1:
    case POWERUP_KEY2:
    case POWERUP_KEY3:
        giveKey( currLevelObjs[idx]->Type() - POWERUP_KEY1 + 1 );
        setMessage( "Key %d", currLevelObjs[idx]->Type() - POWERUP_KEY1 + 1 );
        break;

    case WEAPON_RIFLE:
        addWeaponSwitch( WPN_RIFLE, "Energy rifle", "Rifle ammo full" );
        break;
    case WEAPON_SHOTGUN:
        addWeaponSwitch( WPN_SHOTGUN, "Shotgun", "Shotgun ammo full" );
        break;
    case WEAPON_HEPA:
        addWeaponSwitch( WPN_HEPA, "H.E.P.A.", "H.E.P.A. ammo full" );
        break;
    case POWERUP_GRENADE:
        addWeapon( WPN_GRENADE, "Frag grenade", "Grenades full" );
        break;
    case WEAPON_RED_LASER:
        addWeaponSwitch( WPN_LASER, "Laser", "Laser ammo full" );
        break;
    case WEAPON_ESG:
        addWeaponSwitch( WPN_ESG, "EMP Shock Gen", "ESG ammo full" );
        break;

    }

    if( pickedUp )
    {
        currLevelObjs[idx]->kill( true );

        if( snd == 1 ) playSound( SND_PICKUP_ITEM, 7 );
        else if( snd == 2 ) playSound( SND_PICKUP_HEALTH, 7 );
    }
}

char Entity::weaponState( int wp )
{
    if( !HAS_WEAPON( weapons, wp ) )
        return ' ';
    else
        return '1' + wp;
}

char Entity::keyState( int k )
{
    if( !HAS_KEY( keys, k ) )
        return ' ';
    else
        return '?';
}

void Particle::launch( int typ, float x, float y, float height, float head, float alph, int sprite )
{
    Object::set( typ, x, y, height, head, 0, 0, 0, 0 );
    color.white();
    pType = typ - PART_PARTICLE;
    alpha = alph;
    active = true;
    rotation = head;
    spr1 = sprite;
    additionalSpeed = 0;

    health += frand() * partType[pType].maxRandAddedToLife;

    speed = objType[type].maxVel + frand() * partType[pType].maxRandAddedToSpeed;

    hvel = objType[type].jumpVel + frand() * partType[pType].maxRandAddedToJumpSpeed;

    rotVel = partType[pType].minRotVel + frand() * partType[pType].maxRandAddedToRotVel;
    fadeSpeed = partType[pType].minFadeSpeed + frand() * partType[pType].maxRandAddedToFadeSpeed;

    anim.set( partType[pType].animNumFrames, partType[pType].animDelay + frand()*partType[pType].maxRandAddedToAnimDelay, partType[pType].animLoop );
    anim.setDirection( 1 );
}

void Particle::launch( int typ, float x, float y, float height, float head, float alph, int sprite, float r, float g, float b )
{
    launch( typ, x, y, height, head, alph, sprite );
    color.set( r, g, b );
}


void Particle::reset()
{
    Object::reset();
    bounceCount = 0;
    scale = 1.0;
}

void Particle::drawShadow()
{
    if( active && objType[type].hasShadow && h >= 0 )
    {
        glPushMatrix();
        // translate ever so slightly above the ground to avoid being
        // screwed over by the depth test
        glTranslatef( pos.c[X], pos.c[Y], 0.01 );
        glRotatef( todeg( heading ), 0.0, 0.0, 1.0 );
        glEnable( GL_TEXTURE_2D );
        glColor4f( 1.0, 1.0, 1.0, alpha );
        texQuadSize( TEX_SHADOW, objType[type].spriteSize.c[X]*scale, objType[type].spriteSize.c[Y]*scale );
        glDisable( GL_TEXTURE_2D );
        glPopMatrix();
    }
}

void Particle::draw()
{
    if( active )
    {
        glPushMatrix();

        glTranslatef( pos.c[X], pos.c[Y], 0 );
        glRotatef( todeg( rotation ), 0.0, 0.0, 1.0 );

        glEnable( GL_TEXTURE_2D );
        glColor4f( color.r, color.g, color.b, alpha );

        // translate ever so slightly above the ground to avoid being
        // screwed over by the depth test
        glTranslatef( 0.0, 0.0, 0.01 );
        if( objType[type].hasShadow )
        {
            texQuadSize( TEX_SHADOW, objType[type].spriteSize.c[X]*scale, objType[type].spriteSize.c[Y]*scale );
        }

        glTranslatef( 0.0, 0.0, h );

        if( partType[pType].glow )
            glBlendFunc( GL_SRC_ALPHA, GL_ONE );

        if( partType[pType].animNumFrames <= 1 )
        {
            if( spr1 != NO_SPRITE )
            {
                texQuadSize( objType[type].sprites[spr1], objType[type].spriteSize.c[X]*scale, objType[type].spriteSize.c[Y]*scale );
            }
            glTranslatef( 0.0, 0.0, 0.01 );
            if( spr2 != NO_SPRITE )
            {
                texQuadSize( objType[type].sprites[spr2], objType[type].spriteSize.c[X]*scale, objType[type].spriteSize.c[Y]*scale );
            }
        }
        else
        {
            if( spr1 != NO_SPRITE )
            {
                texQuadClip( objType[type].sprites[spr1], anim.Frame(), 1.0 / anim.NumFrames() );
            }
            glTranslatef( 0.0, 0.0, 0.01 );
            if( spr2 != NO_SPRITE )
            {
                texQuadClip( objType[type].sprites[spr2], anim.Frame(), 1.0 / anim.NumFrames() );
            }
        }
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        glDisable( GL_TEXTURE_2D );
        glPopMatrix();
    }
}

void Particle::levelCollisionDetection()
{
    if( currLevel.inGrid( pos ) )
    {
        int cell = currLevel.cellNumber( pos );
        if( objType[type].collide )
        {
            Vector2D intersect( 0, 0 );
            Wall* closestHitWall = wallCollisionDetect( cell, &intersect );

            if( closestHitWall )
            {
                // Bullet ricochets if ricochet was specified.
                if( closestHitWall->CollFlags() & RICOCHET_BULLET )
                {
                    vel -= closestHitWall->normal * ( closestHitWall->normal * vel ) * 2;
                    heading = atan2( vel.c[Y], vel.c[X] );
                    rotation = heading;
                }
                else
                    switch( partType[pType].hitBehavior )
                    {
                    case HIT_DIE:
                        health = 0;
                        break;
                    case HIT_STOP:
                        vel.set( 0, 0 );
                        break;
                    case HIT_BOUNCE:
                        // Snell's law, woot.
                        vel -= closestHitWall->normal * ( closestHitWall->normal * vel ) * 2;
                        heading = atan2( vel.c[Y], vel.c[X] );
                        rotation = heading;
                    }

                if( IS_BULLET( type ) )
                {
                    closestHitWall->spewParticles( intersect + closestHitWall->normal * 0.25 );
                    closestHitWall->handleTrigger( true, false );
                }
            }
        }
        if( !floorCollisionDetect( cell, NULL ) )
            fall = true;
    }
}

// Updates a Particle.
void Particle::update()
{
    spr2 = NO_SPRITE;

    if( active )
    {
        if( !currLevel.inGrid( pos ) )
        {
            active = false;
            return;
        }

        ev.setFB( 1 );

        speed += partType[pType].accel * timer.dT();

        if( speed < 0 )
            speed = 0;

        vel.set( cos( heading )*speed, sin( heading )*speed );

        levelCollisionDetection();

        pos += vel * timer.dT();

        if( type == ESG_EXPLODER )
        {
            pos.c[X] = PLAYER_OBJECT->PosX() + ( wpnType[WPN_ESG].spriteSize.c[X] / 3.5 * cos( PLAYER_OBJECT->LookAngle() ) );
            pos.c[Y] = PLAYER_OBJECT->PosY() + ( wpnType[WPN_ESG].spriteSize.c[X] / 3.5 * sin( PLAYER_OBJECT->LookAngle() ) );
        }

        // Object is affected by gravity if it is not on the ground
        h += hvel * timer.dT();

        if( objType[type].gravAffect )
        {
            if( h != 0 )
                hvel -= ACCEL_GRAVITY * timer.dT();
            if( h < 0 && !fall )
                hitGround = true;
        }

        if( type == BLT_HEPA )
        {
            launchParticle( PART_HEPA_TRAIL,
                            pos.c[X], pos.c[Y], 0.5, torad( rand() % 360 ), 1.0, 0 );
        }
        else if( type == BLT_GRENADE || type == BLT_SHRAPNEL )
        {
            launchParticle( PART_GRENADE_TRAIL,
                            pos.c[X], pos.c[Y], h, -rotation, 1.0, 0 );
        }

        if( partType[pType].startFade == 0 || health <= partType[pType].startFade )
            alpha -= fadeSpeed * timer.dT();

        if( alpha <= 0 )
            health = 0;

        rotation += rotVel * timer.dT();

        if( partType[pType].scaleStop == 0 ||
                ( partType[pType].scaleStop > 0 && scale <= partType[pType].scaleStop ) )
            scale += partType[pType].scaleSpeed * timer.dT();

        if( scale <= 0 )
            health = 0;

        anim.update();

        if( anim.isDone() )
            health = 0;

        health -= 1 * timer.dT();

        if( health != INF && health <= 0 )
        {
            active = false;
            destroyMe = true;
        }

        if( hitGround )
        {
            hitGround = false;
            h = 0.00001;
            hvel = -hvel * 0.5;

            if( type == BLT_GRENADE )
                playSound( SND_GRENADE_BOUNCE, 1 );
        }
    }
}

void Bullet::update()
{
    if( frameDelay > 0 )
        frameDelay--;

    Particle::update();
}

LaserBeam::LaserBeam( Vector2D np1, Vector2D np2 )
{
    p1 = np1;
    p2 = np2;
    angle = atan2( p2.c[Y] - p1.c[Y], p2.c[X] - p1.c[X] );
    length = ( p2 - p1 ).mag();
}

void Laser::launch( int typ, float x, float y, float height, float head )
{
    h = height;
    alpha = 1.0;
    type = typ;
    beams.clear();

    float distLeft = lasType[type].maxRange;
    int bounces = 0;
    Vector2D startPos( x - cos( head )*wpnType[WPN_LASER].spriteSize.c[X], y - sin( head )*wpnType[WPN_LASER].spriteSize.c[X] );
    while( distLeft > 0.001 && bounces < lasType[type].maxBounces )
    {
        Vector2D endPos;
        Wall* closestHitWall = wallCollisionDetect( startPos, head, distLeft, &endPos );

        // If we didn't hit a wall, the beam keeps going for the remainder of its distance
        if( closestHitWall == NULL )
            endPos = startPos + Vector2D( cos( head ) * distLeft, sin( head ) * distLeft );
        else
        {
            closestHitWall->handleTrigger( true, false );
            Vector2D dir = endPos - startPos;
            dir.normalize();

            dir -= closestHitWall->normal *
                   ( closestHitWall->normal * dir ) * 2;
            head = atan2( dir.c[Y], dir.c[X] );

            endPos += closestHitWall->normal * 0.1;

            bounces++;
        }

        distLeft -= ( endPos - startPos ).mag();
        beams.push_back( LaserBeam( startPos, endPos ) );
        startPos.set( endPos.c[X], endPos.c[Y] );
    }
    beams[0].p1.set( x, y );

    live = 2;
    active = true;
    destroyMe = false;
}

Wall* Laser::wallCollisionDetect( Vector2D pos, float head, float distLeft, Vector2D* inter )
{
    Vector2D intersect;
    int numWalls = currLevel.wall.size();
    Wall* currWall;

    Wall* closestHitWall = NULL;
    Vector2D closestHitIntersect;
    float closestHitDistance = 1e37;

    for( int i = 0; i < numWalls + 4 * currLevel.door.size(); i++ )
    {
        // A pointer allows us to combine collision detection with
        // walls and doors into one loop.
        if( i < numWalls )
            currWall = &currLevel.wall[i];
        else
            currWall = &currLevel.door[( i - numWalls ) / 4].sides[( i - numWalls ) % 4];

        if( currWall->CollFlags() & COLLIDE_BULLET )
        {
            Vector2D offset( cos( head )*distLeft, sin( head )*distLeft );

            if( currWall->intersection( pos, pos + offset, &intersect ) )
            {
                if( ( intersect - pos ).mag() < closestHitDistance )
                {
                    closestHitDistance = ( intersect - pos ).mag();
                    closestHitWall = currWall;
                    closestHitIntersect.set( intersect.c[X], intersect.c[Y] );
                }
            }
            else
                currWall->renewTrigger();
        }
    }
    if( closestHitWall )
    {
        if( inter )
            inter->set( closestHitIntersect.c[X], closestHitIntersect.c[Y] );

        return closestHitWall;
    }

    return NULL;
}

void Laser::draw()
{
    if( active )
    {
        glPushMatrix();

        glEnable( GL_TEXTURE_2D );
        glColor4f( 1.0, 1.0, 1.0, alpha );
        glTranslatef( 0.0, 0.0, h );

        glBlendFunc( GL_SRC_ALPHA, GL_ONE );

        glBindTexture( GL_TEXTURE_2D, textures[lasType[type].sprite] );
        for( int i = 0; i < beams.size(); i++ )
        {
            glPushMatrix();
            glTranslatef( beams[i].p1.c[X], beams[i].p1.c[Y], 0.0 );
            glRotatef( todeg( beams[i].angle ), 0.0, 0.0, 1.0 );
            glBegin( GL_QUADS );
            glTexCoord2f( 0.05, 1 );
            glVertex2f( 0, -lasType[type].width * 0.5 * alpha );
            glTexCoord2f( 1, 1 );
            glVertex2f( beams[i].length, -lasType[type].width * 0.5 * alpha );
            glTexCoord2f( 1, 0 );
            glVertex2f( beams[i].length, lasType[type].width * 0.5 * alpha );
            glTexCoord2f( 0.05, 0 );
            glVertex2f( 0, lasType[type].width * 0.5 * alpha );
            glEnd();
            glPopMatrix();
        }


        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glPopMatrix();
        glDisable( GL_TEXTURE_2D );
    }

}

void Laser::update()
{
    if( active )
    {
        if( live > 0 )
            live--;

        alpha -= lasType[type].fadeSpeed * timer.dT();

        if( alpha <= 0 )
        {
            alpha = 0;
            active = false;
            destroyMe = true;
        }
    }
}

// Defines the type of all objects
void initObjectTypes()
{
    for( int i = 0; i < NUM_OBJ_TYPES; i++ )
        switch( i )
        {
        // float im, float iw, float ih, float ir, float imv, float ijv, float imtv, float ia, float ita, float hlth, int dth, bool cd, bool grav, bool shad
        case ENT_PLAYER:
            objType[i].set( 160, 1.5, 6.0, 1.0, 17, 8, torad( 150 ), 25, torad( 5000 ), 100, DEATH_PLAYER, true, true, true );
            objType[i].setModels( MDL_PLAYER_LEGS, MDL_PLAYER_TORSO );
            break;
        case ENT_BOX:
            objType[i].set( 140, 3.0, 3.0, 2.0,  0, 0, 0, 0, 0, 20, DEATH_WOOD, true, true, true );
            objType[i].setModels( MDL_BOX, NO_MODEL );
            break;
        case ENT_BARREL:
            objType[i].set( 180, 2.5, 2.5, 1.25,  0, 0, 0, 0, 0, 30, DEATH_EXPLOSION, true, true, true );
            objType[i].setModels( MDL_BARREL, NO_MODEL );
            break;
        case ENT_STEEL_CRATE:
            objType[i].set( 250, 3.0, 3.0, 2.0,  0, 0, 0, 0, 0, INF, DEATH_NONE, true, true, true );
            objType[i].setModels( MDL_STEELBOX, NO_MODEL );
            break;

        case ENT_UNARMED_GUARD:
            objType[i].set( 100, 3.0, 3.0, 1.5,  15, 0, torad( 150 ), 25, torad( 5000 ), 30, DEATH_SMALL_EXPLOSION, true, true, true );
            objType[i].setModels( MDL_UNARMED_GUARD, NO_MODEL );
            break;
        case ENT_MIB:
            objType[i].set( 100, 3.0, 3.0, 1.5,  12, 0, torad( 150 ), 25, torad( 5000 ), 50, DEATH_NONE, true, true, true );
            objType[i].setModels( MDL_MIB, NO_MODEL );
            break;
        case ENT_TURRET1:
        case ENT_SLAVE_TURRET:
            objType[i].set( 10000, 5.0, 5.0, 1.5, 0, 0, torad( 75 ), 0, torad( 5000 ), 50, DEATH_ROBOT, true, true, true );
            objType[i].setModels( MDL_TURRET, NO_MODEL );
            break;
        case ENT_ARMED_DRONE:
            objType[i].set( 200, 4.0, 4.0, 1.25, 12, 0, torad( 200 ), 25, torad( 4000 ), 50, DEATH_ROBOT, true, true, true );
            objType[i].setModels( MDL_ARMED_DRONE, NO_MODEL );
            break;
        case ENT_HUNTER:
            objType[i].set( 450, 4.0, 4.0, 1.25, 14, 0, torad( 200 ), 25, torad( 4000 ), 80, DEATH_ROBOT, true, true, true );
            objType[i].setModels( MDL_HUNTER, NO_MODEL );
            break;
        case ENT_GLADIATOR:
            objType[i].set( 700, 4.0, 4.0, 1.5, 8, 0, torad( 200 ), 25, torad( 4000 ), 160, DEATH_GLADIATOR, true, true, true );
            objType[i].setModels( MDL_GLADIATOR, NO_MODEL );
            break;
        case POWERUP_HEALTH:
            objType[i].set( 5, 1.5, 0.75, 0.5, 0, 0, 0, 0, 0, INF, DEATH_NONE, true, true, true );
            objType[i].setSprites( 1, TEX_POWERUP_HEALTH );
            break;
        case POWERUP_ENERGY_CLIP:
            objType[i].set( 1, 0.5, 1.0, 0.25, 0, 0, 0, 0, 0, INF, DEATH_NONE, true, true, true );
            objType[i].setSprites( 1, TEX_POWERUP_ENERGY_CLIP );
            break;
        case POWERUP_SHOTGUN_AMMO:
            objType[i].set( 1, 1.0, 1.0, 0.5, 0, 0, 0, 0, 0, INF, DEATH_NONE, true, true, true );
            objType[i].setSprites( 1, TEX_POWERUP_SHOTGUN_AMMO );
            break;
        case POWERUP_HEPA_CLIP:
            objType[i].set( 1, 0.5, 1.0, 0.25, 0, 0, 0, 0, 0, INF, DEATH_NONE, true, true, true );
            objType[i].setSprites( 1, TEX_POWERUP_HEPA_CLIP );
            break;
        case POWERUP_LASER_CELL:
            objType[i].set( 1, 1.5, 0.45, 0.3, 0, 0, 0, 0, 0, INF, DEATH_NONE, true, true, true );
            objType[i].setSprites( 1, TEX_POWERUP_LASER_CELL );
            break;
        case POWERUP_ESG_BATTERY:
            objType[i].set( 1, 1.0, 1.0, 0.5, 0, 0, 0, 0, 0, INF, DEATH_NONE, true, true, true );
            objType[i].setSprites( 1, TEX_POWERUP_ESG_BATTERY );
            break;
        case POWERUP_KEY1:
            objType[i].set( 1, 0.5, 1.0, 0.25, 0, 0, 0, 0, 0, INF, DEATH_NONE, true, true, true );
            objType[i].setSprites( 1, TEX_POWERUP_KEY1 );
            break;
        case POWERUP_KEY2:
            objType[i].set( 1, 0.5, 1.0, 0.25, 0, 0, 0, 0, 0, INF, DEATH_NONE, true, true, true );
            objType[i].setSprites( 1, TEX_POWERUP_KEY2 );
            break;
        case POWERUP_KEY3:
            objType[i].set( 1, 0.5, 1.0, 0.25, 0, 0, 0, 0, 0, INF, DEATH_NONE, true, true, true );
            objType[i].setSprites( 1, TEX_POWERUP_KEY3 );
            break;
        case WEAPON_RIFLE:
            objType[i].set( 10, 4.0, 1.0, 1.0, 0, 0, 0, 0, 0, INF, DEATH_NONE, true, true, true );
            objType[i].setSprites( 1, TEX_WEAPON_RIFLE );
            break;
        case WEAPON_SHOTGUN:
            objType[i].set( 10, 4.0, 1.0, 1.0, 0, 0, 0, 0, 0, INF, DEATH_NONE, true, true, true );
            objType[i].setSprites( 1, TEX_WEAPON_SHOTGUN );
            break;
        case WEAPON_HEPA:
            objType[i].set( 10, 4.0, 2.0, 1.0, 0, 0, 0, 0, 0, INF, DEATH_NONE, true, true, true );
            objType[i].setSprites( 1, TEX_WEAPON_HEPA );
            break;
        case POWERUP_GRENADE:
            objType[i].set( 1, 0.75, 0.75, 0.25, 0, 0, 0, 0, 0, INF, DEATH_NONE, true, true, true );
            objType[i].setSprites( 1, TEX_WPN_GRENADE );
            break;
        case WEAPON_RED_LASER:
            objType[i].set( 10, 4.0, 2.0, 1.0, 0, 0, 0, 0, 0, INF, DEATH_NONE, true, true, true );
            objType[i].setSprites( 1, TEX_WEAPON_LASER );
            break;
        case WEAPON_ESG:
            objType[i].set( 10, 4.0, 4.0, 1.0, 0, 0, 0, 0, 0, INF, DEATH_NONE, true, true, true );
            objType[i].setSprites( 1, TEX_WEAPON_ESG );
            break;

        case EXPLODER:
            objType[i].set( 1000000, 0, 0, 0, 0, 0, 0, 0, 0, INF, DEATH_EXPLOSION, false, false, false );
            break;

        case PART_PARTICLE:
            objType[i].set( 0.01, 0.25, 0.25, 0.25, 1, 0, 0, 100, 0, 2.0, DEATH_NONE, false, true, false );
            objType[i].setSprites( 1, TEX_PARTICLE );
            break;
        case PART_PARTICLE_BLOOD:
            objType[i].set( 0.01, 0.25, 0.25, 0.25, 1, 0, 0, 100, 0, 4, DEATH_NONE, false, true, false );
            objType[i].setSprites( 1, TEX_PARTICLE_BLOOD );
            break;
        case PART_PARTICLE_SLIME:
            objType[i].set( 0.01, 0.25, 0.25, 0.25, 1, 0, 0, 100, 0, 1, DEATH_NONE, false, true, false );
            objType[i].setSprites( 1, TEX_PARTICLE_SLIME );
            break;
        case PART_PARTICLE_SPARK:
            objType[i].set( 0.01, 1.5, 0.125, 0.25, 15, 0, 0, 100, 0, 4, DEATH_NONE, false, true, false );
            objType[i].setSprites( 1, TEX_PARTICLE_SPARK );
            break;
        case PART_PARTICLE_ENERGY:
            objType[i].set( 0.01, 0.75, 0.75, 0.75, 0.5, 0, 0, 100, 0, 1, DEATH_NONE, false, true, false );
            objType[i].setSprites( 1, TEX_PARTICLE_ENERGY );
            break;
        case PART_PARTICLE_WOOD:
            objType[i].set( 0.01, 1.0, 0.5, 0.5, 1, 10, 0, 100, 0, 2, DEATH_NONE, false, true, true );
            objType[i].setSprites( 1, TEX_PARTICLE_WOOD );
            break;
        case PART_PARTICLE_METAL:
            objType[i].set( 0.01, 1.0, 0.25, 0.5, 3, 20, 0, 100, 0, 2, DEATH_NONE, true, true, true );
            objType[i].setSprites( 1, TEX_PARTICLE_METAL );
            break;
        case PART_MF_RIFLE:
            objType[i].set( 0.01, 1.0, 0.5, 1.0, 0, 0, 0, 0, 0, 0.0625, DEATH_NONE, false, false, false );
            objType[i].setSprites( 1, TEX_MF_RIFLE );
            break;
        case PART_MF_SHOTGUN:
            objType[i].set( 0.01, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0.0625, DEATH_NONE, false, false, false );
            objType[i].setSprites( 1, TEX_MF_SHOTGUN );
            break;
        case BLT_RIFLE:
            objType[i].set( 0.0044, 2.0, 0.15, 0.5, 150, 0, 0, 0, 0, 1.0, DEATH_NONE, true, false, false );
            objType[i].setSprites( 1, TEX_BLT_RIFLE );
            break;
        case BLT_SHOTGUN:
            objType[i].set( 0.0044, 0.5, 0.15, 0.5, 150, 0, 0, 0, 0, 1.0, DEATH_NONE, true, false, false );
            objType[i].setSprites( 1, TEX_BLT_RIFLE );
            break;
        case BLT_HEPA:
            objType[i].set( 0.0001, 1.0, 1.0, 0.5, 20, 0, 0, 0, 0, 2.0, DEATH_HEPA_EXPLOSION, true, false, false );
            objType[i].setSprites( 1, TEX_PARTICLE_ENERGY );
            break;
        case BLT_GRENADE:
            objType[i].set( 1, 0.65, 0.65, 0.3, 20, 0, 0, 0, 0, 3.0, DEATH_GRENADE_EXPLOSION, true, true, true );
            objType[i].setSprites( 1, TEX_BLT_GRENADE );
            break;
        case BLT_SHRAPNEL:
            objType[i].set( 1, 0.5, 0.5, 0.25, 100, 0, 0, 0, 0, .35, DEATH_NONE, true, false, false );
            objType[i].setSprites( 1, TEX_BLT_SHRAPNEL );
            break;
        case BLT_BLASTER:
            objType[i].set( 0.0044, 6.0, 2.0, 0.5, 110, 0, 0, 0, 0, 1.0, DEATH_NONE, true, false, false );
            objType[i].setSprites( 1, TEX_BLT_BLASTER );
            break;
        case PART_RIFLE_SHELL:
            objType[i].set( 0.0044, 0.03125, 0.125, 0.5, 0.5, 10, 0, 0.1, 0, 2.0, DEATH_NONE, false, true, true );
            objType[i].setSprites( 1, TEX_RIFLE_SHELL );
            break;
        case PART_SHOTGUN_SHELL:
            objType[i].set( 0.0044, 0.125, 0.25, 0.5, 0.5, 5, 0, 0.1, 0, 2.0, DEATH_NONE, false, true, true );
            objType[i].setSprites( 1, TEX_SHOTGUN_SHELL );
            break;
        case PART_EXPLOSION:
            objType[i].set( 0, 8.0, 8.0, 8.0, 0, 0, 0, 0, 0, 0.4, DEATH_NONE, false, false, false );
            objType[i].setSprites( 4, TEX_EXPLOSION1, TEX_EXPLOSION2, TEX_EXPLOSION3, TEX_EXPLOSION4 );
            break;
        case PART_EXPLOSIONPART:
            objType[i].set( 0, 1.0, 0.5, 0.5, 10, 0, 0, 0, 0, 0.5, DEATH_NONE, false, true, false );
            objType[i].setSprites( 1, TEX_EXPLOSIONPART );
            break;
        case PART_SMOKE:
            objType[i].set( 0, 2.0, 2.0, 2.0, 2, 1, 0, 0, 0, 30, DEATH_NONE, false, false, false );
            objType[i].setSprites( 1, TEX_SMOKE );
            break;
        case PART_PARTICLE_GLOW:
            objType[i].set( 0, 0.4, 0.4, 0.4, 0, 0, 0, 0, 0, 5, DEATH_NONE, false, false, false );
            objType[i].setSprites( 1, TEX_PARTICLE_GLOW );
            break;
        case PART_HEPA_TRAIL:
            objType[i].set( 0, 2.0, 2.0, 1.0, 0, 0, 0, 0, 0, 5, DEATH_NONE, false, false, false );
            objType[i].setSprites( 1, TEX_PARTICLE_ENERGY );
            break;
        case PART_HEPA_EXPLOSION:
            objType[i].set( 0, 4.0, 4.0, 2.0, 0, 0, 0, 0, 0, 5, DEATH_NONE, false, false, false );
            objType[i].setSprites( 1, TEX_LIGHT );
            break;
        case PART_HEPA_SPARK:
            objType[i].set( 0.01, 3.0, 3.0, 0.25, 1, 0, 0, 100, 0, 6, DEATH_NONE, false, true, false );
            objType[i].setSprites( 1, TEX_PARTICLE_ENERGY );
            break;
        case PART_GRENADE_TRAIL:
            objType[i].set( 0.01, 0.125, 0.125, 0.125, 2, 0, 0, 100, 0, 1, DEATH_NONE, false, true, false );
            objType[i].setSprites( 1, TEX_PARTICLE_SLIME );
            break;
        case PART_LASER_SPARK1:
            objType[i].set( 0.01, 1.0, 1.0, 0.25, 0.5, 5, 0, 100, 0, 6, DEATH_NONE, false, false, false );
            objType[i].setSprites( 1, TEX_LASER_SPARK1 );
            break;
        case PART_LASER_SPARK2:
            objType[i].set( 0.01, 2.0, 2.0, 1.0, 0.5, 5, 0, 100, 0, 6, DEATH_NONE, false, false, false );
            objType[i].setSprites( 1, TEX_LASER_SPARK2 );
            break;
        case PART_ESG_SHOCKWAVE:
            objType[i].set( 0.01, 1.0, 1.0, 2.5, 0, 0, 0, 100, 0, 0.5, DEATH_NONE, false, false, false );
            objType[i].setSprites( 1, TEX_ESG_SHOCKWAVE );
            break;
        case PART_BIT:
            objType[i].set( 0.01, 5.0, 5.0, 1.0, 0, 0, 0, 0, 0, 10, DEATH_NONE, false, false, false );
            objType[i].setSprites( 1, TEX_BIT );
            break;
        case ESG_EXPLODER:
            objType[i].set( 1, 0, 0, 0, 0, 0, 0, 0, 0, 0.75, DEATH_ESG, false, false, false );
            break;
        }

    for( int i = 0; i < NUM_PART_TYPES; i++ )
        switch( i )
        {
        // float mrl, float mrs, float mrjs, float rv, float mrrv, float fs, float mrfs, float sf, float ss, float sst, int hb, bool gl, float acc, int anf, float ad, float mrad, int al
        case P_PART_PARTICLE:
        case P_PART_PARTICLE_SLIME:
        case P_PART_PARTICLE_SPARK:
            partType[i].set( 0, 5, 25,   0, 0,  0.5, 0.1, 0, 0.0, 0.0, HIT_BOUNCE, true, 0,    1, 0, 0, 0 );
            break;
        case P_PART_PARTICLE_WOOD:
        case P_PART_PARTICLE_METAL:
            partType[i].set( 0, 5, 5,   -torad( 90 ), torad( 180 ), 2.0, 0.1, 0.5, 0.0,  0.0, HIT_BOUNCE, false, 0,    1, 0, 0, 0 );
            break;
        case P_PART_PARTICLE_BLOOD:
            partType[i].set( 0, 5, 5,   0, 0,   0.25, 0.1, 0, 0.0,  0.0, HIT_BOUNCE, false, 0,   1, 0, 0, 0 );
            break;
        case P_PART_PARTICLE_ENERGY:
            partType[i].set( 0, 2, 20,   0, 0,   1.0, 0.1, 0, 0.0,  0.0, HIT_BOUNCE, true, 0,   1, 0, 0, 0 );
            break;
            break;
        case P_PART_MF_RIFLE:
        case P_PART_MF_SHOTGUN:
            partType[i].set( 0, 0, 0, 0, 0, 4.0, 0, 0, 0.0,  0.0, HIT_NOTHING, false, 0,    1, 0, 0, 0 );
            break;
        case P_BLT_RIFLE:
        case P_BLT_SHOTGUN:
            partType[i].set( 0, 0, 0, 0, 0, 0, 0, 0, 0.0,  0.0, HIT_DIE, true, 0,   1, 0, 0, 0 );
            break;
        case P_BLT_BLASTER:
            partType[i].set( 0, 0, 0, 0, 0, 0, 0, 0, 0.0,  0.0, HIT_DIE, true, 0,   1, 0, 0, 0 );
            break;
        case P_BLT_HEPA:
            partType[i].set( 0, 0, 0, 0, 0, 0, 0, 0, 0.0,  0.0, HIT_DIE, true, 100,   1, 0, 0, 0 );
            break;
        case P_BLT_SHRAPNEL:
            partType[i].set( .5, 50, 0, -torad( 360 ), torad( 360 ), 0, 0, 0, 0.0,  0.0, HIT_BOUNCE, true, 0,   4, 0.25, 0, INF );
            break;
        case P_BLT_GRENADE:
            partType[i].set( 0, 0, 20, -torad( 360 ), torad( 360 ), 0, 0, 0, 0.0,  0.0, HIT_BOUNCE, false, -7.5,   2, 0.5, 0, INF );
            break;
        case P_PART_RIFLE_SHELL:
        case P_PART_SHOTGUN_SHELL:
            partType[i].set( 0, 1, 5, -torad( 180 ), 0, 0.5, 0, 0, 0.0,  0.0, HIT_DIE, false, 0,   1, 0, 0, 0 );
            break;
        case P_PART_EXPLOSION:
            partType[i].set( 0, 0, 0, 0, 0, 4.0, 0, 0.25, 2,  0.0, HIT_NOTHING, true, 0,    1, 0, 0, 0 );
            break;
        case P_PART_EXPLOSIONPART:
            partType[i].set( 0, 25, 5,   0, 0,  4.0, 0.0, 0.25, 0.0,  0.0, HIT_DIE, true, 0,   1, 0, 0, 0 );
            break;
        case P_PART_SMOKE:
            partType[i].set( 1, 2, 5,   0, 0,  0.5, 0.0, 0, 2,  0.0, HIT_NOTHING, false, 0,   1, 0, 0, 0 );
            break;
        case P_PART_PARTICLE_GLOW:
            partType[i].set( 0, 1, 0,   0, 0,  0.9, 0.3, 0, 3.5,  0.0, HIT_NOTHING, true, 0,   1, 0, 0, 0 );
            break;
        case P_PART_HEPA_TRAIL:
            partType[i].set( 0, 5, 0,   0, 0,  0.5, 0.5, 0, -1,  0.0, HIT_NOTHING, true, 0,   1, 0, 0, 0 );
            break;
        case P_PART_HEPA_EXPLOSION:
            partType[i].set( 0, 0, 0,   0, 0,  0.5, 0.5, 0, 50,  3.0, HIT_NOTHING, true, 0,   1, 0, 0, 0 );
            break;
        case P_PART_HEPA_SPARK:
            partType[i].set( 1, 10, 15,   0, 0,  0, 0, 0, -0.5, 0.0, HIT_NOTHING, true, 0,    1, 0, 0, 0 );
            break;
        case P_PART_GRENADE_TRAIL:
            partType[i].set( 0, 5, 25,   0, 0,  1.0, 0.1, 0, 0.0, 0.0, HIT_BOUNCE, true, 0,    1, 0, 0, 0 );
            break;
        case P_PART_LASER_SPARK1:
            partType[i].set( 1, 2, 25,   0, 0,  0, 0, 0, -0.5, 0.0, HIT_NOTHING, true, 0,    1, 0, 0, 0 );
            break;
        case P_PART_LASER_SPARK2:
            partType[i].set( 1, 3, 15,   0, 0,  0, 0, 0, 0, 0.0, HIT_NOTHING, true, 0,    1, 0, 0, 0 );
            break;
        case P_PART_ESG_SHOCKWAVE:
            partType[i].set( 0, 0, 0,   0, 0,  0, 0, 0, 200.0, 0.0, HIT_NOTHING, true, 0,    1, 0, 0, 0 );
            break;
        case P_PART_BIT:
            partType[i].set( 0, 0, 0,   0, 0,  0, 0, 0, 0, 0.0, HIT_NOTHING, false, 0,    2, 0.25, 1.0, INF );
            break;
        case P_ESG_EXPLODER:
            partType[i].set( 0, 0, 0,  0, 0,  0, 0, 0, 0, 0, HIT_NOTHING, true, 0,    1, 0, 0, 0 );
            break;
        }

    for( int i = 0; i < NUM_BLT_TYPES; i++ )
        switch( i )
        {
        case B_BLT_RIFLE:
            bltType[i].set( 6, 260 );
            break;
        case B_BLT_SHOTGUN:
            bltType[i].set( 8, 400 );
            break;
        case B_BLT_HEPA:
            bltType[i].set( 20, 1200 );
            break;
        case B_BLT_GRENADE:
            bltType[i].set( 40, 1200 );
            break;
        case B_BLT_SHRAPNEL:
            bltType[i].set( 13, 400 );
            break;
        case B_BLT_BLASTER:
            bltType[i].set( 45, 480 );
            break;
        }

    // void set(int spr, float w, float mr, int mb, float dmg, float fs)
    for( int i = 0; i < NUM_LASER_TYPES; i++ )
        switch( i )
        {
        case L_STANDARD:
            lasType[i].set( TEX_BEAM1, 0.5, 200, 7, 40, 0.5 );
            break;
        case L_CHARGED:
            lasType[i].set( TEX_BEAM2, 3.0, 100, 1, 240, 0.25 );
        }
}
