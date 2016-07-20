#ifndef MODELLOADER_H_
#define MODELLOADER_H_

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <tiny_obj_loader.h>

#include <ActionUnitBinding.h>
#include <VisageTracker2.h>

#include <Mesh.h>

#include <Logging.h>

class ModelLoader {

public:
	std::vector<Mesh> blendedMeshes;
	std::vector<std::vector<float> > originalFixedMeshVertices;

private:
	AAssetManager *aMgr;

public:
	ModelLoader();
	ModelLoader(AAssetManager *assetManager);

	bool ModelExists(const char* modelName);
	void LoadModel(const char* modelName);
	void LoadBindings(const char* bindingsFileName);
	void UpdateAubs(VisageSDK::FaceData* trackingData);
	void blendMeshes();

	std::vector<Mesh> meshVector;

	void updateTrackerReference(VisageSDK::VisageTracker2 *_tracker)
	{
		this->tracker = _tracker;
	}

	VisageSDK::VisageTracker2 *getTracker()
	{
		return this->tracker;
	}

	void updateFaceData(VisageSDK::FaceData fd)
	{
		this->faceData = fd;
	}

	const VisageSDK::FaceData *getFaceData()
	{
		return &this->faceData;
	}

	AAssetManager *getAssetManager()
	{
		return this->aMgr;
	}

private:
	std::vector<ActionUnitBinding*> actionUnitBindings;
	VisageSDK::VisageTracker2 *tracker;
	VisageSDK::FaceData faceData;

};


#endif /* MODELLOADER_H_ */
