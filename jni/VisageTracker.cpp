#include <jni.h>

extern "C"
{
	JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_trackerInit(JNIEnv *env, jstring configFilename);
}

JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_trackerInit(JNIEnv *env, jstring configFilename)
{
	const char *_configFilename = env->GetStringUTFChars(configFilename, 0);


}


void Java_com_visage_visagetracker_JavaCamTracker_WriteFrameCamera(JNIEnv *env, jobject obj, jbyteArray frame)
{
	// TODO: Write code for looking at the received frame
}
