/*
The MIT License (MIT)

Copyright (c) 2012-2016 Syoyo Fujita and many contributors.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

//
// version 1.0.0 : Change data structure. Change license from BSD to MIT.
// Support different index for
// vertex/normal/texcoord(#73, #39)
// version 0.9.20: Fixes creating per-face material using `usemtl`(#68)
// version 0.9.17: Support n-polygon and crease tag(OpenSubdiv extension)
// version 0.9.16: Make tinyobjloader header-only
// version 0.9.15: Change API to handle no mtl file case correctly(#58)
// version 0.9.14: Support specular highlight, bump, displacement and alpha
// map(#53)
// version 0.9.13: Report "Material file not found message" in `err`(#46)
// version 0.9.12: Fix groups being ignored if they have 'usemtl' just before
// 'g' (#44)
// version 0.9.11: Invert `Tr` parameter(#43)
// version 0.9.10: Fix seg fault on windows.
// version 0.9.9 : Replace atof() with custom parser.
// version 0.9.8 : Fix multi-materials(per-face material ID).
// version 0.9.7 : Support multi-materials(per-face material ID) per
// object/group.
// version 0.9.6 : Support Ni(index of refraction) mtl parameter.
//                 Parse transmittance material parameter correctly.
// version 0.9.5 : Parse multiple group name.
//                 Add support of specifying the base path to load material
//                 file.
// version 0.9.4 : Initial support of group tag(g)
// version 0.9.3 : Fix parsing triple 'x/y/z'
// version 0.9.2 : Add more .mtl load support
// version 0.9.1 : Add initial .mtl load support
// version 0.9.0 : Initial
//

//
// Use this in *one* .cc
//   #define TINYOBJLOADER_IMPLEMENTATION
//   #include "tiny_obj_loader.h"
//

#ifndef TINY_OBJ_LOADER_H_
#define TINY_OBJ_LOADER_H_

#include <string>
#include <vector>
#include <map>
#include <BlendShape.h>

namespace tinyobj {

typedef struct {
  std::string name;

  float ambient[3];
  float diffuse[3];
  float specular[3];
  float transmittance[3];
  float emission[3];
  float shininess;
  float ior;       // index of refraction
  float dissolve;  // 1 == opaque; 0 == fully transparent
  // illumination model (see http://www.fileformat.info/format/material/)
  int illum;

  int dummy;  // Suppress padding warning.

  std::string ambient_texname;             // map_Ka
  std::string diffuse_texname;             // map_Kd
  std::string specular_texname;            // map_Ks
  std::string specular_highlight_texname;  // map_Ns
  std::string bump_texname;                // map_bump, bump
  std::string displacement_texname;        // disp
  std::string alpha_texname;               // map_d
  std::map<std::string, std::string> unknown_parameter;
} material_t;

typedef struct {
  std::string name;

  std::vector<int> intValues;
  std::vector<float> floatValues;
  std::vector<std::string> stringValues;
} tag_t;

// Index struct to support differnt indices for vtx/normal/texcoord.
// -1 means not used.
typedef struct {
  int vertex_index;
  int normal_index;
  int texcoord_index;
} index_t;

typedef struct {
  std::vector<index_t> indices;
  std::vector<unsigned char>
      num_vertices;               // The number of vertices per face. Up to 255.
  std::vector<int> material_ids;  // per-face material ID
  std::vector<tag_t> tags;        // SubD tag
} mesh_t;

typedef struct {
  std::string name;
  mesh_t mesh;
  std::vector<BlendShape> blendshapes;
} shape_t;

// Vertex attributes
typedef struct {
  std::vector<float> vertices;   // 'v'
  std::vector<float> normals;    // 'vn'
  std::vector<float> texcoords;  // 'vt'
} attrib_t;

typedef struct callback_t_ {
  void (*vertex_cb)(void *user_data, float x, float y, float z);
  void (*normal_cb)(void *user_data, float x, float y, float z);
  void (*texcoord_cb)(void *user_data, float x, float y);
  // -2147483648 will be passed for undefined index
  void (*index_cb)(void *user_data, int v_idx, int vn_idx, int vt_idx);
  // `name` material name, `materialId` = the array index of material_t[]. -1 if
  // a material not found in .mtl
  void (*usemtl_cb)(void *user_data, const char *name, int materialId);
  // `materials` = parsed material data.
  void (*mtllib_cb)(void *user_data, const material_t *materials,
                    int num_materials);
  // There may be multiple group names
  void (*group_cb)(void *user_data, const char **names, int num_names);
  void (*object_cb)(void *user_data, const char *name);

  callback_t_()
      : vertex_cb(NULL),
        normal_cb(NULL),
        texcoord_cb(NULL),
        index_cb(NULL),
        usemtl_cb(NULL),
        mtllib_cb(NULL),
        group_cb(NULL),
        object_cb(NULL) {}

} callback_t;

class MaterialReader {
 public:
  MaterialReader() {}
  virtual ~MaterialReader();

  virtual bool operator()(const std::string &matId,
                          std::vector<material_t> *materials,
                          std::map<std::string, int> *matMap,
                          std::string *err, std::istream *materialStream) = 0;
};

class MaterialFileReader : public MaterialReader {
 public:
  explicit MaterialFileReader(const std::string &mtl_basepath)
      : m_mtlBasePath(mtl_basepath) {}
  virtual ~MaterialFileReader() {}
  virtual bool operator()(const std::string &matId,
                          std::vector<material_t> *materials,
                          std::map<std::string, int> *matMap, std::string *err, std::istream *materialStream);

 private:
  std::string m_mtlBasePath;
};

/// Loads .obj from a file.
/// 'attrib', 'shapes' and 'materials' will be filled with parsed shape data
/// 'shapes' will be filled with parsed shape data
/// Returns true when loading .obj become success.
/// Returns warning and error message into `err`
/// 'mtl_basepath' is optional, and used for base path for .mtl file.
/// 'triangulate' is optional, and used whether triangulate polygon face in .obj
/// or not.
bool LoadObj(attrib_t *attrib, std::vector<shape_t> *shapes,
             std::vector<material_t> *materials, std::string *err,
             const char *filename, const char *mtl_basepath = NULL,
             bool triangulate = true);

/// Loads .obj from a file with custom user callback.
/// .mtl is loaded as usual and parsed material_t data will be passed to
/// `callback.mtllib_cb`.
/// Returns true when loading .obj/.mtl become success.
/// Returns warning and error message into `err`
/// 'mtl_basepath' is optional, and used for base path for .mtl file.
/// 'triangulate' is optional, and used whether triangulate polygon face in .obj
/// or not.
bool LoadObjWithCallback(void *user_data, const callback_t &callback,
                         std::string *err, std::istream *inStream,
                         MaterialReader *readMatFn, std::istream *matStream);

/// Loads object from a std::istream, uses GetMtlIStreamFn to retrieve
/// std::istream for materials.
/// Returns true when loading .obj become success.
/// Returns warning and error message into `err`
bool LoadObj(attrib_t *attrib, std::vector<shape_t> *shapes,
             std::vector<material_t> *materials, std::string *err,
             std::istream *inStream, MaterialReader *readMatFn,
             bool triangulate = true);

/// Loads materials into std::map
void LoadMtl(std::map<std::string, int> *material_map,
             std::vector<material_t> *materials, std::istream *inStream);

}  // namespace tinyobj

//#define TINYOBJLOADER_IMPLEMENTATION
#ifdef TINYOBJLOADER_IMPLEMENTATION
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cctype>
#include <utility>

#include <fstream>
#include <sstream>

namespace tinyobj {

MaterialReader::~MaterialReader() {}

#define TINYOBJ_SSCANF_BUFFER_SIZE (4096)

struct vertex_index {
  int v_idx, vt_idx, vn_idx;
  vertex_index() : v_idx(-1), vt_idx(-1), vn_idx(-1) {}
  explicit vertex_index(int idx) : v_idx(idx), vt_idx(idx), vn_idx(idx) {}
  vertex_index(int vidx, int vtidx, int vnidx)
      : v_idx(vidx), vt_idx(vtidx), vn_idx(vnidx) {}
};

struct tag_sizes {
  tag_sizes() : num_ints(0), num_floats(0), num_strings(0) {}
  int num_ints;
  int num_floats;
  int num_strings;
};

struct obj_shape {
  std::vector<float> v;
  std::vector<float> vn;
  std::vector<float> vt;
};

#define IS_SPACE(x) (((x) == ' ') || ((x) == '\t'))
#define IS_DIGIT(x) \
  (static_cast<unsigned int>((x) - '0') < static_cast<unsigned int>(10))
#define IS_NEW_LINE(x) (((x) == '\r') || ((x) == '\n') || ((x) == '\0'))

// Make index zero-base, and also support relative index.
static inline int fixIndex(int idx, int n) {
  if (idx > 0) return idx - 1;
  if (idx == 0) return 0;
  return n + idx;  // negative value = relative
}

static inline std::string parseString(const char **token) {
  std::string s;
  (*token) += strspn((*token), " \t");
  size_t e = strcspn((*token), " \t\r");
  s = std::string((*token), &(*token)[e]);
  (*token) += e;
  return s;
}

static inline int parseInt(const char **token) {
  (*token) += strspn((*token), " \t");
  int i = atoi((*token));
  (*token) += strcspn((*token), " \t\r");
  return i;
}

// Tries to parse a floating point number located at s.
//
// s_end should be a location in the string where reading should absolutely
// stop. For example at the end of the string, to prevent buffer overflows.
//
// Parses the following EBNF grammar:
//   sign    = "+" | "-" ;
//   END     = ? anything not in digit ?
//   digit   = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
//   integer = [sign] , digit , {digit} ;
//   decimal = integer , ["." , integer] ;
//   float   = ( decimal , END ) | ( decimal , ("E" | "e") , integer , END ) ;
//
//  Valid strings are for example:
//   -0  +3.1417e+2  -0.0E-3  1.0324  -1.41   11e2
//
// If the parsing is a success, result is set to the parsed value and true
// is returned.
//
// The function is greedy and will parse until any of the following happens:
//  - a non-conforming character is encountered.
//  - s_end is reached.
//
// The following situations triggers a failure:
//  - s >= s_end.
//  - parse failure.
//
static bool tryParseDouble(const char *s, const char *s_end, double *result) {
  if (s >= s_end) {
    return false;
  }

  double mantissa = 0.0;
  // This exponent is base 2 rather than 10.
  // However the exponent we parse is supposed to be one of ten,
  // thus we must take care to convert the exponent/and or the
  // mantissa to a * 2^E, where a is the mantissa and E is the
  // exponent.
  // To get the final double we will use ldexp, it requires the
  // exponent to be in base 2.
  int exponent = 0;

  // NOTE: THESE MUST BE DECLARED HERE SINCE WE ARE NOT ALLOWED
  // TO JUMP OVER DEFINITIONS.
  char sign = '+';
  char exp_sign = '+';
  char const *curr = s;

  // How many characters were read in a loop.
  int read = 0;
  // Tells whether a loop terminated due to reaching s_end.
  bool end_not_reached = false;

  /*
          BEGIN PARSING.
  */

  // Find out what sign we've got.
  if (*curr == '+' || *curr == '-') {
    sign = *curr;
    curr++;
  } else if (IS_DIGIT(*curr)) { /* Pass through. */
  } else {
    goto fail;
  }

  // Read the integer part.
  end_not_reached = (curr != s_end);
  while (end_not_reached && IS_DIGIT(*curr)) {
    mantissa *= 10;
    mantissa += static_cast<int>(*curr - 0x30);
    curr++;
    read++;
    end_not_reached = (curr != s_end);
  }

  // We must make sure we actually got something.
  if (read == 0) goto fail;
  // We allow numbers of form "#", "###" etc.
  if (!end_not_reached) goto assemble;

  // Read the decimal part.
  if (*curr == '.') {
    curr++;
    read = 1;
    end_not_reached = (curr != s_end);
    while (end_not_reached && IS_DIGIT(*curr)) {
      // NOTE: Don't use powf here, it will absolutely murder precision.
      mantissa += static_cast<int>(*curr - 0x30) * pow(10.0, -read);
      read++;
      curr++;
      end_not_reached = (curr != s_end);
    }
  } else if (*curr == 'e' || *curr == 'E') {
  } else {
    goto assemble;
  }

  if (!end_not_reached) goto assemble;

  // Read the exponent part.
  if (*curr == 'e' || *curr == 'E') {
    curr++;
    // Figure out if a sign is present and if it is.
    end_not_reached = (curr != s_end);
    if (end_not_reached && (*curr == '+' || *curr == '-')) {
      exp_sign = *curr;
      curr++;
    } else if (IS_DIGIT(*curr)) { /* Pass through. */
    } else {
      // Empty E is not allowed.
      goto fail;
    }

    read = 0;
    end_not_reached = (curr != s_end);
    while (end_not_reached && IS_DIGIT(*curr)) {
      exponent *= 10;
      exponent += static_cast<int>(*curr - 0x30);
      curr++;
      read++;
      end_not_reached = (curr != s_end);
    }
    exponent *= (exp_sign == '+' ? 1 : -1);
    if (read == 0) goto fail;
  }

