
# Stop and exit on error
set -e

# Setup compiler build flags
CC="em++"
CFLAGS="-Os -std=c++14 -lSDL -lpthread -DSDL=true -s WASM=1"

# FIXME: If we didn't have to source emscripten sdk this way, we
# Could change to building with an incremental build system such
# as Cmake, Raise, or even a Makefile.
# Setup Emscripten/WebAssembly SDK
source ../emsdk/emsdk_env.sh

# Delete generated files
rm -f *.wasm
rm -f index.js

#rm -f *.o
#rm -f -rf build
mkdir -p build

# FIXME: This wont rebuild if any of the header files changed
# Build each C++ file into an object file. But only if the C++ file is newer.
for entry in src/*.cc; do
	filename=$(basename "$entry")
	filename=$(echo $filename | cut -f 1 -d '.')

	if [[ src/"$filename".cc -nt build/"$filename".o ]]; then
		echo Building "$entry" ...
		$CC src/"$filename".cc $CFLAGS -c -o build/"$filename".o
	fi
done

# Build the HTML file
echo Building index.html ...
$CC build/*.o $CFLAGS -o index.js
