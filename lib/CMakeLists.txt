set( lib globe_viewer )

find_package( OpenGL REQUIRED )
find_package( glm REQUIRED )

set( HEADERS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/include" )
set( HEADERS_IMPL "${HEADERS_ROOT}/impl" )
set( HEADERS_SUPP "${HEADERS_ROOT}/support" )
set( HEADERS_GLAD "${HEADERS_SUPP}/glad" )

set( SOURCES_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src" )
set( SOURCES_SUPP "${SOURCES_ROOT}/support" )

set( HDRS
    ${HEADERS_ROOT}/GlobeViewer.h
    ${HEADERS_IMPL}/DataKeeper.h
    ${HEADERS_IMPL}/Renderer.h
    ${HEADERS_IMPL}/Viewport.h
    ${HEADERS_SUPP}/LoadGL.h
    ${HEADERS_SUPP}/Shader.h
)

set( SRCS
    ${SOURCES_ROOT}/GlobeViewer.cpp
    ${SOURCES_ROOT}/DataKeeper.cpp
    ${SOURCES_ROOT}/Renderer.cpp
    ${SOURCES_ROOT}/Viewport.cpp
    ${SOURCES_SUPP}/glad/glad.c
    ${SOURCES_SUPP}/Shader.cpp
)

include_directories(
    ${HEADERS_ROOT}
    ${HEADERS_IMPL}
    ${HEADERS_SUPP}
    ${HEADERS_GLAD}
    ${GLM_INCLUDE_DIRS}
)

add_library( ${lib} ${SRCS} ${HDRS} )

target_link_libraries( ${lib}
    ${OPENGL_LIBRARIES}
)

if ( MSVC )
    add_definitions( -DNOMINMAX )
endif()

target_include_directories( ${lib} INTERFACE
    ${HEADERS_ROOT}
)

set( DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data" PARENT_SCOPE )
set( SHADERS_DIR "${DATA_DIR}/shaders" PARENT_SCOPE )
