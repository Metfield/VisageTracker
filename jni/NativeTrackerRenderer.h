#include <jni.h>

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
	void onSurfaceCreated();
	void onSurfaceChanged(int w, int h);
	void onDrawFrame();
};

// Extern JNI bullcrap
extern "C"
{
	// onSurfaceCreated
	JNIEXPORT void JNICALL Java_com_visage_visagetracker_TrackerRenderer_nativeOnSurfaceCreated(JNIEnv *env, jclass cls)
	{
		NativeTrackerRenderer::getInstance().onSurfaceCreated();
	}

	// onSurfaceChanged
	JNIEXPORT void JNICALL Java_com_visage_visagetracker_TrackerRenderer_nativeOnSurfaceChanged(JNIEnv *env, jclass cls, jint width, jint height)
	{
		NativeTrackerRenderer::getInstance().onSurfaceChanged(width, height);
	}

	// onDrawFrame
	JNIEXPORT void JNICALL Java_com_visage_visagetracker_TrackerRenderer_nativeOnDrawFrame(JNIEnv *env, jclass cls)
	{
		NativeTrackerRenderer::getInstance().onDrawFrame();
	}
}

