/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * interface.cpp - user interface routines
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
#include <string>

void TerminalWindow::set(float px, float py, int r, int c, float cw, float ch, float t, float rt, RGBAColor fg, RGBAColor bg)
{
  x = px; y = py;
  rows = r; cols = c;
  charWidth = cw; charHeight = ch;
  thick = t;
  rate = rt;
  reset();
  
  calculateDimensions();
  
  if (x == CENTERED) x = -w/2;
  if (y == CENTERED) y = h/2;
  
  fgColor = fg;
  bgColor = bg;
  
  active = fullyDrawn = false;
  
  cursor = false;
  curTop = curBottom = curPos = curCol = 0;
  curString = "";
}

void TerminalWindow::setCursor(int ct, int cb, int cc, int cp, string st, int ev)
{
  cursor = true;
  curTop = ct;
  curBottom = cb;
  curCol = cc;
  curPos = cp;
  curString = st;
  escVal = ev;
}

void TerminalWindow::activate(Uint8 *keystate)
{
  activate();
  upPr = keystate[SDLK_UP];
  downPr = keystate[SDLK_DOWN];
  enterPr = keystate[SDLK_RETURN];
  escPr = keystate[SDLK_ESCAPE];
}

void TerminalWindow::draw()
{
  if (active)
  {
    glEnable(GL_TEXTURE_2D);
    
    glBindTexture(GL_TEXTURE_2D, textures[TEX_BORDER]);
    glBegin(GL_QUADS);
      glColor4f(1.0, 1.0, 1.0, (fullyDrawn) ? 1.0 :
        ((drawTime > 0.1) ? 1.0 : 0.0));
      
      glTexCoord2f(0.0, 0.0); glVertex2f(x, y);
      glTexCoord2f(1.0, 0.0); glVertex2f(x+w, y);
      glTexCoord2f(1.0, 1.0); glVertex2f(x+w-thick, y-thick);
      glTexCoord2f(0.0, 1.0); glVertex2f(x+thick, y-thick);
      
      glColor4f(1.0, 1.0, 1.0, (fullyDrawn) ? 1.0 :
        ((drawTime > 0.2) ? 1.0 : 0.0));
      
      glTexCoord2f(0.0, 0.0); glVertex2f(x, y);
      glTexCoord2f(0.0, 1.0); glVertex2f(x+thick, y-thick);
      glTexCoord2f(1.0, 1.0); glVertex2f(x+thick, y-h+thick);
      glTexCoord2f(1.0, 0.0); glVertex2f(x, y-h);
      
      glColor4f(1.0, 1.0, 1.0, (fullyDrawn) ? 1.0 :
        ((drawTime > 0.3) ? 1.0 : 0.0));
      
      glTexCoord2f(0.0, 0.0); glVertex2f(x, y-h);
      glTexCoord2f(1.0, 0.0); glVertex2f(x+w, y-h);
      glTexCoord2f(1.0, 1.0); glVertex2f(x+w-thick, y-h+thick);
      glTexCoord2f(0.0, 1.0); glVertex2f(x+thick, y-h+thick);

      glColor4f(1.0, 1.0, 1.0, (fullyDrawn) ? 1.0 :
        ((drawTime > 0.4) ? 1.0 : 0.0));
            
      glTexCoord2f(0.0, 0.0); glVertex2f(x+w, y);
      glTexCoord2f(0.0, 1.0); glVertex2f(x+w-thick, y-thick);
      glTexCoord2f(1.0, 1.0); glVertex2f(x+w-thick, y-h+thick);
      glTexCoord2f(1.0, 0.0); glVertex2f(x+w, y-h);
    glEnd();
    
    bgColor.apply();
    glBindTexture(GL_TEXTURE_2D, textures[TEX_TERMINAL_BG]);
    
    float tmpH = (fullyDrawn) ? h-thick : ((drawTime > 0.4) ? (h-thick)*MIN((drawTime-0.4)/0.2, 1.0) : thick);
    
    glBegin(GL_QUADS);
      glTexCoord2f(0.0, 0.0); glVertex2f(x+thick, y-thick);
      glTexCoord2f(1.0, 0.0); glVertex2f(x+w-thick, y-thick);
      glTexCoord2f(1.0, tmpH); glVertex2f(x+w-thick, y-tmpH);
      glTexCoord2f(0.0, tmpH); glVertex2f(x+thick, y-tmpH);
    glEnd();
    
    if (fullyDrawn)
    {
      for (int i = 0; i < rows; i++)
      {
        if (i >= lines.size()) break;
       
        switch (styles[rowStart+i])
        {
          case 0: fgColor.apply(); break;
          case 1: glColor3f(1.0, 1.0, 1.0); break;
          case 2: glColor3f(0.0, 1.0, 0.0); break;
          case 3: glColor3f(1.0, 1.0, 0.0); break;
        }
          
        if (i < line-rowStart)
          font.printf(x+thick+charWidth,
          (y-thick-charHeight)-(i*(charHeight*1.5)),
          charWidth/0.75, charHeight*0.75, "%s", lines[rowStart+i].c_str());
        else if (i == line-rowStart)
          font.printf(x+thick+charWidth,
          (y-thick-charHeight)-(i*(charHeight*1.5)),
          charWidth/0.75, charHeight*0.75, "%s",
          (lines[rowStart+i].substr(0,ch)).c_str());
      }
      
      if (cursor && linesDisplayed == lines.size())
      {
        font.printf(x+thick+(charWidth*(curCol+1)),
        (y-thick-charHeight)-(curPos*(charHeight*1.5)),
        charWidth/0.75, charHeight*0.75, "%s", curString.c_str());
      }
    }
  }
}

void TerminalWindow::addString(int style, const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  char buf[513];
  vsnprintf(buf, 512, fmt, ap);
  
  string s = buf;
  
  styles.push_back(style);
  lines.push_back(s.substr(0, cols));
  if (lines.size() > rows)
    rowStart = lines.size() - rows;
}

void TerminalWindow::changeString(int line, int style, const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  char buf[513];
  vsnprintf(buf, 512, fmt, ap);
  
  string s = buf;
  
  styles[line] = style;
  lines[line] = s.substr(0, cols);
}

// Returns -1 most of the time. But if the cursor is enabled
// and the player hits Enter, returns the cursor position.
int TerminalWindow::update(Uint8 *keystate)
{
  int retVal = -1;
  if (active)
  {
    if (!fullyDrawn)
    {
      drawTime += timer.dT();
      
      if (drawTime >= 0.6)
        fullyDrawn = true;
    }
    else
    {
      if (linesDisplayed < lines.size())
      {     
        printTime += timer.dT();
        ch = (int)((float)cols*(printTime/rate));
        if (ch >= lines[line].length())
        { 
          ch = 0;
          line++;
          linesDisplayed++;
          printTime = 0.0;
          
          if (line < lines.size())
          {
            ch = lines[line].find_first_not_of(" ");
            if (ch < 0) ch = 0;
            printTime = rate*((float)ch/(float)cols);
          }
        }
        
        if (printTime >= rate)
          printTime = 0.0;
      }
    }
    
    if (cursor)
    {
      if (keystate[SDLK_UP])
      {
        if (!upPr) 
        {
          if (!fullyDrawn || linesDisplayed != lines.size())
            flush();
           
          cursorUp();
          upPr = true;
         }
      }
      else upPr = false;
        
      if (keystate[SDLK_DOWN])
      {
        if (!downPr)
        {
          if (!fullyDrawn || linesDisplayed != lines.size())
            flush();
            
          cursorDown();
          downPr = true;
        }
      }  
      else downPr = false;
      
      if (keystate[SDLK_RETURN])
      {
        if (!enterPr)
        {
          if (fullyDrawn && linesDisplayed == lines.size())
            retVal = curPos;
          else
            flush();
            
          enterPr = true;
        } 
      }
      else
        enterPr = false;
        
      if (keystate[SDLK_ESCAPE])
      {
        if (!escPr)
        {
          retVal = escVal;            
          escPr = true;
        }
      }
      else
        escPr = false;
    }
  }
  
  return retVal;
}

