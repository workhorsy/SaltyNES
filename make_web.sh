
# Stop and exit on error
set -e

# Setup compiler build flags
CC="em++"
CFLAGS="-O3 -std=c++14 -lpthread -DWEB=true -s WASM=1 -s USE_SDL=2"

# FIXME: If we didn't have to source emscripten sdk this way, we
# Could change to building with an incremental build system such
# as Cmake, Raise, or even a Makefile.
# Setup Emscripten/WebAssembly SDK
source ../emsdk/emsdk_env.sh

# Delete generated files
rm -f *.wasm
rm -f index.js

#rm -f *.o
#rm -f -rf build/web
mkdir -p build/web

# FIXME: This wont rebuild if any of the header files changed
# Build each C++ file into an object file. But only if the C++ file is newer.
for entry in src/*.cc; do
	filename=$(basename "$entry")
	filename=$(echo $filename | cut -f 1 -d '.')

	if [[ src/"$filename".cc -nt build/web/"$filename".o ]]; then
		echo Building "$entry" ...
		$CC src/"$filename".cc $CFLAGS -c -o build/web/"$filename".o
	fi
done

# Build the wasm file
echo Building WASM ...
$CC build/web/*.o $CFLAGS -s EXPORTED_FUNCTIONS="['_main', '_toggle_sound']" -o index.js
