function onRuntimeInitializedHanderler(module, audio, processor, file) {
  console.log('onRuntimeInitialized');

  const objectURL = window.URL.createObjectURL(file);

  audio = new Audio(objectURL);

  const source = audiocontext.createMediaElementSource(audio);

  source.connect(processor);
  processor.connect(audiocontext.destination);

  const bufferSize = processor.bufferSize;

  const pitchshifter = module.cwrap('pitchshifter', null, ['number', 'Float32Array', 'Float32Array', 'Float32Array', 'Float32Array', 'number']);

  processor.onaudioprocess = (event) => {
    const inputLs  = event.inputBuffer.getChannelData(0);
    const inputRs  = event.inputBuffer.getChannelData(1);
    const outputLs = event.outputBuffer.getChannelData(0);
    const outputRs = event.outputBuffer.getChannelData(1);

    const pointerRealL  = module._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);
    const pointerRealR  = module._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);
    const pointerImagL  = module._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);
    const pointerImagR  = module._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);
    const apointerRealL = module._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);
    const apointerRealR = module._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);
    const apointerImagL = module._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);
    const apointerImagR = module._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);

    const realLs = new Float32Array(inputLs);
    const realRs = new Float32Array(inputRs);
    const imagLs = new Float32Array(bufferSize);
    const imagRs = new Float32Array(bufferSize);

    const arealLs = new Float32Array(module.HEAPF32.buffer, apointerRealL, bufferSize);
    const arealRs = new Float32Array(module.HEAPF32.buffer, apointerRealR, bufferSize);
    const aimagLs = new Float32Array(module.HEAPF32.buffer, apointerImagL, bufferSize);
    const aimagRs = new Float32Array(module.HEAPF32.buffer, apointerImagR, bufferSize);

    module.HEAPF32.set(realLs, pointerRealL / Float32Array.BYTES_PER_ELEMENT);
    module.HEAPF32.set(realRs, pointerRealR / Float32Array.BYTES_PER_ELEMENT);
    module.HEAPF32.set(imagLs, pointerImagL / Float32Array.BYTES_PER_ELEMENT);
    module.HEAPF32.set(imagRs, pointerImagR / Float32Array.BYTES_PER_ELEMENT);
    module.HEAPF32.set(arealLs, apointerRealL / Float32Array.BYTES_PER_ELEMENT);
    module.HEAPF32.set(arealRs, apointerRealR / Float32Array.BYTES_PER_ELEMENT);
    module.HEAPF32.set(aimagLs, apointerImagL / Float32Array.BYTES_PER_ELEMENT);
    module.HEAPF32.set(aimagRs, apointerImagR / Float32Array.BYTES_PER_ELEMENT);

    pitchshifter(pitch, pointerRealL, pointerImagL, apointerRealL, apointerImagL, bufferSize);
    pitchshifter(pitch, pointerRealR, pointerImagR, apointerRealR, apointerImagR, bufferSize);

    outputLs.set(arealLs);
    outputRs.set(arealRs);

    module._free(pointerRealL);
    module._free(pointerImagL);
    module._free(pointerRealR);
    module._free(pointerImagR);
    module._free(apointerRealL);
    module._free(apointerImagL);
    module._free(apointerRealR);
    module._free(apointerImagR);
  };

  audio.play();
}
