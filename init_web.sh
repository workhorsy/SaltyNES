
git submodule init
git submodule update

SDK_VER="latest"

cd emsdk
./emsdk update-tags
./emsdk install $SDK_VER
./emsdk activate $SDK_VER


