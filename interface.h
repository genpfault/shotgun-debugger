/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * interface.h - header file for user interface routines
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
 
#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include "sdb.h"

#define SCROLL1_RATE 0.007
#define SCROLL2_RATE 0.01

#define NUM_DIGITS 300

enum TITLE_MENUS { TITLE_NOMENU, TITLE_MAINMENU, TITLE_CONTINUE, TITLE_NEWGAME, TITLE_OPTIONS, TITLE_RECORDS, TITLE_HELP, TITLE_CREDITS, TITLE_EXIT, TITLE_NEWCONFIRM, TITLE_RCLEAR, OPTIONS_ECONFIRM, OPTIONS_RCONFIRM, OPTIONS_KEYS };

class TerminalWindow
{
  public:
    TerminalWindow() { reset(); }
    TerminalWindow(float px, float py, int r, int c, float cw, float ch, float t, float rt, RGBAColor fg, RGBAColor bg)
    { set(px, py, r, c, cw, ch, t, rt, fg, bg); }
    void set(float px, float py, int r, int c, float cw, float ch, float t, float rt, RGBAColor fg, RGBAColor bg);
    void reset() { lines.clear(); styles.clear(); rowStart = 0; line = ch = linesDisplayed = 0; drawTime = printTime = 0; upPr = downPr = enterPr = false;}
    void setCursor(int ct, int cb, int cc, int cp, string st, int ev);
    void calculateDimensions() { w = (charWidth*cols)+(2*thick)+charWidth; h = (charHeight*rows*1.5)+(2*thick)+charHeight/1.5; }
    void draw();
    void deactivate() { active = false; fullyDrawn = false; ch = line = linesDisplayed = 0;}
    void activate() { active = true; fullyDrawn = false; drawTime = 0.0; playSound(SND_TERM_SELECT, 3); }
    void activate(Uint8 *keystate);
    void flush() { fullyDrawn = true; line = linesDisplayed = lines.size(); }
    void addString(int style, const char *fmt, ...);
    void changeString(int line, int style, const char *fmt, ...);
    int update(Uint8 *keystate);
    void setCols(int c) { cols = c; calculateDimensions(); }
    int Cols() { return cols; }
    
    void cursorUp()   { curPos = (curPos > curTop) ? curPos-1 : curBottom; playSound(SND_TERM_MOVE, 3); }
    void cursorDown() { curPos = (curPos < curBottom) ? curPos+1 : curTop; playSound(SND_TERM_MOVE, 3); }
    int getCursorPos() { return curPos; }
    bool doneDrawing() { return fullyDrawn && linesDisplayed == lines.size(); }
    bool isActive() { return active; }
    void resetKeys(Uint8 *k) { upPr = k[SDLK_UP]; downPr = k[SDLK_DOWN]; enterPr = k[SDLK_RETURN]; escPr = k[SDLK_ESCAPE]; }
  private:
    int rows, cols;
    float charWidth, charHeight;
    float thick;
    
    bool active;
    bool fullyDrawn;
    
    float drawTime;
    float printTime;
    float rate;
    
    int line;
    int ch;
    int linesDisplayed;
    
    bool cursor;
    int curTop, curBottom, curPos;
    int curCol;
    string curString;
    int escVal;
    
    float x, y, w, h;
    RGBAColor fgColor, bgColor;
    
    int rowStart;
    vector<string> lines;
    vector<int> styles;
    
    bool upPr, downPr, enterPr, escPr;
};

int title(bool first, bool playInSequence);
void loadingScreen(int n);
void levelIntroScreen(bool final);
void levelWinScreen(bool playInSequence);
void finalStatistics();
void objectStats(int obj);
void errorMessage(int retVal, const char *heading, const char *msg, ...);
void drawPaths();
void configScreen(bool persp, float &scroll1, float &scroll2);
void finalCredits();
void takeScreenshot();

struct Digit
{
  float x, y, z;
  int num;
};

#endif
