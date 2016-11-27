#include "AndroidCameraCapture.h"
#include <android/log.h>

#define  LOG_TAG    "libandroid-opencv"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

namespace VisageSDK
{

AndroidCameraCapture::AndroidCameraCapture()
{
	pts = 0;
}

AndroidCameraCapture::AndroidCameraCapture(int width, int height, int orientation, int flip)
{
	buffer = vsCreateImage(vsSize(width, height),VS_DEPTH_8U,3);
	bufferN = vsCreateImage(vsSize(width, height),VS_DEPTH_8U,3);
	bufferT = vsCreateImage(vsSize(height, width),VS_DEPTH_8U,3);
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);
	pts = 0;
	frameArrived = false;
	this->orientation = orientation;
	this->flip = flip;
	this->width = width;
	this->height = height;	
}

AndroidCameraCapture::~AndroidCameraCapture(void)
{
	// cleaning up
	vsReleaseImage(&buffer);
	vsReleaseImage(&bufferN);
	vsReleaseImage(&bufferT);
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);	
}
 
void AndroidCameraCapture::WriteFrame(unsigned char* imageData)
{
	pthread_mutex_lock(&mutex);
	//data = imageData;
	//YUV420toRGB(data, buffer, width, height);
	for (int i=0;i<width*height;i++){
			buffer->imageData[i*3] = imageData[i*4];
			buffer->imageData[i*3+1] = imageData[i*4+1];
			buffer->imageData[i*3+2] = imageData[i*4+2];
		}
	frameArrived = true;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
}

void AndroidCameraCapture::WriteFrameYUV(unsigned char* imageData)
{
	pthread_mutex_lock(&mutex);
	//YUV420toRGB(imageData, buffer, width, height);
	YUV_NV21_TO_RGB(imageData, buffer, width, height);
	frameArrived = true;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);	
}

VsImage *AndroidCameraCapture::GrabFrame(long &timeStamp)
{
    struct timespec   ts;
    struct timeval    tp;
    VsImage* ret;

    int rc = gettimeofday(&tp, NULL);
    ts.tv_sec  = tp.tv_sec + 2;
    ts.tv_nsec = 0;
	pthread_mutex_lock(&mutex);
	while (!frameArrived){
		int ret_cond = pthread_cond_timedwait(&cond, &mutex, &ts);
        if (ret_cond == ETIMEDOUT){
            pthread_mutex_unlock(&mutex);
            return 0;
    	}
    }
    frameArrived = false;

	switch(orientation){
	case 0: case 360:
				if (flip)
					vsFlip(buffer, bufferN, 1);
				else
					vsCopy(buffer, bufferN);
				ret = bufferN;
				break;
			case 90:
				vsTranspose(buffer, bufferT);
				if (!flip)
					vsFlip(bufferT, bufferT, 1);
				ret = bufferT;
				break;
			case 180:
				if (flip)
					vsFlip(buffer, bufferN, 0);
				else
					vsFlip(buffer, bufferN, -1);
				ret = bufferN;
				break;
			case 270:
				vsTranspose(buffer, bufferT);
				if (flip)
					vsFlip(bufferT, bufferT, -1);
				else
					vsFlip(bufferT, bufferT, 0);
				ret = bufferT;
				break;
	}

	pthread_mutex_unlock(&mutex);
	timeStamp = pts++;
	return ret;
}
void AndroidCameraCapture::YUV420toRGB(unsigned char* data, VsImage* buff, int width, int height){
    int size = width*height;
    int offset = size;
    int u, v, y1, y2, y3, y4;
 	
 
    // i along Y and the final pixels
    // k along pixels U and V
    for(int i=0, k=0; i < size; i+=2, k+=2) {
        y1 = data[i  ]&0xff;
        y2 = data[i+1]&0xff;
        y3 = data[width+i  ]&0xff;
        y4 = data[width+i+1]&0xff;
 
        u = data[offset+k  ]&0xff;
        v = data[offset+k+1]&0xff;
        u = u-128;
        v = v-128;
 
		convertYUVtoARGB(y1,u,v, &buff->imageData[i*3], &buff->imageData[i*3+1], &buff->imageData[i*3+2]);
		convertYUVtoARGB(y2,u,v, &buff->imageData[(i+1)*3], &buff->imageData[(i+1)*3+1], &buff->imageData[(i+1)*3+2]);
		convertYUVtoARGB(y3,u,v, &buff->imageData[(width+i)*3], &buff->imageData[(width+i)*3+1], &buff->imageData[(width+i)*3+2]);
		convertYUVtoARGB(y4,u,v, &buff->imageData[(width+i+1)*3], &buff->imageData[(width+i+1)*3+1], &buff->imageData[(width+i+1)*3+2]);
 
        if (i!=0 && (i+2)%width==0)
            i+=width;
    }
}
 
int AndroidCameraCapture::clamp(int x) {
    unsigned y;
    return !(y=x>>8) ? x : (0xff ^ (y>>24));
}

void AndroidCameraCapture::YUV_NV21_TO_RGB(unsigned char* yuv, VsImage* buff, int width, int height)
{
    const int frameSize = width * height;

    const int ii = 0;
    const int ij = 0;
    const int di = +1;
    const int dj = +1;

    unsigned char* rgb = (unsigned char*)buff->imageData;

    int a = 0;
    for (int i = 0, ci = ii; i < height; ++i, ci += di)
    {
        for (int j = 0, cj = ij; j < width; ++j, cj += dj)
        {
            int y = (0xff & ((int) yuv[ci * width + cj]));
            int v = (0xff & ((int) yuv[frameSize + (ci >> 1) * width + (cj & ~1) + 0]));
            int u = (0xff & ((int) yuv[frameSize + (ci >> 1) * width + (cj & ~1) + 1]));
            y = y < 16 ? 16 : y;

            //int r = (int) (1.164f * (y - 16) + 1.596f * (v - 128));
            //int g = (int) (1.164f * (y - 16) - 0.813f * (v - 128) - 0.391f * (u - 128));
            //int b = (int) (1.164f * (y - 16) + 2.018f * (u - 128));

            int a0 = 1192 * (y -  16);
            int a1 = 1634 * (v - 128);
            int a2 =  832 * (v - 128);
            int a3 =  400 * (u - 128);
            int a4 = 2066 * (u - 128);

            int r = (a0 + a1) >> 10;
            int g = (a0 - a2 - a3) >> 10;
            int b = (a0 + a4) >> 10;

            *rgb++ = clamp(r);
            *rgb++ = clamp(g);
            *rgb++ = clamp(b);
        }
    }
}

void AndroidCameraCapture::convertYUVtoARGB(int y, int u, int v, char *r, char *g, char *b) {
 	int rr, gg, bb;
    rr = y + (int)1.402f*u;
    gg = y - (int)(0.344f*v +0.714f*u);
    bb = y + (int)1.772f*v;
    *r = (rr)>255? 255 : (rr)<0 ? 0 : (rr);
    *g = (gg)>255? 255 : (gg)<0 ? 0 : (gg);
    *b = (bb)>255? 255 : (bb)<0 ? 0 : (bb);
}
}
