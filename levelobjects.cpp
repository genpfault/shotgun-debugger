/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * levelobjects.cpp - level object functions
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

Configuration config;

void LevelObject::handleTrigger(bool bullet, bool player)
{
  if (trigger != 0 && doTrigger)
  {
    if (!bullet || (bullet && trigtype & TRIG_WHENSHOT))
    {
      if ((trigger == -1 && player) || trigger != -1)
      {
        if (trigtype & TRIG_ADD)
          incrementTrigger(trigger);
        else
        {
          if (trigtype & TRIG_SOUND1)
            playSound(SND_BUTTON, 3);
          else if (trigtype & TRIG_SOUND2)
            playSound(SND_ALARM, 3);
            
          setTrigger(trigger);
        }
          
        if (!(trigtype & TRIG_EVERY))
          doTrigger = false;
      }
    }
  }
}

// Draws the light spot on the ground
void Light::draw()
{
  if (!drawn)
  {
    if (lightSpot)
    {
      glPushMatrix();
      glTranslatef(pos.c[X], pos.c[Y], 0.1);
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, textures[TEX_LIGHT]);
      glColor4f(color.r, color.g, color.b, color.a*0.5);
      glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex2f(-radius, radius);
        glTexCoord2f(1.0, 0.0);
        glVertex2f(radius, radius);
        glTexCoord2f(1.0, 1.0);
        glVertex2f(radius, -radius);
        glTexCoord2f(0.0, 1.0);
        glVertex2f(-radius, -radius);
      glEnd();
      glPopMatrix();
    }
    drawn = true;
  }
}

void Wall::setColor(float r, float g, float b, float a)
{
  baseColor.set(r, g, b, a);
  litColor1.set(r, g, b, a);
  litColor2.set(r, g, b, a);
}  

void Wall::applyAmbientLight(RGBAColor light, float direction)
{
  if (lit)
  {
    litColor1.blend(light, USE_ALPHA);
    litColor2.blend(light, USE_ALPHA);
    float normAngle = atan2(-normal.c[Y], -normal.c[X]);
    if (normAngle < 0)
      normAngle += 2*PI;
    if (direction < 0)
      direction += 2*PI;
    
    // recenter the angle's scale so that normAngle is the relative
    // difference in angle from direction
    float diff = normAngle - direction;
    
    if (diff > PI)
      diff -= 2*PI;
    
    diff = fabs(diff)/PI;
    
    litColor1.blend(RGBAColor(0.0, 0.0, 0.0, 1.0), diff*0.50);
    litColor2.blend(RGBAColor(0.0, 0.0, 0.0, 1.0), diff*0.50);
  }  
}

void Wall::applyLightSources(vector<Light> lights)
{
  if (lit)
  {
    Vector2D testPoint1 = p1 + normal;
    Vector2D testPoint2 = p2 + normal;
    for (int i = 0; i < lights.size(); i++)
    {
      if (dist(testPoint1, lights[i].pos) <= lights[i].radius)
        litColor1.blend(lights[i].color, ((lights[i].radius-dist(testPoint1, lights[i].pos))/lights[i].radius)*lights[i].Strength());
      if (dist(testPoint2, lights[i].pos) <= lights[i].radius)
        litColor2.blend(lights[i].color, ((lights[i].radius-dist(testPoint2, lights[i].pos))/lights[i].radius)*lights[i].Strength());
    }
  }
}

void Wall::print()
{
  printf("(%0.3f, %0.3f)-(%0.3f, %0.3f), ", p1.c[X], p1.c[Y], p2.c[X], p2.c[Y]);
  baseColor.print();
}

void Wall::draw()
{
  if (!drawn)
  {
    draw(true);
    drawn = true;
  }
}

void Wall::draw(bool d)
{
  glDisable(GL_TEXTURE_2D);
  
  glBegin(GL_QUADS);
    litColor1.apply();
    glVertex3f(p1.c[X], p1.c[Y], 0);
    litColor2.apply();
    glVertex3f(p2.c[X], p2.c[Y], 0);
    glVertex3f(p2.c[X], p2.c[Y], height);
    litColor1.apply();
    glVertex3f(p1.c[X], p1.c[Y], height);
  glEnd();
  
  // draw normals
  if (config.normals)
  {
    Vector2D midp = midpoint();
        
    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_LINES);
      glVertex3f(midp.c[X], midp.c[Y], height/2);
      glVertex3f(midp.c[X]+normal.c[X], midp.c[Y]+normal.c[Y], height/2);
    glEnd();
    
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
      glVertex3f(midp.c[X], midp.c[Y], height/2);
      glVertex3f(midp.c[X]-normal.c[X], midp.c[Y]-normal.c[Y], height/2);
    glEnd();
  }
}

