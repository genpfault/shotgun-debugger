cmake_minimum_required( VERSION 2.8.7 )
project( ShotgunDebugger )

file( GLOB glob_SRC "include/*.h" "src/*.cpp" )
include_directories( "include" )
add_executable( shotgun-debugger ${glob_SRC} )

# point PKGDATADIR at the right place
add_definitions( "-DPKGDATADIR=\"${CMAKE_INSTALL_PREFIX}/\"" )

install( TARGETS shotgun-debugger DESTINATION . )
install( DIRECTORY res/ DESTINATION . )


if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    # TODO: remove after switching to GLM & a 3rd-party triangulation library
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations" )
endif()

# TODO:
# flags: -O2 -Wall -ansi
# http://stackoverflow.com/questions/23995019/what-is-the-modern-method-for-setting-general-compile-flags-in-cmake

find_package( SDL REQUIRED )
include_directories( ${SDL_INCLUDE_DIR} )
target_link_libraries( shotgun-debugger ${SDL_LIBRARY} )

find_package( SDL_image REQUIRED )
include_directories( ${SDLIMAGE_INCLUDE_DIR} )
target_link_libraries( shotgun-debugger ${SDLIMAGE_LIBRARY} )

find_package( SDL_mixer REQUIRED )
include_directories( ${SDLMIXER_INCLUDE_DIR} )
target_link_libraries( shotgun-debugger ${SDLMIXER_LIBRARY} )

find_package( OpenGL REQUIRED )
include_directories( ${OPENGL_INCLUDE_DIR} )
target_link_libraries( shotgun-debugger ${OPENGL_LIBRARIES} )
