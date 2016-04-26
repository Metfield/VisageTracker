#ifndef HELPER_H_
#define HELPER_H_

#include <vector>
#include <GLES2/gl2.h>

// Super simple class for a vector of size 3
// Intended for use with vertices, normals and tangents.
class float3 {

public:
	float x;
	float y;
	float z;

	inline float3(float _x, float _y, float _z)
	{
		this->x = _x;
		this->y = _y;
		this->z = _z;
	}

	inline float3(){}

	inline const float3 operator * (float s) const {
		float3 a;
		a.x = x*s;
		a.y = y*s;
		a.z = z*s;
		return a;
	}

	inline const void operator += (float3 s) {
		x += s.x;
		y += s.y;
		z += s.z;
	}
};

// Struct for holding texture coordinates
struct texCoord {
  float u;
  float v;
};

struct triMeshStruct {
	std::vector<float3> vertices;
	std::vector<float3> normals;
	std::vector<float3> tangents;
	std::vector<texCoord> texCoords;

	GLuint vertex_bo;
	GLuint normals_bo;
	GLuint tangents_bo;
	GLuint texCoords_bo;

	GLuint vaob;
};


#endif /* HELPER_H_ */
