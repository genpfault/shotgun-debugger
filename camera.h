/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * camera.h - camera class
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

#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "sdb.h"

#define CAMERA_DEFAULT_HEIGHT 50.0

class Camera
{
  public:
    Camera() { set(0.0, 0.0, CAMERA_DEFAULT_HEIGHT, 0.0); }
    void setPos(float x, float y) { pos.c[X] = x; pos.c[Y] = y; }
    void setHeight(float h) { height = h; }
    void setRotation(float theta) { rot = theta; }
    void set(float x, float y, float h, float th) { setPos(x, y); setHeight(h); setRotation(th); }
    void change(float dx, float dy, float dh, float dth) { pos.c[X]+=dx; pos.c[Y]+=dy; height+=dh; rot+=dth; }
    void apply() { glRotatef(rot, 0.0, 0.0, 1.0); gluLookAt(pos.c[X],pos.c[Y]-0,height, pos.c[X],pos.c[Y],0.0, 0.0,1.0,0.0); }
    void applyPerspective() { glRotatef(rot, 0.0, 0.0, 1.0); gluLookAt(pos.c[X],pos.c[Y],0.0, pos.c[X],10000.0,0.0, 0.0,0.0,1.0); }
    void drawCursor()
    {
      glColor4f(0.0, 1.0, 0.0, 0.75);
      glBegin(GL_LINES);
        glVertex2f(pos.c[X]-5.0, pos.c[Y]);
        glVertex2f(pos.c[X]+5.0, pos.c[Y]);
        
        glVertex2f(pos.c[X], pos.c[Y]-5.0);
        glVertex2f(pos.c[X], pos.c[Y]+5.0);  
      glEnd();
    }
    
    Vector2D Pos() { return pos; }
    float Height() { return height; }
  private:
    Vector2D pos;
    float height;
    float rot;
};

#endif
