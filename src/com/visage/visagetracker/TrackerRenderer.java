package com.visage.visagetracker;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLSurfaceView;

public class TrackerRenderer implements GLSurfaceView.Renderer
{
	public int width, height;
	
	public TrackerRenderer(int width, int height)
	{
		this.width = width;
		this.height = height;
	}
	
	static
	{
		System.loadLibrary("VisageTracker");
	}
	
	// OpenGL Native Calls
	public static native void nativeOnSurfaceCreated(int width, int height);	 
    public static native void nativeOnSurfaceChanged(int width, int height); 
    public static native void nativeOnDrawFrame();
	
	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config) 
	{
		// TODO Auto-generated method stub
		//gl.glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
	
		nativeOnSurfaceCreated(this.width, this.height);		
	}

	@Override
	public void onSurfaceChanged(GL10 gl, int width, int height) 
	{
		// TODO Auto-generated method stub
		nativeOnSurfaceChanged(width, height);
	}

	@Override
	public void onDrawFrame(GL10 gl) 
	{
		// TODO Auto-generated method stub
		//gl.glClear(GL_COLOR_BUFFER_BIT);
		nativeOnDrawFrame();
	}	
}
