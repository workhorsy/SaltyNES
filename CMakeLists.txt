cmake_minimum_required(VERSION 3.4.3)
project (SaltyNES)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_BUILD_TYPE_INIT "Release")
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_STANDARD 14)

file(GLOB SOURCES "src/*.cc")
message(STATUS "system name: " ${CMAKE_SYSTEM_NAME} ", build: " ${MY_TYPE})

if ({MY_TYPE} MATCHES "Debug")
	set(MORE_FLAGS_NATIVE="-g -DDEBUG")
	set(MORE_FLAGS_WEB="-O0 -DDEBUG -g4 -s SAFE_HEAP=1 -s ASSERTIONS=1 -s SAFE_HEAP_LOG=1 -s STACK_OVERFLOW_CHECK=2") # add additional debug emscripten flags ?
endif ()

if ({MY_TYPE} MATCHES "Release")
	set(MORE_FLAGS_NATIVE="")
	set(MORE_FLAGS_WEB="-O3")
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
	set(CMAKE_C_COMPILER "emcc")
	set(EMCC_LINKER_FLAGS "--bind -s WASM=1 -DWEB=true -std=c++14 -s USE_SDL=2 " ${MORE_FLAGS_WEB})
	set(CMAKE_CXX_FLAGS "${EMCC_LINKER_FLAGS}")
endif ()


if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
	find_package(sdl2 REQUIRED)
	set(CMAKE_CXX_FLAGS "-O3 -std=c++14 -lSDL2 -lSDL2_mixer -DDESKTOP=true" ${MORE_FLAGS_NATIVE})
endif ()

add_executable(SaltyNES ${SOURCES})
target_link_libraries(SaltyNES ${SDL2_LIBRARIES})
