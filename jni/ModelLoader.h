#ifndef MODELLOADER_H_
#define MODELLOADER_H_

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <tiny_obj_loader.h>

#include <ActionUnitBinding.h>
#include <VisageTracker2.h>

#include <Mesh.h>

class ModelLoader {

private:
	AAssetManager *aMgr;

public:
	ModelLoader();
	ModelLoader(AAssetManager *assetManager);

	bool ModelExists(const char* modelName);
	void LoadModel(const char* modelName);
	void LoadBindings(const char* bindingsFileName);
	void UpdateAubs(const VisageSDK::FaceData* trackingData);
	void blendMeshes();

	/*tinyobj::attrib_t modelData;
	std::vector<tinyobj::shape_t> meshes;
	std::vector<tinyobj::material_t> materials;*/

	std::vector<Mesh> meshVector;

private:
	std::vector<ActionUnitBinding*> actionUnitBindings;

};

class ShapeWrapper {
public:

};



#endif /* MODELLOADER_H_ */
