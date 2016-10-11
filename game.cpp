/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * game.cpp - core functions, utilities, other miscellany
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

float SCREEN_TOP;
float SCREEN_CENTER;
float SCREEN_ASPECT;
//bool GFXMODE_ISACTIVE;

MD2Loader modelLoader;
SDL_Surface *texdata[NUM_TEXTURES];
Model models[NUM_MODELS];
Mix_Chunk *snddata[NUM_SOUNDS];
Mix_Music *musicdata[NUM_SONGS];

unsigned int *blurdata[NUM_BLUR_TEXTURES];
GLuint blurTextures[NUM_BLUR_TEXTURES];

SDL_Event event;
Uint8 *keystate;
SDLMod modstate;
int mouse_dX;
int mouse_dY;
Uint8 mouseButtons;

bool winLevel;
bool captured;
bool transitioning;
int systemExplode;

Level currLevel;
Player P;
vector<Object *> currLevelObjs;
vector<AI *> currLevelEnemies;
Particle particles[NUM_PARTICLES];
int liveParticles;
Bullet bullets[NUM_BULLETS];
int liveBullets;
Laser lasers[NUM_LASERS];
int liveLasers;
Force forces[NUM_FORCES];
int liveForces;
Shockwave shockwaves[NUM_SHOCKWAVES];
int liveShockwaves;
float screenQuake;
float screenBlur;
RGBAColor screenBlurColor;
Trigger triggers[NUM_TRIGGERS];
char message[MESSAGE_LENGTH];
float messageTime;

vector<GLdouble *> tessAllocatedVertices;

//Timer timer;

// returns a random float in the range [0.0, 1.0)
float frand()
{ return rand()%10000/10000.0; }

// returns distance-squared between two points
float distSquared(Vector2D a, Vector2D b)
{ return (b.c[X]-a.c[X])*(b.c[X]-a.c[X]) + (b.c[Y]-a.c[Y])*(b.c[Y]-a.c[Y]); }  

// plain ol' distance formula, complete with square root
float dist(Vector2D a, Vector2D b)
{ return sqrt((b.c[X]-a.c[X])*(b.c[X]-a.c[X]) + (b.c[Y]-a.c[Y])*(b.c[Y]-a.c[Y])); }  


// linear interpolation (t is between 0.0 and 1.0)
float lerp(float a0, float a1, float t)
{ return a0*(1-t)+a1*t; }

// Grabs the next token from a line read from a file.
string nextToken(string line, int &start, int &end)
{
  end = line.find_first_of(" \t", start);
  if (end < 0 || end > line.length())
    end = line.length();

  string token = line.substr(start, end-start);
  start = line.find_first_not_of(" \t", end+1);
  return token;
}  

// Takes a level number and makes the proper filename.
string getLevelFileName(int levelNum)
{
  string s = LEVEL_PREFIX;
  char n[4];
  snprintf(n, 4, "%d", levelNum+1);
  s += n;
  return s;
}

string getLevelIntroFileName(int levelNum)
{
  string s = LEVEL_INTRO_PREFIX;
  char n[4];
  snprintf(n, 4, "%d", levelNum+1);
  s += n;
  return s;
}



// Load the configuration file. If it cannot find the file,
// the function exits, and the program uses the defaults.
void Configuration::loadFromFile(const char *filename)
{
  defaults();
  
  string dir = getenv("HOME");
  dir += LOCALSTATEDIR;

  string file = dir + filename;

  ifstream cfgfile(file.c_str());
  if (!cfgfile.is_open())
  {
    printf("Could not open configuration file %s, using default settings.\n", file.c_str());
    return;
  }
  
  int start, end;
  string line, option, value, keyvalues[13];
  while (getline(cfgfile, line, '\n'))
  {
    start = line.find_first_not_of(" \t\n");
    end = 0;
    
    // Parse the line
    if (line != "")
    {
      option = NEXT_TOK;
      
      if (option == "keybindings")
      { for (int i = 0; i < 13; i++) keyvalues[i] = NEXT_TOK; }
      else if (option != "level") 
        value = NEXT_TOK;
      
      // User config options
      if (option == "xres"              && value != "") xres       = S2I(value);
      if (option == "yres"              && value != "") yres       = S2I(value);
      if (option == "depth"             && value != "") depth      = S2I(value);
      if (option == "fullscreen"        && value != "") fullscreen = (bool)S2I(value);
      if (option == "antialias"         && value != "") antialias  = (bool)S2I(value);
      if (option == "blur"              && value != "") blur       = (bool)S2I(value);
      if (option == "use_sound"         && value != "") use_sound  = (bool)S2I(value);
      if (option == "sfx"               && value != "") sfx        = (bool)S2I(value);
      if (option == "music"             && value != "") music      = (bool)S2I(value);
      if (option == "mouse_threshold"   && value != "") mthresh    = S2I(value);
      if (option == "mouse_sensitivity" && value != "") msense     = S2I(value);
      if (option == "mouse_grab"        && value != "") mgrab      = S2I(value);
      if (option == "framecap"          && value != "") framecap   = S2I(value);

      // Key bindings
      if (option == "keybindings")
      {
        for (int i = 0; i < 13; i++)
        {
          if (keyvalues[i] != "")
            keys[i] = (SDLKey)S2I(keyvalues[i]);
        }
      }

      // Debugging options
      if (option == "show_fps"   && value != "") fps        = (bool)S2I(value);
      if (option == "normals"    && value != "") normals    = (bool)S2I(value);
      if (option == "paths"      && value != "") paths      = (bool)S2I(value);
      if (option == "info"       && value != "") info       = (bool)S2I(value);
      if (option == "culling"    && value != "") culling    = (bool)S2I(value);
      if (option == "level")     defaultLevel = line.substr(start);
      
      // The Cheat(s)
      if (option == CHEAT_INVUL   && value != "") cheat_invul   = (bool)S2I(value);
      if (option == CHEAT_WEAPONS && value != "") cheat_weapons = (bool)S2I(value);
      if (option == CHEAT_AMMO    && value != "") cheat_ammo    = (bool)S2I(value);
      if (option == CHEAT_KEYS    && value != "") cheat_keys    = (bool)S2I(value);
      if (option == CHEAT_NOAI    && value != "") cheat_noai    = (bool)S2I(value);
      if (option == CHEAT_NOMASKS && value != "") cheat_nomasks = (bool)S2I(value);
      if (option == CHEAT_SKIP    && value != "") cheat_skip    = (bool)S2I(value);
      if (option == CHEAT_ZOOM    && value != "") cheat_zoom    = (bool)S2I(value);
      if (option == CHEAT_ALL     && value != "")
      { 
        bool val = (bool)S2I(value);
        cheat_all = cheat_invul = cheat_weapons = cheat_ammo = cheat_keys = cheat_noai =
        cheat_nomasks = cheat_skip = cheat_zoom = val; 
      }
    }
  }
  cfgfile.close();
}

// Writes current configuration data to a file. If the file
// cannot be opened for writing, the function exits.
void Configuration::writeToFile(const char *filename)
{
  string dir = getenv("HOME");
  dir += LOCALSTATEDIR;

  string file = dir + filename;
  ofstream cfgfile(file.c_str());
  
  if (!cfgfile.is_open())
  {
    // try creating directory
    mkdir(dir.c_str(), 0777);
    cfgfile.clear();
    cfgfile.open(file.c_str());
    if (!cfgfile.is_open())
    { printf("Could not open configuration file %s for writing.\n", file.c_str()); return; }
  }
  cfgfile.clear();

  cfgfile << "xres " << xres << endl;
  cfgfile << "yres " << yres << endl;
  cfgfile << "depth " << depth << endl;
  cfgfile << "fullscreen " << fullscreen << endl;
  cfgfile << "antialias " << antialias << endl;
  cfgfile << "blur " << blur << endl;
  cfgfile << "use_sound " << use_sound << endl;
  cfgfile << "sfx " << sfx << endl;
  cfgfile << "music " << music << endl;
  cfgfile << "mouse_threshold " << mthresh << endl;
  cfgfile << "mouse_sensitivity " << msense << endl;
  cfgfile << "mouse_grab " << mgrab << endl;
  
  if (framecap >= 15) cfgfile << "framecap " << framecap << endl;
  
  cfgfile << "keybindings ";
  for (int i = 0; i < 13; i++)
  {
    cfgfile << (int)keys[i];
    if (i < 12)
      cfgfile << " ";
    else
      cfgfile << endl;
  }

  // Only output debugging options if they've been set

  if (fps) cfgfile << "show_fps " << fps << endl;
  if (normals) cfgfile << "normals " << normals << endl;
  if (paths) cfgfile << "paths " << paths << endl;
  if (info) cfgfile << "info " << info << endl;
  if (culling) cfgfile << "culling " << culling << endl;
  if (defaultLevel != "") cfgfile << "level " << defaultLevel << endl;
  
  if (cheat_all)
    cfgfile << CHEAT_ALL << " " << cheat_all << endl;
  else
  {
    if (cheat_invul) cfgfile   << CHEAT_INVUL   << " " << cheat_invul << endl;
    if (cheat_weapons) cfgfile << CHEAT_WEAPONS << " " << cheat_weapons << endl;
    if (cheat_ammo) cfgfile    << CHEAT_AMMO << " " << cheat_ammo << endl;
    if (cheat_keys) cfgfile    << CHEAT_KEYS << " " << cheat_keys << endl;
    if (cheat_noai) cfgfile    << CHEAT_NOAI << " " << cheat_noai << endl;
    if (cheat_nomasks) cfgfile << CHEAT_NOMASKS << " " << cheat_nomasks << endl;
    if (cheat_skip) cfgfile    << CHEAT_SKIP << " " << cheat_skip << endl;
    if (cheat_zoom) cfgfile    << CHEAT_ZOOM << " " << cheat_zoom << endl;
  }
  cfgfile.close();
}

// Blend a color into this color. Specifying USE_ALPHA
// for amountOfNew will blend with the second color's alpha value,
// else the function will use the specified value.
void RGBAColor::blend(RGBAColor c, float amountOfNew)
{
  r = lerp(r, c.r, (amountOfNew == USE_ALPHA) ? c.a : amountOfNew);
  g = lerp(g, c.g, (amountOfNew == USE_ALPHA) ? c.a : amountOfNew);
  b = lerp(b, c.b, (amountOfNew == USE_ALPHA) ? c.a : amountOfNew);
}

