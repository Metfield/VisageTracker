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

#define PI 3.14159265
#define MESH_EYES 1

std::vector<std::string> meshNames;
std::vector<Mesh> meshTemporal;

std::vector<std::vector<float> > verticesTemporal;

unsigned int gNumMeshes = -1;

void vertex_cb(void *user_data, float x, float y, float z)
{
	verticesTemporal.at(gNumMeshes).push_back(x);
	verticesTemporal.at(gNumMeshes).push_back(y);
	verticesTemporal.at(gNumMeshes).push_back(z);
}

void normal_cb(void *user_data, float x, float y, float z)
{
	meshTemporal.at(gNumMeshes).normals.push_back(x);
	meshTemporal.at(gNumMeshes).normals.push_back(y);
	meshTemporal.at(gNumMeshes).normals.push_back(z);
}

void texcoord_cb(void *user_data, float x, float y)
{
	meshTemporal.at(gNumMeshes).texcoords.push_back(x);
	meshTemporal.at(gNumMeshes).texcoords.push_back(y);
}

void index_cb(void *user_data, int v_idx, int vn_idx, int vt_idx)
{
	unsigned int vertexCount = 0;
	unsigned int vt_count = 0;
	unsigned int vn_count = 0;

	if (v_idx != 0x80000000)
	{
		for(int i = 0; i < gNumMeshes; i++)
		{
			vertexCount += verticesTemporal.at(i).size() / 3;
		}

		meshTemporal.at(gNumMeshes).v_indices.push_back((unsigned short)(v_idx - 1) - vertexCount);
	}
	if (vn_idx != 0x80000000)
	{
		for(int i = 0; i < gNumMeshes; i++)
		{
			vn_count += meshTemporal.at(i).normals.size() / 3;
		}

		meshTemporal.at(gNumMeshes).vn_indices.push_back((unsigned short)(vn_idx - 1) - vn_count);
	}

	if (vt_idx != 0x80000000)
	{
		for(int i = 0; i < gNumMeshes; i++)
		{
			vt_count += meshTemporal.at(i).texcoords.size() / 2;
		}

		meshTemporal.at(gNumMeshes).vt_indices.push_back((unsigned short)(vt_idx - 1) - vt_count);
	}
}

void usemtl_cb(void *user_data, const char* name, int material_idx)
{
	Mesh *mesh = reinterpret_cast<Mesh*>(user_data);

	if ((material_idx > -1) && (material_idx < mesh->materials.size()))
	{
		meshTemporal.at(gNumMeshes).materials.push_back(mesh->materials[material_idx]);
		LOGI("usemtl. material id = %d(name = %s)\n", material_idx, mesh->materials[material_idx].name.c_str());
	}
	else
	{
		LOGI("usemtl. name = %s\n", name);
	}
}

void mtllib_cb(void *user_data, const tinyobj::material_t *materials, int num_materials)
{
	Mesh *mesh = reinterpret_cast<Mesh*>(user_data);
	LOGI("mtllib. # of materials = %d\n", num_materials);

	for (int i = 0; i < num_materials; i++)
	{
		mesh->materials.push_back(materials[i]);
	}
}

void group_cb(void *user_data, const char **names, int num_names)
{
	gNumMeshes++;
	Mesh aux;
	aux.name = names[0];

	// Push new empty mesh
	meshTemporal.push_back(aux);

	// Push new empty vertex vector
	verticesTemporal.push_back(std::vector<float>());

	for (int i = 0; i < num_names; i++)
	{
		LOGI("group : name = %s\n", names[i]);
		meshNames.push_back(names[i]);
	}
}

void object_cb(void *user_data, const char *name)
{
//	Not useful right now
	LOGI("object : name = %s\n", name);
}

class vectorwrapbuf : public std::basic_streambuf<char>
{
public:
    vectorwrapbuf(std::vector<char> &vec)
	{
        setg(vec.data(), vec.data(), vec.data() + vec.size());
    }
};

ModelLoader::ModelLoader(AAssetManager *assetManager)
{
	aMgr = assetManager;
}

