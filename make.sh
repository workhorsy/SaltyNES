

#COPTS=-O3
COPTS="-Os -std=c++14 -lSDL -lpthread -DSDL=true -s WASM=1"

source ../emsdk/emsdk_env.sh

rm -f *.wasm
rm -f index.html
rm -f index.js
rm -f *.o

em++  src/*.cc $COPTS -o index.html
