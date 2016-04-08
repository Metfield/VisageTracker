#ifndef MODEL_H_
#define MODEL_H_

#include <BlendShapeTriMesh.h>
#include <Helper.h>


class Model {

public:
	inline void AddTriMesh(const triMeshStruct* meshData) {meshes.push_back(meshData);}
	inline void AddBlendShapeTriMesh(const BlendShapeTriMesh mesh) {blendShapeMeshes.push_back(mesh);}

	// Rendering the model will first interpolate the blendshapes(if there are any) then draw the final product.
	void Render();

private:
	std::vector<const triMeshStruct*> meshes;
	std::vector<BlendShapeTriMesh> blendShapeMeshes;
};


#endif /* MODEL_H_ */
