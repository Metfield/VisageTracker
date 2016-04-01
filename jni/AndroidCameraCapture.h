#ifndef __AndroidCameraCapture_h__
#define __AndroidCameraCapture_h__

#include <pthread.h>
#include <cv.h>
#include <cerrno>
#include "VisageTrackerFrameGrabber.h"

namespace VisageSDK
{

/** AndroidCameraCapture is implementation of VisageTrackerFrameGrabber that demonstrates use of raw camera image 
 * input to track from Android camera. 
 * @ref VisageTracker2 object will periodically call @ref GrabFrame method to get new frame.
 * If there is no new frame, @ref GrabFrame method will block and wait. When the new frame
 * arrives @ref GrabFrame will unblock and tracker will continue tracking.
 * For inputing new frame, @ref WriteFrame should be used. This method expects frame in 
 * Android camera NV21 format (YUV420sp). YUV420sp to RGB converting, rotation and flipping
 * is done in @ref GrabFrame.
 */
class AndroidCameraCapture : public VisageTrackerFrameGrabber {
		
public:

	bool frameArrived;
		
	/** Constructor.
	 *	
	 */
	AndroidCameraCapture();
	
	/** Constructor.
	* 
	* @param width width of image
	* @param height height of image
	* @param orientation Orientation of image. Allowed values are 0, 90, 180, 270
	* @param flip Flip image horizontaly.
	*/
	AndroidCameraCapture(int width, int height, int orientation=0, int flip = 0);

	/** Destructor.
	 *	
	 */
	~AndroidCameraCapture(void);

	/**
	 * Implementation of a virtual function from VisageTrackerFrameGrabber.
	 * 
	 * VisageTracker2 calls this function periodically to get the new video frame to process.
	 * 
	 */
	unsigned char *GrabFrame(long &timeStamp);
	
	/**
	* Method for writing imageData to buffer object.
	* @param imageData raw pixel data of image used for tracking
	*/
	void WriteFrame(unsigned char *imageData);

	void WriteFrameYUV(unsigned char* imageData);

	void YUV_NV21_TO_RGB(unsigned char* yuv, IplImage* buff, int width, int height);

	int clamp(int x);


private:

	/**
	* Convert default Android camera output format (YUV420sp) to RGB. 
	*/
	void YUV420toRGB(unsigned char* data, IplImage* buff, int width, int height);
	void convertYUVtoARGB(int y, int u, int v, char *r, char *g, char *b);

	IplImage* buffer;
	IplImage* bufferN;
	IplImage* bufferT;
	unsigned char *data;
	int orientation;
	int flip;
	int pts;
	int width, height;
	pthread_mutex_t mutex;
	pthread_cond_t cond;	
};

}
	
#endif // __AndroidCameraCapture_h__
