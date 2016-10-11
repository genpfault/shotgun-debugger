# "make" will build the executable
# "make install" will install the executable and game data in $(PREFIX)
# "make uninstall" will remove the executable and game data

# Change this if you want to install Shotgun Debugger somewhere else
# This should be an absolute path
PREFIX = /usr

CXX  = g++
INSTALL = install
SRC  = bitmapfont.cpp levelobjects.cpp level.cpp md2.cpp weapons.cpp \
       objects.cpp enemies.cpp input.cpp player.cpp game.cpp \
       interface.cpp main.cpp

LIBDIRS = -L/usr/local/lib -L/usr/X11R6/lib
LIBS = $(LIBDIRS) `sdl-config --libs` -lGL -lGLU -lSDL_image -lSDL_mixer
# Uncomment the following line for Mac OS X 10.5 and higher
#LIBS += -Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib

BIN  = shotgun-debugger
CFLAGS = `sdl-config --cflags` -I/opt/local/include -O2 -Wall -ansi
CXXFLAGS = $(CFLAGS)
PKGDATADIR = $(PREFIX)/share/$(BIN)
BINDIR = $(PREFIX)/bin

OBJ = $(SRC:.cpp=.o)

all: $(BIN)

.SUFFIXES: .cpp

.cpp.o:
	$(CXX) -DPKGDATADIR="\"$(PKGDATADIR)/\"" -c $< -o $@ $(CXXFLAGS)

$(BIN): $(OBJ)
	$(CXX) $(OBJ) -o $(BIN) $(LIBS)

clean:
	rm -f $(OBJ) $(BIN)
	
install:
	install -d $(BINDIR)
	install $(BIN) $(BINDIR)
	install -d $(PKGDATADIR)/levels
	install -m 644 levels/* $(PKGDATADIR)/levels
	install -d $(PKGDATADIR)/snd
	install -m 644 snd/* $(PKGDATADIR)/snd
	install -d $(PKGDATADIR)/models
	install -m 644 models/* $(PKGDATADIR)/models
	install -d $(PKGDATADIR)/sprites
	install -m 644 sprites/*.png $(PKGDATADIR)/sprites
	install -d $(PKGDATADIR)/sprites/skins
	install -m 644 sprites/skins/* $(PKGDATADIR)/sprites/skins

uninstall:
	rm -v $(BINDIR)/$(BIN)
	rm -rv $(PKGDATADIR)
