/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * player.cpp - player data functions
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
#include <string>

void Player::set( int obj )
{
    object = obj;
    headingLock = true;
    zoom = CAMERA_DEFAULT_HEIGHT;

    level = 0;

    dataDefaults();
}

void Player::dataDefaults()
{
    cheated = canAugment = usedAugment = augmented = false;
    resetLevelProgress();
    eraseRecords();
    eraseSavedGame();
}

void Player::eraseRecords()
{
    for( int i = 0; i < NUM_LEVELS + 1; i++ )
    {
        levelRecords[i].time  = -1.0;
        levelRecords[i].kills = -1;
        levelRecords[i].shots = -1;
        levelRecords[i].hits  = -1;
    }
}

void Player::eraseSavedGame()
{
    startingLevel = -1;
    for( int i = 0; i < NUM_LEVELS + 1; i++ )
    {
        currStats[i].time  = -1.0;
        currStats[i].kills = -1;
        currStats[i].shots = -1;
        currStats[i].hits  = -1;
    }
    level = -1;
    cheated = usedAugment = false;
}

void Player::loadPlayerData()
{
    string dir = getenv( "HOME" );
    dir += LOCALSTATEDIR;

    string file = dir + PLAYER_DATA_FILE;

    ifstream datafile( file.c_str() );
    if( !datafile.is_open() )
    {
        printf( "Could not open player data file %s, using default settings.\n", file.c_str() );
        return;
    }

    int start, end, check;
    string line, value;
    int lineNum = 0;
    int cksum;
    while( getline( datafile, line, '\n' ) )
    {
        start = line.find_first_not_of( " \t\n" );
        end = 0;

        if( line != "" )
        {
            value = NEXT_TOK;

            if( lineNum == 0 )
                levelProgress = S2I( value );
            else if( lineNum >= 1 && lineNum <= 1 + NUM_LEVELS )
            {
                levelRecords[lineNum - 1].time = S2F( value );
                value = NEXT_TOK;
                levelRecords[lineNum - 1].kills = S2I( value );
                value = NEXT_TOK;
                levelRecords[lineNum - 1].shots = S2I( value );
                value = NEXT_TOK;
                levelRecords[lineNum - 1].hits =  S2I( value );
            }
            else if( lineNum == 2 + NUM_LEVELS )
                startingLevel = S2I( value );
            else if( lineNum == 3 + NUM_LEVELS )
                level = S2I( value );
            else if( lineNum >= 4 + NUM_LEVELS && lineNum <= 3 + 2 * NUM_LEVELS )
            {
                currStats[lineNum - ( 4 + NUM_LEVELS )].time = S2F( value );
                value = NEXT_TOK;
                currStats[lineNum - ( 4 + NUM_LEVELS )].kills = S2I( value );
                value = NEXT_TOK;
                currStats[lineNum - ( 4 + NUM_LEVELS )].shots = S2I( value );
                value = NEXT_TOK;
                currStats[lineNum - ( 4 + NUM_LEVELS )].hits =  S2I( value );
            }
            else if( lineNum == 4 + 2 * NUM_LEVELS )
                cheated = ( bool )S2I( value );
            else if( lineNum == 5 + 2 * NUM_LEVELS )
                usedAugment = ( bool )S2I( value );
            else if( lineNum == 6 + 2 * NUM_LEVELS )
                canAugment = ( bool )S2I( value );
            else if( lineNum == 7 + 2 * NUM_LEVELS )
                cksum = S2I( value );

            lineNum++;
        }
    }
    datafile.close();

    // Verify the data
    if( !hasGoodChecksum( cksum ) )
    {
        dataDefaults();
        writePlayerData();
        errorMessage( 7, "FLAGRANT SYSTEM ERROR",
                      "Player profile corrupt. Defaults restored." );
    }
}