bool ModelLoader::ModelExists(const char* modelName)
{
	std::string tmp(modelName);
	std::string filePath = "models/" + tmp + "/" + tmp + ".obj";
	AAsset* asset = AAssetManager_open(aMgr, filePath.c_str(), AASSET_MODE_UNKNOWN);

	if(asset)
	{
		// Remember to close the asset if we are done with it
		AAsset_close(asset);
		return true;
	}
	return false;
}

// Loads obj file, json file with blendshapes,
// and transforms obj data for proper rendering
void ModelLoader::LoadModel(const char* modelName)
{
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
	std::string err;

	// Done with asset, close it
	AAsset_close(asset);

	// Load Material buffer and pass it to tinyobj
	std::string materialName = tmp;
	materialName[0] = tolower(materialName[0]);
	std::string materialFilePath = "models/" + tmp + "/Materials/" + materialName + ".mtl";

	asset = AAssetManager_open(aMgr, materialFilePath.c_str(), AASSET_MODE_UNKNOWN);
	asset_size = AAsset_getLength(asset);
	std::vector<char> matBuffer(asset_size);
	assetsRead = AAsset_read(asset, &matBuffer[0], asset_size);
	vectorwrapbuf matdatabuf(matBuffer);
	std::istream materialStream(&matdatabuf);

	tinyobj::MaterialFileReader mfr(materialFilePath.c_str());

	tinyobj::callback_t callback;
	callback.vertex_cb = vertex_cb;
	callback.normal_cb = normal_cb;
	callback.texcoord_cb = texcoord_cb;
	callback.index_cb = index_cb;
	callback.usemtl_cb = usemtl_cb;
	callback.mtllib_cb = mtllib_cb;
	callback.group_cb = group_cb;
	callback.object_cb = object_cb;

	Mesh meshAux;

	LOGI("Loading model!");

	if(tinyobj::LoadObjWithCallback(&meshAux, callback, &err, &is, &mfr, &materialStream))
	{
		std::string strMsg = tmp + ".obj successfully loaded";
		LOGI("%s" ,strMsg.c_str());
		LOGI("Additional information: %s", err.c_str());
	}
	else
	{
		LOGE("%s", err.c_str());
		return;
	}

	LOGI("Model Loaded! Filling Structure..");

	// Fill member structures with data
	this->meshVector.swap(meshTemporal);
	this->originalFixedMeshVertices.swap(verticesTemporal);

	// Copy original vertex data to mesh structure
	for(int i = 0; i < meshVector.size(); i++)
	{
		meshVector.at(i).originalVertices_blending = originalFixedMeshVertices.at(i);
	}

	// Close material stream
	AAsset_close(asset);

	// Re-organize geometry
	for(int i = 0; i < meshVector.size(); i++)
	{
		std::vector<float> vt;
		std::vector<float> vn;

		Mesh *mesh = &meshVector.at(i);

		// Copy original array to the reference list
		meshVector.at(i).originalVertices_fixedReference.reserve(meshVector.at(i).originalVertices_blending.size());   // absdlasdkihas dlkajsdh asdasdas
		copy(meshVector.at(i).originalVertices_blending.begin(), meshVector.at(i).originalVertices_blending.end(), back_inserter(meshVector.at(i).originalVertices_fixedReference));

		// Loop through the whole index array
		for(int j = 0; j < meshVector.at(i).v_indices.size(); j++)
		{
			// Do vertex position coordinates
			mesh->vertices.push_back(&mesh->originalVertices_blending.at(mesh->v_indices.at(j) * 3 + 0));
			mesh->vertices.push_back(&mesh->originalVertices_blending.at(mesh->v_indices.at(j) * 3 + 1));
			mesh->vertices.push_back(&mesh->originalVertices_blending.at(mesh->v_indices.at(j) * 3 + 2));

			// Do texture coordinates
			vt.push_back(meshVector.at(i).texcoords.at(meshVector.at(i).vt_indices.at(j) * 2 + 0));
			vt.push_back(meshVector.at(i).texcoords.at(meshVector.at(i).vt_indices.at(j) * 2 + 1));

			// Do normals
			vn.push_back(meshVector.at(i).normals.at(meshVector.at(i).vn_indices.at(j) * 3 + 0));
			vn.push_back(meshVector.at(i).normals.at(meshVector.at(i).vn_indices.at(j) * 3 + 1));
			vn.push_back(meshVector.at(i).normals.at(meshVector.at(i).vn_indices.at(j) * 3 + 2));
		}

		// Copy new data to structure
		meshVector.at(i).texcoords.resize(vt.size());
		meshVector.at(i).texcoords = vt;

		meshVector.at(i).normals.resize(vn.size());
		meshVector.at(i).normals = vn;
	}

	// Load blendshapes
	LOGI("Loading blendshapes...");
	this->LoadBlendshapes(modelName);

	// Load Config file
	this->LoadConfigFile("config.json");
}

