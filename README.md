
An experimental port of SaltyNES to WebAssembly

Copied from https://launchpad.net/saltynes

# Build
```
./make.sh
```

# Run in browser
```
python3 -m http.server 9999
```

TODO

* Is there a way to check if running asm.js instead of WebAssembly?
* Keyboard input
* Game loading from HTML GUI
* Cleanup whitespace
* Remove NACL code
* Shrink wasm file by removing cout and other complex libraries
* Make it build as Desktop SDL app
* Add sound playing
* Change the name
