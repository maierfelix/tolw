# tolw

WebAssembly port of [tinyobjloader](https://github.com/syoyo/tinyobjloader)

## Install:
````
npm install tolw
````

## Usage:
````js
const tolw = require("tolw");
// initialise tolw
tolw.init().then(() => {
  // have fun
  let binaryFile = new Uint8Array(fs.readFileSync("./model.obj", null));
  let {vertices, normals, uvs, indices} = tolw.loadObj(binaryFile);
});
````

## Build:

````
em++ --bind -o tolw.js tolw.cc -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s NODEJS_CATCH_EXIT -O3
````
