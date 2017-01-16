/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * bitmapfont.cpp - bitmap font routines
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

#include "bitmapfont.h"

GLuint textures[NUM_TEXTURES];

// Creates a bitmap font from a loaded texture.
// Must be 16 characters wide, 8 characters high.
void BitmapFont::create( int index )
{
    lastY = 0.0;
    lastYScale = 1.0;
    texIndex = index;
}

void BitmapFont::drawChar( char c )
{
    int row = c / 16;
    int col = c % 16;

    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, textures[texIndex] );
    glBegin( GL_QUADS );
    glTexCoord2f( col * .0625,    row * .125 );
    glVertex2f( -0.5, 1.0 );
    glTexCoord2f( ( col + 1 )*.0625, row * .125 );
    glVertex2f( 0.5, 1.0 );
    glTexCoord2f( ( col + 1 )*.0625, ( row + 1 )*.125 );
    glVertex2f( 0.5, -1.0 );
    glTexCoord2f( col * .0625, ( row + 1 )*.125 );
    glVertex2f( -0.5, -1.0 );
    glEnd();
}

void BitmapFont::print( float x, float y, float xscale, float yscale, float spacing, const char* text )
{
    if( x == CENTERED )
        x = SCREEN_CENTER - ( ( float )strlen( text ) * spacing * xscale ) / 2.0 + xscale / 2.0;

    if( y >= LAST_Y - 5000.0 && y <= LAST_Y + 5000.0 )
        y = lastY + ( y - LAST_Y ) - lastYScale - yscale;

    glPushMatrix();
    glTranslatef( x, y, 0.0 );
    glScalef( xscale, yscale, 0.0 );
    for( int i = 0; i < strlen( text ); i++ )
    {
        drawChar( text[i] );
        glTranslatef( spacing, 0.0, 0.0 );
    }
    glPopMatrix();
    glDisable( GL_TEXTURE_2D );

    lastY = y;
    lastYScale = yscale;
}

void BitmapFont::printf( float x, float y, float xscale, float yscale, const char* fmt, ... )
{
    char buffer[513];
    va_list ap;

    va_start( ap, fmt );
    vsnprintf( buffer, 512, fmt, ap );
    print( x, y, xscale, yscale, 0.75, buffer );
}

void BitmapFont::LEDprintf( float x, float y, float xscale, float yscale, const char* fmt, ... )
{
    char buffer[513];
    va_list ap;

    va_start( ap, fmt );
    vsnprintf( buffer, 512, fmt, ap );

    // convert digits to LED numerals
    char* p = buffer;
    while( *p )
    {
        if( *p >= '0' && *p <= '?' )
            *p -= 32;

        p++;
    }

    print( x, y, xscale, yscale, 1.0, buffer );
}

void BitmapFont::vprintf( float x, float y, float xscale, float yscale, const char* fmt, va_list ap )
{
    char text[513];
    vsprintf( text, fmt, ap );
    if( x == CENTERED )
        x = SCREEN_CENTER - ( ( float )strlen( text ) * xscale * 0.75 ) / 2.0;

    if( y >= LAST_Y - 5000.0 && y <= LAST_Y + 5000.0 )
        y = lastY + ( y - LAST_Y ) - lastYScale - yscale;

    glPushMatrix();
    glTranslatef( x, y, 0.0 );
    glScalef( xscale, yscale, 0.0 );
    for( int i = 0; i < strlen( text ); i++ )
    {
        drawChar( text[i] );
        glTranslatef( 0.75, 0.0, 0.0 );
    }
    glPopMatrix();

    lastY = y;
    lastYScale = yscale;
}