int setMenu(TerminalWindow *m, int type, Uint8 *keystate)
{
  string levelName, line;
  char buf[4];
  double foo;
  int r;
  switch(type)
  {
    case TITLE_MAINMENU:
      m->set(32.0, CENTERED, 8+((P.startingLevel > -1 && P.level > 0)?1:0), 17, 2.0, 3.0, 3, 0.25, RGBAColor(0.5, 1.0, 0.76, 1.0), RGBAColor(0.5, 1.0, 0.76, 0.2));
      m->setCursor(0, 5+((P.startingLevel > -1 && P.level > 0)?1:0), 0, 0, ">", -1);
      
      if (P.startingLevel > -1 && P.level > 0)
        m->addString(0, "  continue game");
      
      m->addString(0, "  new game");
      m->addString(0, "  options");
      m->addString(0, "  records");
      m->addString(0, "  help");
      m->addString(0, "  credits");
      m->addString(0, "  exit");
      m->addString(0, "");
      m->addString(0, "(sdb)");
      m->activate();
      break;
    case TITLE_CONTINUE:
      if (P.level < NUM_LEVELS)
        levelName = fetchLevelName((char *)getLevelFileName(P.level).c_str());
      else if (P.level == NUM_LEVELS)
        levelName = "(completed)";
        
      m->set(10.0, CENTERED, 9, MAX(14, levelName.length()+3), 2.0, 3.0, 3, 0.25, RGBAColor(0.5, 1.0, 0.76, 1.0), RGBAColor(0.5, 1.0, 0.76, 0.2));
      m->setCursor(5, 6, 0, 5, ">", -1);
      m->addString(1, "Continue game?");
      m->addString(0, "");
      m->addString(0, "%02d %s", P.level+((P.level != NUM_LEVELS)?1:0), levelName.c_str());
      if (P.startingLevel == 0 && !P.cheated && !P.usedAugment)
        m->addString(0, "%3d:%02d.%02d", (int)(P.totalTime()/60), (int)fmod(P.totalTime(), 60),
          (int)(modf(P.totalTime(), &foo)*100));
      else
        m->addString(0, "");
          
      m->addString(0, "");
      m->addString(0, "  yes");
      m->addString(0, "  no");
      m->addString(0, "");
      m->addString(0, "(sdb)");
      m->activate();
      break;
    case TITLE_NEWGAME:
      m->set(10.0, CENTERED, (P.levelProgress+1)+5, 24, 2.0, 3.0, 3, 0.25, RGBAColor(0.5, 1.0, 0.76, 1.0), RGBAColor(0.5, 1.0, 0.76, 0.2));
      m->setCursor(2, 3+P.levelProgress, 0, 3, ">", -1);
      m->addString(1, "Select starting level");
      m->addString(0, "");
      m->addString(0, "  (cancel)");
      
      // Get the titles of all unlocked levels
      for (int i = 1; i <= P.levelProgress+1; i++)
      {
        line = "  ";
        snprintf(buf, 4, "%02d", i);
        line += buf;
        line += " ";
        
        levelName = fetchLevelName((char *)getLevelFileName(i-1).c_str());
        if (levelName.length()+5 > m->Cols())
          m->setCols(levelName.length()+5);

        line += levelName;
        m->addString(0, "%s", line.c_str());
      }

      m->addString(0, "");
      m->addString(0, "(sdb)");
      m->activate();
      break;
    case TITLE_RECORDS:
      m->set(CENTERED, CENTERED, NUM_LEVELS+6, 91, 2.0, 3.0, 3, 0.8, RGBAColor(0.5, 1.0, 0.76, 1.0), RGBAColor(0.5, 1.0, 0.76, 0.2));
      m->setCursor(NUM_LEVELS+4, NUM_LEVELS+5, 0, NUM_LEVELS+4, ">", -1);
      m->addString(0, "                         Level | Fastest time | Most kills | Fewest shots | Least times hit");
      m->addString(0, "-------------------------------+--------------+------------+--------------+----------------");
      r = 2;                                                
      for (int i = 0; i < NUM_LEVELS+1; i++)
      {
        if (P.levelRecords[i].time > 0.0)
        {
          levelName = (i < NUM_LEVELS) ? fetchLevelName((char *)getLevelFileName(i).c_str()) : "Full game";
          m->addString((i < NUM_LEVELS)?0:1, "%30s |     %2d:%02d.%02d |      %5d |        %5d |           %5d",
          levelName.c_str(), (int)(P.levelRecords[i].time/60), (int)fmod(P.levelRecords[i].time, 60),
            (int)(modf(P.levelRecords[i].time, &foo)*100), P.levelRecords[i].kills,
            P.levelRecords[i].shots, P.levelRecords[i].hits);
          r++;
        }
      }
      
      for (int i = 0; i < NUM_LEVELS+3-r; i++)
        m->addString(0, "");
      
      m->addString(0, "");
      m->addString(0, "  return to menu");
      m->addString(0, "  clear records");
      
      m->activate();
      break;
    case TITLE_HELP:
      m->set(CENTERED, CENTERED, 18, 64, 2.0, 3.0, 3, 0.4, RGBAColor(0.5, 1.0, 0.76, 1.0), RGBAColor(0.5, 1.0, 0.76, 0.2));
      m->setCursor(17, 17, 0, 17, ">", -1);
      m->addString(0, "                        CONTROLS                        ");
      m->addString(0, "");
      m->addString(0, "move forward:   %12s | turn left:  %12s or mouse",
                   SDL_GetKeyName(config.keys[KEY_FWD]), SDL_GetKeyName(config.keys[KEY_LEFT]));
      m->addString(0, "move backward:  %12s | turn right: %12s or mouse",
                   SDL_GetKeyName(config.keys[KEY_BACK]), SDL_GetKeyName(config.keys[KEY_RIGHT]));
      m->addString(0, "sidestep left:  %12s | jump:       %12s",
                   SDL_GetKeyName(config.keys[KEY_STRAFE_L]), SDL_GetKeyName(config.keys[KEY_JUMP]));
      m->addString(0, "sidestep right: %12s |", SDL_GetKeyName(config.keys[KEY_STRAFE_R]));
      m->addString(0, "----------------------------------------------------------------");
      m->addString(0, "       fire weapon:    %12s or left mouse button",
                   SDL_GetKeyName(config.keys[KEY_FIRE]));
      m->addString(0, "       throw grenade:  %12s or right mouse button",
                   SDL_GetKeyName(config.keys[KEY_GRENADE]));
      m->addString(0, "       reload weapon:  %12s", SDL_GetKeyName(config.keys[KEY_RELOAD]));
      m->addString(0, "       switch weapon:  %12s", SDL_GetKeyName(config.keys[KEY_SWITCH_WPN]));
      m->addString(0, "       select weapon:   numbers 1-7");
      m->addString(0, "----------------------------------------------------------------");
      m->addString(0, "       switch view:    %12s", SDL_GetKeyName(config.keys[KEY_SWITCH_VIEW]));
      m->addString(0, "       quick zoom out: %12s or middle mouse button",
                   SDL_GetKeyName(config.keys[KEY_ZOOM_OUT]));
      m->addString(0, "       options:                  F2");
      m->addString(0, "       pause/abort:             ESC");
      m->addString(0, "  press " ENTER_KEY " to return to menu");
      m->activate();
      break;
    case TITLE_CREDITS:
      m->set(CENTERED, CENTERED, 19, 68, 2.0, 3.0, 3, 0.8, RGBAColor(1.0, 1.0, 1.0, 1.0), RGBAColor(1.0, 1.0, 1.0, 0.2));
      m->setCursor(18, 18, 0, 18, ">", -1);
      m->addString(0, "                          SHOTGUN DEBUGGER");
      m->addString(0, "");
      m->addString(0, "project leader, programming, concepts, 2d art          MATT SARNOFF");
      m->addString(0, "lead level designer, design feedback                   CHRIS DELEON");
      m->addString(0, "character models                                       JOHN NESKY");
      m->addString(0, "level designers                                        GREGORY PENG");
      m->addString(0, "                                                       JEFF THOENE");
      m->addString(0, "sound effects                                          CHRIS DELEON");
      m->addString(0, "                                                       MATT SARNOFF");
      m->addString(0, "music                                                  TUSCAN KNOX");
      m->addString(0, "testing                                                MICHAEL WEBER");
      m->addString(0, "");
      m->addString(0, "A project of the Game Creation Society at Carnegie Mellon University");
      m->addString(0, "                        sdb.gamecreation.org");
      m->addString(0, "");
      m->addString(0, "Version %s; %s", PACKAGE_VERSION, PACKAGE_RELEASE_DATE);
      m->addString(0, "Copyright 2005 Game Creation Society; Released under the GNU GPL");
      m->addString(0, "");
      m->addString(0, "  press " ENTER_KEY " to return to menu");
      m->activate();
      break;
    case TITLE_EXIT:
      m->set(32.0, CENTERED, 6, 5, 2.0, 3.0, 3, 0.1, RGBAColor(0.5, 1.0, 0.76, 1.0), RGBAColor(0.5, 1.0, 0.76, 0.2));
      m->setCursor(2, 3, 0, 2, ">", -1);
      m->addString(1, "Exit?");
      m->addString(0, "");
      m->addString(0, "  yes");
      m->addString(0, "  no");
      m->addString(0, "");
      m->addString(0, "(sdb)");
      m->activate();
      m->flush();
      break;
    case TITLE_NEWCONFIRM:
      m->set(32.0, CENTERED, 8, 20, 2.0, 3.0, 3, 0.1, RGBAColor(0.5, 1.0, 0.76, 1.0), RGBAColor(0.5, 1.0, 0.76, 0.2));
      m->setCursor(4, 5, 0, 4, ">", -1);
      m->addString(1, "This will erase your");
      m->addString(1, "current saved game.");
      m->addString(1, "Proceed?");
      m->addString(0, "");
      m->addString(0, "  yes");
      m->addString(0, "  no");
      m->addString(0, "");
      m->addString(0, "(sdb)");
      m->activate();
      break;
    case TITLE_RCLEAR:
      m->set(32.0, CENTERED, 6, 20, 2.0, 3.0, 3, 0.1, RGBAColor(0.5, 1.0, 0.76, 1.0), RGBAColor(0.5, 1.0, 0.76, 0.2));
      m->setCursor(2, 3, 0, 3, ">", -1);
      m->addString(1, "Erase all records?");
      m->addString(0, "");
      m->addString(0, "  yes");
      m->addString(0, "  no");
      m->addString(0, "");
      m->addString(0, "(sdb)");
      m->activate();
      break;
    case OPTIONS_ECONFIRM:
      m->set(CENTERED, CENTERED, 8, 20, 2.0, 3.0, 3, 0.1, RGBAColor(0.5, 1.0, 0.76, 1.0), RGBAColor(0.5, 1.0, 0.76, 0.2));
      m->setCursor(4, 5, 0, 5, ">", -1);
      m->addString(1, "This will erase your");
      m->addString(1, "current saved game.");
      m->addString(1, "Proceed?");
      m->addString(0, "");
      m->addString(0, "  yes");
      m->addString(0, "  no");
      m->addString(0, "");
      m->addString(0, "(sdb)");
      m->activate();
      break;
    case OPTIONS_RCONFIRM:
      m->set(CENTERED, CENTERED, 8, 33, 2.0, 3.0, 3, 0.1, RGBAColor(0.5, 1.0, 0.76, 1.0), RGBAColor(0.5, 1.0, 0.76, 0.2));
      m->setCursor(4, 5, 0, 5, ">", -1);
      m->addString(1, "This will erase ALL game progress");
      m->addString(1, "and your current saved game.");
      m->addString(1, "Proceed?");
      m->addString(0, "");
      m->addString(0, "  yes");
      m->addString(0, "  no");
      m->addString(0, "");
      m->addString(0, "(sdb)");
      m->activate();
      break;
  case OPTIONS_KEYS:
    m->set(CENTERED, CENTERED, 18, 30, 2.0, 3.0, 3, 0.1, RGBAColor(0.5, 1.0, 0.76, 1.0), RGBAColor(0.5, 1.0, 0.76, 0.2));
    m->setCursor(1, 15, 0, 1, ">", -1);
    m->addString(0, "        CUSTOMIZE KEYS");

    for (int i = 0; i < 13; i++)
      m->addString(0, "");

    m->addString(0, "  Defaults");
    m->addString(0, "  Save and return");
    m->addString(0, "");
    m->addString(0, "(sdb)");
    break;
  }
  
  m->resetKeys(keystate);
  return type;
}

