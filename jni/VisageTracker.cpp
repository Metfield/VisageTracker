#include <_VisageTracker.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <string>
#include <sstream>
#include "ModelLoader.h"
#include <Logging.h>
// VisageSDK Includes
#include <VisageTracker2.h>

using namespace VisageSDK;

VisageTracker2 *tracker;
ModelLoader *mLoader;

JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_trackerInit(JNIEnv *env, jobject obj, jstring configFilename, jobject assetManager)
{
	const char *_configFilename = env->GetStringUTFChars(configFilename, 0);
	tracker = new VisageTracker2(_configFilename);

	// Initialize camera and what not

	// Init asset manager and model loader
	AAssetManager *aMgr = AAssetManager_fromJava(env, assetManager);
	const char* modelName = "Jones";
	mLoader = new ModelLoader(aMgr);
	if(mLoader->ModelExists(modelName)) {
		mLoader->LoadModel(modelName);
		LOGI("Scene loaded successfully");
	}
}


JNIEXPORT void JNICALL Java_com_visage_visagetracker_JavaCamTracker_WriteFrameCamera(JNIEnv *env, jobject obj, jbyteArray frame)
{
	// TODO: Write code for looking at the received frame
	// TODO: Refresh all ActionUnitBindings
}
