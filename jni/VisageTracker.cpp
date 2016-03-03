#include <jni.h>

// VisageSDK Includes
#include <VisageTracker2.h>

extern "C"
{
	JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_trackerInit(JNIEnv *env, jstring configFilename);
}

VisageSDK::VisageTracker2 *tracker;

JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_trackerInit(JNIEnv *env, jstring configFilename)
{
	const char *_configFilename = env->GetStringUTFChars(configFilename, 0);
	tracker = new VisageSDK::VisageTracker2(_configFilename);


}
