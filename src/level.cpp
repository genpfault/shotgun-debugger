/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * level.cpp - level loading and managing functions
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
#include <cmath>
#include <string>

string fetchLevelName( char* file )
{
    string n = "";

    if( strcmp( file, "" ) == 0 )
        return n;

    string filename = file;
    if( filename.rfind( ".vl", filename.size() - 1 ) == string::npos )
        filename += ".vl";

    ifstream levelfile( filename.c_str() );

    if( !levelfile.is_open() )
        errorMessage( 3, "FLAGRANT SYSTEM ERROR", "Could not open level file %s", filename.c_str() );

    // The title should be on the very first line
    int start, end, lineLength;
    string line, token;
    if( getline( levelfile, line, '\n' ) )
    {
        start = line.find_first_not_of( " \t\n" );
        end = 0;

        if( line != "" )
        {
            token = NEXT_TOK;
            if( token != "vl" )
                errorMessage( 4, "FLAGRANT SYSTEM ERROR", "%s has an invalid header", filename.c_str() );
            else
                n = line.substr( start );
        }
    }
    levelfile.close();

    return n;
}

// Loads a level file; a .vl extension will be appended if none given.
void Level::loadFromFile( char* file )
{
    defaults();
    readLevelData( file );
    calculateLighting();
    calculateGrid();
    calculateNodeGraph();
    makeDisplayList();
}

