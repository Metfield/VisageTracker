#include <ModelLoader.h>

#include <string>
#include <vector>

#include <cstdio>
#include <stdlib.h>
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

#include <algorithm>
#include <Logging.h>

#include <NativeTrackerRenderer.h>

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
			if(!meshes.at(x).name.compare(shapeName))
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
			tmp_blendshape.id = j;

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


	// Once everything is done store a reference to the data in the renderer
	NativeTrackerRenderer::getInstance().setMeshData(&meshes);


	// Test this bullshit!
	/*for(int x = 0; x < meshes.size(); x++)
	{
		if(!meshes.at(x).blendshapes.empty())
		{
			for(int y = 0; y < meshes.at(x).blendshapes.size(); y++)
			{
				int id = meshes.at(x).blendshapes.at(y).id;

				//LOGI("ID: %i, Blendshapes: %i", id, meshes.at(x).blendshapes.at(y).vertices.size());

				for(int z; z < meshes.at(x).blendshapes.at(y).vertices.size(); z++)
				{
					float xx = meshes.at(x).blendshapes.at(y).vertices.at(z).x;
					float yy = meshes.at(x).blendshapes.at(y).vertices.at(z).y;
					float zz = meshes.at(x).blendshapes.at(y).vertices.at(z).z;

					//LOGI("X: %f Y: %f Z: %f", xx, yy, zz);
				}
			}
		}
	}*/




}

tinyobj::attrib_t ModelLoader::getInterpolatedMesh(tinyobj::shape_t* shape) {
	// We need to create a temporary data holder since we need to save all original mesh data
	tinyobj::attrib_t meshData;

	// Populate the mesh data with the original data
	for(int i = 0; i < shape->mesh.indices.size(); i++) {
		meshData.vertices.push_back(modelData.vertices[shape->mesh.indices[i].vertex_index]);
		meshData.normals.push_back(modelData.normals[shape->mesh.indices[i].normal_index]);
		meshData.texcoords.push_back(modelData.texcoords[shape->mesh.indices[i].texcoord_index]);
	}

	// Loop over all the blendshapes connected to this mesh and add the deltavalues
	for(int i = 0; i < shape->blendshapes.size(); i+3) {
		float weight = shape->blendshapes[i].actionUnitBinding->GetValue();

		for(int j = 0; j < shape->blendshapes[i].vertices.size(); j++) {
			meshData.vertices[j + (j * 3)] += shape->blendshapes[i].vertices[j].x * weight;
			meshData.vertices[j + (j * 3)] += shape->blendshapes[i].vertices[j].y * weight;
			meshData.vertices[j + (j * 3)] += shape->blendshapes[i].vertices[j].z * weight;
		}

	}

	return meshData;
}

// Local helper function to remove some unwanted non-numerical/alphabetical chars.
bool checkIfAlNum(char c){
	if(std::isalnum(c)) {
		return false;
	}
	return true;
}

