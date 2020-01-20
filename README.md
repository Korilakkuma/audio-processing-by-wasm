Audio Processing by WebAssembly
======

# Emscripten SDK

## Mac OS X

- `Xcode`
  - git and clang
- `CMake`
- `Python`
  - Version 2.7.0 or above.
- `Java`
  - For running closure compiler (optional)

```bash
$ git clone git@github.com:emscripten-core/emsdk.git
$ cd emsdk
$ ./emsdk install --build=Release sdk-incoming-64bit binaryen-master-64bit  # Cost much time ...
$ ./emsdk activate --global --build=Release sdk-incoming-64bit binaryen-master-64bit
$ source ./emsdk_env.sh
```

# Setup

```bash
$ git clone git@github.com:Korilakkuma/audio-processing-by-wasm.git
$ cd audio-processing-by-wasm
```

## Noise

```bash
$ cd noise
$ emcc noise.c -s WASM=1 -o noise.js
```

## Pitch Shifter

```bash
$ cd pitchshifter
$ emcc pitchshifter.c -s WASM=1 -s MODULARIZE=1 -s "EXPORTED_FUNCTIONS=['_pitchshifter']" -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']" -o pitchshifter.js
```
