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

std::vector<std::string> meshNames;
std::vector<Mesh> meshTemporal;

unsigned int gNumMeshes = -1;

void vertex_cb(void *user_data, float x, float y, float z)
{
	//Mesh *mesh = reinterpret_cast<Mesh*>(user_data);
//  LOGI("v[%ld] = %f, %f, %f\n", meshTemporal.at(gNumMeshes).vertices.size() / 3, x, y, z);

 /* mesh->vertices.push_back(x);
  mesh->vertices.push_back(y);
  mesh->vertices.push_back(z);*/

  meshTemporal.at(gNumMeshes).vertices.push_back(x);
  meshTemporal.at(gNumMeshes).vertices.push_back(y);
  meshTemporal.at(gNumMeshes).vertices.push_back(z);

  //LOGI("Blow me! %f %f %f", meshTemporal.at(gNumMeshes).vertices.at(meshTemporal.at(gNumMeshes).vertices.size()-3), meshTemporal.at(gNumMeshes).vertices.at(meshTemporal.at(gNumMeshes).vertices.size()-2), meshTemporal.at(gNumMeshes).vertices.at(meshTemporal.at(gNumMeshes).vertices.size()-1));
}

void normal_cb(void *user_data, float x, float y, float z)
{
	//	Mesh *mesh = reinterpret_cast<Mesh*>(user_data);
	// LOGI("vn[%ld] = %f, %f, %f\n", mesh->normals.size() / 3, x, y, z);

	meshTemporal.at(gNumMeshes).normals.push_back(x);
	meshTemporal.at(gNumMeshes).normals.push_back(y);
	meshTemporal.at(gNumMeshes).normals.push_back(z);
}

void texcoord_cb(void *user_data, float x, float y)
{
	//	Mesh *mesh = reinterpret_cast<Mesh*>(user_data);
	//LOGI("vt[%ld] = %f, %f\n", mesh->texcoords.size() / 2, x, y);

	meshTemporal.at(gNumMeshes).texcoords.push_back(x);
	meshTemporal.at(gNumMeshes).texcoords.push_back(y);
}

