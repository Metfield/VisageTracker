#include <jni.h>
#include <android/log.h>

#define  LOG_TAG    "VisageTracker"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

extern "C"
{
	JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_trackerInit(JNIEnv *env, jobject obj, jstring configFilename);
	JNIEXPORT void JNICALL Java_com_visage_visagetracker_JavaCamTracker_WriteFrameCamera(JNIEnv *env, jobject obj, jbyteArray frame);
}
