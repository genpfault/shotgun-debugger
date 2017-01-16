/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * enemies.cpp - enemy AI routines
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

#include "sdb.h"

void AI::set( int st, int obj, int tgt,  float sigr, float ad, float fov )
{
    state = st;
    object = obj;
    target = tgt;
    doSound = true;
    ultimateMoveTarget.set( 0, 0 );
    moveTarget.set( 0, 0 );
    hasMoveTarget = movingToNode = false;
    nodeTarget = -1;
    targetReacquireTimer = -999;
    task = AIT_STOP;
    nodePriorities.clear();

    signalRadius = sigr;
    alertDistance = ad;
    fieldOfVision = fov;

    for( int i = 0; i < currLevel.node.size(); i++ )
        nodePriorities.push_back( 0 );
}

void AI::angleCalculations( bool targetOrNode )
{
    if( targetOrNode )
        angleToTarget = atan2( currLevelObjs[target]->PosY() - currLevelObjs[object]->PosY(),
                               currLevelObjs[target]->PosX() - currLevelObjs[object]->PosX() );
    else
        angleToTarget = atan2( moveTarget.c[Y] - currLevelObjs[object]->PosY(),
                               moveTarget.c[X] - currLevelObjs[object]->PosX() );

    angleDiff = currLevelObjs[object]->LookAngle() - angleToTarget;
    if( angleDiff > PI )
        angleDiff -= 2 * PI;
}

void AI::distanceCalculation( bool targetOrNode )
{
    if( targetOrNode == true )
        distToTarget = dist( currLevelObjs[object]->Pos(), currLevelObjs[target]->Pos() );
    else
        distToTarget = dist( currLevelObjs[object]->Pos(), moveTarget );
}

// returns true if the object can see it, false if there's a wall in the way
bool AI::wallOccludeCalculation()
{
    return hasLineOfSight( currLevelObjs[object]->Pos(), currLevelObjs[target]->Pos(), true );
}

void AI::setNodePriorities( Vector2D pos )
{
    for( int i = 0; i < currLevel.node.size(); i++ )
        nodePriorities[i] = 0;

    for( int i = 0; i < currLevel.node.size(); i++ )
    {
        if( hasLineOfSight( currLevel.node[i].pos, pos, false ) )
        {
            nodePriorities[i] = 10;
            //printf("%d can see this position\n", i);
            setChildNodePriorities( currLevel.node[i].visible, 9 );
        }
    }
}

float AI::getAngle( Vector2D pos )
{
    return atan2( pos.c[Y] - currLevelObjs[object]->PosY(),
                  pos.c[X] - currLevelObjs[object]->PosX() );
}

float AI::getAngleDiff( float a1, float a2 )
{
    float diff = a1 - a2;
    if( diff > PI )
        diff -= 2 * PI;

    return diff;
}

float AI::getDistance( Vector2D pos )
{
    return dist( currLevelObjs[object]->Pos(), pos );
}


void AI::setChildNodePriorities( vector<int> children, int priority )
{
    if( priority == 0 )
        return;

    for( int i = 0; i < children.size(); i++ )
    {
        if( priority > nodePriorities[children[i]] )
        {
            nodePriorities[children[i]] = priority;
            setChildNodePriorities( currLevel.node[children[i]].visible, priority - 1 );
        }
    }
}

void AI::setMoveTarget( Vector2D tgtPos, int tsk )
{
    //state = MOVING;
    ultimateMoveTarget = tgtPos;
    hasMoveTarget = true;
    task = tsk;

    setNodePriorities( tgtPos );
    updateMoveTarget();
}

void AI::updateMoveTarget()
{
    //printf("Old target %d: ", nodeTarget);
    // If we have line-of-sight with the ultimate target, target it
    if( hasLineOfSight( currLevelObjs[object]->Pos(), ultimateMoveTarget, false ) )
    {
        //printf("LOS\n");
        moveTarget = ultimateMoveTarget;
    }
    // If not, look at the visible nodes. Find the closest node of the highest priority
    // and set that as the target. If no priority nodes are found, abort the movement.
    else
    {
        float closestDistance = 1e17, distToNode;
        int closestNode = 0;
        int highestPriority = 0;
        vector<int> visibleNodes;

        // Find visible nodes
        for( int i = 0; i < currLevel.node.size(); i++ )
        {
            if( hasLineOfSight( currLevelObjs[object]->Pos(), currLevel.node[i].pos, false ) && i != nodeTarget )
            {
                visibleNodes.push_back( i );
                if( nodePriorities[i] > highestPriority ) highestPriority = nodePriorities[i];
            }
        }

        // Die if we didn't find any good nodes, or no nodes are visible
        if( highestPriority == 0 )
        {
            hasMoveTarget = false;
            state = AWAKE;
            return;
        }

        // Search nodes of highest priority and find closest "prime" node
        for( int i = 0; i < visibleNodes.size(); i++ )
        {
            if( nodePriorities[visibleNodes[i]] == highestPriority )
            {
                distToNode = dist( currLevelObjs[object]->Pos(), currLevel.node[visibleNodes[i]].pos );
                if( distToNode < closestDistance )
                {
                    closestDistance = distToNode;
                    closestNode = visibleNodes[i];
                }
            }
        }

        // We've got it; set this node's position as our new target.
        moveTarget = currLevel.node[closestNode].pos;

        nodeTarget = closestNode;
        //printf("Heading to node %d\n", closestNode);
    }
}

