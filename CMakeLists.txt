cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0072 NEW)

project(BuGame)

set(CMAKE_CXX_STANDARD_REQUIRED ON)



set(BU_EIGEN_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/vendor/eigen")
set(BU_MINIDNN_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/vendor/MiniDNN/include")

add_subdirectory(vendor/miniz)
add_subdirectory(vendor/poly2tri)
add_subdirectory(vendor/box2d)
set(CUSTOMIZE_BUILD ON CACHE BOOL "" FORCE)
set(USE_AUDIO OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/raylib)

add_subdirectory(libbu)
add_subdirectory(graphics)
add_subdirectory(runtime)

add_subdirectory(vendor/imgui)
add_subdirectory(gameeditor)
add_subdirectory(leveleditor)

add_subdirectory(main)


 

 

 
