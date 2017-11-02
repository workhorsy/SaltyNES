
# Stop and exit on error
set -e

touch src/build_date.cc

if [ "$1" == "debug" ]; then
	echo "Building debug version"
	BUILD_DIR="build_emscripten_debug"
	BUILD_TYPE=Debug
else
	echo "Building release version by default"
	BUILD_DIR="build_emscripten_release"
	BUILD_TYPE=Release
fi

# Setup compiler build flags
if [ ! -f ./emsdk/emsdk_env.sh ]; then
	echo "File emsdk_env.sh not found, run init_web.sh first"
	exit 1
fi
source ./emsdk/emsdk_env.sh --build=Release

if [ ! -d $BUILD_DIR ]; then
	mkdir $BUILD_DIR
fi

cd $BUILD_DIR
emcmake cmake .. -DMY_TYPE=$BUILD_TYPE
make -j 1
