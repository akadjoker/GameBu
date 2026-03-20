project(gameeditor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(EDITOR_SOURCES
    src/main.cpp
    src/rlImGui.cpp
    src/TextEditor.cpp
    src/LanguageDefinitions.cpp
    src/DocumentSymbols.cpp
    src/GifRecorder.cpp
    src/ImGuiConsole.cpp
    src/ImGuiDebugPanel.cpp
    src/ImGuiFileDialog.cpp
    src/ImGuiPopup.cpp
    src/EmbeddedVM.cpp
)

# Vendored boost regex sources
file(GLOB BOOST_REGEX_SOURCES CONFIGURE_DEPENDS "src/vendor/regex/src/*.cpp")

add_executable(gameeditor ${EDITOR_SOURCES} ${BOOST_REGEX_SOURCES})

target_include_directories(gameeditor PRIVATE
    src
    src/vendor
    src/vendor/regex/include
    ${CMAKE_SOURCE_DIR}/vendor/imgui
    ${CMAKE_SOURCE_DIR}/vendor/raylib/src
    ${CMAKE_SOURCE_DIR}/vendor/raylib/src/external
    ${CMAKE_SOURCE_DIR}/libbu/include
    ${CMAKE_SOURCE_DIR}/graphics/src
    ${CMAKE_SOURCE_DIR}/main/src
    ${CMAKE_SOURCE_DIR}/vendor/box2d/include
    ${CMAKE_SOURCE_DIR}/vendor/poly2tri
)

# Link imgui, raylib, and the runtime library (VM + bindings + graphics)
target_link_libraries(gameeditor imgui raylib runtime)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    if(UNIX)
        target_compile_options(gameeditor PRIVATE -fsanitize=address -fsanitize=undefined -fsanitize=leak -g -D_DEBUG -DVERBOSE)
        target_link_options(gameeditor PRIVATE -fsanitize=address -fsanitize=undefined -fsanitize=leak -g -D_DEBUG)
    endif()
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    target_compile_options(gameeditor PRIVATE -O3 -DNDEBUG)
endif()

if(WIN32)
    target_link_libraries(gameeditor Winmm.lib)
endif()

if(UNIX)
    target_link_libraries(gameeditor m)
endif()