void index_cb(void *user_data, int v_idx, int vn_idx, int vt_idx)
{
  // NOTE: the value of each index is raw value.
  // For example, the application must manually adjust the index with offset
  // (e.g. v_indices.size()) when the value is negative(relative index).
  // See fixIndex() function in tiny_obj_loader.h for details.
  // Also, -2147483648(0x80000000) is set for the index value which does not exist in .obj
//	Mesh *mesh = reinterpret_cast<Mesh*>(user_data);
 // LOGI("idx[%ld] = %d, %d, %d\n", mesh->v_indices.size(), v_idx, vn_idx, vt_idx);

  if (v_idx != 0x80000000)
  {
	  unsigned int vertexCount = 0;

	  for(int i = 0; i < gNumMeshes; i++)
	  {
		  vertexCount += meshTemporal.at(i).vertices.size() / 3;
	  }

	meshTemporal.at(gNumMeshes).v_indices.push_back((unsigned short)(v_idx - 1) - vertexCount);
  }
  if (vn_idx != 0x80000000)
  {
//	mesh->vn_indices.push_back(vn_idx);
	meshTemporal.at(gNumMeshes).vn_indices.push_back(vn_idx);
  }
  if (vt_idx != 0x80000000)
  {
//	mesh->vt_indices.push_back(vt_idx);
	meshTemporal.at(gNumMeshes).vt_indices.push_back(vt_idx);
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

	meshTemporal.push_back(aux);

	for (int i = 0; i < num_names; i++)
	{
		LOGI("group : name = %s\n", names[i]);
		meshNames.push_back(names[i]);
	}
}

void object_cb(void *user_data, const char *name)
{
//	Mesh *mesh = reinterpret_cast<Mesh*>(user_data);
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

	this->meshVector.swap(meshTemporal);
	delete(&meshTemporal);

	// Close material stream
	AAsset_close(asset);

	// Write group names in structure
	for(int i = 0; i < this->meshVector.size(); i++)
	{
		LOGI("Name: %s, Verts: %i, Normals: %i, Indices: %i", this->meshVector.at(i).name.c_str(), this->meshVector.at(i).vertices.size()/3, this->meshVector.at(i).normals.size()/3, this->meshVector.at(i).v_indices.size()/3);
	}

	/*for(int h = 0; h < meshVector.at(0).v_indices.size()-2; h+=3)
	{
		LOGI("%u %u %u", meshVector.at(0).v_indices.at(h),meshVector.at(0).v_indices.at(h+1),meshVector.at(0).v_indices.at(h+2));
	}*/

	/*LOGI("Verts: %i", mesh.vertices.size());
	LOGI("Normals: %i", mesh.normals.size());
	LOGI("TexCoords: %i", mesh.texcoords.size());
	LOGI("Materials: %i", mesh.materials.size());

	for(int i = 0; i < mesh.materials.size(); i++)
	{
		LOGI("	Material: %s", mesh.materials.at(i).name.c_str());
	}*/

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

	// Test this bullshit!
	/*for(int x = 0; x < meshVector.size(); x++)
	{
		if(!meshVector.at(x).blendshapes.empty())
		{
			for(int y = 0; y < meshVector.at(x).blendshapes.size(); y++)
			{
				int id = meshVector.at(x).blendshapes.at(y).id;

				LOGI("ID: %i, Blendshapes: %i", id, meshVector.at(x).blendshapes.at(y).vertices.size());

				for(int z; z < meshVector.at(x).blendshapes.at(y).vertices.size(); z++)
				{
					float xx = meshVector.at(x).blendshapes.at(y).vertices.at(z).x;
					float yy = meshVector.at(x).blendshapes.at(y).vertices.at(z).y;
					float zz = meshVector.at(x).blendshapes.at(y).vertices.at(z).z;

					LOGI("X: %f Y: %f Z: %f", xx, yy, zz);
				}
			}
		}
	}*/
}



void ModelLoader::blendMeshes()
{
	/*static float sinFeed = 0;
	sinFeed+=3;
	float sinWave = fabs(sin(sinFeed*(PI/180)));*/

	float weight;
	float x, y, z;

	for(int i = 0; i < this->meshVector.size(); i++)
	{
		if(!meshVector.at(i).blendshapes.empty())
		{
			for(int j = 0; j < this->meshVector.at(i).blendshapes.size(); j++)
			{
				BlendShape bs = this->meshVector.at(i).blendshapes.at(j);
				weight = this->meshVector.at(i).blendshapes.at(j).actionUnitBinding->GetValue();

		//		LOGI("Mesh: %s BlendShape %s", blendedMeshes.at(i).name.c_str(), this->meshVector.at(i).blendshapes.at(j).actionUnitBinding->name.c_str() );
		//		LOGI("AUB Update. Name: %s, Weight: %f", this->meshVector.at(i).blendshapes.at(j).actionUnitBinding->actionUnitName.c_str(), weight);

				for(int k = 0; k < this->meshVector.at(i).blendshapes.at(j).vertices.size(); k++)
				{
	//				LOGI("Original[%i]: %f %f %f", k, blendedMeshes.at(i).vertices.at(k * 3), blendedMeshes.at(i).vertices.at((k * 3)+1), blendedMeshes.at(i).vertices.at((k * 3)+2));
//					LOGI("BlendShape %s[%i]: %f %f %f", this->meshVector.at(i).blendshapes.at(j).actionUnitBinding->name.c_str(), k,  this->meshVector.at(i).blendshapes.at(j).vertices.at(k).x, this->meshVector.at(i).blendshapes.at(j).vertices.at(k).y, this->meshVector.at(i).blendshapes.at(j).vertices.at(k).z);

					// Calculate the delta position
					x = (bs.vertices.at(k).x * 10) - this->meshVector.at(i).vertices.at((k * 3) + 0);
					y = (bs.vertices.at(k).y * 10) - this->meshVector.at(i).vertices.at((k * 3) + 1);
					z = (bs.vertices.at(k).z * 10) - this->meshVector.at(i).vertices.at((k * 3) + 2);

					// Add the difference to the original
					blendedMeshes.at(i).vertices.at((k * 3) + 0) = this->meshVector.at(i).vertices.at((k * 3) + 0) + x * weight;
					blendedMeshes.at(i).vertices.at((k * 3) + 1) = this->meshVector.at(i).vertices.at((k * 3) + 1) + y * weight;
					blendedMeshes.at(i).vertices.at((k * 3) + 2) = this->meshVector.at(i).vertices.at((k * 3) + 2) + z * weight;

	//				LOGI("Result: %f %f %f", blendedMeshes.at(i).vertices.at(k * 3), blendedMeshes.at(i).vertices.at((k * 3)+1), blendedMeshes.at(i).vertices.at((k * 3)+2));
				}
			}
		}
	}

	// Set reference for the renderer
	NativeTrackerRenderer::getInstance().setBlendedMeshes(&blendedMeshes);
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

		//TODO: Change manual access to vector-type
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
