/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * levelobjects.h - header file for level object functions
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

#ifndef _LEVELOBJECTS_H_
#define _LEVELOBJECTS_H_

#include "sdb.h"

#define DOOR_SLIDE1 0
#define DOOR_SLIDE2 1

#define DOOR_SOUND 1
#define DOOR_MESSAGE 2

#define DOOR_SPEED  1.0

enum {WALL, FLOOR, CEIL, SURF, LIGHT, DOOR, MASK, OBJ, OTYPES};

class LevelObject
{
public:
    void resetDrawn()
    {
        drawn = false;
    }
    void trigInit( int tr, int tt )
    {
        trigger = tr;
        trigtype = tt;
        resetTrigger();
    }
    void renewTrigger()
    {
        if( !( trigtype & TRIG_ONCE ) ) doTrigger = true;
    }
    void handleTrigger( bool bullet, bool player );
    void resetTrigger()
    {
        doTrigger = true;
    }
    bool drawn;
protected:
    int trigger;
    int trigtype;
    bool doTrigger;
};

class GridCell
{
public:
    GridCell()
    {
        reset();
    }

    vector<int> floor, wall, ceil, surf, light, door, mask;

    void reset()
    {
        floor.clear();
        wall.clear();
        ceil.clear();
        surf.clear();
        light.clear();
        door.clear();
        mask.clear();
    }

    void add( int objtype, int op )
    {
        switch( objtype )
        {
        case WALL:
            wall.push_back( op );
            break;
        case FLOOR:
            floor.push_back( op );
            break;
        case CEIL:
            ceil.push_back( op );
            break;
        case SURF:
            surf.push_back( op );
            break;
        case LIGHT:
            light.push_back( op );
            break;
        case DOOR:
            door.push_back( op );
            break;
        case MASK:
            mask.push_back( op );
            break;
        }
    }

    void free() {}
};

class PathNode
{
public:
    PathNode()
    {
        set( 0, 0, 0 );
    }
    PathNode( float x, float y )
    {
        set( 0, x, y );
    }
    void set( int pr, float x, float y )
    {
        priority = pr;
        pos.set( x, y );
    }
    void addToVisibleSet( int num, float dst )
    {
        visible.push_back( num );
        distance.push_back( dst );
    }
    bool isVisible( int num )
    {
        for( int i = 0; i < visible.size(); i++ )
        {
            if( i == num ) return true;
        }
        return false;
    }

    int priority;
    Vector2D pos;
    vector<int> visible;
    vector<float> distance;
};

class Light : public LevelObject
{
public:
    Light()
    {
        drawn = false;
        color.zero();
        pos.set( 0, 0 );
        radius = 0;
    }
    Light( float r, float g, float b, float a, float x, float y, float rad, bool spot )
    {
        drawn = false;
        color.set( r, g, b, a );
        pos.set( x, y );
        radius = rad;
        lightSpot = spot;
        bb.set( Vector2D( x - radius, y - radius ), Vector2D( x + radius, y + radius ) );
    }
    float Strength()
    {
        return color.a;
    }
    void draw();

    Vector2D pos;
    float radius;
    RGBAColor color;
    bool lightSpot;
    BoundingBox bb;
};

class Wall : public Line, public LevelObject
{
public:
    Wall() {}
    Wall( float p1x, float p1y, float p2x, float p2y, float r, float g, float b, float a, float h, float frc, float hlth, int pt, float pq, bool lt, int cf, int tr, int tt )
    {
        set( p1x, p1y, p2x, p2y, r, g, b, a, h, frc, hlth, pt, pq, lt, cf, tr, tt );
    }
    void set( float p1x, float p1y, float p2x, float p2y, float r, float g, float b, float a, float h, float frc, float hlth, int pt, float pq, bool lt, int cf, int tr, int tt )
    {
        drawn = false;
        p1.set( p1x, p1y );
        p2.set( p2x, p2y );
        setColor( r, g, b, a );
        force = frc;
        health = hlth;
        height = h;
        particleType = pt;
        particleQty = pq;
        lit = lt;
        collFlags = cf;
        trigInit( tr, tt );
        normal = findNormal();
        bb.addPoint( p1 );
        bb.addPoint( p2 );
    }


