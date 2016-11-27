#include <_VisageTracker.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <string>
#include <ActionUnitBinding.h>
#include <AndroidCameraCapture.h>
#include <unistd.h>
#include <sstream>
#include <ModelLoader.h>
#include <Logging.h>
#include <NativeTrackerRenderer.h>

#include "WrapperOpenCV.h"

// VisageSDK Includes
#include <VisageTracker.h>

using namespace VisageSDK;

static VisageTracker *tracker;
static FaceData trackingData;
AAssetManager *aMgr;

static AndroidCameraCapture *a_cap_camera = 0;

int camOrientation;
int camHeight;
int camWidth;
int camFlip;
bool trackerStopped;
bool isTracking = false;
int trackingStatus = 0;
pthread_mutex_t mutex;

ModelLoader *mLoader;

JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_trackerInit(JNIEnv *env, jobject obj, jstring configFilename, jobject assetManager)
{
	pthread_mutex_destroy(&mutex);
	pthread_mutex_init(&mutex, NULL);

	// Init tracker
	const char *_configFilename = env->GetStringUTFChars(configFilename, 0);
	tracker = new VisageTracker(_configFilename);
	trackerStopped = false;
	env->ReleaseStringUTFChars(configFilename, _configFilename);

	// Init asset mananger
	aMgr = AAssetManager_fromJava(env, assetManager);

	// Init asset manager and model loader
	AAssetManager *aMgr = AAssetManager_fromJava(env, assetManager);
	const char* modelName = "Jones";
	mLoader = new ModelLoader(aMgr);

	if(mLoader->ModelExists(modelName))
	{
		mLoader->LoadModel(modelName);
	}
}

/**
 * Method that sets frame parameters for tracking from camera
 */
JNIEXPORT void JNICALL Java_com_visage_visagetracker_JavaCamTracker_SetCamParameters(JNIEnv *env, jobject obj, jint width, jint height, jint orientation, jint flip)
{
	camOrientation = orientation;
	camHeight = height;
	camWidth = width;
	camFlip = flip;
	trackerStopped = false;
}

/**
 * Method for tracking from camera
 */
JNIEXPORT void JNICALL Java_com_visage_visagetracker_JavaCamTracker_TrackFromCam(JNIEnv *env, jobject obj)
{
	while (!trackerStopped)
	{
		if (tracker && a_cap_camera && !trackerStopped)
		{
			pthread_mutex_lock(&mutex);

			long ts;
			VsImage *pixelData = a_cap_camera->GrabFrame(ts);

			if (trackerStopped || pixelData == 0)
			{
					pthread_mutex_unlock(&mutex);
					return;
			}

			if (camOrientation == 90 || camOrientation == 270)
			{
				trackingStatus = tracker->track(camHeight, camWidth, pixelData->imageData, &trackingData, VISAGE_FRAMEGRABBER_FMT_RGB, VISAGE_FRAMEGRABBER_ORIGIN_TL, 0, -1);
			}
			else
			{
				trackingStatus = tracker->track(camWidth, camHeight, pixelData->imageData, &trackingData, VISAGE_FRAMEGRABBER_FMT_RGB, VISAGE_FRAMEGRABBER_ORIGIN_TL, 0, -1);
			}
			isTracking = true;

			// Send current tracker state
			mLoader->updateTrackerReference(tracker);

			// Update the aubs with the newly tracked facial data.
			mLoader->UpdateAubs(&trackingData);

			pthread_mutex_unlock(&mutex);
		}
		else
		{
			usleep(10000);
		}
	}
	return;
}


/**
* Writes raw image data into @ref VisageSDK::AndroidCameraCapture object. VisageTracker2 reads this image and performs tracking. User should call this
* function whenever new frame from camera is available. Data inside frame should be in Android NV21 (YUV420sp) format and @ref VisageSDK::AndroidCameraCapture
* will perform conversion to RGB.
* @param frame byte array with image data
*/
JNIEXPORT void JNICALL Java_com_visage_visagetracker_JavaCamTracker_WriteFrameCamera(JNIEnv *env, jobject obj, jbyteArray frame)
{
	if (trackerStopped)
		return;
	if (!a_cap_camera)
		a_cap_camera = new AndroidCameraCapture(camWidth, camHeight, camOrientation, camFlip);
	jbyte *f = env->GetByteArrayElements(frame, 0);
	a_cap_camera->WriteFrameYUV((unsigned char *)f);
	env->ReleaseByteArrayElements(frame, f, 0);
}


JNIEXPORT void JNICALL Java_com_visage_visagetracker_JavaCamTracker_PauseTracker(JNIEnv *env, jobject obj)
{
	if(tracker) {
		trackerStopped = true;
		pthread_mutex_lock(&mutex);
		tracker->stop();
		pthread_mutex_unlock(&mutex);
	}
}


JNIEXPORT void JNICALL Java_com_visage_visagetracker_JavaCamTracker_StopTracker(JNIEnv *env, jobject obj)
{
	if(tracker) {
		trackerStopped = true;
		pthread_mutex_lock(&mutex);
		tracker->stop();
		delete tracker;
		tracker = 0;
		pthread_mutex_unlock(&mutex);
	}
}


JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_setupBinding(JNIEnv *env, jobject obj, jstring bindFilename)
{
	// Load asset using android asset manager
	const char *bindingsFileName = env->GetStringUTFChars(bindFilename, 0);
	mLoader->LoadBindings(bindingsFileName);

	// Save reference to be used in the renderer
	NativeTrackerRenderer::getInstance().setModelLoaderRef(mLoader);
}