void AI::sendAlertSignal( double dst )
{
    for( int i = 0; i < currLevelEnemies.size(); i++ )
    {
        if( dist( currLevelObjs[currLevelEnemies[i]->Object()]->Pos(), currLevelObjs[object]->Pos() ) < dst )
        {
            if( hasLineOfSight( currLevelObjs[object]->Pos(), currLevelObjs[currLevelEnemies[i]->Object()]->Pos(), true ) )
                currLevelEnemies[i]->setState( ATTACKING );
        }
    }
}

void KamikazeAI::move()
{
    if( !currLevelObjs[object]->Stunned() )
    {
        currLevelObjs[object]->ev.setFB( 1 );
        currLevelObjs[object]->setLookAngle( angleToTarget );
    }
}

void KamikazeAI::idleAnimations()
{
    if( state != ATTACKING && !currLevelObjs[object]->Stunned() )
    {
        /*idleActionTimer -= timer.dT();
        if (idleActionTimer <= 0)
        {
          currLevelObjs[object]->setAnimation(MA_SPECIAL1+rand()%3, ANIM_TRANSIENT);
          idleActionTimer = frand()*5.0+2.0;
        }*/
    }
}

void KamikazeAI::alertCheck()
{
    // If we were alerted, turn in that direction //
    if( currLevelObjs[object]->DirectionAlertType() != ALERT_NONE )
    {
        float ang = currLevelObjs[object]->DirectionAlert();
        currLevelObjs[object]->setLookAngle( ang );

        if( currLevelObjs[object]->DirectionAlertType() != ALERT_TOUCH && currLevelObjs[object]->Type() != ENT_MIB )
            if( rand() % 5 == 0 ) playSound( SND_ROBOT_ALERT, 1 );

        currLevelObjs[object]->setDirectionAlert( NO_ALERT, ALERT_NONE );
    }
}

void KamikazeAI::lineOfSightBehavior()
{
    angleCalculations( true );
    distanceCalculation( true );
    hasMoveTarget = true;
    moveTarget = currLevelObjs[target]->Pos();
    movingToNode = false;

    attack();
}

void KamikazeAI::attack()
{
    if( !currLevelObjs[object]->Stunned() )
    {
        float killDist = ( currLevelObjs[object]->Type() == ENT_MIB ) ? 3 : 5;
        if( dist( currLevelObjs[object]->Pos(), currLevelObjs[target]->Pos() ) < 15 &&
                !currLevelObjs[object]->Dying() && currLevelObjs[object]->Type() != ENT_MIB )
            currLevelObjs[object]->setAnimation( MA_SHORT, ANIM_TRANSIENT );
        if( dist( currLevelObjs[object]->Pos(), currLevelObjs[target]->Pos() ) < killDist &&
                !currLevelObjs[object]->Dying() )
        {
            if( currLevelObjs[object]->Type() == ENT_MIB )
                setTrigger( -3 );
            else
                currLevelObjs[object]->kill( true );
        }
    }
}

void KamikazeAI::nodeFollowBehavior()
{
    if( !movingToNode )
    {
        targetReacquireTimer = 2.0;
        int closest = currLevel.closestNode( currLevelObjs[target]->Pos() );
        setMoveTarget( currLevel.node[closest].pos, AIT_ATTACK );

        movingToNode = true;
    }
    else
    {
        targetReacquireTimer -= timer.dT();

        if( targetReacquireTimer <= 0 )
        {
            int closest = currLevel.closestNode( currLevelObjs[target]->Pos() );
            setMoveTarget( currLevel.node[closest].pos, AIT_ATTACK );
            targetReacquireTimer = 2.0;
        }
    }

    // If we hit our destination node, find the next one.
    // Note that as soon as we have line of sight with the target, we abandon
    // the node path. If we lose line of sight again, we calculate
    // another path and start over.
    distanceCalculation( false );
    if( distToTarget < 1 )
    {
        //printf("Got to target node %d\n", nodeTarget);
        updateMoveTarget();
    }
}

void KamikazeAI::targetDetect()
{
    if( currLevelObjs[target]->isActive() && ( distToTarget < alertDistance ) && ( fabs( angleDiff ) < torad( fieldOfVision ) ) && wallOccludeCalculation() )
    {
        sendAlertSignal( signalRadius );
        state = ATTACKING;
    }
}


