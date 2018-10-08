set( example example_glfw )

set( glfw3_include "" CACHE PATH "Path to GLFW3 include directory" )
set( glfw3_lib "" CACHE FILEPATH "Path to GLFW3 library" )

add_executable( ${example} main.cpp )

include_directories(
    ${glfw3_include}
)

target_link_libraries( ${example}
    globe_viewer
    ${glfw3_lib}
)

if ( WIN32 AND MSVC )
    set_target_properties( ${example} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE /ENTRY:\"mainCRTStartup\"" )
    set_target_properties( ${example} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:\"mainCRTStartup\"" )
endif()

file( COPY ${DATA_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )

install( TARGETS ${example} DESTINATION ${example} )
install( DIRECTORY ${DATA_DIR}/shaders DESTINATION ${example} )