void Level::readLevelData( char* file )
{
    if( strcmp( file, "" ) == 0 )
        errorMessage( 3, "FLAGRANT SYSTEM ERROR", "No level file specified" );

    // Append the .vl file extension if not present
    string filename = file;
    if( filename.rfind( ".vl", filename.size() - 1 ) == string::npos )
        filename += ".vl";

    ifstream levelfile( filename.c_str() );

    if( !levelfile.is_open() )
        errorMessage( 3, "FLAGRANT SYSTEM ERROR", "Could not load level file %s", filename.c_str() );

    //printf("Loaded file %s\n", filename.c_str());
    string line, token, token2, tR, tG, tB, tA, tX, tY, tX2, tY2,
           tC1, tC2, tC3, tC4, tC5, tC6, tC7, tC8, tC9, tC10, tC11, tC12, tC13, tC14, tC15, tC16, tC17, tC18;

    Object::resetIDCounter();
    vector<Wall> tempWalls;
    Surface tempWallPolygon;
    int numWalls = 0;
    int lineCount = 1, start, end, lineLength;

    while( getline( levelfile, line, '\n' ) )
    {
        start = line.find_first_not_of( " \t\n" );
        end = 0;

        // Parse the line
        if( line != "" )
        {
            // Read the first token
            token = NEXT_TOK;

            // If the very first token isn't "vl", this is a phony level file
            if( lineCount == 1 && token != "vl" )
                errorMessage( 4, "FLAGRANT SYSTEM ERROR", "%s has an invalid header", filename.c_str() );

            // Decide what to do with the line
            if( token == "vl" )
                // vl specifies the level's name; just give it the rest of the string.
                name = line.substr( start );
            else if( token == "by" )
                author = line.substr( start );
            else if( token == "bg" )
            {
                // bg specifies the background color, requires 3 float values
                tR = NEXT_TOK;
                tG = NEXT_TOK;
                tB = NEXT_TOK;

                // make sure we got all three values
                if( tR == "" || tG == "" || tB == "" )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "'bg' needs 3 values (%s:%d)", filename.c_str(), lineCount );

                bg.set( S2F( tR ), S2F( tG ), S2F( tB ), 1.0 );
            }

            else if( token == "grid" )
            {
                tX = NEXT_TOK;
                tY = NEXT_TOK;
                tX2 = NEXT_TOK;
                tY2 = NEXT_TOK;
                tC1 = NEXT_TOK;

                if( tX == "" || tY == "" || tX2 == "" || tY2 == "" || tC1 == "" )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "'grid' needs 5 values (%s:%d)", filename.c_str(), lineCount );

                gridLL.set( S2F( tX ), S2F( tY ) );
                gridUR.set( S2F( tX2 ), S2F( tY2 ) );
                gridCellSize = S2F( tC1 );
            }
            else if( token == "ambient" )
            {
                tR = NEXT_TOK;
                tG = NEXT_TOK;
                tB = NEXT_TOK;
                tA = NEXT_TOK;
                tC1 = NEXT_TOK;
                if( tR == "" || tG == "" || tB == "" || tA == "" || tC1 == "" )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "'ambient' needs 5 values (%s:%d)", filename.c_str(), lineCount );

                ambLight.set( S2F( tR ), S2F( tG ), S2F( tB ), S2F( tA ) );
                ambLightDirection = torad( S2F( tC1 ) );
            }
            else if( token == "player" )
            {
                // player position requires 2 float values
                tX = NEXT_TOK;
                tY = NEXT_TOK;
                tC1 = NEXT_TOK;
                tC2 = NEXT_TOK;

                if( tX == "" || tY == "" || tC1 == "" )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "'player' needs 3 values (%s:%d)", filename.c_str(), lineCount );

                playerStart.set( S2F( tX ), S2F( tY ) );
                playerStartHeading = torad( S2F( tC1 ) );
                playerAugmented = ( bool )S2I( tC2 );
            }
            else if( token == "walls" )
            {
                tR = NEXT_TOK;
                tG = NEXT_TOK;
                tB  = NEXT_TOK;
                tA  = NEXT_TOK;
                tC1 = NEXT_TOK;
                tC2 = NEXT_TOK;
                tC3 = NEXT_TOK;
                tC4 = NEXT_TOK;
                tC5 = NEXT_TOK;
                tC6 = NEXT_TOK;
                tC7 = NEXT_TOK;
                tC8 = NEXT_TOK;
                tC9 = NEXT_TOK;
                tX = NEXT_TOK;
                tY = NEXT_TOK;
                tX2 = NEXT_TOK;
                tY2 = NEXT_TOK;

                if( tR == "" || tG == "" | tB == "" || tA == "" || tC1 == "" || tC2 == "" || tC3 == "" || tC4 == "" || tC5 == "" || tC6 == "" || tC7 == "" || tC8 == "" || tC9 == "" || tX == "" || tY == "" || tX2 == "" || tY2 == "" )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "'walls' needs >= 17 values (%s:%d)", filename.c_str(), lineCount );

                if( S2I( tC5 ) < 0 || S2I( tC5 ) > MAX_PART )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "Invalid particle type '%d' (%s:%d)", S2I( tC5 ), filename.c_str(), lineCount );

                if( S2I( tC8 ) < -4 || S2I( tC8 ) >= NUM_TRIGGERS )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "Trigger < -4 or > %d (%s:%d)", NUM_TRIGGERS - 1, filename.c_str(), lineCount );
                if( S2I( tC8 ) > maxTrigger ) maxTrigger = S2I( tC8 );

                if( S2I( tC9 ) < 0 || S2I( tC9 ) > MAX_TRIGTYPE )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "Invalid trigger type (%s:%d)", filename.c_str(), lineCount );
                if( S2I( tC9 ) & TRIG_ADD && S2I( tC8 ) > 0 ) triggers[S2I( tC8 )].hitsRequired++;

                addWall( S2F( tX ), S2F( tY ), S2F( tX2 ), S2F( tY2 ), S2F( tR ), S2F( tG ), S2F( tB ), S2F( tA ), S2F( tC1 ), S2F( tC3 ), S2F( tC4 ), S2I( tC5 ), S2F( tC6 ), ( bool )S2I( tC2 ), S2I( tC7 ), S2I( tC8 ), S2I( tC9 ) );

                tempWalls.clear();
                tempWallPolygon.vert.clear();
                tempWalls.push_back( Wall( S2F( tX ), S2F( tY ), S2F( tX2 ), S2F( tY2 ), 0, 0, 0, 0, 0, 0, 0, 0, 0, false, 0, 0, 0 ) );
                tempWallPolygon.addVert( S2F( tX2 ), S2F( tY2 ) );
                float tmpX = S2F( tX2 ), tmpY = S2F( tY2 );

                while( start != -1 )
                {
                    tX = NEXT_TOK;

                    if( start != -1 )
                        tY = NEXT_TOK;
                    else
                        errorMessage( 6, "FLAGRANT SYSTEM ERROR", "X without Y in 'walls' (%s:%d)", filename.c_str(), lineCount );

                    addWallFromLast( S2F( tX ), S2F( tY ), S2F( tR ), S2F( tG ), S2F( tB ), S2F( tA ), S2F( tC1 ), S2F( tC3 ), S2F( tC4 ), S2I( tC5 ), S2F( tC6 ), ( bool )S2I( tC2 ), S2I( tC7 ), S2I( tC8 ), S2I( tC9 ) );
                    tempWalls.push_back( Wall( tmpX, tmpY, S2F( tX ), S2F( tY ), 0, 0, 0, 0, 0, 0, 0, 0, 0, false, 0, 0, 0 ) );
                    tempWallPolygon.addVert( S2F( tX ), S2F( tY ) );
                    tmpX = S2F( tX );
                    tmpY = S2F( tY );
                }

                // Automatic Normal Reckoning (tm) and (C) 2005
                for( int i = 0; i < tempWalls.size(); i++ )
                {
                    Vector2D normalTestPoint = tempWalls[i].midpoint() + ( tempWalls[i].normal * 0.0001 );
                    if( tempWallPolygon.pointInPolygon( normalTestPoint ) )
                        wall[numWalls + i].normal.neg();
                }
                numWalls += tempWalls.size();
            }
            else if( token == "door" )
            {
                tR = NEXT_TOK;
                tG = NEXT_TOK;
                tB  = NEXT_TOK;
                tA  = NEXT_TOK;
                tC1 = NEXT_TOK;
                tC2 = NEXT_TOK;
                tC3 = NEXT_TOK;
                tC4 = NEXT_TOK;
                tC5 = NEXT_TOK;
                tC6 = NEXT_TOK;
                tC7 = NEXT_TOK;
                tC8 = NEXT_TOK;
                tC9 = NEXT_TOK;
                tC10 = NEXT_TOK;
                tC11 = NEXT_TOK;
                tC12 = NEXT_TOK;
                tC13 = NEXT_TOK;
                tC14 = NEXT_TOK;
                tC15 = NEXT_TOK;
                tC16 = NEXT_TOK;
                tC17 = NEXT_TOK;
                tC18 = NEXT_TOK;
                tX = NEXT_TOK;
                tY = NEXT_TOK;
                tX2 = NEXT_TOK;
                tY2 = NEXT_TOK;


                if( tR == "" || tG == "" | tB == "" || tA == "" || tC1 == "" || tC2 == "" || tC3 == "" || tC4 == "" || tC5 == "" || tC6 == "" ||
                        tC7 == "" || tC8 == "" || tC9 == "" || tC10 == "" || tC11 == "" || tC12 == "" || tC13 == "" || tC14 == "" || tC15 == "" || tC16 == "" || tX == "" || tY == "" || tX2 == "" || tY2 == "" )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "'door' needs 23 values (%s:%d)", filename.c_str(), lineCount );

                if( S2I( tC11 ) < 0 || S2I( tC11 ) > MAX_PART )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "Invalid particle type '%d' (%s:%d)", S2I( tC9 ), filename.c_str(), lineCount );

                if( S2I( tC6 ) < 0 || S2I( tC6 ) >= NUM_TRIGGERS )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "Unlockwhen < 0 or > %d (%s:%d)", NUM_TRIGGERS - 1, filename.c_str(), lineCount );
                if( S2I( tC6 ) > maxTrigger ) maxTrigger = S2I( tC6 );

                if( S2I( tC7 ) < 0 || S2I( tC7 ) >= NUM_TRIGGERS )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "Openwhen < 0 or > %d (%s:%d)", NUM_TRIGGERS - 1, filename.c_str(), lineCount );
                if( S2I( tC7 ) > maxTrigger ) maxTrigger = S2I( tC7 );

                if( S2I( tC8 ) < 0 || S2I( tC8 ) >= NUM_TRIGGERS )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "Jamwhen < 0 or > %d (%s:%d)", NUM_TRIGGERS - 1, filename.c_str(), lineCount );
                if( S2I( tC8 ) > maxTrigger ) maxTrigger = S2I( tC8 );

                if( S2I( tC17 ) < -4 || S2I( tC17 ) >= NUM_TRIGGERS )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "Trigger < -4 or > %d (%s:%d)", NUM_TRIGGERS - 1, filename.c_str(), lineCount );
                if( S2I( tC17 ) > maxTrigger ) maxTrigger = S2I( tC17 );

                if( S2I( tC18 ) < 0 || S2I( tC18 ) > MAX_TRIGTYPE )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "Invalid trigger type (%s:%d)", filename.c_str(), lineCount );
                if( S2I( tC18 ) & TRIG_ADD && S2I( tC18 ) > 0 ) triggers[S2I( tC17 )].hitsRequired++;

                addDoor( S2F( tX ), S2F( tY ), S2F( tX2 ), S2F( tY2 ), S2F( tR ), S2F( tG ), S2F( tB ), S2F( tA ),
                         S2F( tC10 ), S2F( tC12 ), S2F( tC13 ), S2I( tC14 ), S2F( tC15 ), ( bool )S2I( tC11 ), S2I( tC16 ), S2I( tC2 ), S2I( tC3 ),
                         S2F( tC1 ), S2I( tC4 ), S2I( tC5 ), S2I( tC6 ), S2I( tC7 ), S2I( tC8 ), S2I( tC9 ), S2I( tC17 ), S2I( tC18 ) );
            }
            else if( token == "floor" || token == "ceil" || token == "surf" || token == "mask" )
            {
                tR = NEXT_TOK;
                tG = NEXT_TOK;
                tB  = NEXT_TOK;
                tA  = NEXT_TOK;

                if( token == "floor" )
                {
                    tC1 = NEXT_TOK;
                    tC2 = NEXT_TOK;
                    tC3 = NEXT_TOK;
                    tC4 = NEXT_TOK;
                    tC5 = NEXT_TOK;
                    tX = NEXT_TOK;
                    tY = NEXT_TOK;
                    tX2 = NEXT_TOK;
                    tY2 = NEXT_TOK;
                    if( tR == "" || tG == "" | tB == "" || tA == "" || tC1 == "" || tC2 == "" || tC3 == "" || tC4 == "" || tC5 == "" || tX == "" || tY == "" || tX2 == "" || tY2 == "" )
                        errorMessage( 5, "FLAGRANT SYSTEM ERROR", "'%s' needs >= 14 values (%s:%d)", token.c_str(), filename.c_str(), lineCount );

                    if( S2I( tC4 ) < -4 || S2I( tC4 ) >= NUM_TRIGGERS )
                        errorMessage( 5, "FLAGRANT SYSTEM ERROR", "Trigger < -4 or > %d (%s:%d)", NUM_TRIGGERS - 1, filename.c_str(), lineCount );
                    if( S2I( tC4 ) > maxTrigger ) maxTrigger = S2I( tC4 );

                    if( S2I( tC5 ) < 0 || S2I( tC5 ) > MAX_TRIGTYPE )
                        errorMessage( 5, "FLAGRANT SYSTEM ERROR", "Invalid trigger type (%s:%d)", filename.c_str(), lineCount );
                    if( S2I( tC5 ) & TRIG_ADD && S2I( tC4 ) > 0 ) triggers[S2I( tC4 )].hitsRequired++;

                    addFloor( S2F( tR ), S2F( tG ), S2F( tB ), S2F( tA ), S2F( tC1 ), ( bool )S2I( tC2 ), S2I( tC3 ), S2I( tC4 ), S2I( tC5 ) );
                    addVertToNewestFloor( S2F( tX ), S2F( tY ) );
                    addVertToNewestFloor( S2F( tX2 ), S2F( tY2 ) );
                }
                else if( token == "ceil" )
                {
                    tC1 = NEXT_TOK;
                    tC2 = NEXT_TOK;
                    tX = NEXT_TOK;
                    tY = NEXT_TOK;
                    tX2 = NEXT_TOK;
                    tY2 = NEXT_TOK;
                    if( tR == "" || tG == "" | tB == "" || tA == "" || tC1 == "" || tC2 == "" || tX == "" || tY == "" || tX2 == "" || tY2 == "" )
                        errorMessage( 5, "FLAGRANT SYSTEM ERROR", "'%s' needs >= 10 values (%s:%d)", token.c_str(), filename.c_str(), lineCount );

                    addCeiling( S2F( tR ), S2F( tG ), S2F( tB ), S2F( tA ), S2F( tC1 ), ( bool )S2I( tC2 ) );
                    addVertToNewestCeiling( S2F( tX ), S2F( tY ) );
                    addVertToNewestCeiling( S2F( tX2 ), S2F( tY2 ) );
                }
                else if( token == "surf" )
                {
                    tC1 = NEXT_TOK;
                    tC2 = NEXT_TOK;
                    tC3 = NEXT_TOK;
                    tC4 = NEXT_TOK;
                    tC5 = NEXT_TOK;
                    tC6 = NEXT_TOK;
                    tC7 = NEXT_TOK;
                    tC8 = NEXT_TOK;
                    tX = NEXT_TOK;
                    tY = NEXT_TOK;
                    tX2 = NEXT_TOK;
                    tY2 = NEXT_TOK;
                    if( tR == "" || tG == "" | tB == "" || tA == "" || tC1 == "" || tC2 == "" || tC3 == "" || tC4 == "" || tC5 == "" || tC6 == "" || tC7 == "" || tC8 == "" || tX == "" || tY == "" || tX2 == "" || tY2 == "" )
                        errorMessage( 5, "FLAGRANT SYSTEM ERROR", "'%s' needs >= 15 values (%s:%d)", token.c_str(), filename.c_str(), lineCount );

                    if( S2I( tC3 ) < 0 || S2I( tC3 ) > MAX_PART )
                        errorMessage( 5, "FLAGRANT SYSTEM ERROR", "Invalid particle type '%d' (%s:%d)", S2I( tC3 ), filename.c_str(), lineCount );

                    if( S2I( tC7 ) < -4 || S2I( tC7 ) >= NUM_TRIGGERS )
                        errorMessage( 5, "FLAGRANT SYSTEM ERROR", "Trigger < -4 or > %d (%s:%d)", NUM_TRIGGERS - 1, filename.c_str(), lineCount );
                    if( S2I( tC7 ) > maxTrigger ) maxTrigger = S2I( tC7 );

                    if( S2I( tC8 ) < 0 || S2I( tC8 ) > MAX_TRIGTYPE )
                        errorMessage( 5, "FLAGRANT SYSTEM ERROR", "Invalid trigger type (%s:%d)", filename.c_str(), lineCount );
                    if( S2I( tC8 ) & TRIG_ADD && S2I( tC7 ) > 0 ) triggers[S2I( tC7 )].hitsRequired++;

                    addSurface( S2F( tR ), S2F( tG ), S2F( tB ), S2F( tA ), S2F( tC1 ), S2F( tC2 ), S2I( tC3 ), S2F( tC4 ), ( bool )S2I( tC5 ), S2I( tC6 ), S2I( tC7 ), S2I( tC8 ) );
                    addVertToNewestSurface( S2F( tX ), S2F( tY ) );
                    addVertToNewestSurface( S2F( tX2 ), S2F( tY2 ) );
                }
                else if( token == "mask" )
                {
                    tC1 = NEXT_TOK;
                    tC2 = NEXT_TOK;
                    tC3 = NEXT_TOK;
                    tC4 = NEXT_TOK;
                    tC5 = NEXT_TOK;
                    tX = NEXT_TOK;
                    tY = NEXT_TOK;
                    tX2 = NEXT_TOK;
                    tY2 = NEXT_TOK;
                    if( tR == "" || tG == "" | tB == "" || tA == "" || tC1 == "" || tC2 == "" || tC3 == "" || tC4 == "" || tC5 == "" || tX == "" || tY == "" || tX2 == "" || tY2 == "" )
                        errorMessage( 5, "FLAGRANT SYSTEM ERROR", "'%s' needs >= 13 values (%s:%d)", token.c_str(), filename.c_str(), lineCount );

                    if( S2I( tC5 ) < -4 || S2I( tC5 ) >= NUM_TRIGGERS )
                        errorMessage( 5, "FLAGRANT SYSTEM ERROR", "Openwhen < -4 or > %d (%s:%d)", NUM_TRIGGERS - 1, filename.c_str(), lineCount );

                    addMask( S2F( tR ), S2F( tG ), S2F( tB ), S2F( tA ), S2F( tC1 ), ( bool )S2I( tC3 ), S2F( tC2 ), ( bool )S2I( tC4 ), S2I( tC5 ) );
                    addVertToNewestMask( S2F( tX ), S2F( tY ) );
                    addVertToNewestMask( S2F( tX2 ), S2F( tY2 ) );
                }

                while( start != -1 )
                {
                    tX = NEXT_TOK;

                    if( start != -1 )
                        tY = NEXT_TOK;
                    else
                        errorMessage( 6, "FLAGRANT SYSTEM ERROR", "X without Y in '%s' (%s:%d)", token.c_str(), filename.c_str(), lineCount );

                    if( token == "floor" )
                        addVertToNewestFloor( S2F( tX ), S2F( tY ) );
                    else if( token == "ceil" )
                        addVertToNewestCeiling( S2F( tX ), S2F( tY ) );
                    else if( token == "surf" )
                        addVertToNewestSurface( S2F( tX ), S2F( tY ) );
                    else if( token == "mask" )
                        addVertToNewestMask( S2F( tX ), S2F( tY ) );
                }
                if( token == "floor" )
                    tessellateNewestFloor();
                else if( token == "ceil" )
                    tessellateNewestCeiling();
                else if( token == "surf" )
                    tessellateNewestSurface();
                else if( token == "mask" )
                    tessellateNewestMask();
            }
            else if( token == "node" )
            {
                tX = NEXT_TOK;
                tY = NEXT_TOK;
                addNode( S2F( tX ), S2F( tY ) );
            }
            else if( token == "obj" )
            {
                tC1 = NEXT_TOK;
                tC2 = NEXT_TOK;
                tC3 = NEXT_TOK;
                tC4 = NEXT_TOK;
                tC5 = NEXT_TOK;
                tC6 = NEXT_TOK;
                tC7 = NEXT_TOK;
                tC8 = NEXT_TOK;
                tX = NEXT_TOK;
                tY = NEXT_TOK;

                if( tC1 == "" || tC2 == "" | tC3 == "" || tC4 == "" || tC5 == "" || tC6 == "" || tC7 == "" || tC8 == "" || tX == "" || tY == "" )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "'obj' needs 10 values (%s:%d)", filename.c_str(), lineCount );

                if( S2I( tC1 ) < 0 || S2I( tC1 ) > MAX_ENT )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "Invalid object type '%d' (%s:%d)", S2I( tC1 ), filename.c_str(), lineCount );

                if( S2I( tC7 ) < -4 || S2I( tC7 ) >= NUM_TRIGGERS )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "Trigger < -4 or > %d (%s:%d)", NUM_TRIGGERS - 1, filename.c_str(), lineCount );
                if( S2I( tC7 ) > maxTrigger ) maxTrigger = S2I( tC7 );

                if( S2I( tC5 ) < 0 || S2I( tC5 ) >= NUM_TRIGGERS )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "Wakewhen < 0 or > %d (%s:%d)", NUM_TRIGGERS - 1, filename.c_str(), lineCount );
                if( S2I( tC5 ) > maxTrigger ) maxTrigger = S2I( tC5 );

                if( S2I( tC6 ) < 0 || S2I( tC6 ) >= NUM_TRIGGERS )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "Killwhen < 0 or > %d (%s:%d)", NUM_TRIGGERS - 1, filename.c_str(), lineCount );
                if( S2I( tC6 ) > maxTrigger ) maxTrigger = S2I( tC6 );

                if( S2I( tC8 ) < 0 || S2I( tC8 ) > MAX_TRIGTYPE )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "Invalid trigger type (%s:%d)", filename.c_str(), lineCount );
                if( S2I( tC8 ) & TRIG_ADD && S2I( tC7 ) > 0 ) triggers[S2I( tC7 )].hitsRequired++;

                addObject( S2I( tC1 ), S2F( tX ), S2F( tY ), torad( S2F( tC2 ) ), S2I( tC3 ), ( bool )S2I( tC4 ), S2I( tC5 ), S2I( tC6 ), S2I( tC7 ), S2I( tC8 ) );
            }
            else if( token == "light" )
            {
                tR = NEXT_TOK;
                tG = NEXT_TOK;
                tB = NEXT_TOK;
                tA = NEXT_TOK;
                tC1 = NEXT_TOK;
                tX = NEXT_TOK;
                tY = NEXT_TOK;
                tC2 = NEXT_TOK;

                if( tR == "" || tG == "" | tB == "" || tA == "" || tX == "" || tY == "" || tC1 == "" || tC2 == "" )
                    errorMessage( 5, "FLAGRANT SYSTEM ERROR", "'light' needs 8 values (%s:%d)", filename.c_str(), lineCount );

                addLight( S2F( tR ), S2F( tG ), S2F( tB ), S2F( tA ), S2F( tX ), S2F( tY ), S2F( tC2 ), ( bool )S2I( tC1 ) );
            }
        }
        lineCount++;
    }
    levelfile.close();
    loadedFileName = file;
    loaded = true;
}