    void print();
    void draw();
    void draw( bool d );

    void setColor( float r, float g, float b, float a );
    void applyAmbientLight( RGBAColor light, float direction );
    void applyLightSources( vector<Light> lights );
    float getAlpha()
    {
        return baseColor.a;
    }
    void spewParticles( Vector2D loc );

    RGBAColor baseColor;
    RGBAColor litColor1;
    RGBAColor litColor2;

    float Force()
    {
        return force;
    }
    float Health()
    {
        return health;
    }
    float Height()
    {
        return height;
    }
    int CollFlags()
    {
        return collFlags;
    }
    bool intersectBB( BoundingBox b )
    {
        return bb.intersectBB( b );
    }

    //private:
    BoundingBox bb;
    float force;
    float health;
    float height;

    int particleType;
    float particleQty;

    int collFlags;
    bool lit;
};

class Surface : public LevelObject
{
public:
    Surface()
    {
        baseColor.zero();
        litColor.zero();
        vert.clear();
        friction = 0;
        health = 0;
        trigInit( 0, 0 );
    }
    Surface( float r, float g, float b, float a, float h, float fric, float hlth, int pt, float pq, bool lt, int cf, float fs, int tr, int tt, bool re, int rt, int numVerts, ... );
    void set( float r, float g, float b, float a, float h, float fric, float hlth, int pt, float pq, bool lt, int cf, float fs, int tr, int tt, bool re, int rt, int numVerts, ... );
    void addVert( GLdouble x, GLdouble y );
    void tessellate();
    void setColor( float r, float g, float b, float a );
    void applyAmbientLight( RGBAColor light );
    void draw();
    void draw( bool d );
    void drawDL();
    void drawDL( bool d );
    void spewParticles( Vector2D loc );
    int pointInPolygon( Vector2D point );

    void setVert( int num, float x, float y )
    {
        if( num >= 0 && num < vert.size() ) vert[num].set( x, y );
    }
    void setAlpha( float a )
    {
        litColor.a = a;
    }
    float Alpha()
    {
        return litColor.a;
    }
    void startFade( int dir )
    {
        fadeDirection = dir;
    }
    void fade();
    bool pointInBB( Vector2D point )
    {
        return bb.pointInBB( point );
    }
    bool intersectBB( BoundingBox b )
    {
        return bb.intersectBB( b );
    }
    float Friction()
    {
        return friction;
    }
    float Health()
    {
        return health;
    }
    int Trigger()
    {
        return trigger;
    }
    int CollFlags()
    {
        return collFlags;
    }
    bool Reobscure()
    {
        return reobscure;
    }

    void resetMask()
    {
        fadeDirection = 0;
        triggered = false;
        litColor.a = 1.0;
    }

    vector<Vector2Dd> vert;

protected:
    BoundingBox bb;
    RGBAColor baseColor;
    RGBAColor litColor;
    int dlistIndex;

    float friction;
    float health;
    float height;

    int particleType;
    float particleQty;

    int collFlags;
    bool lit;

    int fadeDirection;
    float fadeSpeed;

    // Special variables for masks
    bool reobscure;
    int respondTo;
    bool triggered;
};

class Door : public LevelObject
{
public:
    Door( float p1x, float p1y, float p2x, float p2y, float r, float g, float b, float a, float h, float frc, float hlth, int pt,
          float pq, bool lt, int cf, int ot, float spd, float th, float dl, int k, int rt1, int rt2, int rt3, int fl, int tr, int tt );

    void draw();
    void reset();
    void applyAmbientLight( RGBAColor light, float direction );
    void applyLightSources( vector<Light> lights );
    bool intersectBB( BoundingBox b )
    {
        return bb.intersectBB( b );
    }

    void update();
    void updateCoords();
    void detect( Vector2D p, int keys );

    void open();
    void close();

    Wall sides[4];
private:
    Vector2D p1, p2, direction, midpoint;
    Surface top;
    BoundingBox bb;

    int openType;
    float speed;
    float delay;
    float radius;
    float thickness;
    int key;
    int currentKey;

    int respondTo1;
    int respondTo2;
    int respondTo3;

    int flags;

    int moving;
    float pos;
    float doorTime;
    bool waiting;
    bool jammed;
};

#endif