// Finds the intersection point of line segment ab with a line..
// Returns true and stores the intersection point in [ if there is an intersection.
// From the comp.graphics.algorithms FAQ, section 1.03
bool Line::intersection(Vector2D a, Vector2D b, Vector2D *p)
{
  float r, s;
  
  r =  ((a.c[Y]-p1.c[Y])*(p2.c[X]-p1.c[X])-(a.c[X]-p1.c[X])*(p2.c[Y]-p1.c[Y])) /
      ((b.c[X]-a.c[X])*(p2.c[Y]-p1.c[Y])-(b.c[Y]-a.c[Y])*(p2.c[X]-p1.c[X]));
  
  s =  ((a.c[Y]-p1.c[Y])*(b.c[X]-a.c[X])-(a.c[X]-p1.c[X])*(b.c[Y]-a.c[Y])) /
      ((b.c[X]-a.c[X])*(p2.c[Y]-p1.c[Y])-(b.c[Y]-a.c[Y])*(p2.c[X]-p1.c[X]));
    
  if (r < 0 || r > 1 || s < 0 || s > 1)
    return false;
  
  if (p != NULL)
    p->set(a.c[X]+r*(b.c[X]-a.c[X]), a.c[Y]+r*(b.c[Y]-a.c[Y]));
  return true;
}

// Stretches a bounding box (if necessary) to accommodate a point.
void BoundingBox::addPoint(Vector2D point)
{
  if (!pointsAdded)
  { min = point; max = point; pointsAdded = true; }
  else
  {
    if (point.c[X] < min.c[X])  min.c[X] = point.c[X];
    if (point.c[Y] < min.c[Y])  min.c[Y] = point.c[Y];
    if (point.c[X] > max.c[X])  max.c[X] = point.c[X];
    if (point.c[Y] > max.c[Y])  max.c[Y] = point.c[Y];
  }
  center = (min+max)*0.5;
}

// Calculate the size of the bounding box by finding the mininum and maximum x and y
// coordinates from a list of points.
void BoundingBox::calculate(vector<Vector2D> points)
{
  pointsAdded = false;
  for (int i = 0; i < points.size(); i++)
    addPoint(points[i]);
}

// Returns true if the point is contained in this bounding box.
bool BoundingBox::pointInBB(Vector2D point)
{
  return (point.c[X] >= min.c[X] && point.c[X] <= max.c[X] &&
          point.c[Y] >= min.c[Y] && point.c[Y] <= max.c[Y]);
}

// Returns true if this bounding box intersects another
bool BoundingBox::intersectBB(BoundingBox b2)
{
  if (min.c[X] > b2.max.c[X]) return false;
  if (max.c[X] < b2.min.c[X]) return false;
  if (min.c[Y] > b2.max.c[Y]) return false;
  if (max.c[Y] < b2.min.c[Y]) return false;
  return true;
}

void BoundingBox::rotate(float angle)
{
  Vector2D dmax = max-center, dmin = min-center;
  dmax.set(cos(angle)*dmax.c[X]-sin(angle)*dmax.c[Y], sin(angle)*dmax.c[X]+cos(angle)*dmax.c[Y]);
  dmin.set(cos(angle)*dmin.c[X]-sin(angle)*dmin.c[Y], sin(angle)*dmin.c[X]+cos(angle)*dmin.c[Y]);
  max = center + dmax;
  min = center + dmin;
}

// An adaptation of W. Randolph Franklin's pnpoly algorithm.
int Surface::pointInPolygon(Vector2D point)//int npol, float *xp, float *yp, float x, float y)
{
  int i, j, c = 0;
  for (i = 0, j = vert.size()-1; i < vert.size(); j = i++) {
    if ((((vert[i].c[Y]<=point.c[Y]) && (point.c[Y]<vert[j].c[Y])) ||
          ((vert[j].c[Y]<=point.c[Y]) && (point.c[Y]<vert[i].c[Y]))) &&
        (point.c[X] < (vert[j].c[X] - vert[i].c[X]) * (point.c[Y] - vert[i].c[Y]) / (vert[j].c[Y] - vert[i].c[Y]) + vert[i].c[X]))

      c = !c;
  }
  return c;
}

// Checks to see if there are no walls intersecting the line between two points.
bool hasLineOfSight(Vector2D start, Vector2D end, bool includeDoors)
{
  Vector2D ray, intersect;
  int cell;
  
  int startCX, startCY, endCX, endCY;
  int minCX, minCY, maxCX, maxCY;
  currLevel.cellNumber(start, &startCX, &startCY);
  currLevel.cellNumber(end, &endCX, &endCY);
  
  if (startCX <= endCX)
  { minCX = startCX; maxCX = endCX; }
  else   
  { minCX = endCX; maxCX = startCX; }
  
  if (startCY <= endCY)
  { minCY = startCY; maxCY = endCY; }
  else   
  { minCY = endCY; maxCY = startCY; }
    
  for (int cy = minCY; cy <= maxCY; cy++)
  {
    for (int cx = minCX; cx <= maxCX; cx++)
    {
      ray = end - start;

      cell = cy*currLevel.gridCols + cx;
      
      if (currLevel.inGrid(start + ray))
      { 
        int numWalls = currLevel.grid[cell].wall.size();
        Wall *currWall;
        
        for (int i = 0; i < numWalls + ((includeDoors) ? 4*currLevel.grid[cell].door.size() : 0); i++)
        {
          if (i < numWalls)
            currWall = &currLevel.wall[currLevel.grid[cell].wall[i]];
          else
            currWall = &currLevel.door[currLevel.grid[cell].door[(i-numWalls)/4]].sides[(i-numWalls)%4];
                          
          if ((currWall->CollFlags() & COLLIDE_OBJECT) && currWall->height > 0 && currWall->intersection(start, start+ray, &intersect))
            return false;
        }
      }
    }
  }
  return true;
}

bool circleSegmentIntersection(Vector2D c, float r, Vector2D p1, Vector2D p2, Vector2D *intersect)
{
  Vector2D dir = p2 - p1;
  Vector2D diff = c - p1;
  float t = (diff*dir) / (dir*dir);
  if (t < 0.0f)
      t = 0.0f;
  if (t > 1.0f)
      t = 1.0f;
  Vector2D closest = p1 + (dir * t);
  Vector2D d = c - closest;
  intersect->set(closest.c[X], closest.c[Y]);
  float distsqr = d*d;
  return distsqr <= r * r;
}

void gameWelcomeMessage()
{
  time_t rawtime;
  tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  
  printf("%s %s\nBuilt %s %s; written by %s\n", PACKAGE_NAME, PACKAGE_VERSION, __DATE__, __TIME__, PACKAGE_AUTHOR);
  printf("Started %s", asctime(timeinfo));
}  

void gameInitSDL()
{
  printf("  Initializing SDL...");
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    errorMessage(1, "FLAGRANT SYSTEM ERROR", "Could not initialize SDL (%s)", SDL_GetError());

  atexit(SDL_Quit);

  SDL_WM_SetCaption(PACKAGE_NAME, PACKAGE_NAME);
  printf("ok\n");
  
  SDL_ShowCursor(SDL_DISABLE);
    
  if (config.use_sound)
  {
    printf("  Initializing sound...");
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) < 0)
    {
      printf("Could not initialize sound mixer (%s)\nContinuing with no sound.\nTo reenable sound, set \"use_sound\" to 1 in %s%s%s\n", Mix_GetError(), getenv("HOME"), LOCALSTATEDIR, CONFIG_FILE);
      config.use_sound = false;
    }
    else
      printf("ok\n");
  }
}

void gameInitOpenGL()
{
  printf("  Initializing OpenGL...");
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  glClearColor(0.0, 0.0, 0.0, 0.0);
  glShadeModel(GL_SMOOTH);
  
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_LEQUAL, 0);
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glEnable(GL_TEXTURE_2D);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  
  GLfloat light_position[] = {0.0, 0.0, 1.0, 1.0};
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    
  printf("ok\n");
}

void gameSetVideoMode(int width, int height, int depth, int fullscreen)
{
  printf("  Setting video mode %dx%dx%d...", width, height, depth);
  if (!SDL_SetVideoMode(width, height, depth, SDL_OPENGL | SDL_HWSURFACE | fullscreen))
    errorMessage(2, "FLAGRANT SYSTEM ERROR", "Could not set video mode: %s\nChange your video settings in %s%s%s", SDL_GetError(), getenv("HOME"), LOCALSTATEDIR, CONFIG_FILE);

  GFXMODE_ISACTIVE = true;
  
  glViewport(0, 0, width, height);
  SCREEN_ASPECT = (float)width/(float)height;
  gameSetPerspectiveMode();
  printf("ok\n    Using %s %s %s\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER),
    glGetString(GL_VERSION));
}

void gameSetPerspectiveMode()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(55.0, SCREEN_ASPECT, 0.1, 1000.0);
  glMatrixMode(GL_MODELVIEW);  
}  