void Player::writePlayerData()
{
    string dir = getenv( "HOME" );
    dir += LOCALSTATEDIR;

    string file = dir + PLAYER_DATA_FILE;
    ofstream datafile( file.c_str() );

    if( !datafile.is_open() )
    {
        // try creating directory
        mkdir( dir.c_str(), 0777 );
        datafile.clear();
        datafile.open( file.c_str() );
        if( !datafile.is_open() )
        {
            printf( "Could not open player data file %s for writing.\n", file.c_str() );
            return;
        }
    }
    datafile.clear();

    datafile << levelProgress << endl;

    for( int i = 0; i < NUM_LEVELS + 1; i++ )
    {
        datafile << levelRecords[i].time << " " <<
                 levelRecords[i].kills << " " <<
                 levelRecords[i].shots << " " <<
                 levelRecords[i].hits << endl;
    }

    datafile << startingLevel << endl;
    datafile << level << endl;

    for( int i = 0; i < NUM_LEVELS; i++ )
    {
        datafile << currStats[i].time << " " <<
                 currStats[i].kills << " " <<
                 currStats[i].shots << " " <<
                 currStats[i].hits << endl;
    }
    datafile << ( int )cheated << endl;
    datafile << ( int )usedAugment << endl;
    datafile << ( int )canAugment << endl;
    datafile << checksum() << endl;

    datafile.close();
}

bool Player::hasGoodChecksum( int cksum )
{
    // Check data members first
    if( levelProgress < 0 || levelProgress > NUM_LEVELS ) return false;
    if( startingLevel >= NUM_LEVELS ) return false;
    if( level > levelProgress && level != NUM_LEVELS ) return false;

    return ( cksum == checksum() );
}

// Matt's Wild 'n' Crazy Checksum Algorithm!
int Player::checksum()
{
    int sum = 0;
    sum += levelProgress * 17 * ( int )( cos( ( float )levelProgress ) * 13 );

    for( int i = 0; i < NUM_LEVELS + 1; i++ )
    {
        sum += ( int )levelRecords[i].time * ( i + 1 ) * 4;
        sum += levelRecords[i].kills * ( i + 1 ) * 4 + 1;
        sum += levelRecords[i].shots * ( i + 1 ) * 4 + 2;
        sum += levelRecords[i].hits * ( i + 1 ) * 4 + 3;
    }

    sum += startingLevel * 43 * ( int )( sin( ( float )levelProgress ) * 7 );
    sum += level * 37 * ( int )( cos( ( float )levelProgress ) * 19 );

    for( int i = 0; i < NUM_LEVELS; i++ )
    {
        sum += ( int )currStats[i].time * ( i + 11 ) * 4;
        sum += currStats[i].kills * ( i + 11 ) * 4 + 1;
        sum += currStats[i].shots * ( i + 11 ) * 4 + 2;
        sum += currStats[i].hits * ( i + 11 ) * 4 + 3;
    }
    sum += ( int )cheated + 2 * 61;
    sum += ( int )usedAugment + 2 * 17;
    sum += ( int )canAugment + 9 * 43;

    sum = abs( sum );
    return sum;
}

void Player::unlockLevel()
{
    if( level > levelProgress )
        levelProgress = level;
}

void Player::setLevel( int l )
{
    if( l <= levelProgress )
        level = l;
}

void Player::saveLevelData( bool playInSequence )
{
    if( playInSequence )
    {
        int l = ( level < NUM_LEVELS ) ? level : NUM_LEVELS - 1;

        currStats[l].time = gameTime;
        currStats[l].kills = kills;
        currStats[l].shots = shots;
        currStats[l].hits = hits;

        if( !config.cheating() && ( !P.augmented || currLevel.playerAugmented ) )
        {
            if( levelRecords[l].time < 0 || levelRecords[l].time > gameTime )
                levelRecords[l].time = gameTime;
            if( levelRecords[l].kills < 0 || levelRecords[l].kills < kills )
                levelRecords[l].kills = kills;
            if( levelRecords[l].shots < 0 || levelRecords[l].shots > shots )
                levelRecords[l].shots = shots;
            if( levelRecords[l].hits < 0 || levelRecords[l].hits > hits )
                levelRecords[l].hits = hits;
        }
    }
}

float Player::totalTime()
{
    float total = 0.0;

    for( int i = startingLevel; i <= MIN( level, NUM_LEVELS - 1 ); i++ )
    {
        if( currStats[i].time >= 0.0 )
            total += currStats[i].time;
    }

    return total;
}