void Level::readLevelIntroText( char* file )
{
    introText.clear();
    if( strcmp( file, "" ) == 0 )
        return;

    // Append the .dat file extension if not present
    string filename = file;
    if( filename.rfind( ".dat", filename.size() - 1 ) == string::npos )
        filename += ".dat";

    ifstream leveltextfile( filename.c_str() );

    if( !leveltextfile.is_open() )
    {
        introText.clear();
        return;
    }
    //printf("Loaded file %s\n", filename.c_str());
    string line;

    while( getline( leveltextfile, line, '\n' ) )
        introText.push_back( line );

    leveltextfile.close();
}

void Level::defaults()
{
    loaded = false;
    name = "(no name)";
    loadedFileName = "";
    bg.set( 0.0, 0.0, 0.0, 1.0 );
    ambLight.set( 1.0, 1.0, 1.0, 1.0 );
    ambLightDirection = torad( 90 );
    playerStart.set( 0.0, 0.0 );
    playerAugmented = false;
    wall.clear();
    floor.clear();
    ceil.clear();
    surf.clear();
    node.clear();
    door.clear();
    mask.clear();
    light.clear();
    dlistIndex = -1;
    maxTrigger = 0;

    gridLL.set( 0, 0 );
    gridUR.set( 0, 0 );
    gridCellSize = gridCols = gridRows = 0;
}