assemble:
  *result =
      (sign == '+' ? 1 : -1) * ldexp(mantissa * pow(5.0, exponent), exponent);
  return true;
fail:
  return false;
}

static inline float parseFloat(const char **token) {
  (*token) += strspn((*token), " \t");
#ifdef TINY_OBJ_LOADER_OLD_FLOAT_PARSER
  float f = static_cast<float>(atof(*token));
  (*token) += strcspn((*token), " \t\r");
#else
  const char *end = (*token) + strcspn((*token), " \t\r");
  double val = 0.0;
  tryParseDouble((*token), end, &val);
  float f = static_cast<float>(val);
  (*token) = end;
#endif
  return f;
}

static inline void parseFloat2(float *x, float *y, const char **token) {
  (*x) = parseFloat(token);
  (*y) = parseFloat(token);
}

static inline void parseFloat3(float *x, float *y, float *z,
                               const char **token) {
  (*x) = parseFloat(token);
  (*y) = parseFloat(token);
  (*z) = parseFloat(token);
}

static tag_sizes parseTagTriple(const char **token) {
  tag_sizes ts;

  ts.num_ints = atoi((*token));
  (*token) += strcspn((*token), "/ \t\r");
  if ((*token)[0] != '/') {
    return ts;
  }
  (*token)++;

  ts.num_floats = atoi((*token));
  (*token) += strcspn((*token), "/ \t\r");
  if ((*token)[0] != '/') {
    return ts;
  }
  (*token)++;

  ts.num_strings = atoi((*token));
  (*token) += strcspn((*token), "/ \t\r") + 1;

  return ts;
}

