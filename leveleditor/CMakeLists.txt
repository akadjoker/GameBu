project(leveleditor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(LEVELEDITOR_SOURCES
    src/main.cpp
    src/SceneDocument.cpp
    src/Viewport.cpp
    # rlImGui backend (shared with gameeditor)
    ${CMAKE_SOURCE_DIR}/gameeditor/src/rlImGui.cpp
)

add_executable(leveleditor ${LEVELEDITOR_SOURCES})

target_include_directories(leveleditor PRIVATE
    src
    # Reuse gameeditor shared headers (rlImGui, FontAwesome, embedded font, property grid)
    ${CMAKE_SOURCE_DIR}/gameeditor/src
    ${CMAKE_SOURCE_DIR}/gameeditor/src/vendor
    # Engine
    ${CMAKE_SOURCE_DIR}/vendor/imgui
    ${CMAKE_SOURCE_DIR}/vendor/raylib/src
    ${CMAKE_SOURCE_DIR}/vendor/raylib/src/external
    ${CMAKE_SOURCE_DIR}/graphics/src
)

# Link: imgui + raylib + graphics library (Scene, Entity, GraphLib, ParticleSystem)
target_link_libraries(leveleditor imgui raylib graphics)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    if(UNIX)
        target_compile_options(leveleditor PRIVATE -fsanitize=address -fsanitize=undefined -fsanitize=leak -g -D_DEBUG)
        target_link_options(leveleditor PRIVATE -fsanitize=address -fsanitize=undefined -fsanitize=leak -g -D_DEBUG)
    endif()
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    target_compile_options(leveleditor PRIVATE -O3 -DNDEBUG)
endif()

if(WIN32)
    target_link_libraries(leveleditor Winmm.lib)
endif()

if(UNIX)
    target_link_libraries(leveleditor m)
endif()
