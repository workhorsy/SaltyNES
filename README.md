
A NES emulator in WebAssembly.

Forked from the NaCl C++ version: https://github.com/workhorsylegacy/SaltyNESLegacy

Which is based on vNES: https://github.com/workhorsylegacy/vNES

# Build and run in browser
```
./make_web.sh
python3 -m http.server 9999
```

# Build and run in desktop
```
./make_desktop.sh
./saltynes game.nes
```

TODO
* Remove the mutex, or replace it with std::mutex
* see if smb3 and punchout work in vnes
* save memory to localstorage/indexeddb
* tetris has no backgrounds

* Try using opengl for the screen to see if it makes painting faster.
* make fps show in html
* Make it so the emulator can be restarted without reloading the page
* Add gamepad support
* make the emulator easy to embed in other web apps by having hooks for all gui buttons

* Make it not cache the wasm file, even after shift reload
* Is there a way to check if running asm.js instead of WebAssembly?
* Shrink wasm file by removing complex libraries