// Parse triples with index offsets: i, i/j/k, i//k, i/j
static vertex_index parseTriple(const char **token, int vsize, int vnsize,
                                int vtsize) {
  vertex_index vi(-1);

  vi.v_idx = fixIndex(atoi((*token)), vsize);
  (*token) += strcspn((*token), "/ \t\r");
  if ((*token)[0] != '/') {
    return vi;
  }
  (*token)++;

  // i//k
  if ((*token)[0] == '/') {
    (*token)++;
    vi.vn_idx = fixIndex(atoi((*token)), vnsize);
    (*token) += strcspn((*token), "/ \t\r");
    return vi;
  }

  // i/j/k or i/j
  vi.vt_idx = fixIndex(atoi((*token)), vtsize);
  (*token) += strcspn((*token), "/ \t\r");
  if ((*token)[0] != '/') {
    return vi;
  }

  // i/j/k
  (*token)++;  // skip '/'
  vi.vn_idx = fixIndex(atoi((*token)), vnsize);
  (*token) += strcspn((*token), "/ \t\r");
  return vi;
}

// Parse raw triples: i, i/j/k, i//k, i/j
static vertex_index parseRawTriple(const char **token) {
  vertex_index vi(
      static_cast<int>(0x80000000));  // 0x80000000 = -2147483648 = invalid

  vi.v_idx = atoi((*token));
  (*token) += strcspn((*token), "/ \t\r");
  if ((*token)[0] != '/') {
    return vi;
  }
  (*token)++;

  // i//k
  if ((*token)[0] == '/') {
    (*token)++;
    vi.vn_idx = atoi((*token));
    (*token) += strcspn((*token), "/ \t\r");
    return vi;
  }

  // i/j/k or i/j
  vi.vt_idx = atoi((*token));
  (*token) += strcspn((*token), "/ \t\r");
  if ((*token)[0] != '/') {
    return vi;
  }

  // i/j/k
  (*token)++;  // skip '/'
  vi.vn_idx = atoi((*token));
  (*token) += strcspn((*token), "/ \t\r");
  return vi;
}

