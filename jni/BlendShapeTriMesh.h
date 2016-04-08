#ifndef BLENDSHAPETRIMESH_H_
#define BLENDSHAPETRIMESH_H_

#include <Helper.h>
#include <vector>
#include <BlendShape.h>


class BlendShapeTriMesh {

public:
	BlendShapeTriMesh(triMeshStruct *meshData_);
	/*
	inline void SetVertices(const std::vector<float3> *vertices_) {vertices = vertices_;}
	inline void SetNormals(const std::vector<float3> *normals_) {normals = normals_;}
	inline void SetTangents(const std::vector<float3> *tangents_) {tangents = tangents_;}
	inline void SetTexCoords(const std::vector<texCoord> *texCoords_) {texCoords = texCoords_;}
	*/
	inline void AddBlendShape(const BlendShape blendShape) {blendShapes.push_back(blendShape);}

	triMeshStruct* GetInterpolatedMesh();

	// TODO: Fix materials

private:

	triMeshStruct *meshData;
	std::vector<float3> vertices;
	std::vector<float3> normals;
	std::vector<float3> tangents;
	std::vector<texCoord> texCoords;

	std::vector<BlendShape> blendShapes;
};


#endif /* BLENDSHAPETRIMESH_H_ */