void ModelLoader::LoadConfigFile(char *jsonFile)
{
	AAsset* asset = AAssetManager_open(aMgr, jsonFile, AASSET_MODE_UNKNOWN);

	// Create buffer to hold asset data
	size_t asset_size = AAsset_getLength(asset);
	char *json = (char*) malloc(asset_size);

	// Read data into buffer
	int assetsRead = AAsset_read(asset, &json[0], asset_size);

	// Done with asset, close it
	AAsset_close(asset);

	// Rapidjson stuff
	using namespace rapidjson;

	// Garbage was getting into the last positions of the array...
	json[assetsRead] = '\0';

	Document doc;
	doc.Parse(json);

	NativeTrackerRenderer::getInstance().nearPlane = doc["NearPlane"].GetFloat();
	NativeTrackerRenderer::getInstance().farPlane = doc["FarPlane"].GetFloat();
	NativeTrackerRenderer::getInstance().fov = doc["FieldOfView"].IsFloat();

	const Value& mo = doc["MeshOffset"];
	NativeTrackerRenderer::getInstance().meshOffset = glm::vec3(mo[0].GetFloat(), mo[1].GetFloat(), mo[2].GetFloat());

	free(json);
}

// Loads and parses json file with blendshapes
// and copies them to their respective structures
void ModelLoader::LoadBlendshapes(const char* modelName)
{
	std::string tmp(modelName);

	// Read json
	std::string jsonPath = "models/" + tmp + "/" + tmp + ".json";
	AAsset* asset = AAssetManager_open(aMgr, jsonPath.c_str(), AASSET_MODE_UNKNOWN);

	// Create buffer to hold asset data
	off_t asset_size = AAsset_getLength(asset);
	char *json = new char[asset_size];

	// Read data into buffer
	int assetsRead = AAsset_read(asset, &json[0], asset_size);

	// Done with asset, close it
	AAsset_close(asset);

	// Rapidjson stuff
	using namespace rapidjson;

	Document doc;
	doc.Parse(json);

	const Value &shapeArray = doc["shape"];
	assert(shape.IsArray());

	// Temporal pointer to a shape
	Mesh *tmp_shape = NULL;
	BlendShape tmp_blendshape;
	std::string shapeName;

	for(SizeType i = 0; i < shapeArray.Size(); i++)
	{
		shapeName = shapeArray[i]["name"].GetString();

		if(shapeName.empty())
		{
			continue;
		}

		for(int x = 0; x < this->meshVector.size(); x++)
		{
			if(!this->meshVector.at(x).name.compare(shapeName))
			{
				tmp_shape = &this->meshVector.at(x);
				break;
			}
		}

		if(tmp_shape == NULL)
			LOGE("Something went wrong with the blendshape parsing!");

		const Value &blendshapes = shapeArray[i]["blendshapes"];
		assert(blendshapes.IsArray());

		for(SizeType j = 0; j < blendshapes.Size(); j++)
		{
			// Fill blendshape ID
			tmp_blendshape.id = j;

			const Value &vertices = blendshapes[j]["vertices"];
			assert(vertices.IsArray()());

			for(SizeType k = 0; k < vertices.Size(); k++)
			{
				tmp_blendshape.AddVertex(float3(vertices[k]["x"].GetFloat(), vertices[k]["y"].GetFloat(), vertices[k]["z"].GetFloat()));
			}

			tmp_shape->blendshapes.push_back(tmp_blendshape);

			// Clear temporal blendshape variable
			tmp_blendshape.ClearVectors();
		}
	}

	tmp_shape = NULL;
	delete[] json;
}

