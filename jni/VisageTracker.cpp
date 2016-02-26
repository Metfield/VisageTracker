#include <jni.h>

extern "C"
{
	JNIEXPORT jstring JNICALL Java_com_visage_visagetracker_MainActivity_getJNIString(JNIEnv *env, jobject obj);
}

JNIEXPORT jstring JNICALL Java_com_visage_visagetracker_MainActivity_getJNIString(JNIEnv *env, jobject obj)
{
	return env->NewStringUTF("SUCK IT TO ME!");
}
