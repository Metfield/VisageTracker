#include <jni.h>

extern "C"
{
	JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_trackerInit(JNIEnv *env, jstring configFilename);
}

JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_trackerInit(JNIEnv *env, jstring configFilename)
{
	const char *_configFilename = env->GetStringUTFChars(configFilename, 0);


}