int title(bool first, bool playInSequence)
{
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_LIGHTING);  
  glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  SCREEN_TOP = 100/SCREEN_ASPECT;
  glOrtho(-100, 100, -SCREEN_TOP, SCREEN_TOP, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glClearColor(0.0, 0.0, 0.0, 1.0);
  
  // Return value. Title screen loops as long as it's 0.
  // Value is -1 if quit, otherwise the level number to load.
  int retVal = 0;
  
  
  int option = 0;
  
  // flush the keys  
  keystate = SDL_GetKeyState(NULL);
  bool enterPressed = keystate[SDLK_RETURN];
  bool escPressed   = keystate[SDLK_ESCAPE];
  bool upPressed    = keystate[SDLK_UP];
  bool downPressed  = keystate[SDLK_DOWN];
  bool xPressed     = keystate[SDLK_x];
  
  TerminalWindow menu;
  menu.deactivate();
    
  int screen;
   
  float scroll1 = 0.0;
  float scroll2 = 0.0;
  
  if (first)
    screen = TITLE_NOMENU;
  else
    { screen = setMenu(&menu, TITLE_MAINMENU, keystate); menu.flush(); }
    
  while (!retVal)
  {
    timer.reset();
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
        retVal = -1;
    }
    
    // Key checking loop
    keystate = SDL_GetKeyState(NULL);
   
    if (keystate[SDLK_RETURN])
    {
      if (!enterPressed)
      {
        if (screen == TITLE_NOMENU)
          screen = setMenu(&menu, TITLE_MAINMENU, keystate);
         
        enterPressed = true;
      }
    }
    else
      enterPressed = false;

    if (keystate[SDLK_ESCAPE])
    {
      if (!escPressed)
      {
        if (screen <= TITLE_MAINMENU)
          retVal = -1;
        else
          screen = setMenu(&menu, TITLE_MAINMENU, keystate);
          
        escPressed = true;
      }
    }
    else
      escPressed = false;
    
    if (keystate[SDLK_x])
    {
      if (!xPressed)
      {
        if (P.canAugment)
        {
          if (!P.augmented)
            playSound(SND_LAUNCH2, -1);
          P.augmented = !P.augmented;
        }
        xPressed = true;    
      }
    }
    else
      xPressed = false;
      
    SCREENSHOT_CHECK
      
    // Menu handler
    // Gotta love the nested switch statements.
    int sel, c;
    switch(screen)
    {
    case TITLE_MAINMENU:
      c = menu.update(keystate);
      if ((P.startingLevel == -1 || P.level <= 0) && c != -1)
        c++;
        
      switch(c)
      {
        case 0: // Continue game
          screen = setMenu(&menu, TITLE_CONTINUE, keystate);
          break;        
        case 1: // New game
          if (P.startingLevel == -1 || P.level <= 0)
            screen = setMenu(&menu, TITLE_NEWGAME, keystate);
          else
            screen = setMenu(&menu, TITLE_NEWCONFIRM, keystate);
          break;
        case 2: // Options
          screen = setMenu(&menu, TITLE_OPTIONS, keystate);
          break;
        case 3: // Records
          screen = setMenu(&menu, TITLE_RECORDS, keystate);
          break;
        case 4: // Help
          screen = setMenu(&menu, TITLE_HELP, keystate);
          break;
        case 5: // Credits
          screen = setMenu(&menu, TITLE_CREDITS, keystate);
          break;
        case 6: // Exit
          screen = setMenu(&menu, TITLE_EXIT, keystate);
          break;
      }
    break;
    case TITLE_CONTINUE:
      sel = menu.update(keystate);
      if (sel == 6)
        screen = setMenu(&menu, TITLE_MAINMENU, keystate);
      else if (sel == 5)
        retVal = -2;
      break;
    case TITLE_NEWGAME:
      if (playInSequence && P.levelProgress > 0)
      {
        sel = menu.update(keystate);
        if (sel == 2) // Cancel
          screen = setMenu(&menu, TITLE_MAINMENU, keystate);
        else if (sel > 2) // Load level
        {
          retVal = sel - 2;
          P.eraseSavedGame();
        }
      }
      else
        retVal = 1;
      break;
    case TITLE_OPTIONS:
      configScreen(false, scroll1, scroll2);
      escPressed = keystate[SDLK_ESCAPE];
      screen = setMenu(&menu, TITLE_MAINMENU, keystate);
      break;
    case TITLE_CREDITS: case TITLE_HELP:
      if (menu.update(keystate) != -1)
        screen = setMenu(&menu, TITLE_MAINMENU, keystate);
      break;
    case TITLE_RECORDS:
      sel = menu.update(keystate);
      if (sel == NUM_LEVELS+4) // Return to menu
        screen = setMenu(&menu, TITLE_MAINMENU, keystate);
      else if (sel == NUM_LEVELS+5) // Clear records
        screen = setMenu(&menu, TITLE_RCLEAR, keystate);
      break;
    case TITLE_EXIT:
      switch(menu.update(keystate))
      {
        case 2: // Exit
          retVal = -1;
          break;
        case 3: // Cancel
          screen = setMenu(&menu, TITLE_MAINMENU, keystate);
          break;
      }
      break;
    case TITLE_NEWCONFIRM:
      switch(menu.update(keystate))
      {
        case 4: // New game
          screen = setMenu(&menu, TITLE_NEWGAME, keystate);
          break;
        case 5: // Cancel
          screen = setMenu(&menu, TITLE_MAINMENU, keystate);
          break;
      }
    case TITLE_RCLEAR:
      switch(menu.update(keystate))
      {
        case 2: // Clear records
          P.eraseRecords();
          playSound(SND_TERM_CLEAR, -1);
        case 3: // Cancel
          screen = setMenu(&menu, TITLE_RECORDS, keystate);
          break;
      }    
    } 
    
    scroll1 += SCROLL1_RATE * timer.dT();
    scroll2 += SCROLL2_RATE * timer.dT();
    
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
        
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColor4f(1.0, 1.0, 1.0, 0.2);
    glBindTexture(GL_TEXTURE_2D, textures[TEX_HEX1]);
    glBegin(GL_QUADS);
      glTexCoord2f(0.0, scroll1);
      glVertex2f(25, 100);
      glTexCoord2f(1.0, scroll1);
      glVertex2f(125, 100);
      glTexCoord2f(1.0, scroll1+1.0);
      glVertex2f(125,-100);
      glTexCoord2f(0.0, scroll1+1.0);
      glVertex2f(25,-100);
    glEnd();
    
    glColor3f(1.0, 1.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, textures[TEX_HEX2]);
    glBegin(GL_QUADS);
      glTexCoord2f(scroll2, 0.0);
      glVertex2f(-100, -28);
      glTexCoord2f(scroll2+1.0, 0.0);
      glVertex2f(100, -28);
      glTexCoord2f(scroll2+1.0, 1.0);
      glVertex2f(100,-31.125);
      glTexCoord2f(scroll2, 1.0);
      glVertex2f(-100,-31.125);
    glEnd();
    
    glColor3f(1.0, 1.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, textures[TEX_TITLE_BG]);
    glBegin(GL_QUADS);
      glTexCoord2f(0.0, 0.0);
      glVertex2f(-100, 100);
      glTexCoord2f(1.0, 0.0);
      glVertex2f( 100, 100);
      glTexCoord2f(1.0, 1.0);
      glVertex2f( 100,-100);
      glTexCoord2f(0.0, 1.0);
      glVertex2f(-100,-100);
    glEnd();
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    if (screen != TITLE_CREDITS && screen != TITLE_RECORDS && screen != TITLE_HELP)
    {
      glColor3ub(192, 245, 156);    
      font.printf(-90.0, 10.0, 6.0, 4.8, "SHOTGUN DEBUGGER");
      glColor3f(0.7, 1.0, 0.5);
      font.printf(-80.0, -35, 2.5, 2.5,    "A project of the Game Creation Society - sdb.gamecreation.org");
#ifndef __MACOSX__
		font.printf(-80.0, LAST_Y, 2.5, 2.5, "Ver. %s - %s", PACKAGE_VERSION, PACKAGE_RELEASE_DATE);
#else
		font.printf(-80.0, LAST_Y, 2.5, 2.5, "Ver. %s - %s (Mac OS X version built %s)", PACKAGE_VERSION, PACKAGE_RELEASE_DATE, __DATE__);
#endif
    }
    
    if (screen == TITLE_NOMENU)
      font.printf(CENTERED, -SCREEN_TOP+10.0, 2.5, 2.5, "Press " ENTER_KEY_CAP);
      
    if (P.augmented)
    {
      glColor3f(1.0, 0.0, 0.0);
      font.printf(-98.0, SCREEN_TOP-2.0, 3.0, 2.0, "* AUGMENTATIONS ACTIVE *");
    }
    
    menu.draw();
    
    SDL_GL_SwapBuffers();
    
    timer.update();
  }
  return retVal;
}