void Player::computeFinalStatistics()
{
    float tt = 0.0;
    int tk = 0, ts = 0, th = 0;

    for( int i = 0; i < NUM_LEVELS; i++ )
    {
        if( currStats[i].time >= 0.0 ) tt += currStats[i].time;
        if( currStats[i].kills >= 0 ) tk += currStats[i].kills;
        if( currStats[i].shots >= 0 ) ts += currStats[i].shots;
        if( currStats[i].hits >= 0 ) th += currStats[i].hits;
    }
    currStats[NUM_LEVELS].time = tt;
    currStats[NUM_LEVELS].kills = tk;
    currStats[NUM_LEVELS].shots = ts;
    currStats[NUM_LEVELS].hits = th;
}

void Player::saveFinalStatistics()
{
    if( !( cheated || usedAugment ) )
    {
        if( levelRecords[NUM_LEVELS].time < 0 || levelRecords[NUM_LEVELS].time > currStats[NUM_LEVELS].time )
            levelRecords[NUM_LEVELS].time = currStats[NUM_LEVELS].time;
        if( levelRecords[NUM_LEVELS].kills < 0 || levelRecords[NUM_LEVELS].kills < currStats[NUM_LEVELS].kills )
            levelRecords[NUM_LEVELS].kills = currStats[NUM_LEVELS].kills;
        if( levelRecords[NUM_LEVELS].shots < 0 || levelRecords[NUM_LEVELS].shots > currStats[NUM_LEVELS].shots )
            levelRecords[NUM_LEVELS].shots = currStats[NUM_LEVELS].shots;
        if( levelRecords[NUM_LEVELS].hits < 0 || levelRecords[NUM_LEVELS].hits > currStats[NUM_LEVELS].hits )
            levelRecords[NUM_LEVELS].hits = currStats[NUM_LEVELS].hits;

        if( currStats[NUM_LEVELS].time < UNLOCK_TIME && startingLevel == 0 )
            canAugment = true;
    }
}

// Updates the event flags of the player-controlled object
// SDL_GetKeyState() must be called first.
void Player::update()
{
    if( config.cheating() )
        cheated = true;

    if( augmented && !currLevel.playerAugmented )
        usedAugment = true;

    // Set keypresses
    currLevelObjs[object]->ev.setLR( ih.state( KEY_RIGHT ) - ih.state( KEY_LEFT ) );
    currLevelObjs[object]->ev.setFB( ih.state( KEY_FWD ) - ih.state( KEY_BACK ) );
    currLevelObjs[object]->ev.setStrafe( ( int )ih.state( KEY_STRAFE_R ) - ( int )ih.state( KEY_STRAFE_L ) );
    currLevelObjs[object]->ev.setJump( ( int )ih.state( KEY_JUMP ) );
    currLevelObjs[object]->ev.setFire( ( int )ih.state( KEY_FIRE ) );
    currLevelObjs[object]->ev.setGrenade( ( int )ih.state( KEY_GRENADE ) );
    currLevelObjs[object]->ev.setReload( ( int )ih.state( KEY_RELOAD ) );
    currLevelObjs[object]->ev.setSwitchWpn( ( int )ih.state( KEY_SWITCH_WPN ) );

    if( ih.state( KEY_SWITCH_VIEW ) )
    {
        if( hlReady )
        {
            headingLock = ( headingLock ) ? false : true;
            hlReady = false;
        }
    }
    else
        hlReady = true;

    if( ( ih.state( KEY_ZOOM_OUT ) ) )
    {
        zoom += CAMERA_ZOOM_SPEED * timer.dT();
        if( zoom > CAMERA_ZOOM_LIMIT )
            zoom = CAMERA_ZOOM_LIMIT;
    }
    else
    {
        if( zoom > CAMERA_DEFAULT_HEIGHT )
        {
            zoom -= CAMERA_ZOOM_FALL_SPEED * timer.dT();
            if( zoom < CAMERA_DEFAULT_HEIGHT )
                zoom = CAMERA_DEFAULT_HEIGHT;
        }
    }
}
