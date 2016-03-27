#include <ModelLoader.h>
#include <string>
#include <vector>
#include <Importer.hpp>
#include <postprocess.h>

#include <sstream>

ModelLoader::ModelLoader(AAssetManager *assetManager) {
	aMgr = assetManager;
}

bool ModelLoader::ModelExists(const char* modelName) {
	std::string tmp(modelName);
	std::string filePath = "models/" + tmp + "/" + tmp + ".fbx";
	AAsset* asset = AAssetManager_open(aMgr, filePath.c_str(), AASSET_MODE_UNKNOWN);
	if(asset) {
		// Remember to close the asset if we are done with it
		AAsset_close(asset);
		return true;
	}
	return false;
}

const aiScene* ModelLoader::LoadModel(const char* modelName) {
	std::string tmp(modelName);
	std::string filePath = "models/" + tmp + "/" + tmp + ".fbx";
	AAsset* asset = AAssetManager_open(aMgr, filePath.c_str(), AASSET_MODE_UNKNOWN);

	// Create buffer to hold asset data
	off_t asset_size = AAsset_getLength(asset);
	std::vector<char> buffer(asset_size);

	// Read data into buffer
	int assetsRead = AAsset_read(asset, &buffer[0], asset_size);

	// Done with asset, close it
	AAsset_close(asset);

	std::stringstream ss;
	ss << "Asset read: ";
	ss << assetsRead;
	ss << "   Asset size: ";
	ss << asset_size;
	ss << "   Buffer size: ";
	ss << buffer.size();
	__android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s", ss.str().c_str());

	// Read buffer and create an assimp scene from it.
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFileFromMemory( &buffer[0],
														buffer.size(),
														aiProcessPreset_TargetRealtime_Quality);

	// Check so that the scene was loaded correctly
	if( !scene) {
		__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "%s", importer.GetErrorString());
		return NULL;
	}
	return scene;
}
