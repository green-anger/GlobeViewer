set( lib globe_viewer )

set( Boost_USE_STATIC_LIBS ON )
set( Boost_USE_MULTITHREADED ON )
set( Boost_USE_STATIC_RUNTIME OFF )
add_definitions( -DBOOST_ALL_NO_LIB )

find_package( Boost 1.67.0 REQUIRED COMPONENTS filesystem system )
find_package( OpenGL REQUIRED )
find_package( glm REQUIRED )
find_package( PROJ4 5.0.0 REQUIRED )

set( HEADERS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/include" )
set( HEADERS_IMPL "${HEADERS_ROOT}/impl" )
set( HEADERS_SUPP "${HEADERS_ROOT}/support" )
set( HEADERS_GLAD "${HEADERS_SUPP}/glad" )
set( HEADERS_TYPE "${HEADERS_ROOT}/type" )

set( SOURCES_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src" )
set( SOURCES_SUPP "${SOURCES_ROOT}/support" )

set( HDRS
    ${HEADERS_ROOT}/GlobeViewer.h
    ${HEADERS_IMPL}/DataKeeper.h
    ${HEADERS_IMPL}/Projector.h
    ${HEADERS_IMPL}/Renderer.h
    ${HEADERS_IMPL}/TileManager.h
    ${HEADERS_IMPL}/Viewport.h
    ${HEADERS_SUPP}/FpsCounter.h
    ${HEADERS_SUPP}/LoadGL.h
    ${HEADERS_SUPP}/Profiler.h
    ${HEADERS_SUPP}/Shader.h
    ${HEADERS_SUPP}/ThreadSafePrinter.hpp
    ${HEADERS_TYPE}/Tile.h
    ${HEADERS_TYPE}/TileMap.h
)

set( SRCS
    ${SOURCES_ROOT}/GlobeViewer.cpp
    ${SOURCES_ROOT}/DataKeeper.cpp
    ${SOURCES_ROOT}/Projector.cpp
    ${SOURCES_ROOT}/Renderer.cpp
    ${SOURCES_ROOT}/TileManager.cpp
    ${SOURCES_ROOT}/Viewport.cpp
    ${SOURCES_SUPP}/glad/glad.c
    ${SOURCES_SUPP}/FpsCounter.cpp
    ${SOURCES_SUPP}/Profiler.cpp
    ${SOURCES_SUPP}/Shader.cpp
)

include_directories(
    ${HEADERS_ROOT}
    ${HEADERS_IMPL}
    ${HEADERS_SUPP}
    ${HEADERS_GLAD}
    ${Boost_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${PROJ4_INCLUDE_DIRS}
)

add_library( ${lib} ${SRCS} ${HDRS} )

target_link_libraries( ${lib}
    ${Boost_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${PROJ4_LIBRARIES}
)

if ( MSVC )
    add_definitions( -DNOMINMAX )
endif()

target_include_directories( ${lib} INTERFACE
    ${HEADERS_ROOT}
)

set( DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data" PARENT_SCOPE )
set( SHADERS_DIR "${DATA_DIR}/shaders" PARENT_SCOPE )