void ModelLoader::LoadBindings(const char* bindingsFileName) {
	// Load asset
	AAsset *bindingsAsset = AAssetManager_open(aMgr, bindingsFileName, AASSET_MODE_UNKNOWN);

	if(bindingsAsset == NULL)
	{
		LOGE("Could not find model bindings file");
	}

	off_t assetLength = AAsset_getLength(bindingsAsset);

	// Allocate buffer for our binding data
	char *buffer = (char*) malloc(assetLength + 1);

	if(AAsset_read(bindingsAsset, buffer, assetLength) < 0)
	{
		LOGE("Error loading bindings data in the asset manager");
		return;
	}

	// Zero-terminate
	buffer[assetLength] = 0;

	// Asset has been loaded into memory, close asset.
	AAsset_close(bindingsAsset);

	std::string textBuffer(buffer);

	// Parse buffer data into something useful
	size_t pos, linePos;
	std::string line;

	std::string delimiter = "\n";
	std::string lineDelimiter = ";";

	// Loop through the lines
	while((pos = textBuffer.find(delimiter)) != std::string::npos)
	{
		line = textBuffer.substr(0, pos);

		// Skip comments
		if(line.find('#') != std::string::npos)
		{
			textBuffer.erase(0, pos + delimiter.length());
			continue;
		}

		// Name
		linePos = line.find(lineDelimiter);
		std::string auName = line.substr(0, linePos);

		line.erase(0, linePos + lineDelimiter.length());

		// Blendshape Identifier
		linePos = line.find(lineDelimiter);
		std::string blendshapeId = line.substr(0, linePos);

		line.erase(0, linePos + lineDelimiter.length());

		// Create name string
		std::string name = auName + " -> " + blendshapeId;

		// Min Limit
		linePos = line.find(lineDelimiter);
		std::string minLimitStr = line.substr(0, linePos);

		float minLimit = (float)strtod(minLimitStr.c_str(), NULL);

		line.erase(0, linePos + lineDelimiter.length());

		// Max Limit
		linePos = line.find(lineDelimiter);
		std::string maxLimitStr = line.substr(0, linePos);

		float maxLimit = (float)strtod(maxLimitStr.c_str(), NULL);

		line.erase(0, linePos + lineDelimiter.length());

		// Inverted
		linePos = line.find(lineDelimiter);
		std::string invertedStr = line.substr(0, linePos);

		bool inverted = (bool)atoi(invertedStr.c_str());

		line.erase(0, linePos + lineDelimiter.length());

		// Weight
		linePos = line.find(lineDelimiter);
		std::string weightStr = line.substr(0, linePos);

		float weight = (float)strtod(weightStr.c_str(), NULL);

		line.erase(0, linePos + lineDelimiter.length());

		// Filter Window
		linePos = line.find(lineDelimiter);
		std::string filterWindowStr = line.substr(0, linePos);

		int filterWindow = atoi(filterWindowStr.c_str());

		line.erase(0, linePos + lineDelimiter.length());

		// Filter Amount
		linePos = line.find(lineDelimiter);
		std::string filterAmountStr = line.substr(0, linePos);

		float filterAmount = (float)strtod(filterAmountStr.c_str(), NULL);

		line.erase(0, linePos + lineDelimiter.length());

		// Find the correct mesh then the correct blendshape and add the action unit binding
		std::string meshName = blendshapeId.substr(0, blendshapeId.find(":"));
		meshName.erase(std::remove_if(meshName.begin(), meshName.end(), (int(*)(int))checkIfAlNum), meshName.end());
		std::string meshId = blendshapeId.substr(blendshapeId.find(":")+1);
		int id;
		std::istringstream(meshId) >> id;
		for(int i = 0; i < meshes.size(); i++) {
			if(!meshName.compare(meshes[i].name)) {
				for(int j = 0; j < meshes[i].blendshapes.size(); j++){
					if(id == meshes[i].blendshapes[j].id) {
						// Create Action unit binding and add it to the correct blendshape
						meshes[i].blendshapes[j].actionUnitBinding = new ActionUnitBinding(name, auName, inverted, minLimit, maxLimit, weight, filterAmount, filterWindow);
						// This vector is used for faster access when updating the aubs, hence the "double" storage.
						actionUnitBindings.push_back(meshes[i].blendshapes[j].actionUnitBinding);
					}
				}
			}
		}

		// Erase this line and go to next one
		textBuffer.erase(0, pos + delimiter.length());
	}
}

void ModelLoader::UpdateAubs(const VisageSDK::FaceData* trackingData) {
	for(int i = 0; i < actionUnitBindings.size(); i++) {
		for (int j = 0; j < trackingData->actionUnitCount; j++) {
			if (strcmp(actionUnitBindings[i]->actionUnitName.c_str(), trackingData->actionUnitsNames[j]) == 0) {
				actionUnitBindings[i]->UpdateValue(trackingData->actionUnits[j]);
			}
		}
	}
}