void Level::addWall( float p1x, float p1y, float p2x, float p2y, float r, float g, float b, float a, float h, float frc, float hlth, int pt, float pq, bool lt, int cf, int tr, int tt )
{
    wall.push_back( Wall( p1x, p1y, p2x, p2y, r, g, b, a, h, frc, hlth, pt, pq, lt, cf, tr, tt ) );
}

void Level::addWallFromLast( float px, float py, float r, float g, float b, float a, float h, float frc, float hlth, int pt, float pq, bool lt, int cf, int tr, int tt )
{
    if( wall.size() > 0 )
        wall.push_back( Wall( wall[wall.size() - 1].p2.c[X], wall[wall.size() - 1].p2.c[Y], px, py, r, g, b, a, h, frc, hlth, pt, pq, lt, cf, tr, tt ) );
}

void Level::addDoor( float p1x, float p1y, float p2x, float p2y, float r, float g, float b, float a, float h, float frc, float hlth, int pt,
                     float pq, bool lt, int cf, int ot, float spd, float th, float dl, int k, int rt1, int rt2, int rt3, int fl, int tr, int tt )
{
    door.push_back( Door( p1x, p1y, p2x, p2y, r, g, b, a, h, frc, hlth, pt, pq, lt, cf, ot, spd, th, dl, k, rt1, rt2, rt3, fl, tr, tt ) );
}

