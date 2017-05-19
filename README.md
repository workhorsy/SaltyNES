
A NES emulator in WebAssembly. Based on vNES.

Forked from the C++ version https://launchpad.net/saltynes

# Build
```
./make_web.sh
```

# Run in browser
```
python3 -m http.server 9999
```

TODO
* Move the old SaltyNES to a repo called SaltyNESLegacy
* Make sure all supported mappers actually work
* Make a trivial SDL audio app to see why sound playing is broken.
* Try using opengl for the screen to see if it makes painting faster.
* make main loops for desktop and web not have nested while loops
* make fps show in html
* Make it so the emulator can be restarted with out reloading the page
* Add gamepad support
* make the emulator easy to embed in other web apps by having hooks for all gui buttons
* make it show progress for downloading games and wasm

* Make it not cache the wasm file, even after shift reload
* Is there a way to check if running asm.js instead of WebAssembly?
* Shrink wasm file by removing complex libraries
* Sound is distorted when running in browser, but not desktop
* Change the name
* Make it show a message if WebAssembly is not supported
