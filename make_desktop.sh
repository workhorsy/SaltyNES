
# Stop and exit on error
set -e

# Setup compiler build flags
CC="g++"
CFLAGS="-O0 -g -std=c++14 -lSDL2 -lSDL2_mixer -lpthread -DDESKTOP=true"

rm -f nes
#rm -f *.o
#rm -f -rf build_desktop
mkdir -p build_desktop

# FIXME: This wont rebuild if any of the header files changed
# Build each C++ file into an object file. But only if the C++ file is newer.
for entry in src/*.cc; do
	filename=$(basename "$entry")
	filename=$(echo $filename | cut -f 1 -d '.')

	if [[ src/"$filename".cc -nt build_desktop/"$filename".o ]]; then
		echo Building "$entry" ...
		$CC src/"$filename".cc $CFLAGS -c -o build_desktop/"$filename".o
	fi
done

# Build the exe file
echo Building exe ...
$CC build_desktop/*.o $CFLAGS -o nes