void loadingScreen(int n)
{
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_LIGHTING);  
  glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  SCREEN_TOP = 100/SCREEN_ASPECT;
  glOrtho(-100, 100, -SCREEN_TOP, SCREEN_TOP, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  glLoadIdentity();
  glClear(GL_COLOR_BUFFER_BIT);
  
  glColor3f(1.0, 1.0, 1.0);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textures[TEX_BG2]);
  glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex2f(-100, 100);
    glTexCoord2f(1.0, 0.0);
    glVertex2f( 100, 100);
    glTexCoord2f(1.0, 1.0);
    glVertex2f( 100,-100);
    glTexCoord2f(0.0, 1.0);
    glVertex2f(-100,-100);
  glEnd();
  
  glColor3ub(192, 245, 156);    
  font.LEDprintf(CENTERED, 0, 20.0, 20.0, ">>>>");
  
  glColor4f(0.3, 1.0, 0.3, 0.8); glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  switch(n)
  {
    case 4: font.LEDprintf(CENTERED, 0, 20.0, 20.0, "   ?");
    case 3: font.LEDprintf(CENTERED, 0, 20.0, 20.0, "  ? ");
    case 2: font.LEDprintf(CENTERED, 0, 20.0, 20.0, " ?  ");
    case 1: font.LEDprintf(CENTERED, 0, 20.0, 20.0, "?   ");
  }
  
  SDL_GL_SwapBuffers();
}

void levelIntroScreen(bool final)
{
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_LIGHTING);  
  glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  SCREEN_TOP = 100/SCREEN_ASPECT;
  glOrtho(-100, 100, -SCREEN_TOP, SCREEN_TOP, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  keystate = SDL_GetKeyState(NULL);
  bool enterPressed = keystate[SDLK_RETURN];
  
  bool loop = true;
  float fadeTimer = 0.0;
  float fade = 0.0;
  int linesRead = 0;
  float delayTimer = 0.0;
  float delayAmount = 0.0;
  
  bool textDone = false;
  
  float sx = (P.levelRecords[P.level].time < 0.0 || config.cheating()) ? -63 : -30;
  double foo;
  
  TerminalWindow window;
  
  if (!final)
    window.set(sx, 61, 25, 60, 2.0, 3.0, 3, 1.5, RGBAColor(0.5, 1.0, 0.76, 1.0), RGBAColor(0.5, 1.0, 0.76, 0.1));
  else
  {
    window.set(-63, 61, 25, 60, 2.0, 3.0, 0, 1.5, RGBAColor(0.5, 1.0, 0.76, 1.0), RGBAColor(0.0, 0.0, 0.0, 0.0));
    fade = 1.0;
    fadeTimer = 3.99;
  }
    
  while (loop)
  {
    timer.reset();
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
        loop = false;
    }
    
    // Key checking loop
    keystate = SDL_GetKeyState(NULL);
    
    if (keystate[SDLK_RETURN])
    {
      if (!enterPressed)
      {
        if (!textDone)
        {
          while (linesRead < currLevel.introText.size())
          {
            int style = 0;
            if (currLevel.introText[linesRead] != "")
            {
              if (currLevel.introText[linesRead][0] == '`')
                style = 2;
              else if (currLevel.introText[linesRead][0] == '~')
                style = -1;          }
            
            if (style == 0)
              window.addString(0, "%s", currLevel.introText[linesRead].c_str());
            else if (style > 0)
              window.addString(style, "%s", (currLevel.introText[linesRead].substr(1, currLevel.introText[linesRead].length()-1)).c_str());
            linesRead++;
          }
          window.addString(0, "");
          window.addString(1, "press " ENTER_KEY);
          window.flush();
          textDone = true;
          
          if (!window.isActive())
            fadeTimer = 3.99;          
        }
        else if (textDone)
          loop = false;
          
        enterPressed = true;
      }
    }
    else
      enterPressed = false;
      
    SCREENSHOT_CHECK
      
    if (fadeTimer < 1.0)
      fade = fadeTimer;
    else if (fadeTimer >= 1.0 && fadeTimer <= 2.0)
      fade = 1.0;
    else if (fadeTimer >= 2.0 && fadeTimer < 3.0)
      fade = 1.0 - (fadeTimer - 2.0);
    else
      fade = 0.0;
    
    if (fadeTimer < 4.0)
    { 
      fadeTimer += timer.dT();
      if (fadeTimer >= 4.0)
        window.activate(keystate);
    }
           
    window.update(keystate);
    
    if (!textDone)
    {    
      if (linesRead < currLevel.introText.size())
      {
        if (window.doneDrawing() && delayTimer >= delayAmount)
        {
          int style = 0;
          if (currLevel.introText[linesRead] != "")
          {
            if (currLevel.introText[linesRead][0] == '`')
              style = 2;
            else if (currLevel.introText[linesRead][0] == '~')
            {
              delayAmount = S2F(currLevel.introText[linesRead].substr(1, currLevel.introText[linesRead].length()-1));
              style = -1;
            }
          }
          
          if (style == 0)
            window.addString(0, "%s", currLevel.introText[linesRead].c_str());
          else if (style > 0)
            window.addString(style, "%s", (currLevel.introText[linesRead].substr(1, currLevel.introText[linesRead].length()-1)).c_str());
          linesRead++;
          
          delayTimer = 0.0;
          if (style != -1)
            delayAmount = 0.0;
        }
      }
      else
      {
        window.addString(0, "");
        window.addString(1, "press " ENTER_KEY);
        window.flush();
        textDone = true;
      }
    }
   
    delayTimer += timer.dT();
    
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    
    if (!final)
    {
      glColor3f(1.0, 1.0, 1.0);
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, textures[TEX_BG2]);
      glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex2f(-100, 100);
        glTexCoord2f(1.0, 0.0);
        glVertex2f( 100, 100);
        glTexCoord2f(1.0, 1.0);
        glVertex2f( 100,-100);
        glTexCoord2f(0.0, 1.0);
        glVertex2f(-100,-100);
      glEnd();
      
      glColor4f(0.75, 0.96, 0.61, fade); 
      font.printf(CENTERED, 0, 6.0, 4.8, "%s", currLevel.name.c_str());
    
      if (P.levelRecords[P.level].time >= 0.0 && !config.cheating())
      {
        if (fadeTimer >= 3.0)
        {
          glColor4f(0.5, 1.0, 0.76, fadeTimer - 3.0);
          font.printf(-92, LAST_Y, 3.0, 2.6, "Fastest time:    %2d:%02d.%02d",
          (int)(P.levelRecords[P.level].time/60), (int)fmod(P.levelRecords[P.level].time, 60),
          (int)(modf(P.levelRecords[P.level].time, &foo)*100));
          font.printf(-92, LAST_Y, 3.0, 2.6, "Most kills:         %5d", P.levelRecords[P.level].kills);
          font.printf(-92, LAST_Y, 3.0, 2.6, "Fewest shots:       %5d", P.levelRecords[P.level].shots);
          font.printf(-92, LAST_Y, 3.0, 2.6, "Least times hit:    %5d", P.levelRecords[P.level].hits);
        }
      }
    }
    
    window.draw();
    
    SDL_GL_SwapBuffers();
    
    timer.update();
  }
}

