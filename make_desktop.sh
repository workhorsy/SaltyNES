# Stop and exit on error
set -e

touch src/build_date.cc

if [ "$1" == "debug" ]; then
	echo "Building debug version"
	BUILD_DIR="build_desktop_debug"
	BUILD_TYPE=Debug
else
	echo "Building release version by default"
	BUILD_DIR="build_desktop_release"
	BUILD_TYPE=Release
fi

if [ ! -d $BUILD_DIR ]; then
	mkdir $BUILD_DIR
fi

cd $BUILD_DIR
cmake .. -DMY_TYPE=$BUILD_TYPE
make -j 4