void Wall::spewParticles(Vector2D loc)
{
  if (particleType != 0)
    LAUNCH_MULTI_PARTICLES(particleQty, PART_PARTICLE-1+particleType, loc.c[X], loc.c[Y], 0, torad(rand()%360), 1.0, 0);
}

Surface::Surface(float r, float g, float b, float a, float h, float fric, float hlth, int pt, float pq, bool lt, int cf, float fs, int tr, int tt, bool re, int rt, int numVerts, ...)
{
  drawn = false;
  setColor(r, g, b, a);
  friction = fric; health = hlth; height = h;
  particleType = pt; particleQty = pq;
  collFlags = cf;
  lit = lt;
  fadeDirection = 0;
  fadeSpeed = fs;
  
  trigInit(tr, tt);
  
  reobscure = re;
  respondTo = rt;
  triggered = false;
    
  va_list ap;
  va_start(ap, numVerts);
  for (int i = 0; i < numVerts; i++)
    addVert(va_arg(ap, double), va_arg(ap, double));
}

void Surface::set(float r, float g, float b, float a, float h, float fric, float hlth, int pt, float pq, bool lt, int cf, float fs, int tr, int tt, bool re, int rt, int numVerts, ...)
{
  drawn = false; 
  setColor(r, g, b, a);
  friction = fric; health = hlth; height = h;
  particleType = pt; particleQty = pq;
  collFlags = cf;
  lit = lt;
  fadeDirection = 0;
  fadeSpeed = fs;
  trigInit(tr, tt);
  
  reobscure = re;
  respondTo = rt;
  triggered = false;
  
  va_list ap;
  va_start(ap, numVerts);
  for (int i = 0; i < numVerts; i++)
    addVert(va_arg(ap, double), va_arg(ap, double));
}

void Surface::addVert(GLdouble x, GLdouble y)
{
  vert.push_back(Vector2Dd(x, y));
  bb.addPoint(Vector2D(x, y));
}

void Surface::tessellate()
{
  dlistIndex = glGenLists(1);
  glNewList(dlistIndex, GL_COMPILE);
  gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_POSITIVE);
  gluTessBeginPolygon(tess, NULL);
  gluTessBeginContour(tess);
  
  for (int i = 0; i < vert.size(); i++)
  {
    glNormal3f(0.0, 0.0, 1.0);
    gluTessVertex(tess, vert[i].c, vert[i].c);
  }

  gluTessEndContour(tess);
  gluTessEndPolygon(tess);
  glEndList();
}

void Surface::setColor(float r, float g, float b, float a)
{
  baseColor.set(r, g, b, a);
  litColor.set(r, g, b, a);
}  

void Surface::applyAmbientLight(RGBAColor light)
{
  if (lit)
    litColor.blend(light, USE_ALPHA);
}

void Surface::draw()
{
  if (!drawn)
  {
    draw(true);
    drawn = true;
  }
}

void Surface::draw(bool d)
{
  glDisable(GL_TEXTURE_2D);
  litColor.apply();
  glPushMatrix();
  glTranslatef(0, 0, height);
  
  glBegin(GL_POLYGON);
  
    for (vector<Vector2Dd>::iterator i = vert.begin(); i != vert.end(); i++)
      i->apply();
    
  glEnd();
  glPopMatrix();
}

void Surface::drawDL()
{
  if (!drawn || !config.culling)
  {
    drawDL(true);
    if (config.culling) drawn = true;
  }
}

void Surface::drawDL(bool d)
{
  glDisable(GL_TEXTURE_2D);
  litColor.apply();
  glPushMatrix();
  glTranslatef(0, 0, height);
  glCallList(dlistIndex);  
  glPopMatrix();
}

void Surface::spewParticles(Vector2D loc)
{
  if (particleType != 0)
    LAUNCH_MULTI_PARTICLES(particleQty, PART_PARTICLE-1+particleType, loc.c[X], loc.c[Y], 0, torad(rand()%360), 1.0, 0);
}

