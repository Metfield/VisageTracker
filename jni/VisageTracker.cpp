#include <_VisageTracker.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <string>
// VisageSDK Includes
#include <VisageTracker2.h>

using namespace VisageSDK;

VisageTracker2 *tracker;
AAssetManager *aMgr;

JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_trackerInit(JNIEnv *env, jobject obj, jstring configFilename, jobject assetManager)
{
	// Init tracker
	const char *_configFilename = env->GetStringUTFChars(configFilename, 0);
	tracker = new VisageTracker2(_configFilename);

	// Init asset mananger
	aMgr = AAssetManager_fromJava(env, assetManager);

	// Initialize camera and what not
}


void Java_com_visage_visagetracker_JavaCamTracker_WriteFrameCamera(JNIEnv *env, jobject obj, jbyteArray frame)
{
	// TODO: Write code for looking at the received frame
}

JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_setupBinding(JNIEnv *env, jobject obj, jstring bindFilename)
{
	// Load asset using android asset manager
	const char *_bindFilename = env->GetStringUTFChars(bindFilename, 0);
	AAsset *bindingsAsset = AAssetManager_open(aMgr, _bindFilename, AASSET_MODE_UNKNOWN);
	if(bindingsAsset == NULL) {
		LOGE("Could not find model bindings file");
	}
	off_t assetLength = AAsset_getLength(bindingsAsset);

	// Allocate buffer for our binding data
	char *buffer = (char*) malloc(assetLength + 1);

	if(AAsset_read(bindingsAsset, buffer, assetLength) < 0) {
		LOGE("Error loading bindings data in the asset manager");
		return;
	}

	// Zero-terminate
	buffer[assetLength] = 0;

	// Asset has been loaded into memory, close asset.
	AAsset_close(bindingsAsset);

	std::string bindingsData(buffer);

	// TODO: parse buffer data into something useful
}
