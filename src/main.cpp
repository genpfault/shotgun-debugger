/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * main.cpp - where it all begins
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
#include <ctime>
#include <string>

bool GFXMODE_ISACTIVE;
BitmapFont font;
GLUtesselator* tess;
Timer timer;
int screenshotCount;
bool screenshotKeyPressed;

int main( int argc, char* argv[] )
{
    GFXMODE_ISACTIVE = false;
    bool playInSequence = true;
    screenshotCount = 1;
    screenshotKeyPressed = false;

    // Redirect standard output on Win32 since we'll never run from a console
#ifdef _WIN32
    freopen( "stdout.txt", "w", stdout );
#endif

    // Load configuration, load libraries, and start graphics mode
    gameWelcomeMessage();
    srand( time( NULL ) );
    config.loadFromFile( CONFIG_FILE );
    gameInitSDL();
    //gameSetVideoMode(config.xres, config.yres, config.depth, (config.fullscreen) ? SDL_FULLSCREEN : 0);
    gameSetVideoMode( config.xres, config.yres, config.depth, 0 );
    gameInitOpenGL();
    gameSetAntialiasing( config.antialias );
    gameInitTessellator();

    // Load game resources
    gameLoadSprites();     // loads the sprite textures
    gameLoadModels();

    if( config.use_sound )
    {
        gameLoadSounds();      // loads the sounds
        gameLoadMusic();       // loads the music
    }

    font.create( TEX_FONT ); // loads the system font from texture slot 0

    // initialize types
    initObjectTypes();
    initWeaponTypes();
    P.loadPlayerData();
    P.setKeys( config.keys );

    printf( "Initialization complete\n" );

    string levelToPlay = "";

    if( argc >= 2 )
    {
        config.setDefaultLevel( argv[1] );
        levelToPlay = argv[1];
        playInSequence = false;
    }
    else if( config.defaultLevel != "" )
    {
        levelToPlay = config.defaultLevel;
        playInSequence = false;
    }

    bool loop = true, first = true;
    int lvl = 0;
    while( loop )
    {
        stopMusic();
        startMusic( MUS_TITLE );

        lvl = title( first, playInSequence );

        if( first ) first = false;

        if( lvl == -1 ) // Exit
            loop = false;
        else if( lvl == -2 ) // Continue game
            game( levelToPlay, true );
        else
        {
            if( playInSequence )
                P.level = P.startingLevel = lvl - 1;

            game( levelToPlay, playInSequence );
        }
    }
    currLevel.unload();
    gameShutdown();

    // Save configuration data
    config.writeToFile( CONFIG_FILE );
    P.writePlayerData();
    return 0;
}

// Starts the sequence of levels at the given starting point.
void game( string levelToPlay, bool playInSequence )
{
    string lvl = ( playInSequence ) ? getLevelFileName( P.level - ( ( P.level == NUM_LEVELS ) ? 1 : 0 ) ) : levelToPlay;

    bool loop = true;
    bool music = false;
    while( loop )
    {
        if( lvl != currLevel.loadedFileName )
        {
            currLevel.unload();

            // Level loading sequence
            loadingScreen( 0 );
            currLevel.defaults();
            currLevel.readLevelData( ( char* )lvl.c_str() );

            if( playInSequence )
            {
                if( P.level == NUM_LEVELS )
                    currLevel.readLevelIntroText( ( char* )getLevelIntroFileName( P.level - 1 ).c_str() );
                else
                    currLevel.readLevelIntroText( ( char* )getLevelIntroFileName( P.level ).c_str() );
            }

            glColor4f( 0.3, 1.0, 0.3, 0.8 );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE );
            loadingScreen( 1 ); // level built
            currLevel.calculateLighting();
            loadingScreen( 2 ); // lights done
            currLevel.calculateGrid();
            loadingScreen( 3 ); // grid done
            currLevel.calculateNodeGraph();
            loadingScreen( 4 ); // nodes done
            currLevel.makeDisplayList();
        }

        if( !music )
        {
            stopMusic();
            startMusic( MUS_DD2 );
            music = true;
        }

        if( playInSequence )
            levelIntroScreen( false );

        if( config.mgrab ) SDL_WM_GrabInput( SDL_GRAB_ON );
        int levelOutcome = playCurrentLevel( playInSequence );
        if( config.mgrab ) SDL_WM_GrabInput( SDL_GRAB_OFF );
        switch( levelOutcome )
        {
        case LEVEL_QUIT:
            loop = false;
            break;
        case LEVEL_LOSE:
            loop = true;
            break;
        case LEVEL_WIN:
            if( playInSequence )
            {
                P.level++;

                // Beat the game!
                if( P.level >= NUM_LEVELS )
                {
                    if( P.startingLevel == 0 && !P.cheated && !P.usedAugment )
                        P.computeFinalStatistics();

                    finalStatistics();

                    // Unload level, load ending text, roll credits
                    startMusic( MUS_END );
                    currLevel.unload();
                    currLevel.defaults();

                    currLevel.readLevelIntroText( ( char* )getLevelIntroFileName( NUM_LEVELS ).c_str() );

                    levelIntroScreen( true ); // displays ending text
                    finalCredits();

                    printf( "A winner is you!\n" );
                    loop = false;
                    P.level = NUM_LEVELS;
                }
                else
                {
                    P.unlockLevel();
                    lvl = getLevelFileName( P.level );
                }
            }
            else
                loop = false;
            break;
        }
    }
}
