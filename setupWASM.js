'use strict';

async function setupWASM(wasm) {
  try {
    const response = await fetch(`./${wasm}.wasm`)
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