void gameSetOrthoMode(float r)
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  SCREEN_TOP = r/SCREEN_ASPECT;
  glOrtho(-r, r, -SCREEN_TOP, SCREEN_TOP, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void gameSetAntialiasing(bool set)
{
  (set) ? glEnable(GL_LINE_SMOOTH) : glDisable(GL_LINE_SMOOTH);
}

void gameInitTessellator()
{
  tess = gluNewTess();
  tessAllocatedVertices.clear();
  gluTessCallback(tess, GLU_TESS_BEGIN,  (GLvoid (CALLBACK *) ())tcbBegin);
  gluTessCallback(tess, GLU_TESS_VERTEX, (GLvoid (CALLBACK *) ())glVertex3dv);
  gluTessCallback(tess, GLU_TESS_END,    (GLvoid (CALLBACK *) ())tcbEnd);
  gluTessCallback(tess, GLU_TESS_COMBINE,(GLvoid (CALLBACK *) ())tcbCombine);
  gluTessCallback(tess, GLU_TESS_ERROR,  (GLvoid (CALLBACK *) ())tcbError);
}

void gameLoadSprites()
{
  printf("  Loading sprites...");
  // allocate texture space
  glGenTextures(NUM_TEXTURES, textures);
  int i;
  for (i = 0; i < NUM_TEXTURES; i++)
  {
    switch(i)
    {
      case TEX_FONT:                texdata[i] = IMG_Load(PKGDATADIR"sprites/font3.png"); break;
      case TEX_SHADOW:              texdata[i] = IMG_Load(PKGDATADIR"sprites/shadow.png"); break;
      case TEX_LIGHT:               texdata[i] = IMG_Load(PKGDATADIR"sprites/light.png"); break;
      case TEX_SCANLINES:           texdata[i] = IMG_Load(PKGDATADIR"sprites/scanlines.png"); break;      
      case TEX_MF_RIFLE:            texdata[i] = IMG_Load(PKGDATADIR"sprites/muzzleflash.png"); break;
      case TEX_MF_SHOTGUN:          texdata[i] = IMG_Load(PKGDATADIR"sprites/muzzleflash2.png"); break;
      case TEX_EXPLOSION1:          texdata[i] = IMG_Load(PKGDATADIR"sprites/explosion1.png"); break;
      case TEX_EXPLOSION2:          texdata[i] = IMG_Load(PKGDATADIR"sprites/explosion2.png"); break;
      case TEX_EXPLOSION3:          texdata[i] = IMG_Load(PKGDATADIR"sprites/explosion3.png"); break;
      case TEX_EXPLOSION4:          texdata[i] = IMG_Load(PKGDATADIR"sprites/explosion4.png"); break;
      case TEX_SMOKE:               texdata[i] = IMG_Load(PKGDATADIR"sprites/smoke.png"); break;
      case TEX_BEAM1:               texdata[i] = IMG_Load(PKGDATADIR"sprites/beam1.png"); break;
      case TEX_BEAM2:               texdata[i] = IMG_Load(PKGDATADIR"sprites/beam2.png"); break;
      case TEX_LASER_GLOW:          texdata[i] = IMG_Load(PKGDATADIR"sprites/laserglow.png"); break;
      case TEX_ESG_CHARGE:          texdata[i] = IMG_Load(PKGDATADIR"sprites/esgcharge.png"); break;
      case TEX_BORDER:              texdata[i] = IMG_Load(PKGDATADIR"sprites/border.png"); break;
      case TEX_TERMINAL_BG:         texdata[i] = IMG_Load(PKGDATADIR"sprites/scanline.png"); break;
      case TEX_BIT:                 texdata[i] = IMG_Load(PKGDATADIR"sprites/bit.png"); break;
      case TEX_BOX_NORMAL:          texdata[i] = IMG_Load(PKGDATADIR"sprites/box.png"); break;
      case TEX_BARREL_NORMAL:       texdata[i] = IMG_Load(PKGDATADIR"sprites/barrel.png"); break;
      case TEX_STEELBOX_NORMAL:     texdata[i] = IMG_Load(PKGDATADIR"sprites/steelbox.png"); break;
      case TEX_PARTICLE:            texdata[i] = IMG_Load(PKGDATADIR"sprites/particle.png"); break;
      case TEX_PARTICLE_BLOOD:      texdata[i] = IMG_Load(PKGDATADIR"sprites/particle_blood.png"); break;
      case TEX_PARTICLE_SLIME:      texdata[i] = IMG_Load(PKGDATADIR"sprites/particle_slime.png"); break;
      case TEX_PARTICLE_SPARK:      texdata[i] = IMG_Load(PKGDATADIR"sprites/particle_pinkspark.png"); break;
      case TEX_PARTICLE_ENERGY:     texdata[i] = IMG_Load(PKGDATADIR"sprites/particle_energy.png"); break;
      case TEX_PARTICLE_WOOD:       texdata[i] = IMG_Load(PKGDATADIR"sprites/particle_wood.png"); break;
      case TEX_PARTICLE_METAL:      texdata[i] = IMG_Load(PKGDATADIR"sprites/particle_metal.png"); break;
      case TEX_PARTICLE_GLOW:       texdata[i] = IMG_Load(PKGDATADIR"sprites/glowparticle.png"); break;
      case TEX_HEPA_SPARK:          texdata[i] = IMG_Load(PKGDATADIR"sprites/particle_hepa_spark.png"); break;
      case TEX_LASER_SPARK1:        texdata[i] = IMG_Load(PKGDATADIR"sprites/particle_laser_spark.png"); break;
      case TEX_LASER_SPARK2:        texdata[i] = IMG_Load(PKGDATADIR"sprites/particle_laser_spark2.png"); break;
      case TEX_ESG_SHOCKWAVE:       texdata[i] = IMG_Load(PKGDATADIR"sprites/esgshockwave.png"); break;
      case TEX_EXPLOSIONPART:       texdata[i] = IMG_Load(PKGDATADIR"sprites/explosionpart.png"); break;
      case TEX_POWERUP_HEALTH:      texdata[i] = IMG_Load(PKGDATADIR"sprites/health.png"); break;
      case TEX_POWERUP_ENERGY_CLIP: texdata[i] = IMG_Load(PKGDATADIR"sprites/clip.png"); break;
      case TEX_POWERUP_SHOTGUN_AMMO:texdata[i] = IMG_Load(PKGDATADIR"sprites/shotgunammo.png"); break;
      case TEX_POWERUP_HEPA_CLIP:   texdata[i] = IMG_Load(PKGDATADIR"sprites/hepaclip.png"); break;
      case TEX_POWERUP_LASER_CELL:  texdata[i] = IMG_Load(PKGDATADIR"sprites/lasercell.png"); break;
      case TEX_POWERUP_ESG_BATTERY: texdata[i] = IMG_Load(PKGDATADIR"sprites/esgcell.png"); break;
      case TEX_POWERUP_KEY1:        texdata[i] = IMG_Load(PKGDATADIR"sprites/key1.png"); break;
      case TEX_POWERUP_KEY2:        texdata[i] = IMG_Load(PKGDATADIR"sprites/key2.png"); break;
      case TEX_POWERUP_KEY3:        texdata[i] = IMG_Load(PKGDATADIR"sprites/key3.png"); break;
      case TEX_WEAPON_RIFLE:        texdata[i] = IMG_Load(PKGDATADIR"sprites/riflepickup.png"); break;
      case TEX_WEAPON_SHOTGUN:      texdata[i] = IMG_Load(PKGDATADIR"sprites/shotgunpickup.png"); break;
      case TEX_WEAPON_HEPA:         texdata[i] = IMG_Load(PKGDATADIR"sprites/hepapickup.png"); break;
      case TEX_WEAPON_LASER:        texdata[i] = IMG_Load(PKGDATADIR"sprites/laserpickup.png"); break;
      case TEX_WEAPON_ESG:          texdata[i] = IMG_Load(PKGDATADIR"sprites/esgpickup.png"); break;
      case TEX_RIFLE_SHELL:         texdata[i] = IMG_Load(PKGDATADIR"sprites/rifleshell.png"); break;
      case TEX_SHOTGUN_SHELL:       texdata[i] = IMG_Load(PKGDATADIR"sprites/shotgunshell.png"); break;
      case TEX_BLT_RIFLE:           texdata[i] = IMG_Load(PKGDATADIR"sprites/bullet.png"); break;
      case TEX_BLT_SHOTGUN:         texdata[i] = IMG_Load(PKGDATADIR"sprites/pellet.png"); break;
      case TEX_BLT_GRENADE:         texdata[i] = IMG_Load(PKGDATADIR"sprites/grenadelive.png"); break;
      case TEX_BLT_SHRAPNEL:        texdata[i] = IMG_Load(PKGDATADIR"sprites/shrapnel.png"); break;
      case TEX_BLT_BLASTER:         texdata[i] = IMG_Load(PKGDATADIR"sprites/blaster.png"); break;
      case TEX_WPN_RIFLE:           texdata[i] = IMG_Load(PKGDATADIR"sprites/rifle.png"); break;
      case TEX_WPN_SHOTGUN:         texdata[i] = IMG_Load(PKGDATADIR"sprites/shotgun.png"); break;
      case TEX_WPN_HEPA:            texdata[i] = IMG_Load(PKGDATADIR"sprites/hepa.png"); break;
      case TEX_WPN_LASER:           texdata[i] = IMG_Load(PKGDATADIR"sprites/laser.png"); break;
      case TEX_WPN_ESG:             texdata[i] = IMG_Load(PKGDATADIR"sprites/esg.png"); break;
      case TEX_WPN_GRENADE:         texdata[i] = IMG_Load(PKGDATADIR"sprites/grenade.png"); break;
      case TEX_TITLE_BG:            texdata[i] = IMG_Load(PKGDATADIR"sprites/title.png"); break;
      case TEX_BG2:                 texdata[i] = IMG_Load(PKGDATADIR"sprites/bg2.png"); break;
      case TEX_HEX1:                texdata[i] = IMG_Load(PKGDATADIR"sprites/hex1.png"); break;
      case TEX_HEX2:                texdata[i] = IMG_Load(PKGDATADIR"sprites/hex2.png"); break;
      
      case MTEX_PLAYER:             texdata[i] = IMG_Load(PKGDATADIR"sprites/skins/playerskin.png"); break;
      case MTEX_PLAYER2:            texdata[i] = IMG_Load(PKGDATADIR"sprites/skins/playerskin2.png"); break;
      case MTEX_BARREL:             texdata[i] = IMG_Load(PKGDATADIR"sprites/skins/barrelskin.png"); break;
      case MTEX_TURRET:             texdata[i] = IMG_Load(PKGDATADIR"sprites/skins/turretskin.png"); break;
      case MTEX_MIB:                texdata[i] = IMG_Load(PKGDATADIR"sprites/skins/mibskin.png"); break;
      case MTEX_UNARMED_GUARD:      texdata[i] = IMG_Load(PKGDATADIR"sprites/skins/unarmedskin.png"); break;
      case MTEX_ARMED_DRONE:        texdata[i] = IMG_Load(PKGDATADIR"sprites/skins/armedskin.png"); break;
      case MTEX_HUNTER:             texdata[i] = IMG_Load(PKGDATADIR"sprites/skins/hunterskin.png"); break;
      case MTEX_GLADIATOR:          texdata[i] = IMG_Load(PKGDATADIR"sprites/skins/gladiatorskin.png"); break;
      case MTEX_FADE:               texdata[i] = IMG_Load(PKGDATADIR"sprites/skins/fade.png"); break;
    }
    
    if (!texdata[i])
      errorMessage(3, "FLAGRANT SYSTEM ERROR", "Could not load texture %d", i);
    
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    SDL_LockSurface(texdata[i]);
	GLenum format = (texdata[i]->format->Rshift == 16) ? GL_BGRA : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, 4, texdata[i]->w, texdata[i]->h,
              0, format, GL_UNSIGNED_BYTE, texdata[i]->pixels);
    SDL_UnlockSurface(texdata[i]);
  }
  
  // Generate blur textures
  glGenTextures(NUM_BLUR_TEXTURES, blurTextures);
  for (int j = 0; j < NUM_BLUR_TEXTURES; j++)
  {
    blurdata[j] = (unsigned int*)new GLuint[((BLUR_TEXTURE_WIDTH * BLUR_TEXTURE_HEIGHT)* 4 * sizeof(unsigned int))];
    memset(blurdata[j], 0, ((BLUR_TEXTURE_WIDTH * BLUR_TEXTURE_HEIGHT)* 4 * sizeof(unsigned int)));
    
    glBindTexture(GL_TEXTURE_2D, blurTextures[j]);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, BLUR_TEXTURE_WIDTH, BLUR_TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, blurdata[j]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    
    delete[] blurdata[j]; 
  }
  
  printf("ok (loaded %d)\n", i);
}

