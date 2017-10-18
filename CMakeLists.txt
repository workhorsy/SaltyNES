cmake_minimum_required(VERSION 3.7)
project (SaltyNES)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
# set(CMAKE_BUILD_TYPE_INIT "Release")
set(CMAKE_CXX_STANDARD 14)

file(GLOB SOURCES "src/*.cc")
message(STATUS "system name: " ${CMAKE_SYSTEM_NAME} ", build: " ${CMAKE_BUILD_TYPE})

if ({CMAKE_BUILD_TYPE} MATCHES "Debug")
	set(MORE_FLAGS_NATIVE="-g")
	set(MORE_FLAGS_WEB="-s SAFE_HEAP=1") # add addition debug emscripten flags ?
endif ()

if ({CMAKE_BUILD_TYPE} MATCHES "Release")
	set(MORE_FLAGS_NATIVE="")
	set(MORE_FLAGS_WEB="") # add addition debug emscripten flags ?
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
	set(CMAKE_C_COMPILER "emcc")
	set(EMCC_LINKER_FLAGS "--bind -s WASM=1 -O3 -DWEB=true -std=c++14 -s USE_SDL=2 " ${MORE_FLAGS_WEB})
	set(CMAKE_CXX_FLAGS "${EMCC_LINKER_FLAGS}")
endif ()


if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
	find_package(sdl2 REQUIRED)
	set(CMAKE_CXX_FLAGS "-O3 -std=c++14 -lSDL2 -lSDL2_mixer -lpthread -DDESKTOP=true" ${MORE_FLAGS_NATIVE})
endif ()

add_executable(SaltyNES ${SOURCES})
