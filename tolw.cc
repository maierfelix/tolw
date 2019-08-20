#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <streambuf>

#include <bitset>
#include <unordered_map>

#include "emscripten.h"
#include <emscripten/bind.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tolw.h"

using namespace emscripten;

struct Vector2 {
  float x;
  float y;
};

struct Vector3 {
  float x;
  float y;
  float z;
};

struct MemoryAddr {
  std::vector<Vector3>* vertices;
  std::vector<Vector3>* normals;
  std::vector<Vector2>* uvs;
  std::vector<unsigned int>* indices;
  int lengths[4];
};

struct membuf:std::streambuf {
  membuf(char* begin, char* end) {
    this->setg(begin, begin, end);
  }
};

extern "C" {
  void EMSCRIPTEN_KEEPALIVE freeMemoryAddr(int addr) {
    MemoryAddr* mem = reinterpret_cast<MemoryAddr *>(addr);
    mem->vertices->clear();
    mem->normals->clear();
    mem->uvs->clear();
    mem->indices->clear();
    delete mem->vertices;
    delete mem->normals;
    delete mem->uvs;
    delete mem->indices;
    delete mem;
  }
}

std::bitset<256> VertexBitHash(Vector3* v, Vector3* n, Vector2* u) {
  std::bitset<256> bits{};
  bits |= reinterpret_cast<unsigned &>(v->x); bits <<= 32;
  bits |= reinterpret_cast<unsigned &>(v->y); bits <<= 32;
  bits |= reinterpret_cast<unsigned &>(v->z); bits <<= 32;
  bits |= reinterpret_cast<unsigned &>(n->x); bits <<= 32;
  bits |= reinterpret_cast<unsigned &>(n->y); bits <<= 32;
  bits |= reinterpret_cast<unsigned &>(n->z); bits <<= 32;
  bits |= reinterpret_cast<unsigned &>(u->x); bits <<= 32;
  bits |= reinterpret_cast<unsigned &>(u->y);
  return bits;
}

extern "C" {
  int EMSCRIPTEN_KEEPALIVE loadObj(char* input, int len) {

    tinyobj::attrib_t attrib = {};
    std::vector<tinyobj::shape_t> shapes = {};
    std::vector<tinyobj::material_t> materials = {};

    membuf sbuf(input, input + len);
    std::istream stream(&sbuf);

    std::string err;
    std::string warn;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &warn, &stream, NULL, true);
    if (!err.empty()) std::cerr << err << std::endl;

    std::vector<Vector3>* vertices = new std::vector<Vector3>();
    std::vector<Vector3>* normals = new std::vector<Vector3>();
    std::vector<Vector2>* uvs = new std::vector<Vector2>();
    std::vector<unsigned int>* indices = new std::vector<unsigned int>();

    int unique_vertex_counter = 0;
    std::unordered_map<std::bitset<256>, int> vimap;

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            unsigned fv = shapes[s].mesh.num_face_vertices[f];

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3*idx.vertex_index+0];
                tinyobj::real_t vy = attrib.vertices[3*idx.vertex_index+1];
                tinyobj::real_t vz = attrib.vertices[3*idx.vertex_index+2];
                tinyobj::real_t nx = attrib.normals[3*idx.normal_index+0];
                tinyobj::real_t ny = attrib.normals[3*idx.normal_index+1];
                tinyobj::real_t nz = attrib.normals[3*idx.normal_index+2];
                tinyobj::real_t tx = attrib.texcoords[2 * abs(idx.texcoord_index) + 0];
                tinyobj::real_t ty = attrib.texcoords[2 * abs(idx.texcoord_index) + 1];

                Vector3 vertex = {vx, vy, vz};
                Vector3 normal = {nx, ny, nz};
                Vector2 uv = {tx, ty};

                auto bits = VertexBitHash(&vertex, &normal, &uv);

                if (vimap.count(bits) == 0) {
                  vertices->push_back(vertex);
                  normals->push_back(normal);
                  uvs->push_back(uv);
                  indices->push_back(unique_vertex_counter);
                  vimap[bits] = unique_vertex_counter;
                  ++unique_vertex_counter;
                } else {
                  indices->push_back(vimap[bits]);
                }
            }

            index_offset += fv;
        }
    }

    // hack to share memory address of our data to the outside
    MemoryAddr* mem = new MemoryAddr();
    mem->vertices = vertices;
    mem->normals = normals;
    mem->uvs = uvs;
    mem->indices = indices;
    mem->lengths[0] = vertices->size() * 3;
    mem->lengths[1] = normals->size() * 3;
    mem->lengths[2] = uvs->size() * 2;
    mem->lengths[3] = indices->size();

    return ret ? (int) mem : 0;
  }
}

EMSCRIPTEN_BINDINGS(module) {
  register_vector<int>("int");
  register_vector<unsigned int>("unsigned int");
  register_vector<unsigned char>("unsigned char");
}