void gameLoadModels()
{
  printf("  Loading models...");
  int i;
  for (i = 0; i < NUM_MODELS; i++)
  {
    switch(i)
    {
      case MDL_PLAYER_LEGS:   modelLoader.ImportMD2(&models[i], PKGDATADIR"models/player_legs" MD2_SUFFIX, MTEX_PLAYER);
                              models[i].setTransforms(0.65, 90.0, 0.0, 90.0, 0.0, 0.0, 0.0); break;
      case MDL_PLAYER_TORSO:  modelLoader.ImportMD2(&models[i], PKGDATADIR"models/player_torso" MD2_SUFFIX, MTEX_PLAYER);
                              models[i].setTransforms(1.0, 90.0, 0.0, 90.0,  0.0, 0.0, 0.0); break;
      case MDL_PLAYER_TORSO2: modelLoader.ImportMD2(&models[i], PKGDATADIR"models/player_torso" MD2_SUFFIX, MTEX_PLAYER2);
                              models[i].setTransforms(1.0, 90.0, 0.0, 90.0,  0.0, 0.0, 0.0); break;
      case MDL_BOX:           modelLoader.ImportMD2(&models[i], PKGDATADIR"models/box" MD2_SUFFIX, TEX_BOX_NORMAL);
                              models[i].setTransforms(1.0, 90.0, 0.0, 0.0,   0.0, 0.0, 0.0); break;      
      case MDL_BARREL:        modelLoader.ImportMD2(&models[i], PKGDATADIR"models/barrel" MD2_SUFFIX, MTEX_BARREL);
                              models[i].setTransforms(1.0, 90.0, 0.0, 0.0,   0.0, 0.0, 0.0); break;
      case MDL_STEELBOX:      modelLoader.ImportMD2(&models[i], PKGDATADIR"models/box" MD2_SUFFIX, TEX_STEELBOX_NORMAL);
                              models[i].setTransforms(1.0, 90.0, 0.0, 0.0,   0.0, 0.0, 0.0); break;
      case MDL_TURRET:        modelLoader.ImportMD2(&models[i], PKGDATADIR"models/turret" MD2_SUFFIX, MTEX_TURRET);
                              models[i].setTransforms(1.0, 90.0, 0.0, 90.0,   0.0, 0.0, 1.8); break;
      case MDL_MIB:           modelLoader.ImportMD2(&models[i], PKGDATADIR"models/mib" MD2_SUFFIX, MTEX_MIB);
                              models[i].setTransforms(1.0, 90.0, 0.0, 90.0,  0.0, 0.0, 2.3); break; 
      case MDL_UNARMED_GUARD: modelLoader.ImportMD2(&models[i], PKGDATADIR"models/unarmedguard" MD2_SUFFIX, MTEX_UNARMED_GUARD);
                              models[i].setTransforms(0.15, 0.0, 0.0, 90.0,  0.0, 0.0, 11.0); break;
      case MDL_ARMED_DRONE:   modelLoader.ImportMD2(&models[i], PKGDATADIR"models/armeddrone" MD2_SUFFIX, MTEX_ARMED_DRONE);
                              models[i].setTransforms(1.0, 0.0, 0.0, 90.0,  0.0, 0.0, 1.5); break;
      case MDL_HUNTER:        modelLoader.ImportMD2(&models[i], PKGDATADIR"models/hunter" MD2_SUFFIX, MTEX_HUNTER);
                              models[i].setTransforms(0.25, 90.0, 0.0, 90.0,  0.0, 0.0, 11.0); break;      
      case MDL_GLADIATOR:     modelLoader.ImportMD2(&models[i], PKGDATADIR"models/gladiator" MD2_SUFFIX, MTEX_GLADIATOR);
                              models[i].setTransforms(0.5, 90.0, 0.0, 90.0,  0.0, 0.0, 5.0); break;
      case MDL_ZERO:          modelLoader.ImportMD2(&models[i], PKGDATADIR"models/zero" MD2_SUFFIX, MTEX_FADE);
                              models[i].setTransforms(0.15, 0.0, 0.0, 0.0,  0.0, 0.0, 0.0); break;
      case MDL_ONE:           modelLoader.ImportMD2(&models[i], PKGDATADIR"models/one" MD2_SUFFIX, MTEX_FADE);
                              models[i].setTransforms(0.15, 0.0, 0.0, 0.0,  0.0, 0.0, 0.0); break;
    }
  }
  printf("ok (loaded %d)\n", i);
}

void gameLoadSounds()
{
  printf("  Loading sounds...");

  int i;
  for (i = 0; i < NUM_SOUNDS; i++)
  {
    switch(i)
    {    
      case SND_WPN_RIFLE:         snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/rifle.wav"); break;
      case SND_WPN_SHOTGUN:       snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/shotgun.wav"); break;
      case SND_WPN_HEPA:          snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/hepa.wav"); break;
      case SND_WPN_LASER:         snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/laser.wav"); break;
      case SND_WPN_LASER2:        snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/lasercharged.wav"); break;
      case SND_WPN_ESG:           snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/esg.wav"); break;
      case SND_GRENADE_BOUNCE:    snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/gren-bounce.wav"); break;
      case SND_DOOR_OPEN:         snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/dooropen.wav"); break;
      case SND_DOOR_CLOSE:        snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/doorclose.wav"); break;
      case SND_DOOR_UNLOCK:       snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/doorunlockopen.wav"); break;
      case SND_EXPLOSION:         snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/explosion.wav"); break;
      case SND_HEPA_EXPLOSION:    snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/hepa-explode.wav"); break;
      case SND_GRENADE_EXPLOSION: snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/smallexplosion.wav"); break;
      case SND_BULLET_HIT_HARD:   snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/bullet-on-hard.wav"); break;
      case SND_BULLET_HIT_SOFT:   snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/bullet-on-soft.wav"); break;
      case SND_PICKUP_HEALTH:     snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/pickuphealth.wav"); break;
      case SND_PICKUP_ITEM:       snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/pickupitem.wav"); break;
      case SND_JUMP:              snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/jump.wav"); break;
      case SND_LAND:              snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/land.wav"); break;
      case SND_RELOAD:            snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/reload.wav"); break;
      case SND_CHANGE_WEAPON:     snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/changegun.wav"); break;
      case SND_ROBOT_SEES:        snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/botseesyou.wav");  break;
      case SND_ROBOT_DEATH:       snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/botexplode.wav");  break;
      case SND_ROBOT_PARALYZE:    snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/botparalyze.wav");  break;
      case SND_ROBOT_ALERT:       snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/botalert.wav");  break;
      
      case SND_EXPLODE:           snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/pexplode.wav");  break;
      case SND_FALL:              snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/pfall.wav");  break;
      case SND_HEAL:              snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/phealingsurface.wav");  break;
      case SND_HURT1:             snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/phurt.wav");  break;
      case SND_HURT2:             snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/phurt2.wav");  break;
      case SND_HURTSURFACE:       snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/phurtsurface.wav");  break;
      case SND_KILLED:            snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/pkilledbybullet.wav");  break;
      
      case SND_TERM_MOVE:         snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/termcursormove.wav");  break;
      case SND_TERM_SELECT:       snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/termselect.wav");  break;
      case SND_TERM_CLEAR:        snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/termclear.wav");  break;
      case SND_LAUNCH:            snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/launch.wav");  break;
      case SND_LAUNCH2:           snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/launch2.wav");  break;
      case SND_BUTTON:            snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/button.wav");  break;
      case SND_ALARM:             snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/alarm.wav");  break;
      case SND_LASER_CHARGE:      snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/lasercharge.wav");  break;
      case SND_LASER_CHARGE_LOOP: snddata[i] = Mix_LoadWAV(PKGDATADIR"snd/laserchargeloop.wav");  break;

    }
    
    if (!snddata[i])
      errorMessage(3, "FLAGRANT SYSTEM ERROR", "Could not load sound %d", i);    
  }
  printf("ok (loaded %d)\n", i);
}

void gameLoadMusic()
{
  printf("  Loading music...");

  int i;
  for (i = 0; i < NUM_SONGS; i++)
  {
    switch(i)
    {
      case MUS_TITLE:           musicdata[i] = Mix_LoadMUS(PKGDATADIR"snd/blackbubble2.ogg"); break;
      case MUS_DD2:             musicdata[i] = Mix_LoadMUS(PKGDATADIR"snd/dd2.ogg"); break;
      case MUS_END:             musicdata[i] = Mix_LoadMUS(PKGDATADIR"snd/loop13.ogg"); break;
    }
    
    if (!musicdata[i])
      errorMessage(3, "FLAGRANT SYSTEM ERROR", "Could not load song %d", i);    
  }
  printf("ok (loaded %d)\n", i);
}

// run through object list and allocate appropriate AI entities
void loadAI()
{
  if (!config.cheat_noai)
  {
    for (int i = 1; i < currLevelObjs.size(); i++)
    {
      switch(currLevelObjs[i]->Type())
      {
        case ENT_UNARMED_GUARD: case ENT_MIB:
          currLevelEnemies.push_back(new KamikazeAI(i, 0, currLevelObjs[i]->State(), 30, 50, 60)); break;
        case ENT_TURRET1:
          currLevelEnemies.push_back(new TurretAI(i, 0, currLevelObjs[i]->State(), 0, 40, 60)); break;
        case ENT_SLAVE_TURRET:
          currLevelEnemies.push_back(new SlaveTurretAI(i, 0, currLevelObjs[i]->State(), 0, 40, 60)); break;
        case ENT_ARMED_DRONE:
          currLevelEnemies.push_back(new HunterAI(i, 0, currLevelObjs[i]->State(), 25, 40, 60,  0.5,1,  1,3,  0.1,0.3,  0.7,1, 15, 3, torad(-5))); break;
        case ENT_HUNTER:
          // float plm, float plr, float pdm, float pdr, float slm, float slr, float sdm, float sdr, float cdist, float aim
          currLevelEnemies.push_back(new HunterAI(i, 0, currLevelObjs[i]->State(), 25, 30, 90,  0.75,1,  1,2,  0.1,0.3,  0.7,1, 10+frand()*5, 6, torad(15)*((rand()%2)?-1:1))); break;
        case ENT_GLADIATOR:
          currLevelEnemies.push_back(new HunterAI(i, 0, currLevelObjs[i]->State(), 25, 30, 40,  0.75,1,  1,2,  0.2,0,  1.0,3.0, 10+frand()*5, 0, torad(5))); break;

      }
    }
  }
}

void destroyAI()
{
  for (int i = 1; i < currLevelEnemies.size(); i++)
    delete currLevelEnemies[i];
  
  currLevelEnemies.clear();
}

