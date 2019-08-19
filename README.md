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
  let objFile = new Uint8Array(fs.readFileSync("./model.obj", null));
  let { attrib, shapes } = tolw.loadObj(objFile);
});
````

## Example:
````js
let vertices = [];
let indices = [];
for (let shape of shapes) {
  for (let index of shape.mesh.indices) {
    let pos = {
      x: attrib.vertices[3 * index.vertex_index + 0],
      y: attrib.vertices[3 * index.vertex_index + 2],
      z: attrib.vertices[3 * index.vertex_index + 1]
    };
    let normal = {
      x: attrib.normals[3 * index.normal_index + 0],
      y: attrib.normals[3 * index.normal_index + 2],
      z: attrib.normals[3 * index.normal_index + 1]
    };
    let texCoord = {
      x: attrib.texcoords[2 * index.texcoord_index + 0],
      y: 1.0 - attrib.texcoords[2 * index.texcoord_index + 1]
    };
    vertices.push(
      pos.x, pos.y, pos.z,
      normal.x, normal.y, normal.z,
      texCoord.x, texCoord.y
    );
    indices.push(indices.length);
  };
};
````

## Build:

````
em++ --bind -o tolw.js tolw.cc -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s NODEJS_CATCH_EXIT -O3
````
