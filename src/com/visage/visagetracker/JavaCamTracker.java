package com.visage.visagetracker;

import java.io.IOException;
import java.util.List;

import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.Size;
import android.util.Log;

public class JavaCamTracker {
	
	public final String TAG = "JavaCamTracker";
	
	private final int PREFERRED_WIDTH = 300;
	private SurfaceTexture tex;
	private Camera cam;
	
	JavaCamTracker() {
		GrabFromCamera();
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
	
	public void stopCamera() {
		if (cam !=null){
			cam.stopPreview();				
			cam.release();
			cam = null;
		}
	}
	
	/** Interface to native method used for passing raw pixel data to tracker.
	 * This method is called to write camera frames into VisageSDK::VisageTracker2 object through VisageSDK::AndroidCameraCapture
	 * 
	 * @param frame raw pixel data of image used for tracking.
	 */
	public static native void WriteFrameCamera(byte[] frame);
}