void Level::addFloor( float r, float g, float b, float a, float fric, bool lt, int cf, int tr, int tt )
{
    floor.push_back( Surface( r, g, b, a, 0, fric, 0, 0, 0, lt, cf, 0, tr, tt, false, 0, 0 ) );
}

void Level::addVertToNewestFloor( GLdouble x, GLdouble y )
{
    if( floor.size() > 0 )
        floor[floor.size() - 1].addVert( x, y );
}

void Level::tessellateNewestFloor()
{
    if( floor.size() > 0 )
        floor[floor.size() - 1].tessellate();
}

void Level::addCeiling( float r, float g, float b, float a, float h, bool lt )
{
    ceil.push_back( Surface( r, g, b, a, h, 0, 0, 0, 0, lt, 0, 0, 0, 0, false, 0, 0 ) );
}

void Level::addVertToNewestCeiling( GLdouble x, GLdouble y )
{
    if( ceil.size() > 0 )
        ceil[ceil.size() - 1].addVert( x, y );
}

void Level::tessellateNewestCeiling()
{
    if( ceil.size() > 0 )
        ceil[ceil.size() - 1].tessellate();
}

void Level::addSurface( float r, float g, float b, float a, float fric, float hlth, int pt, float pq, bool lt, int cf, int tr, int tt )
{
    surf.push_back( Surface( r, g, b, a, 0.01, fric, hlth, pt, pq, lt, cf, 0, tr, tt, false, 0, 0 ) );
}