// deallocates SDL_Surfaces that store texture data
void gameDestroySprites()
{
  for (int i = 0; i < NUM_TEXTURES; i++)
    SDL_FreeSurface(texdata[i]);
  
  glDeleteTextures(NUM_TEXTURES, textures);
  glDeleteTextures(NUM_BLUR_TEXTURES, blurTextures);
}  

void gameDestroySounds()
{
  for (int i = 0; i < NUM_SOUNDS; i++)
    Mix_FreeChunk(snddata[i]);
}

void gameDestroyMusic()
{
  for (int i = 0; i < NUM_SONGS; i++)
    Mix_FreeMusic(musicdata[i]);
}

// reclaim allocated memory
void gameShutdown()
{
  printf("Shutting down...");
  stopMusic();

  if (config.use_sound)
  {
    gameDestroyMusic();
    gameDestroySounds();
  }

  gameDestroySprites();
  gluDeleteTess(tess);
  for(vector<GLdouble *>::iterator i = tessAllocatedVertices.begin(); i != tessAllocatedVertices.end(); i++)
    free(*i);
  Mix_CloseAudio();
  printf("ok\nEnd program\n");
}

// Launch a particle (if there is a free slot available)
void launchParticle(int typ, float x, float y, float height, float head, float alph, int sprite)
{
  if (liveParticles < NUM_PARTICLES)
  {
    particles[liveParticles].launch(typ, x, y, height, head, alph, sprite);
    liveParticles++;
  }     
}  

void launchParticle(int typ, float x, float y, float height, float head, float alph, int sprite, float r, float g, float b)
{
  if (liveParticles < NUM_PARTICLES)
  {
    particles[liveParticles].launch(typ, x, y, height, head, alph, sprite, r, g, b);
    liveParticles++;
  }     
}  

// Takes a particle out of the array.
// Achieved in constant time by overwriting the particle's data with that
// of the newest particle and decrementing liveParticles by one.
void killParticle(int index)
{ particles[index] = particles[--liveParticles]; }

// Launch a bullet (if there is a free slot available)
void launchBullet(int own, int typ, float x, float y, float height, float head)
{
  if (liveBullets < NUM_BULLETS)
  {
    bullets[liveBullets].launch(own, typ, x, y, height, head);
    liveBullets++;
  }     
}

void killBullet(int index)
{
  bullets[index].kill(!bullets[index].Falling());
  bullets[index] = bullets[--liveBullets];
}

void launchLaser(int typ, float x, float y, float height, float head)
{
  if (liveLasers < NUM_LASERS)
  {
    lasers[liveLasers].launch(typ, x, y, height, head);
    liveLasers++;
  }     
}

void killLaser(int index)
{ lasers[index] = lasers[--liveLasers]; }

// Add a force
void addForce(int obj, float dur, float a, float b, bool mode)
{
  if (liveForces < NUM_FORCES)
  {
    forces[liveForces].set(obj, dur, a, b, mode);
    forces[liveForces].setActive(true);
    liveForces++;  
  }   
}

// Adds a force and applies it for one frame
void addInstantForce(int obj, float a, float b, bool mode)
{
  currLevelObjs[obj]->changeVel((mode == FORCE_RECT) ? Vector2D(a,b) : Vector2D(a*cos(b), a*sin(b)));
}

// Removes a force
void killForce(int index)
{
  forces[index].setActive(false);
  forces[index] = forces[--liveForces];
}

// Add a shockwave
void addShockwave(float x, float y, float ir, float irs, float otr, float ors, float dur, float frc, float hlth, bool ag)
{
  if (liveShockwaves < NUM_SHOCKWAVES)
  {
    shockwaves[liveShockwaves].set(x, y, ir, irs, otr, ors, dur, frc, hlth, ag);
    shockwaves[liveShockwaves].setActive(true);
    liveShockwaves++;  
  }   
}

// Removes a shockwave
void killShockwave(int index)
{
  shockwaves[index].setActive(false);
  shockwaves[index] = shockwaves[--liveShockwaves];
}

void setScreenQuake(float amt)
{ screenQuake = amt; }

void addBlur(float amt, float r, float g, float b)
{
  screenBlur += amt;
  screenBlurColor.set(r, g, b);
}

void setBlur(float amt, float r, float g, float b)
{
  screenBlur = amt;
  screenBlurColor.set(r, g, b);
}

void setTrigger(int t)
{
  printf("Triggered %d\n", t);
  if (t > 0 && t <= currLevel.maxTrigger)
    triggers[t].hit = true;
  else if (t == -1)
    winLevel = true;
  else if (t == -2)
  {
    transitioning = true;
    fadeOutMusic(2000);
    playSound(SND_LAUNCH, 3);
  }
  else if (t == -3)
  {
    PLAYER_OBJECT->kill(true);
    captured = true;
  }
  else if (t == -4)
  {
    setMessage("!!! CATASTROPHIC SYSTEM FAILURE !!!");
    fadeOutMusic(1000);
	PLAYER_OBJECT->makeInvulnerable(); // prevent the player from dying
    systemExplode = true;

  }
}

void incrementTrigger(int t)
{
  if (t > 0 && t <= currLevel.maxTrigger)
  {
    triggers[t].hits++;
    //printf("Incremented %d; hits %d, hitsrequired %d\n", t, triggers[t].hits, triggers[t].hitsRequired);
    
    if (triggers[t].hits >= triggers[t].hitsRequired)
    { setTrigger(t); triggers[t].hits = 0; }
  }
}

void setMessage(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(message, MESSAGE_LENGTH-1, fmt, ap);

  messageTime = MESSAGE_DELAY_TIME;
}

void playSound(int sound, int channel)
{
  if (config.use_sound && config.sfx)
  {   
    if (sound >= 0 && sound < NUM_SOUNDS)
      Mix_PlayChannel(channel, snddata[sound], 0);
  }
}

void playSoundLooped(int sound, int channel)
{
  if (config.use_sound && config.sfx)
  {
    
    if (sound >= 0 && sound < NUM_SOUNDS)
      Mix_PlayChannel(channel, snddata[sound], INT_MAX);
  }
}


void stopSound(int channel)
{
  if (config.use_sound && config.sfx)
    Mix_HaltChannel(channel);
}

void startMusic(int song)
{
  if (config.use_sound && config.music)
  {
    if (song >= 0 && song < NUM_SONGS)
      Mix_PlayMusic(musicdata[song], -1);
  }
}

void stopMusic()
{
  if (config.use_sound && config.music)
  Mix_HaltMusic();
}

void fadeOutMusic(int ms)
{
  if (config.use_sound && config.music)
    Mix_FadeOutMusic(ms);
}

void fadeInMusic(int song, int ms)
{
  if (config.use_sound && config.music)
  {
    if (song >= 0 && song < NUM_SONGS)
      Mix_FadeInMusic(musicdata[song], -1, ms);
  }
}


