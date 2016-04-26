#include <ModelLoader.h>
#include <string>
#include <vector>
#include <Importer.hpp>
#include <postprocess.h>
#include <android/log.h>

#include <tiny_obj_loader.h>

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <sstream>
#include <fstream>

#include <sstream>
#include <cstdio>

// Rapidjson includes
#include <document.h>
#include <writer.h>
#include <stringbuffer.h>

#define  LOG_TAG    "ModelLoader"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)


class vectorwrapbuf : public std::basic_streambuf<char> {
public:
    vectorwrapbuf(std::vector<char> &vec) {
        setg(vec.data(), vec.data(), vec.data() + vec.size());
    }
};

ModelLoader::ModelLoader(AAssetManager *assetManager) {
	aMgr = assetManager;
}

bool ModelLoader::ModelExists(const char* modelName) {
	std::string tmp(modelName);
	std::string filePath = "models/" + tmp + "/" + tmp + ".obj";
	AAsset* asset = AAssetManager_open(aMgr, filePath.c_str(), AASSET_MODE_UNKNOWN);
	if(asset) {
		// Remember to close the asset if we are done with it
		AAsset_close(asset);
		return true;
	}
	return false;
}

void ModelLoader::LoadModel(const char* modelName) {
	std::string tmp(modelName);
	std::string filePath = "models/" + tmp + "/" + tmp + ".obj";
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
	__android_log_print(ANDROID_LOG_INFO, "ModelLoader", "%s", ss.str().c_str());

	bool triangulate = true;

	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	vectorwrapbuf databuf(buffer);
	std::istream is(&databuf);

	std::string err;



	const char *basepath = "/models/Jonas/";

	//tinyobj::MaterialReader readMatFn;
	tinyobj::MaterialFileReader matFileReader(basepath);

	bool ret = tinyobj::LoadObj(shapes, materials, err, is, matFileReader, triangulate);

	if (!err.empty())
	{
		__android_log_print(ANDROID_LOG_INFO, "ModelLoader", "ERROR! %s", err.c_str());

	}

	if (!ret)
	{
		__android_log_print(ANDROID_LOG_INFO, "ModelLoader","Failed to load/parse .obj.\n");
	}

	__android_log_print(ANDROID_LOG_INFO, "ModelLoader","Shapes: %i\n", shapes.size());


	// Read json and shit
	std::string jsonPath = "models/" + tmp + "/" + tmp + ".json";
	asset = AAssetManager_open(aMgr, jsonPath.c_str(), AASSET_MODE_UNKNOWN);

	// Create buffer to hold asset data
	asset_size = AAsset_getLength(asset);
	char *json = new char[asset_size];

	// Read data into buffer
	assetsRead = AAsset_read(asset, &json[0], asset_size);

	// Done with asset, close it
	AAsset_close(asset);

	// Rapidjson stuff
	using namespace rapidjson;

	Document doc;
	doc.Parse(json);

	StringBuffer buf;
	Writer<StringBuffer> writer(buf);
	doc.Accept(writer);

	const Value &shapeArray = doc["shape"];
	assert(shape.IsArray());

	for(SizeType i = 0; i < shapeArray.Size(); i++)
	{
		if(shapeArray[i]["name"].GetString() == NULL)
		{
			continue;
		}

		LOGI("name: %s", shapeArray[i]["name"].GetString());

		const Value &blendshapes = shapeArray[i]["blendshapes"];
		assert(blendshapes.IsArray());

		for(SizeType j = 0; j < blendshapes.Size(); j++)
		{
			const Value &vertices = blendshapes[j]["vertices"];
			assert(vertices.IsArray()());

			LOGI("Blendshape: %s, Vertices: %i", blendshapes[j]["name"].GetString(), vertices.Size());

			for(SizeType k = 0; k < vertices.Size(); k++)
			{
				LOGI("x: %lf, y: %lf, z: %lf", vertices[k]["x"].GetFloat(), vertices[k]["y"].GetFloat(), vertices[k]["z"].GetFloat());
			}
		}
	}
}
