#include <jni.h>
#include <android/log.h>

extern "C"
{
	JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_trackerInit(JNIEnv *env, jobject obj, jstring configFilename, jobject assetManager);
	JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_setupBinding(JNIEnv *env, jobject obj, jstring bindFilename);

	JNIEXPORT void JNICALL Java_com_visage_visagetracker_JavaCamTracker_SetCamParameters(JNIEnv *env, jobject obj, jint width, jint height, jint orientation, jint flip);
	JNIEXPORT void JNICALL Java_com_visage_visagetracker_JavaCamTracker_WriteFrameCamera(JNIEnv *env, jobject obj, jbyteArray frame);
	JNIEXPORT void JNICALL Java_com_visage_visagetracker_JavaCamTracker_TrackFromCam(JNIEnv *env, jobject obj);
	JNIEXPORT void JNICALL Java_com_visage_visagetracker_JavaCamTracker_PauseTracker(JNIEnv *env, jobject obj);
	JNIEXPORT void JNICALL Java_com_visage_visagetracker_JavaCamTracker_StopTracker(JNIEnv *env, jobject obj);
}
