/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * player.h - header file for player data functions
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
 
#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "sdb.h"

#define CAMERA_ZOOM_SPEED      60.0
#define CAMERA_ZOOM_FALL_SPEED 80.0
#define CAMERA_ZOOM_LIMIT      65.0

#define UNLOCK_TIME 2400.0

struct Stats
{
  float time;
  int kills;
  int shots;
  int hits;
};


class Player
{
  public:
    Player() { set(0); }
    Player(int obj) { set(obj); }
    void set(int obj);
    void dataDefaults();
    void eraseRecords();
    void eraseSavedGame();
    void resetLevelProgress() { levelProgress = 0; canAugment = false; }
    void loadPlayerData();
    void writePlayerData();
    void setKeys(SDLKey keys[13]) { ih.setKeyBindings(keys); }
    bool hasGoodChecksum(int cksum);
    void resetCurrentLevelStats() { gameTime = 0.0; kills = shots = hits = 0; }
    int checksum();
    void unlockLevel();
    void setLevel(int l);
    void saveLevelData(bool playInSequence);
    void computeFinalStatistics();
    void saveFinalStatistics();
    void update();
    float totalTime();
    InputHandler ih;
    bool HeadingLock() { return headingLock; }
    float cameraZoom() { return zoom; }
    
    /*** Saved data ***/
    // Status   
    int levelProgress;
    
    // Records
    Stats levelRecords[NUM_LEVELS+1];
    
    // Current game
    int startingLevel; // -1 indicates no saved game
    int level; 
    Stats currStats[NUM_LEVELS+1];
    
    // Current level stats
    float gameTime;
    int kills;
    int shots;
    int hits;
    bool cheated;
    bool usedAugment;
    
    bool canAugment;
    bool augmented;
  private:
    int object; // the object that the player controls
                // (an index in the global array)
    bool headingLock;
    bool hlReady;
    
    float zoom;
};

#endif