void levelWinScreen(bool playInSequence)
{
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_LIGHTING);  
  glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  SCREEN_TOP = 100/SCREEN_ASPECT;
  glOrtho(-100, 100, -SCREEN_TOP, SCREEN_TOP, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  int l = (P.level < NUM_LEVELS) ? P.level : NUM_LEVELS-1;
  
  int scols, srows; float sx;
  if (P.levelRecords[l].time < 0.0 || config.cheating() || !playInSequence || (P.augmented && !currLevel.playerAugmented))
  { scols = 25; srows = 6; sx = -40; }
  else
  { scols = 39; srows = 7; sx = -62; }
     
  TerminalWindow stats(sx, 30, srows, scols, 3.0, 4.5, 1, 0.8, RGBAColor(0.5, 1.0, 0.76, 1.0), RGBAColor(0.5, 1.0, 0.76, 0.1));
  stats.setCursor(srows-1, srows-1, 0, srows-1, ">", -1);
  double foo;
  
  // No previous record
  if (P.levelRecords[l].time < 0.0 || config.cheating() || !playInSequence || (P.augmented && !currLevel.playerAugmented))
  {
    stats.addString(0, "Completion time  %2d:%02d.%02d", (int)(P.gameTime/60),
    (int)fmod(P.gameTime, 60), (int)(modf(P.gameTime, &foo)*100));
    stats.addString(0, "Enemies killed      %5d", P.kills);
    stats.addString(0, "Shots fired         %5d", P.shots);
    stats.addString(0, "Times hit           %5d", P.hits);
    stats.addString(0, "");
    stats.addString(0, "  press " ENTER_KEY " to continue");
  }
  else
  {
    stats.addString(0, "                         | Prev. record");
    
    stats.addString((P.levelRecords[l].time > P.gameTime) ? 3 : 0, 
    "Completion time %2d:%02d.%02d |     %2d:%02d.%02d", (int)(P.gameTime/60),
    (int)fmod(P.gameTime, 60), (int)(modf(P.gameTime, &foo)*100), (int)(P.levelRecords[l].time/60),
    (int)fmod(P.levelRecords[l].time, 60), (int)(modf(P.levelRecords[l].time, &foo)*100));
    
    stats.addString((P.levelRecords[l].kills < P.kills) ? 3 : 0,
    "Enemies killed     %5d |        %5d", P.kills, P.levelRecords[l].kills);
    
    stats.addString((P.levelRecords[l].shots > P.shots) ? 3 : 0,
    "Shots fired        %5d |        %5d", P.shots, P.levelRecords[l].shots);
    
    stats.addString((P.levelRecords[l].hits > P.hits) ? 3 : 0,
    "Times hit          %5d |        %5d", P.hits, P.levelRecords[l].hits);
    
    stats.addString(0, "");
    stats.addString(0, "  press " ENTER_KEY " to continue");  
  }
  
  stats.activate();
  
  // flush the keys  
  keystate = SDL_GetKeyState(NULL);
  bool enterPressed = keystate[SDLK_RETURN];
  
  bool loop = true;
  
  while (loop)
  {
    timer.reset();
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
        loop = false;
    }
    
    // Key checking loop
    keystate = SDL_GetKeyState(NULL);
    
    if (keystate[SDLK_RETURN])
    {
      if (!enterPressed)
        enterPressed = true;
    }
    else
      enterPressed = false;
    
    SCREENSHOT_CHECK
        
    if (stats.update(keystate) != -1)
      loop = false;
    
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);
    
    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[TEX_BG2]);
    glBegin(GL_QUADS);
      glTexCoord2f(0.0, 0.0);
      glVertex2f(-100, 100);
      glTexCoord2f(1.0, 0.0);
      glVertex2f( 100, 100);
      glTexCoord2f(1.0, 1.0);
      glVertex2f( 100,-100);
      glTexCoord2f(0.0, 1.0);
      glVertex2f(-100,-100);
    glEnd();

    glColor3ub(192, 245, 156); 
    font.printf(CENTERED, 37.0, 6.0, 4.8, "LEVEL COMPLETED");
    stats.draw();
    
    SDL_GL_SwapBuffers();
    
    timer.update();
  }
  
  // Save data
  P.saveLevelData(playInSequence);
}

void finalStatistics()
{
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_LIGHTING);  
  glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  SCREEN_TOP = 100/SCREEN_ASPECT;
  glOrtho(-100, 100, -SCREEN_TOP, SCREEN_TOP, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  float fadeTimer = 0.0;
  
  int scols, srows; float sx;
  if (P.levelRecords[NUM_LEVELS].time < 0.0 || P.cheated || P.usedAugment || P.startingLevel != 0)
  { scols = 25; srows = 6; sx = -40; }
  else
  { scols = 39; srows = 7; sx = -62; }   
    
  TerminalWindow stats(sx, 30, srows, scols, 3.0, 4.5, 1, 0.8, RGBAColor(0.5, 1.0, 0.76, 1.0), RGBAColor(0.5, 1.0, 0.76, 0.1));
  stats.setCursor(srows-1, srows-1, 0, srows-1, ">", -1);
  double foo;

  if (P.cheated)
  {
    stats.addString(0, "Final statistics are not");
    stats.addString(0, "recorded if you cheat.");
    stats.addString(0, "");
    stats.addString(0, "");
    stats.addString(0, "");
    stats.addString(0, "  press " ENTER_KEY " to continue");  
  }  
  else if (P.usedAugment)
  {
    stats.addString(0, "Final statistics are not");
    stats.addString(0, "recorded if you use");
    stats.addString(0, "augmentations on levels");
    stats.addString(0, "other than the last.");
    stats.addString(0, "");
    stats.addString(0, "  press " ENTER_KEY " to continue");  
  }  
  else if (P.startingLevel != 0)
  {
    stats.addString(0, "Final statistics are not");
    stats.addString(0, "recorded if you do not");
    stats.addString(0, "start at the first level.");
    stats.addString(0, "");
    stats.addString(0, "");
    stats.addString(0, "  press " ENTER_KEY " to continue");  
  }
  // No previous record
  else if (P.levelRecords[NUM_LEVELS].time < 0.0)
  {
    stats.addString(0, "Completion time  %2d:%02d.%02d", (int)(P.currStats[NUM_LEVELS].time/60),
    (int)fmod(P.currStats[NUM_LEVELS].time, 60), (int)(modf(P.currStats[NUM_LEVELS].time, &foo)*100));
    stats.addString(0, "Enemies killed      %5d", P.currStats[NUM_LEVELS].kills);
    stats.addString(0, "Shots fired         %5d", P.currStats[NUM_LEVELS].shots);
    stats.addString(0, "Times hit           %5d", P.currStats[NUM_LEVELS].hits);
    stats.addString(0, "");
    stats.addString(0, "  press " ENTER_KEY " to continue");
  }
  else
  {
    stats.addString(0, "                         | Prev. record");
    
    stats.addString((P.levelRecords[NUM_LEVELS].time > P.currStats[NUM_LEVELS].time) ? 3 : 0, 
    "Completion time %2d:%02d.%02d |     %2d:%02d.%02d", (int)(P.gameTime/60),
    (int)fmod(P.gameTime, 60), (int)(modf(P.gameTime, &foo)*100), (int)(P.levelRecords[NUM_LEVELS].time/60),
    (int)fmod(P.levelRecords[NUM_LEVELS].time, 60), (int)(modf(P.levelRecords[NUM_LEVELS].time, &foo)*100));
    
    stats.addString((P.levelRecords[NUM_LEVELS].kills < P.currStats[NUM_LEVELS].kills) ? 3 : 0,
    "Enemies killed     %5d |        %5d", P.kills, P.levelRecords[NUM_LEVELS].kills);
    
    stats.addString((P.levelRecords[NUM_LEVELS].shots > P.currStats[NUM_LEVELS].shots) ? 3 : 0,
    "Shots fired        %5d |        %5d", P.shots, P.levelRecords[NUM_LEVELS].shots);
    
    stats.addString((P.levelRecords[NUM_LEVELS].hits > P.currStats[NUM_LEVELS].hits) ? 3 : 0,
    "Times hit          %5d |        %5d", P.hits, P.levelRecords[NUM_LEVELS].hits);
    
    stats.addString(0, "");
    stats.addString(0, "  press " ENTER_KEY " to continue");  
  }
  
  // flush the keys  
  keystate = SDL_GetKeyState(NULL);
  bool enterPressed = keystate[SDLK_RETURN];
  
  stats.activate(keystate);
  
  bool loop = true;
  
  while (loop)
  {
    timer.reset();
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
        loop = false;
    }
    
    // Key checking loop
    keystate = SDL_GetKeyState(NULL);
    
    if (keystate[SDLK_RETURN])
    {
      if (!enterPressed)
        enterPressed = true;
    }
    else
      enterPressed = false;
      
    SCREENSHOT_CHECK      
      
    if (stats.update(keystate) != -1)
      loop = false;
      
    if (fadeTimer < 1.0)
      fadeTimer += timer.dT();
    
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);
    
    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[TEX_BG2]);
    glBegin(GL_QUADS);
      glTexCoord2f(0.0, 0.0);
      glVertex2f(-100, 100);
      glTexCoord2f(1.0, 0.0);
      glVertex2f( 100, 100);
      glTexCoord2f(1.0, 1.0);
      glVertex2f( 100,-100);
      glTexCoord2f(0.0, 1.0);
      glVertex2f(-100,-100);
    glEnd();

    glColor3ub(192, 245, 156); 
    font.printf(CENTERED, 37.0, 6.0, 4.8, "FINAL STATISTICS");
    stats.draw();
    
    if (!P.cheated && !P.usedAugment && P.currStats[NUM_LEVELS].time < UNLOCK_TIME && P.startingLevel == 0)
    {
      glColor4f(1.0, 0.2, 0.0, fadeTimer);
      font.printf(CENTERED, -27, 4.0, 3.2, "Congratulations!");
      font.printf(CENTERED, LAST_Y, 4.0, 3.2, "You've beaten the game amazingly quick.");
      font.printf(CENTERED, LAST_Y, 4.0, 3.2, "As a reward, you can now play ANY level with");
      font.printf(CENTERED, LAST_Y, 4.0, 3.2, "your cyborg enhancements. Turn them on and off by");
      font.printf(CENTERED, LAST_Y, 4.0, 3.2, "pressing X at the main menu. w00t!");
    }
    
    SDL_GL_SwapBuffers();
    
    timer.update();
  }
  
  P.saveFinalStatistics();
}