static void InitMaterial(material_t *material) {
  material->name = "";
  material->ambient_texname = "";
  material->diffuse_texname = "";
  material->specular_texname = "";
  material->specular_highlight_texname = "";
  material->bump_texname = "";
  material->displacement_texname = "";
  material->alpha_texname = "";
  for (int i = 0; i < 3; i++) {
    material->ambient[i] = 0.f;
    material->diffuse[i] = 0.f;
    material->specular[i] = 0.f;
    material->transmittance[i] = 0.f;
    material->emission[i] = 0.f;
  }
  material->illum = 0;
  material->dissolve = 1.f;
  material->shininess = 1.f;
  material->ior = 1.f;
  material->unknown_parameter.clear();
}

static bool exportFaceGroupToShape(
    shape_t *shape, const std::vector<std::vector<vertex_index> > &faceGroup,
    const std::vector<tag_t> &tags, const int material_id,
    const std::string &name, bool triangulate) {
  if (faceGroup.empty()) {
    return false;
  }

  // Flatten vertices and indices
  for (size_t i = 0; i < faceGroup.size(); i++) {
    const std::vector<vertex_index> &face = faceGroup[i];

    vertex_index i0 = face[0];
    vertex_index i1(-1);
    vertex_index i2 = face[1];

    size_t npolys = face.size();

    if (triangulate) {
      // Polygon -> triangle fan conversion
      for (size_t k = 2; k < npolys; k++) {
        i1 = i2;
        i2 = face[k];

        index_t idx0, idx1, idx2;
        idx0.vertex_index = i0.v_idx;
        idx0.normal_index = i0.vn_idx;
        idx0.texcoord_index = i0.vt_idx;
        idx1.vertex_index = i1.v_idx;
        idx1.normal_index = i1.vn_idx;
        idx1.texcoord_index = i1.vt_idx;
        idx2.vertex_index = i2.v_idx;
        idx2.normal_index = i2.vn_idx;
        idx2.texcoord_index = i2.vt_idx;

        shape->mesh.indices.push_back(idx0);
        shape->mesh.indices.push_back(idx1);
        shape->mesh.indices.push_back(idx2);

        shape->mesh.num_vertices.push_back(3);
        shape->mesh.material_ids.push_back(material_id);
      }
    } else {
      for (size_t k = 0; k < npolys; k++) {
        index_t idx;
        idx.vertex_index = face[k].v_idx;
        idx.normal_index = face[k].vn_idx;
        idx.texcoord_index = face[k].vt_idx;
        shape->mesh.indices.push_back(idx);
      }

      shape->mesh.num_vertices.push_back(static_cast<unsigned char>(npolys));
      shape->mesh.material_ids.push_back(material_id);  // per face
    }
  }

  shape->name = name;
  shape->mesh.tags = tags;

  return true;
}

