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

public:
	std::vector<Mesh> blendedMeshes;

	ModelLoader *mLoader = NULL;

	int width, height;

	float auxValue;
	bool touchReleased;

	double sinWave;

	void onSurfaceCreated(int w, int h);
	void onSurfaceChanged(int w, int h);
	void onDrawFrame();

	void setBlendedMeshes(std::vector<Mesh> _meshes)
	{
		this->blendedMeshes.swap(_meshes);
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
};