void objectStats(int obj)
{
  glColor3f(1.0, 1.0, 1.0);
  font.printf(-98.0, SCREEN_TOP-3.0, 4.0, 3.2, "FPS       %03d", timer.FPS());
  font.printf(-98.0, LAST_Y, 4.0, 3.2, "\x7FT        %0.5f", timer.dT());
  font.printf(-98.0, LAST_Y, 4.0, 3.2, "POSITION  %0.3f %0.3f",
    currLevelObjs[obj]->PosX(), currLevelObjs[obj]->PosY());
  font.printf(-98.0, LAST_Y, 4.0, 3.2, "VELOCITY  %0.3f %0.3f",
    currLevelObjs[obj]->VelX(), currLevelObjs[obj]->VelY());
  font.printf(-98.0, LAST_Y, 4.0, 3.2, "SPEED     %0.3f %0.3f",
    currLevelObjs[obj]->Vel().mag(), currLevelObjs[obj]->Speed());
  font.printf(-98.0, LAST_Y, 4.0, 3.2, "HEADING   %0.3f",
    todeg(currLevelObjs[obj]->Heading()));
  font.printf(-98.0, LAST_Y, 4.0, 3.2, "TURN VEL. %0.3f",
    currLevelObjs[obj]->Turnvel());
  font.printf(-98.0, LAST_Y, 4.0, 3.2, "HEIGHT    %0.3f", 
    currLevelObjs[obj]->Height());
  font.printf(-98.0, LAST_Y, 4.0, 3.2, "VERT VEL. %0.3f",
    currLevelObjs[obj]->HVel());
  font.printf(-98.0, LAST_Y, 4.0, 3.2, "EVENTS    % .0f % .0f %d %d %d",
    currLevelObjs[obj]->ev.getLR(), currLevelObjs[obj]->ev.getFB(), currLevelObjs[obj]->ev.getJump(),
    currLevelObjs[obj]->ev.getFire(), currLevelObjs[obj]->ev.getReload());
  font.printf(-98.0, LAST_Y, 4.0, 3.2, "KEYS      %d %d %d %d", currLevelObjs[obj]->Keys() & 1,
  currLevelObjs[obj]->Keys() & 2, currLevelObjs[obj]->Keys() & 4, currLevelObjs[obj]->Keys() & 8);
  font.printf(-98.0, LAST_Y, 4.0, 3.2, "S.C.      %lld", currLevelObjs[obj]->ShockCollisions());

  font.printf(-98.0, LAST_Y, 4.0, 3.2, "PARTICLES %d", liveParticles);
  font.printf(-98.0, LAST_Y, 4.0, 3.2, "BULLETS   %d", liveBullets);
  font.printf(-98.0, LAST_Y, 4.0, 3.2, "FORCES    %d", liveForces);
  font.printf(-98.0, LAST_Y, 4.0, 3.2, "CELL      %d", currLevel.cellNumber(currLevelObjs[obj]->Pos()));
}

void errorMessage(int retVal, const char *heading, const char *msg, ...)
{
  va_list ap;
  va_start(ap, msg);

  if (GFXMODE_ISACTIVE)
  {
    glEndList();
    
   	glEnable(GL_BLEND);
  	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_ALPHA_TEST);
    //glClearColor(0.0, 0.25, 0.125, 0.0);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    gameSetOrthoMode(100);
   
    bool done = false;
    while (!done)
    {
      SDL_Event event;
      while (SDL_PollEvent(&event))
      {
        if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN)
          done = true;
      }
      
      glLoadIdentity();
      glClear(GL_COLOR_BUFFER_BIT);
      
      glColor4f(0.0, 1.0, 0.0, 0.25);
      glBegin(GL_LINES);
      for (int i = (int)SCREEN_TOP; i > -(int)SCREEN_TOP; i--)
      {
        glVertex2i(-100, i);
        glVertex2i(100, i);
      }
      glEnd();
      
      glColor3f(0.0, 1.0, 0.0);
      font.printf(CENTERED, 12.0, 4.0, 3.2, heading);
      font.vprintf(CENTERED, LAST_Y-2.0, 4.0, 3.2, msg, ap);
      font.print(CENTERED, LAST_Y-2.0, 4.0, 3.2, 0.75, "PRESS ANY KEY TO EXIT");
      SDL_GL_SwapBuffers();
    }
  }
  
  printf("%s\n", heading);
  vprintf(msg, ap);
  printf("\n");
  
  if (retVal != 0)
  {
    config.writeToFile(CONFIG_FILE);
    exit(retVal);   
  }
}

void drawPaths()
{
  glPushMatrix();
  
  glColor3f(1.0, 1.0, 1.0);
  for (int i = 0; i < currLevel.node.size(); i++)
  {
    font.printf(currLevel.node[i].pos.c[X], currLevel.node[i].pos.c[Y], 1.0, 0.8, "%d", i);
  }
  
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_LINES);
  for (int i = 0; i < currLevel.node.size(); i++)
  {
    for (int j = 0; j < currLevel.node[i].visible.size(); j++)
    {
      glVertex3f(currLevel.node[i].pos.c[X], currLevel.node[i].pos.c[Y], 0.25);
      glVertex3f(currLevel.node[currLevel.node[i].visible[j]].pos.c[X], currLevel.node[currLevel.node[i].visible[j]].pos.c[Y], 0.25);
    }
  }
  glEnd();
  glPopMatrix();
}