// Main game loop.
int playCurrentLevel(bool playInSequence)
{
  if (!currLevel.loaded)
  {
    printf("Crap! No level loaded!\n");
    return 0;
  }
  setMessage(""); messageTime = 0;
  
  glEnable(GL_BLEND);
  glShadeModel(GL_SMOOTH);
  
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_EQUAL, 1.0);
  
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  bool loop = true;
  
  winLevel = captured = transitioning = false;
  Camera cam;
  BoundingBox viewRegion;
    
  // Load level entities
  currLevel.loadObjects();
  loadAI();  
  
  gameSetPerspectiveMode();
  currLevel.start();
  int errnum;
  float mousePollTimer = 0;
  float healthAtFrameStart;
  int wpnAtFrameStart;
  float healthChange = 0;
  float damageEffectIntensity = 0;
  float damageEffectTimer = 0;
  liveParticles = liveBullets = liveShockwaves = liveLasers = 0;
  
  screenQuake = 0;
  screenBlur = 0;
  screenBlurColor.white();
  
  setMessage(currLevel.name.c_str());    
  
  int blurQueueFront = 0;
  int blurQueueRear = 0;
  int blurQueueFrame = 0;
  float blurUpdateTimer = 0;
  
  // flush the keys
  keystate = SDL_GetKeyState(NULL);
  bool escPressed = keystate[SDLK_ESCAPE];
  bool paused = false;
  bool aborted = false;
  bool died = false;
  
  float blinker = 0;
  float weaponDisplayTimer = 0;
  float transitionTimer = 0;
  bool transitionJump = false;
  float systemExplodeTimer = 0.0;
  systemExplode = 0;
  
  P.resetCurrentLevelStats();
  
  TerminalWindow pauseMenu(15.0, 17.0, 6, 6, 2.0, 3.0, 3, 0.1, RGBAColor(0.5, 1.0, 0.76, 1.0), RGBAColor(0.5, 1.0, 0.76, 0.2));
  pauseMenu.setCursor(2, 3, 0, 3, ">", 3);
  pauseMenu.addString(1, "Abort?"); pauseMenu.addString(0, "");
  pauseMenu.addString(0, "  yes");  pauseMenu.addString(0, "  no");  
  pauseMenu.addString(0, "");       pauseMenu.addString(0, "(sdb)"); 
  pauseMenu.deactivate();
  
  TerminalWindow deadMenu(15.0, 17.0, 6, 16, 2.0, 3.0, 3, 0.1, RGBAColor(0.5, 1.0, 0.76, 1.0), RGBAColor(0.5, 1.0, 0.76, 0.2));
  deadMenu.setCursor(2, 3, 0, 2, ">", 3);
  deadMenu.deactivate();
  
  int returnVal = LEVEL_QUIT;
  
  // warm mouse and clear relative mouse state
  SDL_WarpMouse(config.xres/2, config.yres/2);
  SDL_GetRelativeMouseState(NULL, NULL);
  printf("Start\n");
	   
  while (loop && !winLevel)
  {
    timer.reset();
    
    // Error checking
    errnum = glGetError();
    if (errnum != GL_NO_ERROR)
      { errorMessage(9, "FLAGRANT OPENGL ERROR", "CODE %d: %s", errnum, gluErrorString(errnum)); }
  
    // Event checking loop
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
        case SDL_QUIT:
          loop = false;
          break;
      }
    }
    
    // Key checking loop
    keystate = SDL_GetKeyState(NULL);
    modstate = SDL_GetModState();
    
    wpnAtFrameStart = PLAYER_OBJECT->CurrWeapon();
    
    if (!(paused || died || transitioning))
    {
      mousePollTimer += timer.dT();
      if (mousePollTimer >= 0.05)
      {
        mouseButtons = SDL_GetRelativeMouseState(&mouse_dX, &mouse_dY); mousePollTimer = 0;
      }
      
      P.update();
      
      if (keystate[SDLK_ESCAPE])
      {
        if (!(escPressed || PLAYER_OBJECT->Dying()))
        { 
          paused = true;
          if (config.mgrab)
            SDL_WM_GrabInput(SDL_GRAB_OFF);
          pauseMenu.activate(keystate);
          escPressed = true;
        }
      }
      else
        escPressed = false;
        
      if (config.cheat_skip && keystate[SDLK_F6])
        setTrigger(-1);
        
      SCREENSHOT_CHECK
                
      /*** Unmodifiable keybindings ***/
      // Select a weapon
      for (int i = 0; i < NUM_PLAYER_WEAPONS; i++)
        if (keystate[SDLK_1+i])
          PLAYER_OBJECT->selectWeapon(i);
          
      if (config.cheat_zoom)
      {
        if (keystate[SDLK_h])
          cam.change(0.0, 0.0, -((modstate & KMOD_LSHIFT) ? 500.0 : 50.0)*timer.dT(), 0.0);
        else if (keystate[SDLK_n])
          cam.change(0.0, 0.0, ((modstate & KMOD_LSHIFT) ? 500.0 : 50.0)*timer.dT(), 0.0);
        else if (keystate[SDLK_j])
          cam.setHeight(CAMERA_DEFAULT_HEIGHT);
      }
      
      if (keystate[SDLK_F2])
      {
        float foo1 = 0.0, foo2 = 0.0;
        configScreen(true, foo1, foo2);
        currLevel.setBackgroundColor();
        escPressed = keystate[SDLK_ESCAPE];
      }
        
      // Game logic
      healthAtFrameStart = PLAYER_OBJECT->Health();
      
      for (int i = 0; i <= currLevel.maxTrigger; i++)
        triggers[i].hit = false;
        
      if (systemExplode)
      {
        systemExplodeTimer += timer.dT();
        switch(systemExplode)
        {
          case 1:
            if (systemExplodeTimer > 0.0)
            { setTrigger(50); systemExplode++; setMessage("!!! CATASTROPHIC SYSTEM FAILURE !!!"); }
            break;
          case 2:
            if (systemExplodeTimer > 1.0)
            { setTrigger(51); systemExplode++; setMessage("!!! CATASTROPHIC SYSTEM FAILURE !!!"); }
            break;
          case 3:
            if (systemExplodeTimer > 2.0)
            { setTrigger(52); systemExplode++; setMessage("!!! CA&A&TR34#$ *** sDFd#$!@#34 !?#"); }
            break;
          case 4:
            if (systemExplodeTimer > 2.5)
            { setTrigger(53); systemExplode++; setMessage("!!@ #cAtE4%34#$ SYET#$3#$!SFIEA!!@"); }
            break;
          case 5:
            if (systemExplodeTimer > 3.0)
            { setTrigger(54); systemExplode++; setMessage("!!@ #cAt!@!~@4#$!@(%H*333333364A!"); }
            break;
          case 6:
            if (systemExplodeTimer > 3.5)
            { setTrigger(55); systemExplode++; setMessage("** (* ()*D()*io SF*(ioj45o25364A!"); }
            break;
          case 7:
            if (systemExplodeTimer > 4.0)
            { setTrigger(56); systemExplode++; setMessage("** F4AW T* YU6WU858I6EZR G)*(a89*"); }
            break;
          case 8:
            if (systemExplodeTimer > 4.5)
            { setTrigger(57); systemExplode++; setMessage("*********************************"); }
            break;
          case 9:
            if (systemExplodeTimer > 5.0)
            { setTrigger(-1); }
            break;            
        }
      }
        
        
      // Reset collision flags
      for (int k = 0; k < currLevelObjs.size(); k++)
      {
        currLevelObjs[k]->resetCollided();
        currLevelObjs[k]->setObjectCollisionDetectOnOff(distSquared(currLevelObjs[k]->Pos(), PLAYER_OBJECT->Pos()) < OBJECT_CD_THRESHOLD);
      }
      
      for (int i = 0; i < currLevelObjs.size(); i++)
        currLevelObjs[i]->update();
  
      // Update objects
      for (int i = 0; i < currLevelEnemies.size(); i++)
        currLevelEnemies[i]->update();
            
      currLevel.updateDynamicGeometry();
      
      // Reveal masks
      for (vector<Surface>::iterator i = currLevel.mask.begin(); i != currLevel.mask.end(); i++)
      {
        if (config.cheat_nomasks)
          i->startFade(1);
        else
        {
          if (i->pointInBB(PLAYER_OBJECT->Pos()) && i->pointInPolygon(PLAYER_OBJECT->Pos()))
            i->startFade(1);
          else if (i->Reobscure())
            i->startFade(-1);
        }
      }
              
      for (int i = 0; i < liveBullets; i++)
      for (int j = 0; j < currLevelObjs.size(); j++)
      {
        if (currLevelObjs[j]->isActive() && !IS_POWERUP(currLevelObjs[j]->Type()))
        {
          if ((bullets[i].Owner() == j && bullets[i].Live()) || bullets[i].Owner() != j)
          {
            Vector2D intersect;
            if (circleSegmentIntersection(currLevelObjs[j]->Pos(),
              objType[currLevelObjs[j]->Type()].boundRadius,
              bullets[i].Pos(),
              bullets[i].Pos() + (bullets[i].Vel() * timer.dT()), &intersect))
            {
              currLevelObjs[j]->setDirectionAlert(atan2(bullets[i].PosY() - currLevelObjs[j]->PosY(),
                bullets[i].PosX() - currLevelObjs[j]->PosX()), ALERT_SHOT);
            
              if (bullets[i].Type() == BLT_GRENADE)
              {
                Vector2D diff = currLevelObjs[j]->Pos() - bullets[i].Pos();
                diff.normalize();
                Vector2D tmpVel = bullets[i].Vel();
                tmpVel -= diff * (diff * bullets[i].Vel()) * 2;
                bullets[i].setHeading(atan2(tmpVel.c[Y], tmpVel.c[X]));
              }
              else
              {
                LAUNCH_MULTI_PARTICLES(5, PART_PARTICLE, intersect.c[X],intersect.c[Y],0,torad(rand()%360),1.0,0);
                currLevelObjs[j]->changeForceVel(Vector2D(bltType[bullets[i].BType()].impactForce*cos(bullets[i].Heading()),
                bltType[bullets[i].BType()].impactForce*sin(bullets[i].Heading())));
                
                currLevelObjs[j]->damage(-bltType[bullets[i].BType()].damage, DAMAGE_BULLET);
                
                killBullet(i);
                i--;
              }
            }
          }
        }  
      }
            
      // Update transient stuff
      for (int i = 0; i < liveBullets; i++)
      {
        bullets[i].update();
        if (bullets[i].toBeDestroyed())
        { killBullet(i); i--; }
      }
      for (int i = 0; i < liveLasers; i++)
      {
        lasers[i].update();
        if (lasers[i].toBeDestroyed())
        { killLaser(i); i--; }
      }
  
      for (int i = 0; i < liveForces; i++)
      {
        forces[i].apply();
        if (forces[i].toBeDestroyed())
        { killForce(i); i--; }
      }  
  
      for (int i = 0; i < liveShockwaves; i++)
      {
        shockwaves[i].update();
        if (shockwaves[i].toBeDestroyed())
        { killShockwave(i); i--; }
      }
      
      // Update effects
              
      if (messageTime > 0)
        messageTime -= timer.dT();
      else if (messageTime < 0)
        messageTime = 0;
        
      if (!PLAYER_OBJECT->isActive())
      {
        died = true;
        
        if (captured)
          deadMenu.addString(1, "Captured!");
        else
          deadMenu.addString(1, "You have died.");
        
        deadMenu.addString(0, "");
        deadMenu.addString(0, "  restart level"); deadMenu.addString(0, "  return to menu");  
        deadMenu.addString(0, "");                deadMenu.addString(0, "(sdb)");         
        deadMenu.activate();
        
      }
      
      P.gameTime += timer.dT();
    }
    else if (paused)
    {
      setBlur(1.0, 0.0, 1.0, 0.0);
      switch(pauseMenu.update(keystate))
      {
        case 2: // abort
          PLAYER_OBJECT->setHealth(0);
          aborted = true;
        case 3:
          paused = false;
          pauseMenu.deactivate();
          setBlur(0.0, 0.0, 0.0, 0.0);
          if (config.mgrab)
            SDL_WM_GrabInput(SDL_GRAB_ON);
          break;
      }
    }
    else if (died)
    {
      if (aborted)
      { returnVal = LEVEL_QUIT; loop = false; }
      else
      {
        setBlur(1.0, 1.0, 0.0, 0.0);
        switch(deadMenu.update(keystate))
        {
          case 2: // restart
            returnVal = LEVEL_LOSE;
            loop = false;
            setBlur(0.0, 0.0, 0.0, 0.0);
            break;
          case 3:
            returnVal = LEVEL_QUIT;
            loop = false;
            aborted = true;
            setBlur(0.0, 0.0, 0.0, 0.0);
            break;
        }
      }    
    }
    else if (transitioning)
    {
      transitionTimer += timer.dT();
      
      if (transitionTimer > 1.0 && !transitionJump)
      {
        LAUNCH_MULTI_PARTICLES((int)(2*(transitionTimer-1.0)), PART_PARTICLE_SPARK,
        PLAYER_OBJECT->PosX(), PLAYER_OBJECT->PosY(),0.5,torad(rand()%360),1.0,0);
      }
      
      if (transitionTimer > 2.0 && !transitionJump)
      {
        LAUNCH_MULTI_PARTICLES((int)(0.5*(transitionTimer-3.0)), PART_LASER_SPARK2,
        PLAYER_OBJECT->PosX(), PLAYER_OBJECT->PosY(),0.5,torad(rand()%360),1.0,0);      
      }
      
      if (transitionTimer > 10.0)
      {
        cam.change(0.0, 0.0, (5000.0-MIN((transitionTimer-10)*2000, 4950))*timer.dT(), 0.0);
       
        if (!transitionJump)
        {
          for (int i = 0; i < liveParticles; i++)
            killParticle(i);
            
          LAUNCH_MULTI_PARTICLES(NUM_PARTICLES, PART_BIT,
          PLAYER_OBJECT->PosX()+(frand()*50-25), PLAYER_OBJECT->PosY()+(frand()*50-25), frand()*8000,0.0,1.0,0);
          transitionJump = true;
          playSound(SND_LAUNCH2, 3);
          
          setScreenQuake(0);
        }
        
        if (cam.Height() >= 6850)
        {
          fadeInMusic(MUS_DD2, 2000);
          setTrigger(-1);       
        }
      }
      
      
      
      if (!transitionJump)
      {
        setBlur(1.0, 1.0, 1.0, 1.0);
        setScreenQuake(transitionTimer*0.5);
      }
      else
        setBlur(2.0, 0.0, 1.0, 0.0);
    }
    
    if (screenQuake > 0)
    {
      screenQuake -= SCREEN_QUAKE_DECAY * timer.dT();
      if (screenQuake <= 0) screenQuake = 0;
    }

    if (!(paused || died))
    {
      for (int i = 0; i < liveParticles; i++)
      {
        particles[i].update();
        if (particles[i].toBeDestroyed())
        { killParticle(i); i--; }
      }
    
      if (screenBlur > 0)
      {
        if (PLAYER_OBJECT->Health() > 0)
          screenBlur -= SCREEN_BLUR_DECAY * timer.dT();
          
        if (screenBlur <= 0) screenBlur = 0;
        blurUpdateTimer += timer.dT();
      }
    }
        
    if (PLAYER_OBJECT->Health() > 0)
    {
      blinker += timer.dT();
      if (blinker > 0.6)
        blinker = 0.0;
    }
    else
      blinker = 0.0;
     
    if (weaponDisplayTimer > 0)
    {
      weaponDisplayTimer -= timer.dT();
      if (weaponDisplayTimer <= 0.0)
        weaponDisplayTimer = 0.0;
    }
        
    // Draw scene
    // If we're going to blur, we make two passes over the scene;
    // first to render to the texture, then to render to the framebuffer
    for (int m = ((blurUpdateTimer < BLUR_DELAY && screenBlur && config.blur > 0) ? 1 : 0); m < 2; m++)
    {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glLoadIdentity();
      
      if (m == 0)
      {
        glViewport(0, 0, BLUR_TEXTURE_WIDTH, BLUR_TEXTURE_HEIGHT);
        blurUpdateTimer = 0;
      }
      
      // Do camera
      if (!config.cheat_zoom && transitionTimer < 10.0)
        cam.setHeight(P.cameraZoom());
      
      if (P.HeadingLock() && !transitioning)
      {
        cam.setRotation(-todeg(PLAYER_OBJECT->LookAngle())+90);
        cam.setPos(PLAYER_OBJECT->PosX()+(cos(PLAYER_OBJECT->LookAngle())*15),
                PLAYER_OBJECT->PosY()+(sin(PLAYER_OBJECT->LookAngle())*15));
      }
      else
      {
        cam.setRotation(0);
        cam.setPos(PLAYER_OBJECT->PosX(), PLAYER_OBJECT->PosY());
      }
      if (screenQuake)
        cam.change(frand()*screenQuake*2-screenQuake,frand()*screenQuake*2-screenQuake, 0, 0);
        
      cam.apply();
      
      viewRegion.reset();
      // fudged
      viewRegion.addPoint(cam.Pos()-Vector2D(cam.Height()*SCREEN_ASPECT, cam.Height()));
      viewRegion.addPoint(cam.Pos()+Vector2D(cam.Height()*SCREEN_ASPECT, cam.Height()));
      
      // Draw foreground. Make two passes (first normally, then second
      // with the depth buffer read-only) to ensure both proper depth-occlusion results
      // and proper blending.
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_EQUAL, 1.0);
      drawScene(viewRegion);
      glAlphaFunc(GL_NOTEQUAL, 1.0);
      glDepthMask(GL_FALSE);
      drawScene(viewRegion);
      glDepthMask(GL_TRUE);
      glDisable(GL_ALPHA_TEST); 
                  
      // Blur
      if (m == 0)
      {
        glEnable(GL_TEXTURE_2D);  
        glBindTexture(GL_TEXTURE_2D, blurTextures[blurQueueRear]);
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, BLUR_TEXTURE_WIDTH, BLUR_TEXTURE_HEIGHT, 0);
        // reset the viewport
        glViewport(0, 0, config.xres, config.yres);
       
        // cycle the queue
        blurQueueRear++;
        if (blurQueueRear == NUM_BLUR_TEXTURES)
        {
          blurQueueFront++;
          blurQueueRear = 0;
        }
        
        if (blurQueueRear == blurQueueFront)
          blurQueueFront++;
          
        if (blurQueueFront == NUM_BLUR_TEXTURES)
          blurQueueFront = 0;
      }
    }
    // Draw overlay
    
    gameSetOrthoMode(100);
    glDisable(GL_DEPTH_TEST);
    
    if (!transitionJump)
    {
      if (config.fps)
      {
        glColor3f(0.0, 1.0, 0.75);
        font.printf(CENTERED, SCREEN_TOP-2.0, 2.0, 1.6, "%03d FPS", timer.FPS());
      }
      
      if (config.cheating())
      {
        glColor3f(1.0, 0.0, 0.0);
        font.printf(-98.0, SCREEN_TOP-2.0, 3.0, 2.0, "* SYSTEM BACKDOOR ACTIVE *");
      }
      
      if (config.info)
        objectStats(0);
    }
      
    glEnable(GL_TEXTURE_2D);
    
    // Apply the blur effect
    if (screenBlur > 0 && config.blur)
    {
      screenBlurColor.a = 1.0/NUM_BLUR_TEXTURES;
      screenBlurColor.a *= MIN(1.0, screenBlur);
      screenBlurColor.apply();
      
      int blurIterator = blurQueueFront;
      for (int blur = 0; blur < NUM_BLUR_TEXTURES; blur++)
      {
        glBindTexture(GL_TEXTURE_2D, blurTextures[blurIterator]);
        
        if (PLAYER_OBJECT->Health() > 0)
          glBlendFunc(GL_SRC_ALPHA, GL_ONE);
          
        glBegin(GL_QUADS);
          glTexCoord2f(0.0, 1.0);
          glVertex2f(-100, 100/SCREEN_ASPECT);
          glTexCoord2f(1.0, 1.0);
          glVertex2f( 100, 100/SCREEN_ASPECT);
          glTexCoord2f(1.0, 0.0);
          glVertex2f( 100,-100/SCREEN_ASPECT);
          glTexCoord2f(0.0, 0.0);
          glVertex2f(-100,-100/SCREEN_ASPECT);
        glEnd();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        blurIterator++;
        if (blurIterator == NUM_BLUR_TEXTURES)
          blurIterator = 0;
      }
    }
    

    if (!transitionJump)
    {   
      // Health LED display
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      (PLAYER_OBJECT->Health() >= 26) ? glColor4f(0.2, 0.2, 1.0, 1.0) : glColor4f(1.0, 0.2, 0.2, 1.0);
      font.LEDprintf(-95.0, -SCREEN_TOP+6, 6.0, 6.0, "===");
      (PLAYER_OBJECT->Health() >= 26) ? glColor4f(0.3, 0.3, 1.0, 1.0) : glColor4f(1.0, 0.0, 0.0, (float)(blinker <= 0.3));
      if (PLAYER_OBJECT->Health() > 0.0)
        font.LEDprintf(-95.0, -SCREEN_TOP+6, 6.0, 6.0, "%3.f", ceil(PLAYER_OBJECT->Health()));
      else
        font.LEDprintf(-95.0, -SCREEN_TOP+6, 6.0, 6.0, "<<<");
        
      if (PLAYER_OBJECT->Augmented())
      {
        glColor4f(1.0, 1.0, 0.0, 1.0);
        if (PLAYER_OBJECT->Health() > 0.0)
          font.LEDprintf(-95.0, -SCREEN_TOP+6, 6.0, 6.0, "%3.f", ceil(PLAYER_OBJECT->Health()));
        else
          font.LEDprintf(-95.0, -SCREEN_TOP+6, 6.0, 6.0, "<<<");
          
        setBlur(0.5, 0.0, 1.0, 0.0);
      }
  
      // Weapons LED display   
      glColor4f(0.2, 0.2, 1.0, 1.0);
      
      font.LEDprintf(64, -SCREEN_TOP+6, 6.0, 6.0, "======");
      if (PLAYER_OBJECT->Health() > 0 && PLAYER_OBJECT->Wpn()->Type() != WPN_NONE)
      {
        font.printf(64, -SCREEN_TOP+14.0, 2.8, 2.4, "%16s", wpnType[PLAYER_OBJECT->Wpn()->Type()].name);
        
        if (PLAYER_OBJECT->Wpn()->Type() != WPN_BLASTER)
        {
          if (!PLAYER_OBJECT->Wpn()->isReloading())
          {
            font.LEDprintf(64, -SCREEN_TOP+6, 6.0, 6.0, "%2d%s%3d",
            PLAYER_OBJECT->Wpn()->AmmoInClip(), (PLAYER_OBJECT->Wpn()->Ready()) ? ":" : " ", PLAYER_OBJECT->Wpn()->ReserveAmmo());
            
            if (PLAYER_OBJECT->Wpn()->Type() == WPN_LASER)
            {
              glColor4f(1.0, 1.0, 1.0, (PLAYER_OBJECT->Wpn()->getLaserCharge() < 1.0)?PLAYER_OBJECT->Wpn()->getLaserCharge():(float)(blinker <= 0.3));
              font.LEDprintf(64, -SCREEN_TOP+6, 6.0, 6.0, "%2d%s%3d",
              PLAYER_OBJECT->Wpn()->AmmoInClip(), (PLAYER_OBJECT->Wpn()->Ready()) ? ":" : " ", PLAYER_OBJECT->Wpn()->ReserveAmmo());
            }
          }
          else
          {
            glColor4f(0.2, 0.2, 1.0, (float)(blinker <= 0.3));
            font.LEDprintf(64, -SCREEN_TOP+6, 6.0, 6.0, "<< <<<");
          }
        }
      }
      
      // Obtained weapons display
      if (wpnAtFrameStart != PLAYER_OBJECT->CurrWeapon() && PLAYER_OBJECT->CurrWeapon() > 0)
        weaponDisplayTimer = 2.0;
      
      glColor4f(0.2, 0.2, 1.0, MIN(weaponDisplayTimer, 1.0));
      font.LEDprintf(50, -SCREEN_TOP+14.0, 2.0, 2.0, "======");
      font.LEDprintf(50, -SCREEN_TOP+14.0, 2.0, 2.0, "%c%c%c%c%c%c",PLAYER_OBJECT->weaponState(1),
      PLAYER_OBJECT->weaponState(2),PLAYER_OBJECT->weaponState(3),PLAYER_OBJECT->weaponState(4),
      PLAYER_OBJECT->weaponState(5),PLAYER_OBJECT->weaponState(6)); 
      glColor4f(0.5, 0.5, 1.0, MIN(weaponDisplayTimer, 1.0));
      if (PLAYER_OBJECT->CurrWeapon() > 0)  
        font.LEDprintf(50+2.0*(PLAYER_OBJECT->CurrWeapon()-1), -SCREEN_TOP+14.0, 2.0, 2.0, "%d", PLAYER_OBJECT->CurrWeapon()+1);
      
      // Keys display
      glColor4f(0.4, 0.0, 1.0, 0.7);
      font.LEDprintf(-77.1, -SCREEN_TOP+3.5, 4.5, 4.5, ">");
      font.LEDprintf(-77.1, -SCREEN_TOP+3.5, 4.5, 4.5, "%c", PLAYER_OBJECT->keyState(3));
      glColor4f(0.6, 1.0, 0.0, 0.7);
      font.LEDprintf(-77, -SCREEN_TOP+6, 4.5, 4.5, ">");
      font.LEDprintf(-77, -SCREEN_TOP+6, 4.5, 4.5, "%c", PLAYER_OBJECT->keyState(2));
      glColor4f(1.0, 0.0, 0.0, 0.7);
      font.LEDprintf(-76.9, -SCREEN_TOP+8.5, 4.5, 4.5, ">");
      font.LEDprintf(-76.9, -SCREEN_TOP+8.5, 4.5, 4.5, "%c", PLAYER_OBJECT->keyState(1));
        
      // Timer display
      double foo;
      glColor4f(0.2, 0.2, 1.0, 1.0);
      font.LEDprintf(CENTERED, -SCREEN_TOP+6, 3.0, 3.0, "==:==;==");
      font.LEDprintf(CENTERED, -SCREEN_TOP+6, 3.0, 3.0, "%2d:%02d;%02d", (int)(P.gameTime/60),
      (int)fmod(P.gameTime, 60), (int)(modf(P.gameTime, &foo)*100));
      
      
      // Message display
      if (message != "" && messageTime > 0)
      {
        glColor4f(0.2, 0.2, 1.0, messageTime/MESSAGE_DELAY_TIME);
        font.printf(-95.0, -SCREEN_TOP+14.0, 3.7, 3.2, "%s", message);
      }
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
          
  
      // Draw damage/healing effects
      if (healthAtFrameStart != PLAYER_OBJECT->Health())
      {
        healthChange = PLAYER_OBJECT->Health() - healthAtFrameStart;
        if (damageEffectTimer < 0.5)
        {
          damageEffectTimer = 1.0;
          if (healthChange < 0 && healthChange >= -5)
            damageEffectIntensity = 10;
          else if (healthChange < -5 && healthChange >= -15)
            damageEffectIntensity = 8;
          else if (healthChange < -15 && healthChange >= -30)
            damageEffectIntensity = 6;
          else if (healthChange < -30 && healthChange >= -60)
            damageEffectIntensity = 4;
          else if (healthChange < -30 && healthChange >= -60)
            damageEffectIntensity = 2;
          else if (healthChange < -60)
            damageEffectIntensity = 1;        
          
          damageEffectIntensity *= PLAYER_OBJECT->Health()/100.0;
            
          if (healthChange > 0)
          damageEffectIntensity = 8;
        }
        
        if (PLAYER_OBJECT->Health() == 0 || PLAYER_OBJECT->Dying())
        { damageEffectTimer = 0.0; damageEffectIntensity = 0.0; }
        
        if (damageEffectIntensity < 0.25)
          damageEffectIntensity = 0.25;
      }
      
      if (PLAYER_OBJECT->Health()<= 0)
        healthChange = -100;
      
      if (damageEffectTimer > 0.0)
      {
        (healthChange < 0) ? glColor4f(1.0, 0.0, 0.0, damageEffectTimer) : glColor4f(0.04, 0.3, 1.0, damageEffectTimer);
        
        for (float i = 0; i < SCREEN_TOP; i += damageEffectIntensity)
        {
          glBegin(GL_LINES);
            glVertex2f(-100, i);
            glVertex2f(100, i);
            glVertex2f(-100, -i);
            glVertex2f(100, -i);
  
          glEnd();
        }
        damageEffectTimer -= timer.dT();
      }
    }
    
    
    if (transitionTimer > 0.0 && transitionTimer < 10.0)
    {
      glColor4f(0.0, 1.0, 0.0, 1.0);
      font.printf(CENTERED, 10, 5.4, 4.4, "NANO-ACTIVATION SEQUENCE COMMENCING IN");
      
      if (transitionTimer > 1.0)
        font.printf(CENTERED, 0, 7.4, 6.4, "%d", (int)(11-transitionTimer));
    }
    
    
    if (paused) pauseMenu.draw();
    if (died)   deadMenu.draw();
      
    // Return to perspective projection
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    SDL_GL_SwapBuffers();
  
    timer.update();
    
    if (config.framecap >= 15)
    {
      while (timer.dT() < 1.0/config.framecap)
        timer.update();
    }
  }
  
  if (aborted)
    returnVal = LEVEL_QUIT;
  else
  {
    if (PLAYER_OBJECT->Health() <= 0)
      returnVal = LEVEL_LOSE;
    else if (winLevel)
    {
      levelWinScreen(playInSequence);
      returnVal = LEVEL_WIN;
    }
  }
    
  // deallocate objects
  destroyAI();
  currLevelObjs.clear();
  currLevelEnemies.clear();
  return returnVal;
}

