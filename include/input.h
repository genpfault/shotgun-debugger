/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * input.h - header file for input handler
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

#ifndef _INPUT_H_
#define _INPUT_H_

#include "sdb.h"

enum INPUT_KEYS {KEY_LEFT, KEY_RIGHT, KEY_FWD, KEY_BACK, KEY_STRAFE_L, KEY_STRAFE_R,
                 KEY_JUMP, KEY_FIRE, KEY_GRENADE, KEY_RELOAD, KEY_SWITCH_WPN, KEY_SWITCH_VIEW, KEY_ZOOM_OUT, NUM_INPUT_KEYS
                };

// see sdb.h for relevant defines/macros

class InputHandler
{
public:
    InputHandler()
    {
        defaultBindings();
    }
    void bindKey( int action, int sym1, int sym2 );
    void defaultBindings();
    void setKeyBindings( SDLKey keys[NUM_INPUT_KEYS] );
    float bindingState( int key );
    float state( int action )
    {
        if( bindingState( keysym1[action] ) )
            return bindingState( keysym1[action] );
        else if( keysym2[action] != NO_KEY )
            return bindingState( keysym2[action] );
        else
            return 0;
    }

private:
    int keysym1[NUM_INPUT_KEYS];
    int keysym2[NUM_INPUT_KEYS];
};

#endif
