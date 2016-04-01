package com.visage.visagetracker;

import java.io.IOException;
import java.util.List;

import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.Size;
import android.util.Log;
import android.view.Display;
import android.view.WindowManager;

public class JavaCamTracker {
	
	public final String TAG = "JavaCamTracker";
	
	private final int PREFERRED_WIDTH = 300;
	private SurfaceTexture tex;
	private Camera cam;
	private Context context;
	private Thread trackerThread;
	
	JavaCamTracker(Context context) {
		this.context = context;
	}
	
	private int getCameraId(){
		int cameraId = -1;
		int numberOfCameras = Camera.getNumberOfCameras();
		for (int i = 0; i < numberOfCameras; i++) {
			CameraInfo info = new CameraInfo();
			Camera.getCameraInfo(i, info);
			cameraId = i;
	    	if (info.facing == CameraInfo.CAMERA_FACING_FRONT) {
	    		break;
	    	}
		}
		return cameraId;
	}
	
	/**
	 * Sets preview size so that width is closest to param width
	 * @param parameters
	 * @param width
	 */
	private void setPreviewSize(Camera.Parameters parameters, int width){
		int idx = 0,  dist = 100000;
		List<Size> sizes = parameters.getSupportedPreviewSizes();
		for (int i = 0;i<sizes.size();i++){
			if (Math.abs(sizes.get(i).width-width)<dist){
				idx = i;
				dist = Math.abs(sizes.get(i).width-width);
			}
		}
		parameters.setPreviewSize(sizes.get(idx).width, sizes.get(idx).height);
		
		// Setup VisageTracker cam parameters
    	Display display = ((WindowManager)context.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
    	int screenOrientation = display.getRotation();
		
    	CameraInfo cameraInfo = new CameraInfo();
    	Camera.getCameraInfo(getCameraId(), cameraInfo);
		int orientation = cameraInfo.orientation;
    	int flip = 0;		
		if (cameraInfo.facing == CameraInfo.CAMERA_FACING_FRONT){
			flip = 1; // Mirror image from frontal camera
		}
		if (cameraInfo.facing == CameraInfo.CAMERA_FACING_FRONT) {
			SetCamParameters(sizes.get(idx).width, sizes.get(idx).height, (screenOrientation*90 + orientation)%360, flip);
		}
    	else {
    		SetCamParameters(sizes.get(idx).width, sizes.get(idx).height, (orientation - screenOrientation*90 + 360)%360, flip);
    	}
		
		// Start tracker loop
		trackerThread = new Thread(new TrackerThread());
		trackerThread.start();
	}
	
	/**
	 * Grab frames from camera and store in buffer
	 */
	public void GrabFromCamera(){
		try{
			cam = Camera.open(getCameraId());
		}catch(Exception e){
			Log.e(TAG, "Unable to open camera");
			return;
		}
		Camera.Parameters parameters = cam.getParameters();
		setPreviewSize(parameters, PREFERRED_WIDTH);
		parameters.setPreviewFormat(ImageFormat.NV21);
		cam.setParameters(parameters);
		
		final Size previewSize=cam.getParameters().getPreviewSize();

		int dataBufferSize=(int)(previewSize.height*previewSize.width*
                (ImageFormat.getBitsPerPixel(cam.getParameters().getPreviewFormat())/8.0));
        for (int i=0;i<10;i++){
        	cam.addCallbackBuffer(new byte[dataBufferSize]);
        }
        tex = new SurfaceTexture(0);
        try {
			cam.setPreviewTexture(tex);
		} catch (IOException e) {
			e.printStackTrace();
		}
        cam.setPreviewCallbackWithBuffer(new PreviewCallback() {
			public void onPreviewFrame(byte[] data, Camera camera) {
				camera.addCallbackBuffer(data);
				WriteFrameCamera(data);
			}
		});
        cam.startPreview();
	}
	
	public void pauseCamera() {
		// Stop tracker
		PauseTracker();
		
		// Release camera and stop preview
		if (cam !=null){
			cam.stopPreview();				
			cam.release();
			cam = null;
		}
	}
	
	public void stopCamera() {
		// Stop tracker
		StopTracker();
	}
	
	public void startCamera() {
		GrabFromCamera();
	}
	
	
	private final class TrackerThread implements Runnable {
		@Override
		public void run() {
			TrackFromCam();
			return;
		}
	}
	
	
	/** Interface to native method used for passing raw pixel data to tracker.
	 * This method is called to write camera frames into VisageSDK::VisageTracker2 object through VisageSDK::AndroidCameraCapture
	 * 
	 * @param frame raw pixel data of image used for tracking.
	 */
	public static native void WriteFrameCamera(byte[] frame);
	
	
	/**
	 * Interface to native method called to set camera frame parameteres
	 * 
	 */
	public static native void SetCamParameters(int width, int height, int orientation, int flip);
	
	
	/**
	 * Prepare raw image interface to track from camera.
	 */
	public static native void TrackFromCam();
	
	
	/** Interface to native method used to stop tracking.
	 */
	public static native void PauseTracker();
    
    
	/** Interface to native method used to stop tracking.
	 */
	public static native void StopTracker();
}