void LoadMtl(std::map<std::string, int> *material_map,
             std::vector<material_t> *materials, std::istream *inStream) {
  // Create a default material anyway.
  material_t material;
  InitMaterial(&material);

  size_t maxchars = 8192;           // Alloc enough size.
  std::vector<char> buf(maxchars);  // Alloc enough size.
  while (inStream->peek() != -1) {
    inStream->getline(&buf[0], static_cast<std::streamsize>(maxchars));

    std::string linebuf(&buf[0]);

    // Trim newline '\r\n' or '\n'
    if (linebuf.size() > 0) {
      if (linebuf[linebuf.size() - 1] == '\n')
        linebuf.erase(linebuf.size() - 1);
    }
    if (linebuf.size() > 0) {
      if (linebuf[linebuf.size() - 1] == '\r')
        linebuf.erase(linebuf.size() - 1);
    }

    // Skip if empty line.
    if (linebuf.empty()) {
      continue;
    }

    // Skip leading space.
    const char *token = linebuf.c_str();
    token += strspn(token, " \t");

    assert(token);
    if (token[0] == '\0') continue;  // empty line

    if (token[0] == '#') continue;  // comment line

    // new mtl
    if ((0 == strncmp(token, "newmtl", 6)) && IS_SPACE((token[6]))) {
      // flush previous material.
      if (!material.name.empty()) {
        material_map->insert(std::pair<std::string, int>(
            material.name, static_cast<int>(materials->size())));
        materials->push_back(material);
      }

      // initial temporary material
      InitMaterial(&material);

      // set new mtl name
      char namebuf[TINYOBJ_SSCANF_BUFFER_SIZE];
      token += 7;
#ifdef _MSC_VER
      sscanf_s(token, "%s", namebuf, (unsigned)_countof(namebuf));
#else
      sscanf(token, "%s", namebuf);
#endif
      material.name = namebuf;
      continue;
    }

    // ambient
    if (token[0] == 'K' && token[1] == 'a' && IS_SPACE((token[2]))) {
      token += 2;
      float r, g, b;
      parseFloat3(&r, &g, &b, &token);
      material.ambient[0] = r;
      material.ambient[1] = g;
      material.ambient[2] = b;
      continue;
    }

    // diffuse
    if (token[0] == 'K' && token[1] == 'd' && IS_SPACE((token[2]))) {
      token += 2;
      float r, g, b;
      parseFloat3(&r, &g, &b, &token);
      material.diffuse[0] = r;
      material.diffuse[1] = g;
      material.diffuse[2] = b;
      continue;
    }

    // specular
    if (token[0] == 'K' && token[1] == 's' && IS_SPACE((token[2]))) {
      token += 2;
      float r, g, b;
      parseFloat3(&r, &g, &b, &token);
      material.specular[0] = r;
      material.specular[1] = g;
      material.specular[2] = b;
      continue;
    }

    // transmittance
    if (token[0] == 'K' && token[1] == 't' && IS_SPACE((token[2]))) {
      token += 2;
      float r, g, b;
      parseFloat3(&r, &g, &b, &token);
      material.transmittance[0] = r;
      material.transmittance[1] = g;
      material.transmittance[2] = b;
      continue;
    }

    // ior(index of refraction)
    if (token[0] == 'N' && token[1] == 'i' && IS_SPACE((token[2]))) {
      token += 2;
      material.ior = parseFloat(&token);
      continue;
    }

    // emission
    if (token[0] == 'K' && token[1] == 'e' && IS_SPACE(token[2])) {
      token += 2;
      float r, g, b;
      parseFloat3(&r, &g, &b, &token);
      material.emission[0] = r;
      material.emission[1] = g;
      material.emission[2] = b;
      continue;
    }

    // shininess
    if (token[0] == 'N' && token[1] == 's' && IS_SPACE(token[2])) {
      token += 2;
      material.shininess = parseFloat(&token);
      continue;
    }

    // illum model
    if (0 == strncmp(token, "illum", 5) && IS_SPACE(token[5])) {
      token += 6;
      material.illum = parseInt(&token);
      continue;
    }

    // dissolve
    if ((token[0] == 'd' && IS_SPACE(token[1]))) {
      token += 1;
      material.dissolve = parseFloat(&token);
      continue;
    }
    if (token[0] == 'T' && token[1] == 'r' && IS_SPACE(token[2])) {
      token += 2;
      // Invert value of Tr(assume Tr is in range [0, 1])
      material.dissolve = 1.0f - parseFloat(&token);
      continue;
    }

    // ambient texture
    if ((0 == strncmp(token, "map_Ka", 6)) && IS_SPACE(token[6])) {
      token += 7;
      material.ambient_texname = token;
      continue;
    }

    // diffuse texture
    if ((0 == strncmp(token, "map_Kd", 6)) && IS_SPACE(token[6])) {
      token += 7;
      material.diffuse_texname = token;
      continue;
    }

    // specular texture
    if ((0 == strncmp(token, "map_Ks", 6)) && IS_SPACE(token[6])) {
      token += 7;
      material.specular_texname = token;
      continue;
    }

    // specular highlight texture
    if ((0 == strncmp(token, "map_Ns", 6)) && IS_SPACE(token[6])) {
      token += 7;
      material.specular_highlight_texname = token;
      continue;
    }

    // bump texture
    if ((0 == strncmp(token, "map_bump", 8)) && IS_SPACE(token[8])) {
      token += 9;
      material.bump_texname = token;
      continue;
    }

    // alpha texture
    if ((0 == strncmp(token, "map_d", 5)) && IS_SPACE(token[5])) {
      token += 6;
      material.alpha_texname = token;
      continue;
    }

    // bump texture
    if ((0 == strncmp(token, "bump", 4)) && IS_SPACE(token[4])) {
      token += 5;
      material.bump_texname = token;
      continue;
    }

    // displacement texture
    if ((0 == strncmp(token, "disp", 4)) && IS_SPACE(token[4])) {
      token += 5;
      material.displacement_texname = token;
      continue;
    }

    // unknown parameter
    const char *_space = strchr(token, ' ');
    if (!_space) {
      _space = strchr(token, '\t');
    }
    if (_space) {
      std::ptrdiff_t len = _space - token;
      std::string key(token, static_cast<size_t>(len));
      std::string value = _space + 1;
      material.unknown_parameter.insert(
          std::pair<std::string, std::string>(key, value));
    }
  }
  // flush last material.
  material_map->insert(std::pair<std::string, int>(
      material.name, static_cast<int>(materials->size())));
  materials->push_back(material);
}

