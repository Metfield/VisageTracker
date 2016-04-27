#include <jni.h>
#include <vector>

#include <tiny_obj_loader.h>


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
	std::vector<tinyobj::shape_t> *meshes;

	void onSurfaceCreated();
	void onSurfaceChanged(int w, int h);
	void onDrawFrame();

	inline void setMeshData(std::vector<tinyobj::shape_t> *meshData)
	{
		this->meshes = meshData;
	}
};


