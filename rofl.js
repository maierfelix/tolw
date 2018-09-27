const fs = require("fs");
let tolw = require("./index");

tolw.init().then(() => {
  // cube
  {
    let data = new Uint8Array(fs.readFileSync("./cube.obj", null));
    let res = tolw.loadObj(data);
    console.log(res.shapes[0].name);
    console.log(res.attrib.vertices.length);
  }
  // material_sphere
  {
    let data = new Uint8Array(fs.readFileSync("./material_sphere.obj", null));
    let res = tolw.loadObj(data);
    console.log(res.shapes[0].name);
    console.log(res.attrib.vertices.length);
  }
  // sphere
  {
    let data = new Uint8Array(fs.readFileSync("./sphere.obj", null));
    let res = tolw.loadObj(data);
    console.log(res.shapes[0].name);
    console.log(res.attrib.vertices.length);
  }
});
