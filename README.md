
A NES emulator in WebAssembly.

Forked from the NaCl C++ version: https://github.com/workhorsylegacy/SaltyNESLegacy

Which is based on vNES: https://github.com/workhorsylegacy/vNES

# Build
```
./make_web.sh
```

# Run in browser
```
python3 -m http.server 9999
```

TODO
* in fullscreen on chrome, there is a white line on the bottom.
* Add loading roms from gui.
* see if smb3 and punchout work in vnes
* save memory to localstorage/indexeddb
* tetris has no backgrounds

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
* Make it show a message if WebAssembly is not supported
