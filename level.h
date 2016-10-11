/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * level.h - header file for level class
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
 
#ifndef _LEVEL_H_
#define _LEVEL_H_

#include "sdb.h"

#define GRID_SIZE (gridCols*gridRows)
#define GRID_X_COORD(x) (gridLL.c[X]+((x)*gridCellSize))
#define GRID_Y_COORD(y) (gridLL.c[Y]+((y)*gridCellSize))

class Level
{
  public:
    Level() { defaults(); }
    Level(float psx, float psy) { defaults(); playerStart.set(psx, psy); }
    Level(char *file) { loadFromFile(file); }
    void loadFromFile(char *file);
    void readLevelData(char *file);
    void readLevelIntroText(char *file);
    void unload();
    void defaults();
    
    void addWall(float p1x, float p1y, float p2x, float p2y, float r, float g, float b, float a, float h, float frc, float hlth, int pt, float pq, bool lt, int cf, int tr, int tt);
    void addWallFromLast(float px, float py, float r, float g, float b, float a, float h, float frc, float hlth, int pt, float pq, bool lt, int cf, int tr, int tt);
    
    void addDoor(float p1x, float p1y, float p2x, float p2y, float r, float g, float b, float a, float h, float frc, float hlth, int pt,
          float pq, bool lt, int cf, int ot, float spd, float th, float dl, int k, int rt1, int rt2, int rt3, int fl, int tr, int tt);
    
    void addFloor(float r, float g, float b, float a, float fric, bool lt, int cf, int tr, int tt);
    void addVertToNewestFloor(GLdouble x, GLdouble y);
    void tessellateNewestFloor();
    
    void addCeiling(float r, float g, float b, float a, float h, bool lt);
    void addVertToNewestCeiling(GLdouble x, GLdouble y);
    void tessellateNewestCeiling();
    
    void addSurface(float r, float g, float b, float a, float fric, float hlth, int pt, float pq, bool lt, int cf, int tr, int tt);
    void addVertToNewestSurface(GLdouble x, GLdouble y);
    void tessellateNewestSurface();
    
    void addNode(float x, float y);
    void addObject(int type, float x, float y, float head, int st, bool fixed, int rt1, int rt2, int tr, int tt);
    void loadObjects();
    void deleteObjects();
    
    void addLight(float r, float g, float b, float a, float x, float y, float rad, bool spot);
    
    void addMask(float r, float g, float b, float a, float h, bool lt, float fs, bool re, int rt);
    void addVertToNewestMask(GLdouble x, GLdouble y);
    void tessellateNewestMask();
    
    void calculateLighting();
    void makeDisplayList();
    
    void start();
    void setBackgroundColor();
    void updateDynamicGeometry();
    
    void drawStaticGeometry();
    void drawStaticGeometryDL() { glCallList(dlistIndex); }
    void drawDynamicGeometry();
    void drawCell(int index);
    void resetCell(int index);

    int numWalls()  { return wall.size(); }
    int numFloors() { return floor.size(); }
    
    void calculateGrid();
    void freeGrid();
    int cellNumber(Vector2D v);
    int cellNumber(float x, float y);
    void cellNumber(Vector2D v, int *x, int *y);
    bool inGrid(Vector2D v);
    
    void calculateNodeGraph();
    int closestNode(Vector2D pos);
    
    bool loaded;
    string name;
    string author;
    string loadedFileName;
  //private:
    RGBAColor bg;
    RGBAColor ambLight;
    float ambLightDirection;
    Vector2D playerStart;
    float playerStartHeading;
    bool playerAugmented;
    
    // Static geometry
    vector<Wall> wall;
    vector<Surface> floor;
    vector<Surface> ceil;
    vector<Surface> surf;
    vector<PathNode> node;
    vector<Object *> obj;
    vector<Light> light;
    
    GridCell *grid;
    Vector2D gridLL, gridUR;
    float gridCellSize;
    int gridCols;
    int gridRows;
    
    // Dynamic geometry
    vector<Door> door;
    vector<Surface> mask;

    int maxTrigger;
    
    int dlistIndex;
    
    vector<string> introText;
};  

void ltrim(char *str);
string nextToken(string line, int &start, int &end);
string fetchLevelName(char *file);

#endif