void Level::addVertToNewestSurface( GLdouble x, GLdouble y )
{
    if( surf.size() > 0 )
        surf[surf.size() - 1].addVert( x, y );
}

void Level::tessellateNewestSurface()
{
    if( surf.size() > 0 )
        surf[surf.size() - 1].tessellate();
}

void Level::addNode( float x, float y )
{
    node.push_back( PathNode( x, y ) );
}

void Level::addObject( int type, float x, float y, float head, int st, bool fix, int rt1, int rt2, int tr, int tt )
{
    obj.push_back( new Entity( type, x, y, head, st, fix, rt1, rt2, tr, tt, false ) );
}

void Level::deleteObjects()
{
    for( int i = 0; i < obj.size(); i++ )
        delete obj[i];
    obj.clear();
}

void Level::addLight( float r, float g, float b, float a, float x, float y, float rad, bool spot )
{
    light.push_back( Light( r, g, b, a, x, y, rad, spot ) );
}

void Level::addMask( float r, float g, float b, float a, float h, bool lt, float fs, bool re, int rt )
{
    mask.push_back( Surface( r, g, b, a, h, 0, 0, 0, 0, lt, 0, fs, 0, 0, re, rt, 0 ) );
}

void Level::addVertToNewestMask( GLdouble x, GLdouble y )
{
    if( mask.size() > 0 )
        mask[mask.size() - 1].addVert( x, y );
}

void Level::tessellateNewestMask()
{
    if( mask.size() > 0 )
        mask[mask.size() - 1].tessellate();
}

void Level::calculateLighting()
{
    for( int i = 0; i < wall.size(); i++ )
    {
        wall[i].applyAmbientLight( ambLight, ambLightDirection );
        wall[i].applyLightSources( light );
    }

    for( int i = 0; i < door.size(); i++ )
    {
        door[i].applyAmbientLight( ambLight, ambLightDirection );
        door[i].applyLightSources( light );
    }

    for( int i = 0; i < floor.size(); i++ )
        floor[i].applyAmbientLight( ambLight );

    for( int i = 0; i < ceil.size(); i++ )
        ceil[i].applyAmbientLight( ambLight );

    for( int i = 0; i < surf.size(); i++ )
        surf[i].applyAmbientLight( ambLight );

    for( int i = 0; i < mask.size(); i++ )
        mask[i].applyAmbientLight( ambLight );
}

void Level::makeDisplayList()
{
    if( dlistIndex != -1 )
        glDeleteLists( dlistIndex, 1 );

    dlistIndex = glGenLists( 1 );
    glNewList( dlistIndex, GL_COMPILE );
    drawStaticGeometry();
    glEndList();
}

void Level::start()
{
    setBackgroundColor();

    // reset triggers/dynamic geometry
    for( int i = 0; i < floor.size(); i++ ) floor[i].resetTrigger();
    for( int i = 0; i < wall.size(); i++ )  wall[i].resetTrigger();
    for( int i = 0; i < surf.size(); i++ )  surf[i].resetTrigger();

    for( int i = 0; i < door.size(); i++ )
    {
        door[i].resetTrigger();
        door[i].reset();
    }

    for( int i = 0; i < mask.size(); i++ )
        mask[i].resetMask();
}

void Level::setBackgroundColor()
{
    glClearColor( bg.r, bg.g, bg.b, bg.a );
}

