/*
 * Mesh.h
 *
 *  Created on: Jun 9, 2016
 *      Author: taich
 */

#ifndef MESH_H_
#define MESH_H_

class Mesh
{
public:
	std::string name;

	std::vector<float> originalVertices;
	std::vector<float const *> vertices;

	std::vector<float> normals;
	std::vector<float> texcoords;

	std::vector<unsigned short>   v_indices;
	std::vector<unsigned int>   vn_indices;
	std::vector<unsigned short>   vt_indices;

	std::vector<tinyobj::material_t> materials;

	// Blendshapes
	std::vector<BlendShape> blendshapes;

	float diffuseColor[3];
};



#endif /* MESH_H_ */