void Surface::fade()
{
  if (triggers[respondTo].hit && !triggered)
  {
    fadeDirection = (litColor.a > 0.5) ? 1 : -1;
    reobscure = false;
    triggered = true;
  }
    
  if (fadeDirection)
  {
    litColor.a -= fadeSpeed * fadeDirection * timer.dT();
    
    if (litColor.a <= 0.0)
    { fadeDirection = 0; litColor.a = 0.0; }
    if (litColor.a >= 1.0)
    { fadeDirection = 0; litColor.a = 1.0; }
  }
}

Door::Door(float p1x, float p1y, float p2x, float p2y, float r, float g, float b, float a, float h, float frc, float hlth, int pt, 
         float pq, bool lt, int cf, int ot, float spd, float th, float dl, int k, int rt1, int rt2, int rt3, int fl, int tr, int tt)
{
  drawn = false; 
  p1.set(p1x, p1y);
  p2.set(p2x, p2y);
  direction = p2 - p1;
  radius = fabs(direction.mag())/2;
  direction.normalize();
  openType = ot;
  speed = spd;
  thickness = th;
  delay = dl;
  key = currentKey = k;
  respondTo1 = rt1;
  respondTo2 = rt2;
  respondTo3 = rt3;
  flags = fl;
  trigInit(tr, tt);
  reset();
  pos = 1.0;
  moving = 0;
  doorTime = 0;
  waiting = false;
  jammed = false;
  // Create the walls
  Line l(p1, p2);
  midpoint = l.midpoint();
  sides[0].set(p1.c[X]+l.normal.c[X]*thickness*0.5, p1.c[Y]+l.normal.c[Y]*thickness*0.5,
               p2.c[X]+l.normal.c[X]*thickness*0.5, p2.c[Y]+l.normal.c[Y]*thickness*0.5,
               r, g, b, a, h, frc, hlth, pt, pq, lt, cf, 0, 0);
  sides[1].set(p2.c[X]+l.normal.c[X]*thickness*0.5, p2.c[Y]+l.normal.c[Y]*thickness*0.5,
               p2.c[X]-l.normal.c[X]*thickness*0.5, p2.c[Y]-l.normal.c[Y]*thickness*0.5,
               r, g, b, a, h, frc, hlth, pt, pq, lt, cf, 0, 0);
  sides[2].set(p2.c[X]-l.normal.c[X]*thickness*0.5, p2.c[Y]-l.normal.c[Y]*thickness*0.5,
               p1.c[X]-l.normal.c[X]*thickness*0.5, p1.c[Y]-l.normal.c[Y]*thickness*0.5,
               r, g, b, a, h, frc, hlth, pt, pq, lt, cf, 0, 0);
  sides[3].set(p1.c[X]-l.normal.c[X]*thickness*0.5, p1.c[Y]-l.normal.c[Y]*thickness*0.5,
               p1.c[X]+l.normal.c[X]*thickness*0.5, p1.c[Y]+l.normal.c[Y]*thickness*0.5,
               r, g, b, a, h, frc, hlth, pt, pq, lt, cf, 0, 0);
  
  bb.addPoint(Vector2D(p2.c[X]-l.normal.c[X]*thickness*0.5, p2.c[Y]-l.normal.c[Y]*thickness*0.5));
  bb.addPoint(Vector2D(p1.c[X]-l.normal.c[X]*thickness*0.5, p1.c[Y]-l.normal.c[Y]*thickness*0.5));
  bb.addPoint(midpoint+(l.normal*radius));
  bb.addPoint(midpoint-(l.normal*radius));
               
  top.set(r, g, b, a, h, 0, 0, 0, 0, lt, 0, 0, 0, 0, false, 0, 4, 0,0,  0,0,  0,0,  0,0);
  updateCoords();
}

void Door::reset()
{
  pos = 1.0;
  moving = 0;
  doorTime = 0;
  waiting = false;
  jammed = false;
  updateCoords();
  //printf("pos: %f moving: %d\n", pos, moving);
}

void Door::open()
{
  moving = 1;

  if (pos == 1.0 && flags & DOOR_SOUND)
    playSound(SND_DOOR_OPEN, 6);
    
  handleTrigger(false, false);
}
 
void Door::close()
{
  if (!jammed)
  {
    moving = -1;
    
    if (pos <= 0.011 && flags & DOOR_SOUND)
      playSound(SND_DOOR_CLOSE, 6);
  }
}