void Level::unload()
{
    if( loaded )
    {
        printf( "Level unloaded\n" );
        deleteObjects();
        freeGrid();
        glDeleteLists( dlistIndex, 1 );
        introText.clear();
        loaded = false;
        loadedFileName = "";
    }
}

void Level::updateDynamicGeometry()
{
    if( mask.size() > 0 )
        for( vector<Surface>::iterator m = mask.begin(); m != mask.end(); m++ )
            m->fade();

    if( door.size() > 0 )
        for( vector<Door>::iterator d = door.begin(); d != door.end(); d++ )
            d->update();
}

void Level::loadObjects()
{
    printf( "Loading objects\n" );
    currLevelObjs.clear();
    currLevelEnemies.clear();
    currLevelObjs.push_back( new Entity( ENT_PLAYER, playerStart.c[X], playerStart.c[Y], playerStartHeading, 1, false, 0, 0, 0, 0, playerAugmented || ( P.augmented && P.level > 0 ) ) );
    PLAYER_OBJECT->reset();
    PLAYER_OBJECT->setActive( true );
    for( int i = 0; i < obj.size(); i++ )
    {
        currLevelObjs.push_back( obj[i] );
        currLevelObjs[i + 1]->reset();
        currLevelObjs[i + 1]->setActive( true );
    }
}

void Level::drawStaticGeometry()
{
    if( floor.size() > 0 )
        for( vector<Surface>::iterator f = floor.begin(); f != floor.end(); f++ )
            f->drawDL();

    if( wall.size() > 0 )
        for( vector<Wall>::iterator w = wall.begin(); w != wall.end(); w++ )
            w->draw();

    if( ceil.size() > 0 )
        for( vector<Surface>::iterator c = ceil.begin(); c != ceil.end(); c++ )
            c->drawDL();

    if( surf.size() > 0 )
    {
        glPushMatrix();
        glTranslatef( 0.0, 0.0, 0.0075 );
        for( vector<Surface>::iterator s = surf.begin(); s != surf.end(); s++ )
            s->drawDL();
        glPopMatrix();
    }

    glBlendFunc( GL_SRC_ALPHA, GL_ONE );
    if( light.size() > 0 )
        for( vector<Light>::iterator l = light.begin(); l != light.end(); l++ )
            l->draw();
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
}

void Level::drawCell( int index )
{
    /*glPushMatrix();
    glPointSize(5);
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.0, 1.0, 1.0);
    glBegin(GL_POINTS);
    for (int y = 0; y < gridRows; y++)
      for (int x = 0; x < gridCols; x++)
        glVertex3f(gridLL.c[X]+x*gridCellSize, gridLL.c[Y]+y*gridCellSize, 0.25);
    glEnd();
    glPopMatrix();*/

    for( int i = 0; i < grid[index].wall.size(); i++ )
        currLevel.wall[grid[index].wall[i]].draw();

    for( int i = 0; i < grid[index].floor.size(); i++ )
        currLevel.floor[grid[index].floor[i]].drawDL();

    for( int i = 0; i < grid[index].ceil.size(); i++ )
        currLevel.ceil[grid[index].ceil[i]].drawDL();

    glPushMatrix();
    glTranslatef( 0.0, 0.0, 0.0075 );
    for( int i = 0; i < grid[index].surf.size(); i++ )
        currLevel.surf[grid[index].surf[i]].drawDL();
    glPopMatrix();

    glBlendFunc( GL_SRC_ALPHA, GL_ONE );
    for( int i = 0; i < grid[index].light.size(); i++ )
        currLevel.light[grid[index].light[i]].draw();
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    for( int i = 0; i < grid[index].door.size(); i++ )
        currLevel.door[grid[index].door[i]].draw();

    for( int i = 0; i < grid[index].mask.size(); i++ )
        currLevel.mask[grid[index].mask[i]].drawDL();
}

void Level::resetCell( int index )
{
    for( int i = 0; i < grid[index].wall.size(); i++ )  currLevel.wall[grid[index].wall[i]].resetDrawn();
    for( int i = 0; i < grid[index].floor.size(); i++ ) currLevel.floor[grid[index].floor[i]].resetDrawn();
    for( int i = 0; i < grid[index].ceil.size(); i++ )  currLevel.ceil[grid[index].ceil[i]].resetDrawn();
    for( int i = 0; i < grid[index].surf.size(); i++ )  currLevel.surf[grid[index].surf[i]].resetDrawn();
    for( int i = 0; i < grid[index].light.size(); i++ ) currLevel.light[grid[index].light[i]].resetDrawn();
    for( int i = 0; i < grid[index].door.size(); i++ )  currLevel.door[grid[index].door[i]].resetDrawn();
    for( int i = 0; i < grid[index].mask.size(); i++ )  currLevel.mask[grid[index].mask[i]].resetDrawn();
}

void Level::drawDynamicGeometry()
{
    if( mask.size() > 0 )
        for( vector<Surface>::iterator m = mask.begin(); m != mask.end(); m++ )
            if( m->Alpha() )
                m->drawDL();

    if( door.size() > 0 )
        for( vector<Door>::iterator d = door.begin(); d != door.end(); d++ )
        {
            d->draw();
        }
}