void drawScene(BoundingBox viewRegion)
{
  if (config.culling)
  {
    int cMinX,cMinY,cMaxX,cMaxY, oCX, oCY;
    currLevel.cellNumber(viewRegion.min, &cMinX, &cMinY);
    currLevel.cellNumber(viewRegion.max, &cMaxX, &cMaxY);
    
    for (int y = cMinY; y <= cMaxY; y++)
      for (int x = cMinX; x <= cMaxX; x++)
        if (x >= 0 && x < currLevel.gridCols && y >= 0 && y < currLevel.gridRows)
          currLevel.drawCell(y*currLevel.gridCols+x);
    
    drawObjectShadows(cMinX, cMinY, cMaxX, cMaxY);
    for (int i = 0; i < currLevelObjs.size(); i++)
    {
      currLevel.cellNumber(currLevelObjs[i]->Pos(), &oCX, &oCY);
      if (oCX >= cMinX && oCX <= cMaxX && oCY >= cMinY && oCY <= cMaxY)
        currLevelObjs[i]->draw();
    }
    
    for (int i = 0; i < liveParticles; i++)
    {
      currLevel.cellNumber(particles[i].Pos(), &oCX, &oCY);
      if (oCX >= cMinX && oCX <= cMaxX && oCY >= cMinY && oCY <= cMaxY)
        particles[i].draw();
    }
    for (int i = 0; i < liveBullets; i++)
    {
      currLevel.cellNumber(bullets[i].Pos(), &oCX, &oCY);
      if (oCX >= cMinX && oCX <= cMaxX && oCY >= cMinY && oCY <= cMaxY)
        bullets[i].draw();
    }
    
    for (int y = cMinY; y <= cMaxY; y++)
      for (int x = cMinX; x <= cMaxX; x++)
        if (x >= 0 && x < currLevel.gridCols && y >= 0 && y < currLevel.gridRows)
          currLevel.resetCell(y*currLevel.gridCols+x);
  }
  else
  {
    currLevel.drawStaticGeometryDL();
    currLevel.drawDynamicGeometry();
    
    drawObjectShadows(0, 0, 0, 0);
    for (int i = 0; i < currLevelObjs.size(); i++)
    {
      if (config.cheat_zoom || distSquared(currLevelObjs[i]->Pos(), PLAYER_OBJECT->Pos()) < OBJECT_DRAW_THRESHOLD)
        currLevelObjs[i]->draw();
    }
            
    for (int i = 0; i < liveParticles; i++)
      particles[i].draw();
    
    for (int i = 0; i < liveBullets; i++)
      bullets[i].draw();    
    
    for (int i = 0; i < liveLasers; i++)
      lasers[i].draw();    

  }
  
  if (config.paths)
    drawPaths();
  
  /*for (int i = 0; i < liveShockwaves; i++)
    shockwaves[i].draw();*/
}