void configScreen(bool persp, float &scroll1, float &scroll2)
{ 

  char options[][17] = {"Resolution      ",
                        "Color depth     ",
                        "Full screen     ",
                        "Blur effect     ",
                        "Sound effects   ",
                        "Music           ",
                        "Customize keys  ",
                        "Erase saved game",
                        "Reset progress  ",
                        "Cancel          ",
                        "Save            "};
  char onOff[][4] = {"Off", "On "};

  char keyLabels[][16] = {"turn left:     ",
                          "turn right:    ",
                          "move forward:  ",
                          "move backward: ",
                          "sidestep left: ",
                          "sidestep right:",
                          "jump:          ",
                          "fire weapon:   ",
                          "throw grenade: ",
                          "reload weapon: ",
                          "switch weapon: ",
                          "switch view:   ",
                          "quick zoom out:"};

  // negative X value <==> widescreen
  int resolutions[] = {640,480, 800,600, 1024,768, 1152,864, -1280,768, -1280,800, -1280,854, 1280,960, 1280,1024,
	  -1440,900, 1400,1050, -1680,1050, 1600,1200, -1920,1200, 2048,1536, -2560,1600};
   
  int numRes = 16;
  int depths[] = {16, 32};
  int numDepths = 2;
 
  int currRes = -1;
  for (int i = 0; i < numRes; i++)
  {
    if (config.xres == ::abs(resolutions[2*i]) && config.yres == resolutions[2*i+1])
    { currRes = i; break; }
  }
  
  int currDepth = -1;
  for (int i = 0; i < numDepths; i++)
  {
    if (config.depth == depths[i])
    { currDepth = i; break; }
  }
  
  int oldXres = config.xres, oldYres = config.yres, oldDepth = config.depth;
  bool oldFS = config.fullscreen, oldBlur = config.blur, oldSFX = config.sfx, oldMusic = config.music;

  TerminalWindow window(CENTERED, CENTERED, 15, 36, 2.0, 3.0, 3, 0.4, RGBAColor(0.5, 1.0, 0.76, 1.0), RGBAColor(0.5, 1.0, 0.76, 0.2));
  window.setCursor(2, 12, 0, 2, ">", 11);
  
  window.addString(0, "             OPTIONS");
  window.addString(0, "");
  window.addString(0, "  %s  %d x %d%s", options[0], ((currRes > -1) ? abs(resolutions[2*currRes]) : config.xres),
    ((currRes > -1) ? resolutions[2*currRes+1] : config.yres), (resolutions[2*currRes] >= 0) ? "" : " wide");
  window.addString(0, "  %s  %d bits", options[1],  (currDepth > -1) ? depths[currDepth] : config.depth);
  window.addString(0, "  %s  %s", options[2], onOff[config.fullscreen]);
  window.addString(0, "  %s  %s", options[3], onOff[config.blur]);
  window.addString(0, "  %s  %s", options[4], onOff[config.sfx]);
  window.addString(0, "  %s  %s", options[5], onOff[config.music]);
  
  for (int i = 6; i <= 10; i++)
    window.addString(0, "  %s", options[i]);

  window.addString(0, "");
  window.addString(0, "(sdb)");
    
  TerminalWindow window2;   
  
  if (persp)
  {
    glDisable(GL_LIGHTING);
    gameSetOrthoMode(100);
    glDisable(GL_DEPTH_TEST); 
  }
  
  glClearColor(0.0, 0.0, 0.0, 1.0);
  
  keystate = SDL_GetKeyState(NULL);
  bool escPressed = keystate[SDLK_ESCAPE];
  int option = 0;
  int option2 = 0;
  int menu2 = 0;
  float alertTimer = 0.0, keyAlertTimer = 0.0;
  bool waitingForKey = false;
  int currKey = 0;
  SDLKey keyPressed;

  window.activate(keystate);
  
  int done = 0;
  
  while (!done)
  {
    timer.reset();
    
    SDL_Event event;
    keyPressed = (SDLKey)0;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
        done = 1;
      else if (event.type == SDL_KEYDOWN && waitingForKey)
      { keyPressed = event.key.keysym.sym; }
    }
    
    keystate = SDL_GetKeyState(NULL);
    
    if (keystate[SDLK_ESCAPE])
    {
      if (!escPressed)
      { done = 1; escPressed = true; }
    }
    else
      escPressed = true;    
    
    SCREENSHOT_CHECK  
      
    option = window.update(keystate);
    
    switch(option)
    {
      case 2:
        currRes++;
        if (currRes >= numRes) currRes = 0;
        window.changeString(option, 0, "  %s  %d x %d%s", options[0], ((currRes > -1) ? abs(resolutions[2*currRes]) : config.xres),
        ((currRes > -1) ? resolutions[2*currRes+1] : config.yres), (resolutions[2*currRes] >= 0) ? "" : " wide");
        break;
      case 3:
        currDepth++;
        if (currDepth >= numDepths) currDepth = 0;
        window.changeString(option, 0, "  %s  %d bits", options[1],  (currDepth > -1) ? depths[currDepth] : config.depth);
        break;
      case 4:
        config.fullscreen = !config.fullscreen;
        window.changeString(option, 0, "  %s  %s", options[2], onOff[config.fullscreen]);
        break;
      case 5:
        config.blur = !config.blur;
        window.changeString(option, 0, "  %s  %s", options[3], onOff[config.blur]);
        break;
      case 6:
        config.sfx = !config.sfx;
        window.changeString(option, 0, "  %s  %s", options[4], onOff[config.sfx]);
        break;
      case 7:
        config.music = !config.music;
        window.changeString(option, 0, "  %s  %s", options[5], onOff[config.music]);
        break;
      case 8:
        window.deactivate();
        setMenu(&window2, OPTIONS_KEYS, keystate);
        for (int i = 0; i < NUM_INPUT_KEYS; i++)
          window2.changeString(i+1, 0, "  %s %12s", keyLabels[i], SDL_GetKeyName(config.keys[i]));
        window2.activate();
        menu2 = OPTIONS_KEYS;
        break;
      case 9:
        if (!persp)
        {
          window.deactivate();
          setMenu(&window2, OPTIONS_ECONFIRM, keystate);
          menu2 = OPTIONS_ECONFIRM;
        }
        else
          alertTimer = 2.0;
        break;
      case 10:
        if (!persp)
        {
          window.deactivate();
          setMenu(&window2, OPTIONS_RCONFIRM, keystate);
          menu2 = OPTIONS_RCONFIRM;
        }
        else
          alertTimer = 2.0;
        break;
      case 11:
        done = 1;
        break;
      case 12:
        done = 2;
        break;
    }    
    
    if (menu2)
    {
      if (!waitingForKey)
        option2 = window2.update(keystate);
      else
      {
        if (keyPressed != 0)
        {
          if (keyPressed == SDLK_ESCAPE)
          {
            window2.changeString(currKey+1, 0, "  %s %12s", keyLabels[currKey], SDL_GetKeyName(config.keys[currKey]));
            currKey = 0;
            waitingForKey = false;
            option2 = -1;
          }
          else if ((keyPressed >= SDLK_1 && keyPressed <= SDLK_7) ||
                   (keyPressed >= SDLK_F1 && keyPressed <= SDLK_F15))
            keyAlertTimer = 2.0;
          else
          {
            config.keys[currKey] = keyPressed;
            window2.changeString(currKey+1, 0, "  %s %12s", keyLabels[currKey], SDL_GetKeyName(config.keys[currKey]));
            for (int i = 0; i < NUM_INPUT_KEYS; i++)
            {
              if (keyPressed == config.keys[i] && i != currKey)
              {
                config.keys[i] = (SDLKey)0;
                window2.changeString(i+1, 0, "  %s %12s", keyLabels[i], SDL_GetKeyName(config.keys[i]));
              }
            }
            currKey = 0;
            waitingForKey = false;
            option2 = -1;
          }
        }
      }
      
      if (menu2 == OPTIONS_KEYS)
      {
        if (option2 >= 1 && option2 <= 13)
        {
          currKey = option2-1;  waitingForKey = true;
          window2.changeString(option2, 1, "  %s     new key?", keyLabels[currKey]);
        }
        else if (option2 == 14)
        {
          config.defaultKeys();
          for (int i = 0; i < NUM_INPUT_KEYS; i++)
            window2.changeString(i+1, 0, "  %s %12s", keyLabels[i], SDL_GetKeyName(config.keys[i]));
        }
        if (option2 == 15)
        { P.setKeys(config.keys); window2.deactivate(); menu2 = 0; window.activate(); }
      }
      else
      {
        if (option2 == 4)
        {
          if (menu2 == OPTIONS_RCONFIRM)
            P.resetLevelProgress();
          
          P.eraseSavedGame();
          playSound(SND_TERM_CLEAR, -1);
        }
      
        if (option2 == 4 || option2 == 5)
        { window2.deactivate(); menu2 = 0; window.activate(); }
      }
    }
    
    if (currRes > -1) { config.xres = abs(resolutions[2*currRes]); config.yres = resolutions[2*currRes+1]; }
    if (currDepth > -1) config.depth = depths[currDepth];
    
    scroll1 += SCROLL1_RATE * timer.dT();
    scroll2 += SCROLL2_RATE * timer.dT();
    
    if (alertTimer > 0.0)
    {
      alertTimer -= timer.dT();
      if (alertTimer <= 0.0)
        alertTimer = 0.0;
    }
    if (keyAlertTimer > 0.0)
    {
      keyAlertTimer -= timer.dT();
      if (keyAlertTimer <= 0.0)
        keyAlertTimer = 0.0;
    }
    
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
        
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColor4f(1.0, 1.0, 1.0, 0.2);
    glBindTexture(GL_TEXTURE_2D, textures[TEX_HEX1]);
    glBegin(GL_QUADS);
      glTexCoord2f(0.0, scroll1);
      glVertex2f(25, 100);
      glTexCoord2f(1.0, scroll1);
      glVertex2f(125, 100);
      glTexCoord2f(1.0, scroll1+1.0);
      glVertex2f(125,-100);
      glTexCoord2f(0.0, scroll1+1.0);
      glVertex2f(25,-100);
    glEnd();
    
    glColor3f(1.0, 1.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, textures[TEX_HEX2]);
    glBegin(GL_QUADS);
      glTexCoord2f(scroll2, 0.0);
      glVertex2f(-100, -28);
      glTexCoord2f(scroll2+1.0, 0.0);
      glVertex2f(100, -28);
      glTexCoord2f(scroll2+1.0, 1.0);
      glVertex2f(100,-31.125);
      glTexCoord2f(scroll2, 1.0);
      glVertex2f(-100,-31.125);
    glEnd();
    
    glColor3f(1.0, 1.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, textures[TEX_TITLE_BG]);
    glBegin(GL_QUADS);
      glTexCoord2f(0.0, 0.0);
      glVertex2f(-100, 100);
      glTexCoord2f(1.0, 0.0);
      glVertex2f( 100, 100);
      glTexCoord2f(1.0, 1.0);
      glVertex2f( 100,-100);
      glTexCoord2f(0.0, 1.0);
      glVertex2f(-100,-100);
    glEnd();
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    if (config.xres != oldXres || config.yres != oldYres || config.depth != oldDepth || config.fullscreen != oldFS)
    {
      glColor4ub(192, 245, 156, 128);
      font.printf(CENTERED, -42, 3.0, 2.6, "New display settings take effect on restart");
    }
    
    if (alertTimer > 0.0)
    {
      glColor4f(1.0, 0.0, 0.0, MIN(1.0, alertTimer));
      font.printf(CENTERED, -48, 3.0, 2.6, "You cannot erase progress while playing a game");    
    }
    if (keyAlertTimer > 0.0)
    {
      glColor4f(1.0, 0.0, 0.0, MIN(1.0, keyAlertTimer));
      font.printf(CENTERED, -48, 3.0, 2.6, "That key is reserved");    
    }

    
    if (menu2 == 0)
      window.draw();
    else
      window2.draw();
    
    SDL_GL_SwapBuffers();
    
    timer.update();
  }

  if (done == 1) // Cancel
  {
    config.xres = oldXres; config.yres = oldYres; config.depth = oldDepth;
    config.fullscreen = oldFS; config.blur = oldBlur; config.sfx = oldSFX; config.music = oldMusic;
  }
  else if (done == 2) // Save
  {     
    if (oldMusic != config.music)
    {
      if (!config.music)
        stopMusic();
      else
        startMusic((persp) ? MUS_DD2: MUS_TITLE);
    }
  }
  
  // Return to perspective projection
  if (persp)
  {
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
  }
}

