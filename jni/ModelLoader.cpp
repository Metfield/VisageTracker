#include <ModelLoader.h>
#include <string>
#include <vector>

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
#include <Logging.h>


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

	// Init tinyobj
	vectorwrapbuf databuf(buffer);
	std::istream is(&databuf);

	bool triangulate = true;
	modelData.vertices.clear();
	modelData.normals.clear();
	modelData.texcoords.clear();
	meshes.clear();
	materials.clear();
	std::string err;

	const char *basepath = "/models/asd/";
	tinyobj::MaterialFileReader mfr(basepath);

	// Read the model data
	if(tinyobj::LoadObj(&modelData, &meshes, &materials, &err, &is, &mfr, triangulate)){
		std::string strMsg = tmp + ".obj successfully loaded";
		LOGI("%s" ,strMsg.c_str());
	}
	else {
		LOGE("%s", err.c_str());
		return;
	}

	// Done with asset, close it
	AAsset_close(asset);

	// Read json
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

	// Temporal pointer to a shape
	tinyobj::shape_t *tmp_shape = NULL;
	BlendShape tmp_blendshape;
	std::string shapeName;

	for(SizeType i = 0; i < shapeArray.Size(); i++)
	{
		shapeName = shapeArray[i]["name"].GetString();

		if(shapeName.empty())
		{
			continue;
		}

		for(int x = 0; x < meshes.size(); x++)
		{
			if(meshes.at(x).name == shapeName)
			{
				tmp_shape = &meshes.at(x);
				break;
			}
		}

		if(tmp_shape == NULL)
			LOGE("Something went wrong with the blendshape parsing!");

		//LOGI("name: %s", shapeArray[i]["name"].GetString());

		const Value &blendshapes = shapeArray[i]["blendshapes"];
		assert(blendshapes.IsArray());

		for(SizeType j = 0; j < blendshapes.Size(); j++)
		{
			// Fill blendshape ID
			tmp_blendshape.id = i;

			const Value &vertices = blendshapes[j]["vertices"];
			assert(vertices.IsArray()());

			//LOGI("Blendshape: %s, Vertices: %i", blendshapes[j]["name"].GetString(), vertices.Size());

			for(SizeType k = 0; k < vertices.Size(); k++)
			{
				//LOGI("x: %lf, y: %lf, z: %lf", vertices[k]["x"].GetFloat(), vertices[k]["y"].GetFloat(), vertices[k]["z"].GetFloat());

				tmp_blendshape.AddVertex(float3(vertices[k]["x"].GetFloat(), vertices[k]["y"].GetFloat(), vertices[k]["z"].GetFloat()));
			}

			tmp_shape->blendshapes.push_back(tmp_blendshape);

			// Clear temporal blendshape variable
			tmp_blendshape.ClearVectors();
		}
	}

	tmp_shape = NULL;


	// Test this bullshit!
	for(int x = 0; x < meshes.size(); x++)
	{
		if(!meshes.at(x).blendshapes.empty())
		{
			for(int y = 0; y < meshes.at(x).blendshapes.size(); y++)
			{
				int id = meshes.at(x).blendshapes.at(y).id;

				LOGI("ID: %i, Blendshapes: %i", id, meshes.at(x).blendshapes.at(y).vertices->size());

				for(int z; z < meshes.at(x).blendshapes.at(y).vertices->size(); z++)
				{
					float xx = meshes.at(x).blendshapes.at(y).vertices->at(z).x;
					float yy = meshes.at(x).blendshapes.at(y).vertices->at(z).y;
					float zz = meshes.at(x).blendshapes.at(y).vertices->at(z).z;

					LOGI("X: %f Y: %f Z: %f", xx, yy, zz);
				}
			}
		}
	}




}