void ModelLoader::blendMeshes()
{
	float weight;
	float x, y, z;

	// Iterate through every mesh to blend
	for(int i = 0; i < this->meshVector.size(); i++)
	{
		// Skip meshes without blendshapes
		if(!meshVector.at(i).blendshapes.empty())
		{
			// Reset the vertices to the original value
			meshVector.at(i).originalVertices_blending = this->meshVector.at(i).originalVertices_fixedReference;

			for(int j = 0; j < this->meshVector.at(i).blendshapes.size(); j++)
			{
				BlendShape *blendShape = &this->meshVector.at(i).blendshapes.at(j);
				weight = blendShape->actionUnitBinding->GetValue();

				if(weight == 0)
					continue;

				for(int k = 0; k < blendShape->vertices.size(); k++)
				{
					// Calculate the delta position
					x = (blendShape->vertices.at(k).x * 10) - this->meshVector.at(i).originalVertices_fixedReference.at((k * 3) + 0);
					y = (blendShape->vertices.at(k).y * 10) - this->meshVector.at(i).originalVertices_fixedReference.at((k * 3) + 1);
					z = (blendShape->vertices.at(k).z * 10) - this->meshVector.at(i).originalVertices_fixedReference.at((k * 3) + 2);

					// Add the difference to the original
					meshVector.at(i).originalVertices_blending.at((k * 3) + 0) += (x * weight);
					meshVector.at(i).originalVertices_blending.at((k * 3) + 1) += (y * weight);
					meshVector.at(i).originalVertices_blending.at((k * 3) + 2) += (z * weight);
				}
			}
		}
	}

	// Set reference for the renderer
	NativeTrackerRenderer::getInstance().setBlendedMeshes(&meshVector);
}

// Local helper function to remove some unwanted non-numerical/alphabetical chars.
bool checkIfAlNum(char c)
{
	if(std::isalnum(c))
	{
		return false;
	}

	return true;
}

void ModelLoader::LoadBindings(const char* bindingsFileName)
{
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

		for(int i = 0; i < this->meshVector.size(); i++)
		{
			if(!meshName.compare(this->meshVector[i].name))
			{
				for(int j = 0; j < this->meshVector[i].blendshapes.size(); j++)
				{
					if(id == this->meshVector[i].blendshapes[j].id)
					{
						// Create Action unit binding and add it to the correct blendshape
						this->meshVector[i].blendshapes[j].actionUnitBinding = new ActionUnitBinding(name, auName, inverted, minLimit, maxLimit, weight, filterAmount, filterWindow);

						// This vector is used for faster access when updating the aubs, hence the "double" storage.
						actionUnitBindings.push_back(this->meshVector[i].blendshapes[j].actionUnitBinding);
					}
				}
			}
		}

		// Erase this line and go to next one
		textBuffer.erase(0, pos + delimiter.length());
	}

	// Once everything is done store a reference to the data in the renderer
	NativeTrackerRenderer::getInstance().setBlendedMeshes(&this->meshVector);

	// Save the original data to the blended shapes
	this->blendedMeshes = this->meshVector;
}

void ModelLoader::UpdateAubs(VisageSDK::FaceData* trackingData)
{
	for(int i = 0; i < actionUnitBindings.size(); i++)
	{
		for (int j = 0; j < trackingData->actionUnitCount; j++)
		{
			if (strcmp(actionUnitBindings[i]->actionUnitName.c_str(), trackingData->actionUnitsNames[j]) == 0)
			{
				actionUnitBindings[i]->UpdateValue(trackingData->actionUnits[j]);
			}
		}
	}

	// Set new faceData
	this->updateFaceData(*trackingData);
}
