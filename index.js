const wasm = require("./tolw");

function writeBufferIntoWasmMemory(heap8, address, data) {
  // array view input
  // might be a sub-view, so use byteOffset
  if (ArrayBuffer.isView(data)) {
    let {byteOffset, byteLength} = data;
    heap8.set(new Uint8Array(data.buffer).subarray(byteOffset, byteOffset + byteLength), address);
  // normal arraybuffer input
  } else {
    heap8.set(new Uint8Array(data), address);
  }
};

// ignore emscripten's stupid global exception listener
process.removeAllListeners("uncaughtException");

let loaded = false;
module.exports.init = function() {
  if (loaded) throw `Wasm module already initialized!`;
  return new Promise(resolve => {
    wasm.onRuntimeInitialized = _ => {
      api = {
        loadObj: wasm.cwrap("loadObj", "number", ["number", "number"]),
        freeMemoryAddr: wasm.cwrap("freeMemoryAddr", "undefined", ["number"])
      };
      loaded = true;
      // tell user we're ready
      resolve(true);
    };
  });
};

module.exports.loadObj = function(data) {
  if (!loaded) throw new Error("TOLW isn't initialized!");

  if (!ArrayBuffer.isView(data) && !(data instanceof ArrayBuffer)) {
    throw new TypeError(`Input data must be either of type 'ArrayBuffer' or 'ArrayBufferView'`);
  }

  let addr = wasm._malloc(data.byteLength);

  // write file into wasm memory
  writeBufferIntoWasmMemory(wasm.HEAP8, addr, data);

  // parse obj
  let memAddr = api.loadObj(addr, data.byteLength);
  if (!memAddr) throw `Failed to load object file!`;

  // free file from wasm memory
  wasm._free(addr);

  // read MemoryAddr struct from heap
  let memoryAddrStructSize = 16;
  let memoryAddrView = wasm.HEAPU32.subarray(memAddr >> 2, (memAddr >> 2) + memoryAddrStructSize);
  let lengths = memoryAddrView.subarray(4, 4 + 4); // to know how much data we have to reflect per-member

  let offset = 0x0;

  // create view on vertices
  let verticesPtr = wasm.HEAPU32[(memoryAddrView[offset] >> 2)]; // deref
  let verticesView = wasm.HEAPF32.subarray(
    (verticesPtr >> 2),
    (verticesPtr >> 2) + lengths[offset]
  );

  offset++;

  // create view on normals
  let normalsPtr = wasm.HEAPU32[(memoryAddrView[offset] >> 2)]; // deref
  let normalsView = wasm.HEAPF32.subarray(
    (normalsPtr >> 2),
    (normalsPtr >> 2) + lengths[offset]
  );

  offset++;

  // create view on uvs
  let uvsPtr = wasm.HEAPU32[(memoryAddrView[offset] >> 2)]; // deref
  let uvsView = wasm.HEAPF32.subarray(
    (uvsPtr >> 2),
    (uvsPtr >> 2) + lengths[offset]
  );

  offset++;

  // create view on indices
  let indicesPtr = wasm.HEAPU32[(memoryAddrView[offset] >> 2)]; // deref
  let indicesView = wasm.HEAPU32.subarray(
    (indicesPtr >> 2),
    (indicesPtr >> 2) + lengths[offset]
  );

  offset++;

  let out = {
    vertices: new Float32Array(verticesView),
    normals: new Float32Array(normalsView),
    uvs: new Float32Array(uvsView),
    indices: new Uint32Array(indicesView)
  };

  // free memory addr struct
  // we used this struct to make processed
  // object data visisble to js
  api.freeMemoryAddr(memAddr);

  return out;
};
