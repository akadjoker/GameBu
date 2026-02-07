cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0072 NEW)

project(BuGame)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

 
add_subdirectory(libbu/libbu)
add_subdirectory(graphics)

add_subdirectory(main)
add_subdirectory(teste)
add_subdirectory(testEngine)
 

 

 