void Level::calculateGrid()
{
    if( gridUR.c[X] - gridLL.c[X] <= 0 || gridUR.c[Y] - gridLL.c[Y] <= 0 || gridCellSize <= 0 )
        errorMessage( 7, "FLAGRANT SYSTEM ERROR", "Invalid grid bounds" );

    gridCols = ( int )std::ceil( ( gridUR.c[X] - gridLL.c[X] ) / gridCellSize );
    gridRows = ( int )std::ceil( ( gridUR.c[Y] - gridLL.c[Y] ) / gridCellSize );

    // Load up teh gridx0rz
    grid = new GridCell[gridCols * gridRows];
    if( !grid ) errorMessage( 7, "FLAGRANT SYSTEM ERROR", "Could not allocate grid space" );

    BoundingBox currCell;
    for( int y = 0; y < gridRows; y++ )
        for( int x = 0; x < gridCols; x++ )
        {
            currCell.set( Vector2D( GRID_X_COORD( x ), GRID_Y_COORD( y ) ), Vector2D( GRID_X_COORD( x + 1 ), GRID_Y_COORD( y + 1 ) ) );

            for( int w = 0; w < wall.size(); w++ )
                if( wall[w].intersection( Vector2D( GRID_X_COORD( x ), GRID_Y_COORD( y ) ), Vector2D( GRID_X_COORD( x + 1 ), GRID_Y_COORD( y ) ), NULL ) ||
                        wall[w].intersection( Vector2D( GRID_X_COORD( x ), GRID_Y_COORD( y ) ), Vector2D( GRID_X_COORD( x ), GRID_Y_COORD( y + 1 ) ), NULL ) ||
                        wall[w].intersection( Vector2D( GRID_X_COORD( x ), GRID_Y_COORD( y + 1 ) ), Vector2D( GRID_X_COORD( x + 1 ), GRID_Y_COORD( y + 1 ) ), NULL ) ||
                        wall[w].intersection( Vector2D( GRID_X_COORD( x + 1 ), GRID_Y_COORD( y + 1 ) ), Vector2D( GRID_X_COORD( x + 1 ), GRID_Y_COORD( y ) ), NULL ) ||
                        cellNumber( wall[w].p1 ) == y * gridCols + x || cellNumber( wall[w].p2 ) == y * gridCols + x )
                    grid[y * gridCols + x].add( WALL, w );

            for( int w = 0; w < floor.size(); w++ )
                if( floor[w].intersectBB( currCell ) )
                    grid[y * gridCols + x].add( FLOOR, w );

            for( int w = 0; w < ceil.size(); w++ )
                if( ceil[w].intersectBB( currCell ) )
                    grid[y * gridCols + x].add( CEIL, w );

            for( int w = 0; w < surf.size(); w++ )
                if( surf[w].intersectBB( currCell ) )
                    grid[y * gridCols + x].add( SURF, w );

            for( int w = 0; w < light.size(); w++ )
                if( light[w].bb.intersectBB( currCell ) )
                    grid[y * gridCols + x].add( LIGHT, w );

            for( int w = 0; w < door.size(); w++ )
                if( door[w].intersectBB( currCell ) )
                    grid[y * gridCols + x].add( DOOR, w );

            for( int w = 0; w < mask.size(); w++ )
                if( mask[w].intersectBB( currCell ) )
                    grid[y * gridCols + x].add( MASK, w );
        }
}

void Level::freeGrid()
{
    for( int i = 0; i < gridCols * gridRows; i++ )
        grid[i].free();

    delete[] grid;
}

int Level::cellNumber( Vector2D v )
{
    return cellNumber( v.c[X], v.c[Y] );
}

int Level::cellNumber( float x, float y )
{
    int cx = ( int )( ( x - gridLL.c[X] ) / gridCellSize );
    int cy = ( int )( ( y - gridLL.c[Y] ) / gridCellSize );
    return cy * gridCols + cx;
}

void Level::cellNumber( Vector2D v, int* x, int* y )
{
    *x = ( int )( ( v.c[X] - gridLL.c[X] ) / gridCellSize );
    *y = ( int )( ( v.c[Y] - gridLL.c[Y] ) / gridCellSize );
}

bool Level::inGrid( Vector2D v )
{
    if( v.c[X] < gridLL.c[X] || v.c[X] > gridUR.c[X] ||
            v.c[Y] < gridLL.c[Y] || v.c[Y] > gridUR.c[Y] )
        return false;

    return true;
}

// For each node, determine which nodes it can see (i.e. aren't obstructed by walls)
// It runs in quadratic time--yeeha! I'm too lazy to write something recursive/elegant.
// Just don't tell Klaus.
void Level::calculateNodeGraph()
{
    Vector2D ray, intersect;
    int i, j;
    float distToNode;

    for( i = 0; i < node.size(); i++ )
    {
        for( j = 0; j < node.size(); j++ )
        {
            distToNode = dist( node[i].pos, node[j].pos );
            if( distToNode != 0 && !node[i].isVisible( j ) )
            {
                if( hasLineOfSight( node[i].pos, node[j].pos, false ) )
                {
                    node[i].addToVisibleSet( j, distToNode );
                    node[j].addToVisibleSet( i, distToNode );
                }
            }
        }
    }
}

int Level::closestNode( Vector2D pos )
{
    float distance, closestDistance = 1e37;
    int closestNode = 0;
    for( int i = 0; i < node.size(); i++ )
    {
        distance = dist( pos, node[i].pos );
        if( distance < closestDistance )
        {
            closestDistance = distance;
            closestNode = i;
        }
    }

    return closestNode;
}

// Did I write this? I can't remember. I think so...
void ltrim( char* str )
{
    // Find out how many leading whitespace characters there are
    int white_len = strspn( str, " \t" );

    if( white_len > 0 && white_len < strlen( str ) )
    {
        // Move the non-whitespace characters to the beginning of the string
        str = ( char* )memmove( str, str + white_len, strlen( str ) - white_len );

        // Null-terminate the string
        *( str + strlen( str ) - white_len ) = '\0';
    }
}
