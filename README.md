
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

* Made it not cache the wasm file, even after shift reload
* change desktop build from nes to saltynes
* Is there a way to check if running asm.js instead of WebAssembly?
* Shrink wasm file by removing complex libraries
* Sound is distorted when running in browser, but not desktop
* Change the name
* Make it show a message if WebAssembly is not supported
