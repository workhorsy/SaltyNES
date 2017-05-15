
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

* Is there a way to check if running asm.js instead of WebAssembly?
* Shrink wasm file by removing cout and other complex libraries
* Add sound playing
* Change the name
* Desktop mode does not quit
* Make it show a message if WebAssembly is not supported
