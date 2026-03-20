project(runtime)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# All source files from main/src except main.cpp (the standalone entry point)
set(RUNTIME_SOURCES
    ${CMAKE_SOURCE_DIR}/main/src/bindings.cpp
    ${CMAKE_SOURCE_DIR}/main/src/box2d.cpp
    ${CMAKE_SOURCE_DIR}/main/src/box2d_joints.cpp
    ${CMAKE_SOURCE_DIR}/main/src/camera.cpp
    ${CMAKE_SOURCE_DIR}/main/src/draw.cpp
    ${CMAKE_SOURCE_DIR}/main/src/ease.cpp
    ${CMAKE_SOURCE_DIR}/main/src/image.cpp
    ${CMAKE_SOURCE_DIR}/main/src/input.cpp
    ${CMAKE_SOURCE_DIR}/main/src/message.cpp
    ${CMAKE_SOURCE_DIR}/main/src/particles.cpp
    ${CMAKE_SOURCE_DIR}/main/src/poly2tri_binding.cpp
    ${CMAKE_SOURCE_DIR}/main/src/process.cpp
    ${CMAKE_SOURCE_DIR}/main/src/renderer.cpp
    ${CMAKE_SOURCE_DIR}/main/src/sound.cpp
)

add_library(runtime STATIC ${RUNTIME_SOURCES})

target_include_directories(runtime PUBLIC
    ${CMAKE_SOURCE_DIR}/main/src
    ${CMAKE_SOURCE_DIR}/libbu/include
    ${CMAKE_SOURCE_DIR}/graphics/src
    ${CMAKE_SOURCE_DIR}/vendor/raylib/src
    ${CMAKE_SOURCE_DIR}/vendor/raylib/src/external
    ${CMAKE_SOURCE_DIR}/vendor/box2d/include
    ${CMAKE_SOURCE_DIR}/vendor/poly2tri
)

target_link_libraries(runtime PUBLIC libbu graphics box2d poly2tri miniz raylib)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    if(UNIX)
        target_compile_options(runtime PRIVATE -g -D_DEBUG -DVERBOSE)
    endif()
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    target_compile_options(runtime PRIVATE -O3 -DNDEBUG)
endif()
