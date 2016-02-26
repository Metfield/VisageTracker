#include <jni.h>

#include "VisageTracker2.h"

#include <android/log.h>
#define  LOG_TAG    "TrackerWrapper"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

extern "C"
{
	JNIEXPORT jstring Java_com_visage_visagetracker_MainActivity_getJNIString(JNIEnv *env, jobject obj);
}

JNIEXPORT jstring Java_com_visage_visagetracker_MainActivity_getJNIString(JNIEnv *env, jobject obj)
{
	return env->NewStringUTF("Suck it to me!!");
}