void KamikazeAI::update()
{
    if( target > -1 && currLevelObjs[object]->isActive() && !currLevelObjs[object]->Stunned() )
    {
        idleAnimations();

        // Trigger detection //
        if( triggers[currLevelObjs[object]->RespondTo1()].hit )
        {
            if( state == ASLEEP )
                state = AWAKE;
            else
                state = ATTACKING;
        }

        // Make calculations //
        angleCalculations( true );
        distanceCalculation( true );
        if( state != ASLEEP )
        {
            if( hasMoveTarget )
            {
                angleCalculations( false );
                distanceCalculation( false );
                move();
            }
            else
                alertCheck();

            if( state == ATTACKING )
            {
                if( doSound || currLevelObjs[object]->Type() != ENT_MIB )
                {
                    playSound( SND_ROBOT_SEES, 1 );
                    doSound = false;
                }

                // If we can see the target, head straight for 'im!
                if( hasLineOfSight( currLevelObjs[object]->Pos(), currLevelObjs[target]->Pos(), true ) )
                    lineOfSightBehavior();
                // If we're lost line of sight, calculate a node path
                else if( currLevel.node.size() > 0 )
                    nodeFollowBehavior();
            }
            else
                targetDetect();
        }
    }
}

void TurretAI::setIdleRotation()
{
    idleRotateTimer = idleRotateLength;
    if( !idleRotateDirection ) idleRotateTimer *= 0.5;
    idleRotateDirection = 0.5 * ( ( idleRotateDirection < 0 ) ? 1.0 : -1.0 );
}

void TurretAI::update()
{
    if( target > -1 && currLevelObjs[object]->isActive() && !currLevelObjs[object]->Stunned() )
    {
        angleCalculations( true );
        distanceCalculation( true );
        if( state == AWAKE )
        {
            currLevelObjs[object]->ev.setLR( idleRotateDirection );
            idleRotateTimer -= timer.dT();
            if( idleRotateTimer <= 0 )
                setIdleRotation();
            currLevelObjs[object]->ev.setFire( 0 );
            if( !doSound ) doSound = true;


            if( currLevelObjs[target]->isActive() && distToTarget < alertDistance && fabs( angleDiff ) < torad( fieldOfVision ) && wallOccludeCalculation() )
                state = ATTACKING;
        }

        if( currLevelObjs[object]->DirectionAlertType() != ALERT_NONE )
        {
            float ang = currLevelObjs[object]->DirectionAlert();
            setMoveTarget( currLevelObjs[object]->Pos() + Vector2D( cos( ang ) * 2, sin( ang ) * 2 ), AIT_INVESTIGATE );
            state = ATTACKING;
            attackTimer = 0;
            currLevelObjs[object]->setDirectionAlert( NO_ALERT, ALERT_NONE );
        }

        if( state == ATTACKING )
        {
            if( hasMoveTarget )
            {
                float moveTargetAngleDiff = getAngleDiff( currLevelObjs[object]->LookAngle(), getAngle( moveTarget ) );

                currLevelObjs[object]->ev.setLR( 2 * sign( moveTargetAngleDiff ) );

                currLevelObjs[object]->ev.setFire( 1 );

                attackTimer += timer.dT();
                if( attackTimer >= 3.0 )
                {
                    if( idleRotateLength < 5 ) idleRotateLength += 0.75;
                    state = AWAKE;
                    hasMoveTarget = false;
                    attackTimer = 0;
                }
                if( currLevelObjs[target]->isActive() && distToTarget < alertDistance && fabs( angleDiff ) < torad( fieldOfVision ) &&  wallOccludeCalculation() )
                    hasMoveTarget = false;
            }
            else
            {
                if( angleDiff < torad( 5 ) )
                    currLevelObjs[object]->ev.setLR( -1 );
                else if( angleDiff > torad( 5 ) )
                    currLevelObjs[object]->ev.setLR( 1 );
                else
                    currLevelObjs[object]->setLookAngle( angleToTarget );
                currLevelObjs[object]->ev.setFire( 1 );

                if( doSound )
                {
                    playSound( SND_ROBOT_SEES, 1 );
                    doSound = false;
                }

                if( !( currLevelObjs[target]->isActive() && distToTarget < alertDistance && fabs( angleDiff ) < torad( fieldOfVision ) && wallOccludeCalculation() ) )
                    state = AWAKE;
            }
        }
    }
}

void SlaveTurretAI::update()
{
    if( target > -1 && currLevelObjs[object]->isActive() && !currLevelObjs[object]->Stunned() )
    {
        if( triggers[currLevelObjs[object]->RespondTo1()].hit )
        {
            state = ATTACKING;
            currLevelObjs[object]->ev.setFire( 1 );
        }
        else
        {
            state = AWAKE;
            currLevelObjs[object]->ev.setFire( 0 );
        }
    }
}

