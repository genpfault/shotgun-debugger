/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * input.cpp - input handler
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

//Uint8 *keystate;

// sym is the SDLK_* keysym to bind to the action.
void InputHandler::bindKey( int action, int sym1, int sym2 )
{
    if( action >= 0 && action < NUM_INPUT_KEYS )
    {
        keysym1[action] = sym1;
        keysym2[action] = sym2;
    }
}

void InputHandler::defaultBindings()
{
    bindKey( KEY_LEFT, NO_KEY, MOUSE_X_NEG );
    bindKey( KEY_RIGHT, NO_KEY, MOUSE_X_POS );
    bindKey( KEY_FWD, NO_KEY, NO_KEY );
    bindKey( KEY_BACK, NO_KEY, NO_KEY );
    bindKey( KEY_STRAFE_L, NO_KEY, NO_KEY );
    bindKey( KEY_STRAFE_R, NO_KEY, NO_KEY );
    bindKey( KEY_JUMP, NO_KEY, NO_KEY );
    bindKey( KEY_FIRE, NO_KEY, MOUSE_BUTTON_1 );
    bindKey( KEY_GRENADE, NO_KEY, MOUSE_BUTTON_3 );
    bindKey( KEY_RELOAD, NO_KEY, NO_KEY );
    bindKey( KEY_SWITCH_WPN, NO_KEY, NO_KEY );
    bindKey( KEY_SWITCH_VIEW, NO_KEY, NO_KEY );
    bindKey( KEY_ZOOM_OUT, NO_KEY, MOUSE_BUTTON_2 );
}

void InputHandler::setKeyBindings( SDLKey keys[NUM_INPUT_KEYS] )
{
    for( int i = 0; i < NUM_INPUT_KEYS; i++ )
        bindKey( i, keys[i], keysym2[i] );
}

// Call this only after global keystate and mouse statehas been updated
// with SDL_GetKeyState().
float InputHandler::bindingState( int key )
{
    if( key != NO_KEY )
    {
        if( IS_MOUSE_AXIS( key ) )
        {
            switch( key )
            {
            case MOUSE_X_POS:
                if( mouse_dX > config.mthresh )
                    return ( float )abs( mouse_dX ) * ( ( float )config.msense / 1000.0 );
                else
                    return 0;
                break;
            case MOUSE_X_NEG:
                if( mouse_dX < -config.mthresh )
                    return ( float )abs( mouse_dX ) * ( ( float )config.msense / 1000.0 );
                else
                    return 0;
                break;
            case MOUSE_Y_POS:
                if( mouse_dY > config.mthresh )
                    return ( float )abs( mouse_dY ) * ( ( float )config.msense / 1000.0 );
                else
                    return 0;
                break;
            case MOUSE_Y_NEG:
                if( mouse_dY < -config.mthresh )
                    return ( float )abs( mouse_dY ) * ( ( float )config.msense / 1000.0 );
                else
                    return 0;
                break;
            }
        }
        else if( IS_MOUSE_BUTTON( key ) )
            return ( mouseButtons & SDL_BUTTON( -key ) ) ? 1 : 0;
        else
            return ( keystate[key] ) ? 1 : 0;
    }

    return 0.0;
}