// woo hoo!
void finalCredits()
{
  vector<string> lines;
  lines.push_back("SHOTGUN DEBUGGER");
  lines.push_back("");
  lines.push_back("project leader, programming, concepts, 2d art");
  lines.push_back("MATT SARNOFF");
  lines.push_back("");
  lines.push_back("lead level designer, design feedback");
  lines.push_back("CHRIS DELEON");
  lines.push_back("");
  lines.push_back("character models");
  lines.push_back("JOHN NESKY");
  lines.push_back("");
  lines.push_back("level designers");
  lines.push_back("GREGORY PENG");
  lines.push_back("JEFF THOENE");
  lines.push_back("");
  lines.push_back("sound effects");
  lines.push_back("CHRIS DELEON");
  lines.push_back("MATT SARNOFF");
  lines.push_back("");  
  lines.push_back("featuring the music of");
  lines.push_back("TUSCAN KNOX");
  lines.push_back("");  
  lines.push_back("testing");
  lines.push_back("MICHAEL WEBER");
  lines.push_back("");
  lines.push_back("patches contributed by");
  lines.push_back("DAVID HOLM (proper big-endian support)");
  lines.push_back("ROBERT SCHUSTER (autotools support)");
  lines.push_back("");
  lines.push_back("developed with");
  lines.push_back("open-source software and libraries");
  lines.push_back("THE SIMPLE DIRECTMEDIA LAYER (www.libsdl.org)");
  lines.push_back("THE OGG VORBIS AUDIO CODEC (www.vorbis.com)");
  lines.push_back("INKSCAPE (www.inkscape.org)");
  lines.push_back("BLENDER (www.blender3d.org)");
  lines.push_back("THE GIMP (www.gimp.org)");
  lines.push_back("GCC, THE GNU COMPILER COLLECTION");
  lines.push_back("and of course, GDB, THE GNU DEBUGGER");
  lines.push_back("(no relation)");
  lines.push_back("");
  lines.push_back("thanks to");
  lines.push_back("CAFFEINE");
  lines.push_back("SCENEMUSIC.NET");
  lines.push_back("ALL GCS MEMBERS AND SUPPORTERS");
  lines.push_back("\"AND ALL THE HACKERS IN THE (401)\"");
  lines.push_back("");
  lines.push_back("Shotgun Debugger is a product of the");
  lines.push_back("Game Creation Society,");
  lines.push_back("a nonprofit organization at");
  lines.push_back("Carnegie Mellon University,");
  lines.push_back("and is released under the");
  lines.push_back("GNU General Public License.");
  lines.push_back("");
  lines.push_back("The events portrayed in this game are fictional.");
  lines.push_back("(If they weren't, get me one of those HEPAs!)");
  lines.push_back("");
  lines.push_back("sdb.gamecreation.org");
  lines.push_back("www.msarnoff.org");
  lines.push_back("");
  lines.push_back(PACKAGE_RELEASE_DATE);
   
  glDisable(GL_ALPHA_TEST);
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_EQUAL, 1.0);
  
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  
  glEnable(GL_FOG);
  GLfloat fogColor[4] = {0.0, 0.0, 0.0, 1.0};
  glFogi(GL_FOG_MODE, GL_EXP);
  glFogfv(GL_FOG_COLOR, fogColor);
  glFogf(GL_FOG_DENSITY, 0.01);
  glHint(GL_FOG_HINT, GL_DONT_CARE);
  glFogf(GL_FOG_START, 0.0);
  glFogf(GL_FOG_END, 100.0);  
  gameSetPerspectiveMode();
  
  Camera cam;
  ModelAnimation m1(MDL_PLAYER_TORSO2);
  ModelAnimation m2(MDL_PLAYER_LEGS);
  
  ModelAnimation zero(MDL_ZERO);
  ModelAnimation one(MDL_ONE);
  
  m1.currFrame = m1.nextFrame = 7;
  m2.currFrame = m2.nextFrame = 7;
  
  Digit digits[NUM_DIGITS];
  
  for (int i = 0; i < NUM_DIGITS; i++)
  {
    digits[i].x = frand()*50-25;
    digits[i].y = frand()*50-25;
    digits[i].z = frand()*200-100;
    digits[i].num = rand()%2;
  }
  
  keystate = SDL_GetKeyState(NULL);
  bool enterPressed = keystate[SDLK_RETURN];
  
  bool loop = true;
  
  float y = -SCREEN_TOP-10.0;
  float speed = 8.0;
  float fade = 0.0;
  float msgFade = 0.0;
  bool fadeOut = false, musicFade = false, msgFadeIn = false;
  
  float ang = torad(90);
  float rad = 20;
  
  while (loop)
  {
    timer.reset();
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
        loop = false;
    }
    
    // Key checking loop
    keystate = SDL_GetKeyState(NULL);
    
    if (keystate[SDLK_RETURN])
    {
      if (!enterPressed)
      {
        enterPressed = true;
        fadeOut = true;
      }
    }
    else
      enterPressed = false;
  
    SCREENSHOT_CHECK
      
    y += speed * timer.dT();
    
    if (y > 440)
      msgFadeIn = true;
    if (y > 470)
      speed = 0;
    
    if (fadeOut)
    {
      if (!musicFade)
       { fadeOutMusic(1000); musicFade = true; }
       
      fade -= timer.dT();
      if (fade <= -1)
        loop = false;
    }
    else
    {
      if (fade < 1.0)
        fade += timer.dT();
    }
    
    if (msgFade < 1.0 && msgFadeIn)
    {
      msgFade += timer.dT();
      if (msgFade >= 1.0)
      { msgFade = 1.0; msgFadeIn = false; }
    }
    
    ang += torad(10) * timer.dT();
          
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    
    gluLookAt(cos(ang)*rad,::fabs(sin(ang))*10.0, sin(ang)*rad, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_EQUAL, 1.0);
    glColor4f(0.0, 1.0, 0.0, MAX(fade, 0.0));
    for (int i = 0; i < NUM_DIGITS; i++)
    {
      glPushMatrix();
      glTranslatef(digits[i].x, digits[i].y, digits[i].z);
      if (digits[i].num)
        one.draw();
      else
        zero.draw();
      glPopMatrix();
    }
    
    glPushMatrix();
    glTranslatef(0.0, -5.0, 0.0);
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    glRotatef(-90.0, 0.0, 0.0, 1.0);
    glScalef(2.0, 2.0, 2.0);
    glColor3f(0.0, 1.0, 0.0);
    m1.draw();
    m2.draw();
    glPopMatrix();
    
    glAlphaFunc(GL_NOTEQUAL, 1.0);
    glDepthMask(GL_FALSE);
    glColor4f(0.0, 1.0, 0.0, MAX(fade, 0.0));
    for (int i = 0; i < NUM_DIGITS; i++)
    {
      glPushMatrix();
      glTranslatef(digits[i].x, digits[i].y, digits[i].z);
      if (digits[i].num)
        one.draw();
      else
        zero.draw();
      glPopMatrix();
    }    
    
    glPushMatrix();
    glTranslatef(0.0, -5.0, 0.0);
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    glRotatef(-90.0, 0.0, 0.0, 1.0);
    glScalef(2.0, 2.0, 2.0);
    glColor3f(0.0, 1.0, 0.0);
    m1.draw();
    m2.draw();
    glPopMatrix();
        
    glDepthMask(GL_TRUE);
    glDisable(GL_ALPHA_TEST); 
    
    gameSetOrthoMode(100);
    glDisable(GL_DEPTH_TEST);
     
    glColor4f(0.75, 1.0, 1.0, MAX(fade, 0.0));
    font.printf(CENTERED, y, 6.0, 4.8, "%s", lines[0].c_str());
    glColor4f(0.75, 1.0, 0.8, MAX(fade, 0.0));
    for (int i = 1; i < lines.size(); i++)
      font.printf(CENTERED, LAST_Y, 4.0, 3.2, "%s", lines[i].c_str());
        
    glColor4f(0.75, 1.0, 1.0, (fadeOut)?MIN(fade,msgFade):msgFade);
    font.printf(CENTERED, -SCREEN_TOP+18.0, 4.0, 3.2, "THE END");
    font.printf(CENTERED, LAST_Y, 4.0, 3.2, "COPYRIGHT 2005 GAME CREATION SOCIETY");
    font.printf(CENTERED, LAST_Y, 2.0, 1.6, "press " ENTER_KEY);
      
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
      
    SDL_GL_SwapBuffers();
    
    timer.update();
  }
}

void takeScreenshot()
{
  // Allocate the memory
  unsigned char *screenPixels = new unsigned char[config.xres*config.yres*4];
  if (screenPixels)
  {
    // Read the pixels
    glReadPixels(0, 0, config.xres, config.yres, GL_RGBA, GL_UNSIGNED_BYTE, screenPixels);
    
    // Swap the scanlines, because the image comes out upside down
    for (int i = 0; i < config.yres/2; i++)
    {
      // xor swap
      for (int j = 0; j < config.xres*4; j++)
      {
        screenPixels[i*config.xres*4+j] ^= screenPixels[(config.yres-1-i)*config.xres*4+j];
        screenPixels[(config.yres-1-i)*config.xres*4+j] ^= screenPixels[i*config.xres*4+j];
        screenPixels[i*config.xres*4+j] ^= screenPixels[(config.yres-1-i)*config.xres*4+j];
      }
    }
    
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
      SDL_Surface *bitmap = SDL_CreateRGBSurfaceFrom(screenPixels, config.xres, config.yres, 32, config.xres*4,
      0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
    #else
      SDL_Surface *bitmap = SDL_CreateRGBSurfaceFrom(screenPixels, config.xres, config.yres, 32, config.xres*4,
      0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    #endif
    
    // Write the file
    char filename[256];
    snprintf(filename, 255, "%s%sshot%04d.bmp", getenv("HOME"), LOCALSTATEDIR, screenshotCount);
    
    SDL_SaveBMP(bitmap, filename);
    
    // Free everything
    SDL_FreeSurface(bitmap);
    delete[] screenPixels;
    
    printf("Screenshot %s saved\n", filename);
    screenshotCount++;
  }
}
