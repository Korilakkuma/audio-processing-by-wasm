Audio Processing by WebAssembly
======

# Emscripten SDK

[Repository](https://github.com/emscripten-core/emsdk)

## System Requirements

- `git`
- C compiler (clang ... etc)
- `CMake`

### Mac OS X

- `Python`
  - Version 2.7.0 or above.
- `Java`
  - For running closure compiler (optional)

and,

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

## Vocal Canceler

```bash
$ cd vocalcanceler
$ emcc vocalcanceler.c -s WASM=1 -o vocalcanceler.js
```

## Pitch Shifter

```bash
$ cd pitchshifter
$ emcc pitchshifter.c -s WASM=1 -s MODULARIZE=1 -s "EXPORTED_FUNCTIONS=['_pitchshifter']" -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']" -o pitchshifter.js
```

## Noise Suppressor

```bash
$ cd noisesuppressor
$ emcc noisesuppressor.c -s WASM=1 -s MODULARIZE=1 -s "EXPORTED_FUNCTIONS=['_noisesuppressor']" -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']" -o noisesuppressor.js
```
