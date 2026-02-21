project(main)
cmake_policy(SET CMP0072 NEW)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -D_CRT_SECURE_NO_WARNINGS")
    if (MSVC)
        if(CMAKE_BUILD_TYPE MATCHES Debug)
            add_compile_options(/RTC1 /Od /Zi)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fsanitize=address")
        endif()     
    endif()

endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Keep host-specific tuning only for native desktop builds.
if(NOT CMAKE_CROSSCOMPILING AND NOT EMSCRIPTEN AND NOT ANDROID)
    add_compile_options(
            -O3
            -march=native
            -mtune=native
            -ftree-vectorize
            -DNDEBUG
            -finline-functions
            -funroll-loops
    )
else()
    add_compile_options(
            -O3
            -ftree-vectorize
            -DNDEBUG
            -finline-functions
            -funroll-loops
    )
endif()



file(GLOB SOURCES CONFIGURE_DEPENDS "src/*.cpp")
add_executable(main   ${SOURCES})
target_include_directories(main PUBLIC include src)
if(CMAKE_BUILD_TYPE MATCHES Debug)

if (UNIX)
target_compile_options(main PRIVATE -fsanitize=address -fsanitize=undefined -fsanitize=leak -g  -D_DEBUG -DVERBOSE)
target_link_options(main PRIVATE -fsanitize=address -fsanitize=undefined -fsanitize=leak -g  -D_DEBUG) 
endif()
 
   
endif()

target_link_libraries(main raylib libbu graphics box2d)

if (WIN32)
    target_link_libraries(main Winmm.lib )
endif()


if (UNIX)
    target_link_libraries(main  m )
endif()