bool MaterialFileReader::operator()(const std::string &matId,
                                    std::vector<material_t> *materials,
                                    std::map<std::string, int> *matMap,
                                    std::string *err, std::istream *materialStream) {
  std::string filepath;

  if (!m_mtlBasePath.empty()) {
    filepath = std::string(m_mtlBasePath) + matId;
  } else {
    filepath = matId;
  }

  std::ifstream matIStream(filepath.c_str());
  LoadMtl(matMap, materials, materialStream);
  /*if (!matIStream) {
    std::stringstream ss;
    ss << "WARN: Material file [ " << filepath
       << " ] not found. Created a default material.";
    if (err) {
      (*err) += ss.str();
    }
  }*/
  return true;
}
/*
bool LoadObj(attrib_t *attrib, std::vector<shape_t> *shapes,
             std::vector<material_t> *materials, std::string *err,
             const char *filename, const char *mtl_basepath,
             bool trianglulate) {
  attrib->vertices.clear();
  attrib->normals.clear();
  attrib->texcoords.clear();
  shapes->clear();

  std::stringstream errss;

  std::ifstream ifs(filename);
  if (!ifs) {
    errss << "Cannot open file [" << filename << "]" << std::endl;
    if (err) {
      (*err) = errss.str();
    }
    return false;
  }

  std::string basePath;
  if (mtl_basepath) {
    basePath = mtl_basepath;
  }
  MaterialFileReader matFileReader(basePath);

  return LoadObj(attrib, shapes, materials, err, &ifs, &matFileReader,
                 trianglulate);
}

bool LoadObj(attrib_t *attrib, std::vector<shape_t> *shapes,
             std::vector<material_t> *materials, std::string *err,
             std::istream *inStream, MaterialReader *readMatFn,
             bool triangulate) {
  std::stringstream errss;

  std::vector<float> v;
  std::vector<float> vn;
  std::vector<float> vt;
  std::vector<tag_t> tags;
  std::vector<std::vector<vertex_index> > faceGroup;
  std::string name;

  // material
  std::map<std::string, int> material_map;
  int material = -1;

  shape_t shape;

  int maxchars = 8192;                                   // Alloc enough size.
  std::vector<char> buf(static_cast<size_t>(maxchars));  // Alloc enough size.
  while (inStream->peek() != -1) {
    inStream->getline(&buf[0], maxchars);

    std::string linebuf(&buf[0]);

    // Trim newline '\r\n' or '\n'
    if (linebuf.size() > 0) {
      if (linebuf[linebuf.size() - 1] == '\n')
        linebuf.erase(linebuf.size() - 1);
    }
    if (linebuf.size() > 0) {
      if (linebuf[linebuf.size() - 1] == '\r')
        linebuf.erase(linebuf.size() - 1);
    }

    // Skip if empty line.
    if (linebuf.empty()) {
      continue;
    }

    // Skip leading space.
    const char *token = linebuf.c_str();
    token += strspn(token, " \t");

    assert(token);
    if (token[0] == '\0') continue;  // empty line

    if (token[0] == '#') continue;  // comment line

    // vertex
    if (token[0] == 'v' && IS_SPACE((token[1]))) {
      token += 2;
      float x, y, z;
      parseFloat3(&x, &y, &z, &token);
      v.push_back(x);
      v.push_back(y);
      v.push_back(z);
      continue;
    }

    // normal
    if (token[0] == 'v' && token[1] == 'n' && IS_SPACE((token[2]))) {
      token += 3;
      float x, y, z;
      parseFloat3(&x, &y, &z, &token);
      vn.push_back(x);
      vn.push_back(y);
      vn.push_back(z);
      continue;
    }

    // texcoord
    if (token[0] == 'v' && token[1] == 't' && IS_SPACE((token[2]))) {
      token += 3;
      float x, y;
      parseFloat2(&x, &y, &token);
      vt.push_back(x);
      vt.push_back(y);
      continue;
    }

    // face
    if (token[0] == 'f' && IS_SPACE((token[1]))) {
      token += 2;
      token += strspn(token, " \t");

      std::vector<vertex_index> face;
      face.reserve(3);

      while (!IS_NEW_LINE(token[0])) {
        vertex_index vi = parseTriple(&token, static_cast<int>(v.size() / 3),
                                      static_cast<int>(vn.size() / 3),
                                      static_cast<int>(vt.size() / 2));
        face.push_back(vi);
        size_t n = strspn(token, " \t\r");
        token += n;
      }

      // replace with emplace_back + std::move on C++11
      faceGroup.push_back(std::vector<vertex_index>());
      faceGroup[faceGroup.size() - 1].swap(face);

      continue;
    }

    // use mtl
    if ((0 == strncmp(token, "usemtl", 6)) && IS_SPACE((token[6]))) {
      char namebuf[TINYOBJ_SSCANF_BUFFER_SIZE];
      token += 7;
#ifdef _MSC_VER
      sscanf_s(token, "%s", namebuf, (unsigned)_countof(namebuf));
#else
      sscanf(token, "%s", namebuf);
#endif

      int newMaterialId = -1;
      if (material_map.find(namebuf) != material_map.end()) {
        newMaterialId = material_map[namebuf];
      } else {
        // { error!! material not found }
      }

      if (newMaterialId != material) {
        // Create per-face material
        exportFaceGroupToShape(&shape, faceGroup, tags, material, name,
                               triangulate);
        faceGroup.clear();
        material = newMaterialId;
      }

      continue;
    }

    // load mtl
    if ((0 == strncmp(token, "mtllib", 6)) && IS_SPACE((token[6]))) {
      char namebuf[TINYOBJ_SSCANF_BUFFER_SIZE];
      token += 7;
#ifdef _MSC_VER
      sscanf_s(token, "%s", namebuf, (unsigned)_countof(namebuf));
#else
      sscanf(token, "%s", namebuf);
#endif

      std::string err_mtl;
      bool ok = (*readMatFn)(namebuf, materials, &material_map, &err_mtl);
      if (err) {
        (*err) += err_mtl;
      }

      if (!ok) {
        faceGroup.clear();  // for safety
        return false;
      }

      continue;
    }

    // group name
    if (token[0] == 'g' && IS_SPACE((token[1]))) {
      // flush previous face group.
      bool ret = exportFaceGroupToShape(&shape, faceGroup, tags, material, name,
                                        triangulate);
      if (ret) {
        shapes->push_back(shape);
      }

      shape = shape_t();

      // material = -1;
      faceGroup.clear();

      std::vector<std::string> names;
      names.reserve(2);

      while (!IS_NEW_LINE(token[0])) {
        std::string str = parseString(&token);
        names.push_back(str);
        token += strspn(token, " \t\r");  // skip tag
      }

      assert(names.size() > 0);

      // names[0] must be 'g', so skip the 0th element.
      if (names.size() > 1) {
        name = names[1];
      } else {
        name = "";
      }

      continue;
    }

    // object name
    if (token[0] == 'o' && IS_SPACE((token[1]))) {
      // flush previous face group.
      bool ret = exportFaceGroupToShape(&shape, faceGroup, tags, material, name,
                                        triangulate);
      if (ret) {
        shapes->push_back(shape);
      }

      // material = -1;
      faceGroup.clear();
      shape = shape_t();

      // @todo { multiple object name? }
      char namebuf[TINYOBJ_SSCANF_BUFFER_SIZE];
      token += 2;
#ifdef _MSC_VER
      sscanf_s(token, "%s", namebuf, (unsigned)_countof(namebuf));
#else
      sscanf(token, "%s", namebuf);
#endif
      name = std::string(namebuf);

      continue;
    }

    if (token[0] == 't' && IS_SPACE(token[1])) {
      tag_t tag;

      char namebuf[4096];
      token += 2;
#ifdef _MSC_VER
      sscanf_s(token, "%s", namebuf, (unsigned)_countof(namebuf));
#else
      sscanf(token, "%s", namebuf);
#endif
      tag.name = std::string(namebuf);

      token += tag.name.size() + 1;

      tag_sizes ts = parseTagTriple(&token);

      tag.intValues.resize(static_cast<size_t>(ts.num_ints));

      for (size_t i = 0; i < static_cast<size_t>(ts.num_ints); ++i) {
        tag.intValues[i] = atoi(token);
        token += strcspn(token, "/ \t\r") + 1;
      }

      tag.floatValues.resize(static_cast<size_t>(ts.num_floats));
      for (size_t i = 0; i < static_cast<size_t>(ts.num_floats); ++i) {
        tag.floatValues[i] = parseFloat(&token);
        token += strcspn(token, "/ \t\r") + 1;
      }

      tag.stringValues.resize(static_cast<size_t>(ts.num_strings));
      for (size_t i = 0; i < static_cast<size_t>(ts.num_strings); ++i) {
        char stringValueBuffer[4096];

#ifdef _MSC_VER
        sscanf_s(token, "%s", stringValueBuffer,
                 (unsigned)_countof(stringValueBuffer));
#else
        sscanf(token, "%s", stringValueBuffer);
#endif
        tag.stringValues[i] = stringValueBuffer;
        token += tag.stringValues[i].size() + 1;
      }

      tags.push_back(tag);
    }

    // Ignore unknown command.
  }

  bool ret = exportFaceGroupToShape(&shape, faceGroup, tags, material, name,
                                    triangulate);
  if (ret) {
    shapes->push_back(shape);
  }
  faceGroup.clear();  // for safety

  if (err) {
    (*err) += errss.str();
  }

  attrib->vertices.swap(v);
  attrib->normals.swap(vn);
  attrib->texcoords.swap(vt);

  return true;
}
*/
bool LoadObjWithCallback(void *user_data, const callback_t &callback,
                         std::string *err, std::istream *inStream,
                         MaterialReader *readMatFn, std::istream *matStream) {
  std::stringstream errss;

  // material
  std::map<std::string, int> material_map;
  int materialId = -1;  // -1 = invalid

  int maxchars = 8192;                                   // Alloc enough size.
  std::vector<char> buf(static_cast<size_t>(maxchars));  // Alloc enough size.
  while (inStream->peek() != -1) {
    inStream->getline(&buf[0], maxchars);

    std::string linebuf(&buf[0]);

    // Trim newline '\r\n' or '\n'
    if (linebuf.size() > 0) {
      if (linebuf[linebuf.size() - 1] == '\n')
        linebuf.erase(linebuf.size() - 1);
    }
    if (linebuf.size() > 0) {
      if (linebuf[linebuf.size() - 1] == '\r')
        linebuf.erase(linebuf.size() - 1);
    }

    // Skip if empty line.
    if (linebuf.empty()) {
      continue;
    }

    // Skip leading space.
    const char *token = linebuf.c_str();
    token += strspn(token, " \t");

    assert(token);
    if (token[0] == '\0') continue;  // empty line

    if (token[0] == '#') continue;  // comment line

    // vertex
    if (token[0] == 'v' && IS_SPACE((token[1]))) {
      token += 2;
      float x, y, z;
      parseFloat3(&x, &y, &z, &token);
      if (callback.vertex_cb) {
        callback.vertex_cb(user_data, x, y, z);
      }
      continue;
    }

    // normal
    if (token[0] == 'v' && token[1] == 'n' && IS_SPACE((token[2]))) {
      token += 3;
      float x, y, z;
      parseFloat3(&x, &y, &z, &token);
      if (callback.normal_cb) {
        callback.normal_cb(user_data, x, y, z);
      }
      continue;
    }

    // texcoord
    if (token[0] == 'v' && token[1] == 't' && IS_SPACE((token[2]))) {
      token += 3;
      float x, y;
      parseFloat2(&x, &y, &token);
      if (callback.texcoord_cb) {
        callback.texcoord_cb(user_data, x, y);
      }
      continue;
    }

    // face
    if (token[0] == 'f' && IS_SPACE((token[1]))) {
      token += 2;
      token += strspn(token, " \t");

      while (!IS_NEW_LINE(token[0])) {
        vertex_index vi = parseRawTriple(&token);
        if (callback.index_cb) {
          callback.index_cb(user_data, vi.v_idx, vi.vn_idx, vi.vt_idx);
        }
        size_t n = strspn(token, " \t\r");
        token += n;
      }

      continue;
    }

    // use mtl
    if ((0 == strncmp(token, "usemtl", 6)) && IS_SPACE((token[6]))) {
      char namebuf[TINYOBJ_SSCANF_BUFFER_SIZE];
      token += 7;
#ifdef _MSC_VER
      sscanf_s(token, "%s", namebuf, (unsigned)_countof(namebuf));
#else
      sscanf(token, "%s", namebuf);
#endif

      int newMaterialId = -1;
      if (material_map.find(namebuf) != material_map.end()) {
        newMaterialId = material_map[namebuf];
      } else {
        // { error!! material not found }
      }

      if (newMaterialId != materialId) {
        materialId = newMaterialId;
      }

      if (callback.usemtl_cb) {
        callback.usemtl_cb(user_data, namebuf, materialId);
      }

      continue;
    }

    // load mtl
    if ((0 == strncmp(token, "mtllib", 6)) && IS_SPACE((token[6]))) {
      char namebuf[TINYOBJ_SSCANF_BUFFER_SIZE];
      token += 7;
#ifdef _MSC_VER
      sscanf_s(token, "%s", namebuf, (unsigned)_countof(namebuf));
#else
      sscanf(token, "%s", namebuf);
#endif

      std::string err_mtl;
      std::vector<material_t> materials;
      bool ok = (*readMatFn)(namebuf, &materials, &material_map, &err_mtl, matStream);
      if (err) {
        (*err) += err_mtl;
      }

      if (!ok) {
        return false;
      }

      if (callback.mtllib_cb) {
        callback.mtllib_cb(user_data, &materials.at(0),
                           static_cast<int>(materials.size()));
      }

      continue;
    }

    // group name
    if (token[0] == 'g' && IS_SPACE((token[1]))) {
      std::vector<std::string> names;
      names.reserve(2);

      while (!IS_NEW_LINE(token[0])) {
        std::string str = parseString(&token);
        names.push_back(str);
        token += strspn(token, " \t\r");  // skip tag
      }

      assert(names.size() > 0);

      std::string name;
      // names[0] must be 'g', so skip the 0th element.
      if (names.size() > 1) {
        name = names[1];
      } else {
        name = "";
      }

      if (callback.group_cb) {
        if (names.size() > 1) {
          // create const char* array.
          std::vector<const char *> tmp(names.size() - 1);
          for (size_t j = 0; j < tmp.size(); j++) {
            tmp[j] = names[j + 1].c_str();
          }
          callback.group_cb(user_data, &tmp.at(0),
                            static_cast<int>(tmp.size()));

        } else {
          callback.group_cb(user_data, NULL, 0);
        }

        continue;
      }

      // object name
      if (token[0] == 'o' && IS_SPACE((token[1]))) {
        // @todo { multiple object name? }
        char namebuf[TINYOBJ_SSCANF_BUFFER_SIZE];
        token += 2;
#ifdef _MSC_VER
        sscanf_s(token, "%s", namebuf, (unsigned)_countof(namebuf));
#else
        sscanf(token, "%s", namebuf);
#endif
        name = std::string(namebuf);

        if (callback.object_cb) {
          callback.object_cb(user_data, name.c_str());
        }

        continue;
      }

#if 0  // @todo
    if (token[0] == 't' && IS_SPACE(token[1])) {
      tag_t tag;

      char namebuf[4096];
      token += 2;
#ifdef _MSC_VER
      sscanf_s(token, "%s", namebuf, (unsigned)_countof(namebuf));
#else
      sscanf(token, "%s", namebuf);
#endif
      tag.name = std::string(namebuf);

      token += tag.name.size() + 1;

      tag_sizes ts = parseTagTriple(&token);

      tag.intValues.resize(static_cast<size_t>(ts.num_ints));

      for (size_t i = 0; i < static_cast<size_t>(ts.num_ints); ++i) {
        tag.intValues[i] = atoi(token);
        token += strcspn(token, "/ \t\r") + 1;
      }

      tag.floatValues.resize(static_cast<size_t>(ts.num_floats));
      for (size_t i = 0; i < static_cast<size_t>(ts.num_floats); ++i) {
        tag.floatValues[i] = parseFloat(&token);
        token += strcspn(token, "/ \t\r") + 1;
      }

      tag.stringValues.resize(static_cast<size_t>(ts.num_strings));
      for (size_t i = 0; i < static_cast<size_t>(ts.num_strings); ++i) {
        char stringValueBuffer[4096];

#ifdef _MSC_VER
        sscanf_s(token, "%s", stringValueBuffer,
                 (unsigned)_countof(stringValueBuffer));
#else
        sscanf(token, "%s", stringValueBuffer);
#endif
        tag.stringValues[i] = stringValueBuffer;
        token += tag.stringValues[i].size() + 1;
      }

      tags.push_back(tag);
#endif
    }

    // Ignore unknown command.
  }

  if (err) {
    (*err) += errss.str();
  }

  return true;
}
}  // namespace tinyobj

#endif

#endif  // TINY_OBJ_LOADER_H_
