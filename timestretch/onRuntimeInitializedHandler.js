function onRuntimeInitializedHandler(module, audio, processor, file) {
 console.log('onRuntimeInitialized');

 const objectURL = window.URL.createObjectURL(file);

 audio = new Audio(objectURL);

 const source = audiocontext.createMediaElementSource(audio);

 source.connect(processor);
 processor.connect(audiocontext.destination);

 const bufferSize = processor.bufferSize;

 const timestretch = module.cwrap('timestretch', null, ['number', 'Float32Array', 'Float32Array', 'number', 'number']);

 processor.onaudioprocess = (event) => {
   const inputLs  = event.inputBuffer.getChannelData(0);
   const inputRs  = event.inputBuffer.getChannelData(1);
   const outputLs = event.outputBuffer.getChannelData(0);
   const outputRs = event.outputBuffer.getChannelData(1);

   const resizedBufferSize = Math.floor(bufferSize / rate);

   const pointerInputL  = module._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);
   const pointerInputR  = module._malloc(bufferSize * Float32Array.BYTES_PER_ELEMENT);
   const pointerOutputL = module._malloc(resizedBufferSize * Float32Array.BYTES_PER_ELEMENT);
   const pointerOutputR = module._malloc(resizedBufferSize * Float32Array.BYTES_PER_ELEMENT);

   const tmpLs = new Float32Array(module.HEAPF32.buffer, pointerOutputL, resizedBufferSize);
   const tmpRs = new Float32Array(module.HEAPF32.buffer, pointerOutputR, resizedBufferSize);

   module.HEAPF32.set(inputLs, pointerInputL / Float32Array.BYTES_PER_ELEMENT);
   module.HEAPF32.set(inputRs, pointerInputR / Float32Array.BYTES_PER_ELEMENT);
   module.HEAPF32.set(tmpLs, pointerOutputL / Float32Array.BYTES_PER_ELEMENT);
   module.HEAPF32.set(tmpRs, pointerOutputR / Float32Array.BYTES_PER_ELEMENT);

   timestretch(rate, pointerInputL, pointerOutputL, audiocontext.sampleRate, bufferSize);
   timestretch(rate, pointerInputR, pointerOutputR, audiocontext.sampleRate, bufferSize);

   console.log(tmpLs);

   outputLs.set(tmpLs);
   outputRs.set(tmpRs);

   console.log(outputLs);

   module._free(pointerInputL);
   module._free(pointerInputR);
   module._free(pointerOutputL);
   module._free(pointerOutputR);
 };

 audio.play();
}
