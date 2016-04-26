#ifndef MODELLOADER_H_
#define MODELLOADER_H_

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <tiny_obj_loader.h>


class ModelLoader {

private:
	AAssetManager *aMgr;

public:
	ModelLoader();
	ModelLoader(AAssetManager *assetManager);

	bool ModelExists(const char* modelName);
	void LoadModel(const char* modelName);

	tinyobj::attrib_t modelData;
	std::vector<tinyobj::shape_t> meshes;
	std::vector<tinyobj::material_t> materials;
};



#endif /* MODELLOADER_H_ */