void HunterAI::alertCheck()
{
    // If we were alerted, turn in that direction //
    if( currLevelObjs[object]->DirectionAlertType() != ALERT_NONE )
    {
        float ang = currLevelObjs[object]->DirectionAlert();
        currLevelObjs[object]->setLookAngle( ang );
        anger += 0.03 * currLevelObjs[object]->LastDamageAmount();

        if( currLevelObjs[object]->DirectionAlertType() == ALERT_SHOT )
        {
            if( rand() % 2 ) playSound( SND_ROBOT_ALERT, 1 );
        }

        currLevelObjs[object]->setDirectionAlert( NO_ALERT, ALERT_NONE );
    }
}

void HunterAI::targetDetect()
{
    if( currLevelObjs[target]->isActive() && ( distToTarget < alertDistance ) && ( fabs( angleDiff ) < torad( fieldOfVision ) ) && wallOccludeCalculation() )
    {
        sendAlertSignal( signalRadius );
        state = ATTACKING;
    }
}

void HunterAI::update()
{
    if( target > -1 && currLevelObjs[object]->isActive() && !currLevelObjs[object]->Stunned() )
    {
        idleAnimations();

        // Trigger detection //
        if( triggers[currLevelObjs[object]->RespondTo1()].hit )
            state = ATTACKING;

        // Make calculations //
        angleCalculations( true );
        distanceCalculation( true );

        if( state != ASLEEP )
        {
            if( hasMoveTarget )
            {
                angleCalculations( false );
                distanceCalculation( false );
                move();

                if( distToTarget <= 1.0 && task == AIT_INVESTIGATE )
                    hasMoveTarget = false;
            }

            alertCheck();

            if( state == ATTACKING )
            {
                if( doSound )
                {
                    playSound( SND_ROBOT_SEES, 1 );
                    doSound = false;
                }

                // If we can see the target, head straight for 'im!
                if( hasLineOfSight( currLevelObjs[object]->Pos(), currLevelObjs[target]->Pos(), true ) )
                    lineOfSightBehavior();
                // If we're lost line of sight, calculate a node path
                else if( currLevel.node.size() > 0 )
                {
                    currLevelObjs[object]->ev.setFire( 0 );
                    nodeFollowBehavior();
                }

                inaccuracyTimer -= timer.dT();
                if( inaccuracyTimer <= 0 )
                {
                    angleInaccuracy = torad( frand() * angleInaccuracyMax ) * anger;
                    inaccuracyTimer = ( frand() * 0.5 ) / anger;
                }

                pauseTimer -= timer.dT();
                if( pauseTimer <= 0 )
                {
                    pauseLength = ( frand() * pauseLengthRand + pauseLengthMin ) / anger;
                    pauseTimer = ( frand() * pauseDelayRand + pauseDelayMin ) / anger;
                }

            }
            else
                targetDetect();


            if( anger > 5.0 )
                anger = 5.0;

            anger -= 0.05 * timer.dT();

            if( anger < 1.0 )
                anger = 1.0;
        }
    }
}

void HunterAI::lineOfSightBehavior()
{
    angleCalculations( true );
    distanceCalculation( true );
    hasMoveTarget = true;
    moveTarget = currLevelObjs[target]->Pos();
    movingToNode = false;

    shootTimer -= timer.dT();
    if( shootTimer <= 0 )
    {
        shootTimer = ( frand() * shootDelayRand + shootDelayMin ) / anger;
        shotBurstLength = ( frand() * shootLengthRand + shootLengthMin ) * anger;
    }

    if( shotBurstLength > 0 )
    {
        currLevelObjs[object]->ev.setFire( 1 );
        shotBurstLength -= timer.dT();
    }
    else
    {
        currLevelObjs[object]->ev.setFire( 0 );
        shotBurstLength = 0;
    }
}

void HunterAI::move()
{
    if( !currLevelObjs[object]->Stunned() )
    {
        if( pauseLength > 0 )
        {
            currLevelObjs[object]->ev.setFB( 0 );
            currLevelObjs[object]->ev.setStrafe( 0 );
            pauseLength -= timer.dT();
        }
        else
        {
            if( distToTarget >= closestDistance / anger )
                currLevelObjs[object]->ev.setFB( 1 * anger );
            else
                currLevelObjs[object]->ev.setFB( 0 );

            pauseLength = 0;
        }

        if( currLevelObjs[object]->Type() == ENT_HUNTER )
        {
            if( distToTarget < closestDistance )
                currLevelObjs[object]->ev.setStrafe( strafePreference );
        }

        currLevelObjs[object]->setLookAngle( angleToTarget + angleCompensation + angleInaccuracy );
    }
}
