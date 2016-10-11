/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * enemies.h - header file for enemy AI routines
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

#ifndef _ENEMIES_H_
#define _ENEMIES_H_

#include "sdb.h"

enum AI_STATES {ASLEEP, AWAKE, MOVING, ATTACKING};
enum AI_TASKS {AIT_STOP, AIT_INVESTIGATE, AIT_ATTACK};

class AI
{
  public:
    AI() { set(ASLEEP, 1, -1, 0, 0, 0); }
    AI(int obj, int tgt) { set(AWAKE, obj, tgt, 0, 0, 0); }
    AI(int obj, int tgt, int st, float sigr, float ad, float fov) { set(st, obj, tgt, sigr, ad, fov); }
    void set(int st, int obj, int tgt, float sigr, float ad, float fov);
    virtual void update() {}
    
    void angleCalculations(bool targetOrNode);
    void distanceCalculation(bool targetOrNode);
    bool wallOccludeCalculation();
    
    float getAngle(Vector2D pos);
    float getAngleDiff(float a1, float a2);
    float getDistance(Vector2D pos);
    
    void setNodePriorities(Vector2D pos);
    void setChildNodePriorities(vector<int> children, int priority);
    
    int State()  { return state; }
    int Object() { return object; }
    int Target() { return target; }
    
    void setMoveTarget(Vector2D tgtPos, int tsk);
    void updateMoveTarget();
    
    void setState(int st) { state = st; }

    void sendAlertSignal(double dst);
    
  //protected:
    int state;  // Asleep, awake, attacking
    int object; // Object this AI controls
    int target; // This AI's target
    
    // Properties
    float signalRadius;
    float alertDistance;
    float fieldOfVision;   
    
    bool hasMoveTarget;
    Vector2D moveTarget;
    int nodeTarget;
    float targetReacquireTimer;
    Vector2D ultimateMoveTarget;
    vector<int> nodePriorities;
    bool movingToNode;
    
    float angleToTarget;
    float angleDiff;
    float distToTarget;
    
    int task;
    
    bool doSound;
};

class KamikazeAI : public AI
{
  public:
    KamikazeAI() : AI() { idleActionTimer = 0; }
    KamikazeAI(int obj, int tgt, int st, float sigr, float ad, float fov) : AI(obj, tgt, st, sigr, ad, fov) { idleActionTimer = alertTimer = alertRotateSpeed = 0; }
  
    void move();
    void update();
    
    void idleAnimations();
    void alertCheck();
    void lineOfSightBehavior();
    void attack();
    void nodeFollowBehavior();
    void targetDetect();
    
    float idleActionTimer;
    float alertRotateSpeed;
    float alertTimer;
};

class TurretAI : public AI
{
  public:
    TurretAI() : AI() { idleRotateDirection = 0; idleRotateLength = 1.75; setIdleRotation(); }
    TurretAI(int obj, int tgt, int st, float sigr, float ad, float fov) : AI(obj, tgt, st, sigr, ad, fov) { idleRotateDirection = 0; idleRotateLength = 1.75; setIdleRotation(); }
  
    void update();
    void setIdleRotation();
  private:
    float idleRotateDirection;
    float idleRotateTimer;
    float idleRotateLength;
    float attackTimer;
};

class SlaveTurretAI : public AI
{
  public:
    SlaveTurretAI() : AI() {}
    SlaveTurretAI(int obj, int tgt, int st, float sigr, float ad, float fov) : AI(obj, tgt, st, sigr, ad, fov) {}
    
    void update();
};

class HunterAI : public KamikazeAI
{
  public:
    HunterAI() : KamikazeAI() { shootTimer = shotBurstLength = angleInaccuracy = inaccuracyTimer = pauseLength = pauseTimer = 0; }
    HunterAI(int obj, int tgt, int st, float sigr, float ad, float fov, float plm, float plr, float pdm, float pdr, float slm, float slr, float sdm, float sdr, float cdist, float aim, float ac) : KamikazeAI(obj, tgt, st, sigr, ad, fov)
    { shootTimer = shotBurstLength = angleInaccuracy = inaccuracyTimer = pauseLength = pauseTimer = 0; pauseLengthMin = plm;
    pauseLengthRand = plr; pauseDelayMin = pdm; pauseDelayRand = pdr; shootLengthMin = slm; shootLengthRand = slr;
    shootDelayMin = sdm; shootDelayRand = sdr; closestDistance = cdist; angleInaccuracyMax = aim; angleCompensation = ac; strafePreference = frand()*1.0-0.5; anger = 1; }
    
    void update();
    void move();
    void lineOfSightBehavior();
    void targetDetect();
    void alertCheck();
    
    // Properties
    float pauseLengthRand, pauseLengthMin;
    float pauseDelayRand, pauseDelayMin;
    float shootLengthRand, shootLengthMin;
    float shootDelayRand, shootDelayMin;
    float closestDistance;
    float angleInaccuracyMax;
    float strafePreference;
    
    float shootTimer;
    float shotBurstLength;
    float angleInaccuracy;
    float angleCompensation;
    float inaccuracyTimer;
    float pauseLength;
    float pauseTimer;
    float anger;
};


#endif
