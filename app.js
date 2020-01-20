'use strict';

const audiocontext = new AudioContext();
const processor    = audiocontext.createScriptProcessor(2048, 2, 2);

let pitch = 1;

document.getElementById('select-noise').addEventListener('change', async (event) => {
  const { module, instance } = await setupWASM('noise');
  startNoise(module, instance, event.target.value);
}, false);

document.getElementById('file-uploader').addEventListener('change', async (event) => {
  const { module, instance, bytes } = await setupWASM('pitchshifter');
  startAudio(module, instance, bytes, event.target.files[0]);
}, false);

document.getElementById('range-pitch').addEventListener('input', (event) => {
  pitch = event.currentTarget.valueAsNumber;
  document.getElementById('output-pitch').textContent = event.currentTarget.value;
}, false);

async function setupWASM(wasm) {
  processor.onaudioprocess = null;
  processor.disconnect(0);

  try {
    const response = await fetch(`./wasm/${wasm}.wasm`)
    const bytes    = await response.arrayBuffer();
    const module   = await WebAssembly.compile(bytes);

    const imports = {};

    imports.env = {};
    imports.env.memoryBase = 0;
    imports.env.tableBase  = 0;

    if (!imports.env.memory) {
      imports.env.memory = new WebAssembly.Memory({ initial: 256, maximum: 256 });
    }

    if (!imports.env.table) {
      imports.env.table = new WebAssembly.Table({ initial: 1, maximum : 1, element: 'anyfunc' });
    }

    imports.env.__syscall2 = () => {};
    imports.env.__lock = () => {};
    imports.env.__unlock = () => {};
    imports.env.__handle_stack_overflow = () => {};
    imports.env.emscripten_resize_heap = () => {};
    imports.env.time = () => {};

    const { instance } = await WebAssembly.instantiate(bytes, imports);

    return { module, instance, bytes };
  } catch (e) {
    console.error(e);
  }
}

async function startNoise(module, instance, type) {
  try {
    await audiocontext.resume();

    processor.connect(audiocontext.destination);

    const bufferSize = processor.bufferSize;

    processor.onaudioprocess = (event) => {
      const outputLs = event.outputBuffer.getChannelData(0);
      const outputRs = event.outputBuffer.getChannelData(1);

      for (let i = 0; i < bufferSize; i++) {
        let output = 0;

        switch (type) {
          case 'whitenoise':
            output = instance.exports.whitenoise();
            break;
          case 'pinknoise':
            output = instance.exports.pinknoise();
            break;
          case 'browniannoise':
            output = instance.exports.browniannoise();
            break;
          default:
            break;
        }

        outputLs[i] = output;
        outputRs[i] = output;
      }
    };
  } catch (e) {
    console.error(e);
  }
}

async function startAudio(module, instance, bytes, file) {
  try {
    await audiocontext.resume();

    const m = Module({
      wasmBinary: new Uint8Array(bytes),
      onRuntimeInitialized: () => {
        console.log('onRuntimeInitialized');

        const objectURL = window.URL.createObjectURL(file);
        const audio     = new Audio(objectURL);
        const source    = audiocontext.createMediaElementSource(audio);

        source.connect(processor);
        processor.connect(audiocontext.destination);

        const bufferSize = processor.bufferSize;

        const pitchshifter = m.cwrap('pitchshifter', null, ['number', 'Float32Array', 'Float32Array', 'Float32Array', 'Float32Array', 'number']);

        processor.onaudioprocess = (event) => {
          const inputLs  = event.inputBuffer.getChannelData(0);
          const inputRs  = event.inputBuffer.getChannelData(1);
          const outputLs = event.outputBuffer.getChannelData(0);
          const outputRs = event.outputBuffer.getChannelData(1);

          const pointerRealL  = m._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);
          const pointerRealR  = m._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);
          const pointerImagL  = m._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);
          const pointerImagR  = m._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);
          const apointerRealL = m._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);
          const apointerRealR = m._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);
          const apointerImagL = m._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);
          const apointerImagR = m._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);

          const realLs = new Float32Array(inputLs);
          const realRs = new Float32Array(inputRs);
          const imagLs = new Float32Array(bufferSize);
          const imagRs = new Float32Array(bufferSize);

          const arealLs = new Float32Array(m.HEAPF32.buffer, apointerRealL, bufferSize);
          const arealRs = new Float32Array(m.HEAPF32.buffer, apointerRealR, bufferSize);
          const aimagLs = new Float32Array(m.HEAPF32.buffer, apointerImagL, bufferSize);
          const aimagRs = new Float32Array(m.HEAPF32.buffer, apointerImagR, bufferSize);

          m.HEAPF32.set(realLs, pointerRealL / Float32Array.BYTES_PER_ELEMENT);
          m.HEAPF32.set(realRs, pointerRealR / Float32Array.BYTES_PER_ELEMENT);
          m.HEAPF32.set(imagLs, pointerImagL / Float32Array.BYTES_PER_ELEMENT);
          m.HEAPF32.set(imagRs, pointerImagR / Float32Array.BYTES_PER_ELEMENT);
          m.HEAPF32.set(arealLs, apointerRealL / Float32Array.BYTES_PER_ELEMENT);
          m.HEAPF32.set(arealRs, apointerRealR / Float32Array.BYTES_PER_ELEMENT);
          m.HEAPF32.set(aimagLs, apointerImagL / Float32Array.BYTES_PER_ELEMENT);
          m.HEAPF32.set(aimagRs, apointerImagR / Float32Array.BYTES_PER_ELEMENT);

          pitchshifter(pitch, pointerRealL, pointerImagL, apointerRealL, apointerImagL, bufferSize);
          pitchshifter(pitch, pointerRealR, pointerImagR, apointerRealR, apointerImagR, bufferSize);

          outputLs.set(arealLs);
          outputRs.set(arealRs);

          m._free(pointerRealL);
          m._free(pointerImagL);
          m._free(pointerRealR);
          m._free(pointerImagR);
          m._free(apointerRealL);
          m._free(apointerImagL);
          m._free(apointerRealR);
          m._free(apointerImagR);
        };

        audio.play();
      }
    });
  } catch (e) {
    console.error(e);
  }
}
