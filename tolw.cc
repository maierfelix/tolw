#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <streambuf>

#include "emscripten.h"
#include <emscripten/bind.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tolw.h"

using namespace emscripten;

tinyobj::attrib_t attrib;
std::vector<tinyobj::shape_t> shapes;
std::vector<tinyobj::material_t> materials;

struct membuf:std::streambuf {
  membuf(char* begin, char* end) {
    this->setg(begin, begin, end);
  }
};

extern "C" {
  int EMSCRIPTEN_KEEPALIVE loadObj(char* input, int len) {

    attrib = {};
    shapes.clear();
    materials.clear();

    membuf sbuf(input, input + len);
    std::istream stream(&sbuf);

    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &stream, NULL);
    if (!err.empty()) std::cerr << err << std::endl;
    return ret ? 1 : 0;
  }
}

// mesh_t
std::vector<tinyobj::index_t> getMeshIndices(int meshIdx) { return shapes[meshIdx].mesh.indices; };
std::vector<unsigned char> getMeshNumFaceVertices(int meshIdx) { return shapes[meshIdx].mesh.num_face_vertices; };
std::vector<int> getMeshMaterialIds(int meshIdx) { return shapes[meshIdx].mesh.material_ids; };
std::vector<unsigned int> getMeshSmoothingGroupIds(int meshIdx) { return shapes[meshIdx].mesh.smoothing_group_ids; };

// path_t
std::vector<int> getPathIndices(int shapeIdx) { return shapes[shapeIdx].path.indices; };

// attrib_t
std::vector<tinyobj::real_t> getAttribVertices() { return attrib.vertices; };
std::vector<tinyobj::real_t> getAttribNormals() { return attrib.normals; };
std::vector<tinyobj::real_t> getAttribTexCoords() { return attrib.texcoords; };
std::vector<tinyobj::real_t> getAttribColors() { return attrib.colors; };

// index_t
std::vector<int> getMeshIndex(int meshIdx, int indexIdx) {
  tinyobj::index_t index = shapes[meshIdx].mesh.indices[indexIdx];
  std::vector<int> v = { index.vertex_index, index.normal_index, index.texcoord_index };
  return v;
};

// shape related
int getShapeCount() { return shapes.size(); };
std::string getShapeName(int shapeIdx) { return shapes[shapeIdx].name; };

extern "C" {
  uint8_t* EMSCRIPTEN_KEEPALIVE allocateSpace(int byteLen) {
    uint8_t* ptr = (uint8_t*) malloc(byteLen * sizeof(uint8_t));
    return ptr;
  }
}

EMSCRIPTEN_BINDINGS(module) {
  register_vector<int>("int");
  register_vector<unsigned int>("unsigned int");
  register_vector<unsigned char>("unsigned char");
  register_vector<tinyobj::real_t>("real_t");
  register_vector<tinyobj::index_t>("index_t");
  // mesh_t
  function("getMeshIndices", &getMeshIndices);
  function("getMeshNumFaceVertices", &getMeshNumFaceVertices);
  function("getMeshMaterialIds", &getMeshMaterialIds);
  function("getMeshSmoothingGroupIds", &getMeshSmoothingGroupIds);
  // path_t
  function("getPathIndices", &getPathIndices);
  // attrib_t
  function("getAttribVertices", &getAttribVertices);
  function("getAttribNormals", &getAttribNormals);
  function("getAttribTexCoords", &getAttribTexCoords);
  function("getAttribColors", &getAttribColors);
  // index_t
  function("getMeshIndex", &getMeshIndex);
  // shape_t
  function("getShapeName", &getShapeName);
  // shape related
  function("getShapeCount", &getShapeCount);
}