void Door::update()
{
  // Check the triggers
  
  // Unlocks/locks if trigger 1 is hit
  if (triggers[respondTo1].hit)
  {
    currentKey = 0;
    
    if (flags & DOOR_SOUND)
      playSound(SND_DOOR_UNLOCK, 6);
  }
  
  // Open the door if trigger 2 is hit
  if (moving == 0 && triggers[respondTo2].hit)
    open();

  // Jam the door if trigger 3 is hit
  if (triggers[respondTo3].hit)
  {
    jammed = true;
    open();
  }
    
  if (moving)
  {
    if (!jammed || (jammed && moving != -1))
      pos -= speed * moving * timer.dT();
    
    if (pos <= 0.011) { pos = 0.011; moving = 0; doorTime = delay; waiting = true; }
    if (pos >= 1.0)   { pos = 1.0; moving = 0; renewTrigger(); }
    
    updateCoords();
  }

  if (waiting)
  {
    if (doorTime > 0)
      doorTime -= timer.dT();
    else
      { doorTime = 0; waiting = false; close(); }
  }
  //else
  //{ doorTime = 0; close(); }
}

void Door::updateCoords()
{
  Vector2D edge1 = p1;
  Vector2D edge2 = p2;
  
  switch(openType)
  {
    case DOOR_SLIDE1:
      edge2 = edge1.interpolate(p2, pos);
      break;
    case DOOR_SLIDE2:
      edge1 = edge2.interpolate(p1, pos);
      break;
  }
    
  Line l(edge1, edge2);
  sides[0].setCoords(edge1.c[X]+l.normal.c[X]*thickness*0.5, edge1.c[Y]+l.normal.c[Y]*thickness*0.5,
               edge2.c[X]+l.normal.c[X]*thickness*0.5, edge2.c[Y]+l.normal.c[Y]*thickness*0.5);
  sides[1].setCoords(edge2.c[X]+l.normal.c[X]*thickness*0.5, edge2.c[Y]+l.normal.c[Y]*thickness*0.5,
               edge2.c[X]-l.normal.c[X]*thickness*0.5, edge2.c[Y]-l.normal.c[Y]*thickness*0.5);
  sides[2].setCoords(edge2.c[X]-l.normal.c[X]*thickness*0.5, edge2.c[Y]-l.normal.c[Y]*thickness*0.5,
               edge1.c[X]-l.normal.c[X]*thickness*0.5, edge1.c[Y]-l.normal.c[Y]*thickness*0.5);
  sides[3].setCoords(edge1.c[X]-l.normal.c[X]*thickness*0.5, edge1.c[Y]-l.normal.c[Y]*thickness*0.5,
               edge1.c[X]+l.normal.c[X]*thickness*0.5, edge1.c[Y]+l.normal.c[Y]*thickness*0.5);
    
  top.setVert(0, edge1.c[X]+l.normal.c[X]*thickness*0.5, edge1.c[Y]+l.normal.c[Y]*thickness*0.5);
  top.setVert(1, edge2.c[X]+l.normal.c[X]*thickness*0.5, edge2.c[Y]+l.normal.c[Y]*thickness*0.5),
  top.setVert(2, edge2.c[X]-l.normal.c[X]*thickness*0.5, edge2.c[Y]-l.normal.c[Y]*thickness*0.5),
  top.setVert(3, edge1.c[X]-l.normal.c[X]*thickness*0.5, edge1.c[Y]-l.normal.c[Y]*thickness*0.5);    
}

void Door::detect(Vector2D p, int keys)
{
  if (distSquared(midpoint, p) <= radius*radius)
  {
    if (pos != 1.0 || currentKey == 0)
      open();
    
    if (keys != -1)
    {
      if (HAS_KEY(keys,currentKey))   
        open();
      else if (currentKey > 0 && flags & DOOR_MESSAGE && pos >= 1.0)
        setMessage("Need key %d to unlock this door", currentKey);
      else if (currentKey == -1 && flags & DOOR_MESSAGE && pos >= 1.0)
        setMessage("This door is locked");
    }
  }
}

void Door::draw()
{
  if (!drawn || !config.culling)
  {
    for (int i = 0; i < 4; i++)
      sides[i].draw(true);
  
    top.draw(true);
    if (config.culling) drawn = true;
  }
}

void Door::applyAmbientLight(RGBAColor light, float direction)
{
  for (int i = 0; i < 4; i++)
    sides[i].applyAmbientLight(light, direction);
    
  //top.applyAmbientLight(light);
}

void Door::applyLightSources(vector<Light> lights)
{
  for (int i = 0; i < 4; i++)
    sides[i].applyLightSources(lights);
}
