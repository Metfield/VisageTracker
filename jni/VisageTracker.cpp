#include <_VisageTracker.h>
#include <ai_assert.h>
#include <scene.h>

const struct aiScene* scene = NULL;

// VisageSDK Includes
#include <VisageTracker2.h>

using namespace VisageSDK;

VisageTracker2 *tracker;

JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_trackerInit(JNIEnv *env, jobject obj, jstring configFilename)
{
	const char *_configFilename = env->GetStringUTFChars(configFilename, 0);
	tracker = new VisageTracker2(_configFilename);

	// Initialize camera and what not
}


JNIEXPORT void JNICALL Java_com_visage_visagetracker_JavaCamTracker_WriteFrameCamera(JNIEnv *env, jobject obj, jbyteArray frame)
{
	// TODO: Write code for looking at the received frame
	LOGI("Kiss my ass!");

}
