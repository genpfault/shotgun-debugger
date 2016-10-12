cmake_minimum_required( VERSION 3.0.2 )
project( ShotgunDebugger )

file( GLOB glob_SRC "include/*.h" "src/*.cpp" )
include_directories( "include" )
add_executable( shotgun-debugger ${glob_SRC} )

# point PKGDATADIR at the right place
add_definitions( -DPKGDATADIR="${CMAKE_INSTALL_PREFIX}/" )

install( TARGETS shotgun-debugger DESTINATION . )
install( DIRECTORY res/ DESTINATION . )

# TODO:
# flags: -O2 -Wall -ansi
# http://stackoverflow.com/questions/23995019/what-is-the-modern-method-for-setting-general-compile-flags-in-cmake

find_package( SDL REQUIRED )
if( SDL_FOUND )
  include_directories( ${SDL_INCLUDE_DIR} )
  target_link_libraries( shotgun-debugger ${SDL_LIBRARY} )
endif( SDL_FOUND )

find_package( SDL_image REQUIRED )
if( SDL_IMAGE_FOUND )
  include_directories( ${SDL_IMAGE_INCLUDE_DIRS} )
  target_link_libraries( shotgun-debugger ${SDL_IMAGE_LIBRARIES} )
endif( SDL_IMAGE_FOUND )

find_package( SDL_mixer REQUIRED )
if( SDL_MIXER_FOUND )
  include_directories( ${SDL_MIXER_INCLUDE_DIRS} )
  target_link_libraries( shotgun-debugger ${SDL_MIXER_LIBRARIES} )
endif( SDL_MIXER_FOUND )

find_package( OpenGL REQUIRED )
if( OPENGL_FOUND )
  include_directories( ${OPENGL_INCLUDE_DIR} )
  target_link_libraries( shotgun-debugger ${OPENGL_LIBRARIES} )
endif( OPENGL_FOUND )


