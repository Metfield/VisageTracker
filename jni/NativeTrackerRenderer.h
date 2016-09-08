#include <jni.h>
#include <vector>

#include <tiny_obj_loader.h>
#include <ModelLoader.h>

#include <Mesh.h>

// vec3, vec4, ivec4, mat4
#include <glm.hpp>

class NativeTrackerRenderer
{
	// Singleton stuff
 public:
	static NativeTrackerRenderer& getInstance()
	{
		static NativeTrackerRenderer instance;
		return instance;
	}
private:
	NativeTrackerRenderer() {};

	NativeTrackerRenderer(NativeTrackerRenderer const&);
	void operator=(NativeTrackerRenderer const&);

	// Holds every texture for the mesh
	std::vector<GLuint> meshTextures;

public:
	std::vector<Mesh> *blendedMeshes;
	std::vector<float> newVerts;

	ModelLoader *mLoader = NULL;

	int width, height;

	float auxValueX, auxValueY;
	bool touchReleased;

	double sinWave;

	// Config json file variables
	float nearPlane;
	float farPlane;
	float fov;
	glm::vec3 meshOffset;

	void onSurfaceCreated(int w, int h);
	void onSurfaceChanged(int w, int h);
	void onDrawFrame();

	void setBlendedMeshes(std::vector<Mesh> *_meshes)
	{
		this->blendedMeshes = _meshes;
	}

	inline void setModelLoaderRef(ModelLoader *ml)
	{
		this->mLoader = ml;
	}

	inline void setUniformMVP(glm::vec3 const &Translate, glm::vec3 const &Rotate);

	void setSinWave(double _sinWave)
	{
		this->sinWave = _sinWave;
	}

	inline void bindMeshAttributes(Mesh const *mesh, int textureIndex);

	void loadTextures();


};