void drawObjectShadows(int cMinX, int cMinY, int cMaxX, int cMaxY)
{
  if (config.culling)
  {
    int oCX, oCY;
    for (int i = 0; i < currLevelObjs.size(); i++)
    {
      currLevel.cellNumber(currLevelObjs[i]->Pos(), &oCX, &oCY);
      if (oCX >= cMinX && oCX <= cMaxX && oCY >= cMinY && oCY <= cMaxY)
        currLevelObjs[i]->drawShadow();
    }
    for (int i = 0; i < liveParticles; i++)
    {
      currLevel.cellNumber(particles[i].Pos(), &oCX, &oCY);
      if (oCX >= cMinX && oCX <= cMaxX && oCY >= cMinY && oCY <= cMaxY)
        particles[i].drawShadow();
    }
    for (int i = 0; i < liveBullets; i++)
    {
      currLevel.cellNumber(bullets[i].Pos(), &oCX, &oCY);
      if (oCX >= cMinX && oCX <= cMaxX && oCY >= cMinY && oCY <= cMaxY)
        bullets[i].drawShadow();
    }
  }
  else
  {
    for (int i = 0; i < currLevelObjs.size(); i++)
      currLevelObjs[i]->drawShadow();
    
    for (int i = 0; i < liveParticles; i++)
      particles[i].drawShadow();
    
    for (int i = 0; i < liveBullets; i++)
      bullets[i].drawShadow();
  }
}

double Timer::timerFunc()
{
  #ifdef _WIN32
  LARGE_INTEGER count, freq;
  if (QueryPerformanceCounter(&count))
  {
    QueryPerformanceFrequency(&freq);
    return (double)count.QuadPart/freq.QuadPart;
  }
  else
    return SDL_GetTicks() / 1000.0;  
  #else
  struct timeval tm;
  struct timezone tz;
  gettimeofday(&tm, &tz);
  
  // Convert a timeval to a double
  return (double)(tm.tv_sec) + ((double)(tm.tv_usec) * 0.000001);
  #endif
}  

void Timer::update()
{
  dt = timerFunc() - start;
  fpsCount++;
  fpsCache += dt;
  
  if (fpsCache >= 1.0)
  { fps = fpsCount; fpsCount = 0; fpsCache = 0.0; }
}  

void CALLBACK tcbBegin(GLenum prim)
{ glBegin(prim); }

void CALLBACK tcbEnd() { glEnd(); }

void CALLBACK tcbCombine (GLdouble c[3], void *d[4], GLfloat w[4], void **out)
{
  GLdouble *nv = (GLdouble *) malloc(3*sizeof(GLdouble));
  tessAllocatedVertices.push_back(nv);

  nv[0] = c[0];
  nv[1] = c[1];
  nv[2] = c[2];
  *out = nv; 
}


void CALLBACK tcbError(GLenum errnum)
{
  errorMessage(6, "TESSELLATION ERROR", "CODE %d: %s", errnum, gluErrorString(errnum));
}
