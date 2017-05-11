

CC="em++"
CFLAGS="-Os -std=c++14 -lSDL -lpthread -DSDL=true -s WASM=1"

# FIXME: If we didn't have to source emscripten sdk this way, we
# Could change to building with an incremental build system such
# as Cmake, Raise, or even a Makefile.
source ../emsdk/emsdk_env.sh

rm -f *.wasm
rm -f index.html
rm -f index.js
rm -f *.o

$CC src/*.cc $CFLAGS -o index.html
