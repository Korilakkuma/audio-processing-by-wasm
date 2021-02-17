function onRuntimeInitializedHandler(module, stream, processor) {
  console.log('onRuntimeInitialized');

  const source = audiocontext.createMediaStreamSource(stream);

  source.connect(processor);
  processor.connect(audiocontext.destination);

  const bufferSize = processor.bufferSize;

  const noisesuppressor = module.cwrap('noisesuppressor', null, ['number', 'Float32Array', 'Float32Array', 'number']);

  processor.onaudioprocess = (event) => {
    const inputLs  = event.inputBuffer.getChannelData(0);
    const inputRs  = event.inputBuffer.getChannelData(1);
    const outputLs = event.outputBuffer.getChannelData(0);
    const outputRs = event.outputBuffer.getChannelData(1);

    const pointerInputL  = module._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);
    const pointerInputR  = module._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);
    const pointerOutputL = module._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);
    const pointerOutputR = module._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);

    const tmpLs = new Float32Array(module.HEAPF32.buffer, pointerOutputL, bufferSize);
    const tmpRs = new Float32Array(module.HEAPF32.buffer, pointerOutputR, bufferSize);

    module.HEAPF32.set(inputLs, pointerInputL / Float32Array.BYTES_PER_ELEMENT);
    module.HEAPF32.set(inputRs, pointerInputR / Float32Array.BYTES_PER_ELEMENT);
    module.HEAPF32.set(tmpLs, pointerOutputL / Float32Array.BYTES_PER_ELEMENT);
    module.HEAPF32.set(tmpRs, pointerOutputR / Float32Array.BYTES_PER_ELEMENT);

    noisesuppressor(threshold, pointerInputL, pointerOutputL, bufferSize);
    noisesuppressor(threshold, pointerInputR, pointerOutputR, bufferSize);

    outputLs.set(tmpLs);
    outputRs.set(tmpRs);

    module._free(pointerInputL);
    module._free(pointerInputR);
    module._free(pointerOutputL);
    module._free(pointerOutputR);
  };
}
