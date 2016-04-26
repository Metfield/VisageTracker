#ifndef MODELLOADER_H_
#define MODELLOADER_H_

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
/* assimp includes */
#include <scene.h> // Output data structure


class ModelLoader {

private:
	AAssetManager *aMgr;

public:
	ModelLoader();
	ModelLoader(AAssetManager *assetManager);

	bool ModelExists(const char* modelName);
	void LoadModel(const char* modelName);
};



#endif /* MODELLOADER_H_ */
