package com.visage.visagetracker;

import android.support.v7.app.ActionBarActivity;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.view.Display;
import android.view.Menu;
import android.view.MenuItem;
import android.view.WindowManager;
import android.widget.Toast;
import android.content.res.AssetManager;
import android.graphics.Point;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;

public class MainActivity extends ActionBarActivity 
{
	// OpenGL Surface view
	private GLSurfaceView glView;
	private int glVersion = 2;
	private JavaCamTracker javaCamTracker;
	
	// Need to hold a reference for the AssetManager to prevent garbage collection from destroying it
	// DO NOT REMOVE
	private AssetManager aMgr;
	
	// Initialize JNI stuff
	public native void trackerInit(String configFilename, AssetManager assetManager);
	public native void setupBinding(String bindFilename);
	
	static
	{
		System.loadLibrary("VisageVision");
		System.loadLibrary("VisageTracker");
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		aMgr = getResources().getAssets();
		
		// Get Viewport size
		Display display = getWindowManager().getDefaultDisplay(); 
		int width = display.getWidth(); 
		int height = display.getHeight(); 
		
		// Handle openGL stuff
		ActivityManager activityManager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
	    ConfigurationInfo configurationInfo = activityManager.getDeviceConfigurationInfo();
	 
	    final boolean supportsGLES2 = configurationInfo.reqGlEsVersion >= 0x20000 || isProbablyEmulator();
		
	    // Check if device is compatible
	    if(supportsGLES2)
	    {
			glView = new GLSurfaceView(this);
			glView.setEGLContextClientVersion(glVersion);
			
			glView.setRenderer(new TrackerRenderer(width, height));
			setContentView(glView);
	    }
	    else
	    {
	    	Toast.makeText(this, "Sorry, your device doesn't support OpenGL 2.0 :'(", Toast.LENGTH_LONG).show();
	    	return;
	    }
	    
	    // Init all config files
	    copyFileOrDir("trackerdata");
		
		// Initialize the tracker and asset manager
	    aMgr = getResources().getAssets();
		trackerInit(getFilesDir().getAbsolutePath() + "/Facial Features Tracker - High.cfg", aMgr);
		
		// Load and setup the binding file, keep in mind that bindings file path are relative to assets folder
		setupBinding("models/Jones/Jones.bind.txt");
		
		// Init camera capture
		javaCamTracker = new JavaCamTracker(glView.getContext());
	}
	
    private void copyFileOrDir(String path) {
	    AssetManager assetManager = this.getAssets();
	    String assets[] = null;
	    try {
	        assets = assetManager.list(path);
	        if (assets.length == 0) {
	            copyFile(path);
	        } else {
	            String fullPath = getFilesDir().getAbsolutePath() + "/" + path;
	            File dir = new File(fullPath.replace("trackerdata/",""));
	            if (!dir.exists())
	                dir.mkdir();
	            for (int i = 0; i < assets.length; ++i) {
	                copyFileOrDir(path + "/" + assets[i]);
	            }
	        }
	    } catch (Exception ex) {
	        Log.e("tag", "I/O Exception", ex);
	    }
	}
    
    private void copyFile(String filename) {
	    AssetManager assetManager = this.getAssets();

	    InputStream in = null;
	    OutputStream out = null;
	    try {
	        in = assetManager.open(filename);
	        String newFileName = getFilesDir().getAbsolutePath() + "/" + filename;
	        
	        Log.i("TrackerDemo", newFileName);
	        out = new FileOutputStream(newFileName.replace("trackerdata/",""));

	        byte[] buffer = new byte[1024];
	        int read;
	        while ((read = in.read(buffer)) != -1) {
	            out.write(buffer, 0, read);
	        }
	        in.close();
	        in = null;
	        out.flush();
	        out.close();
	        out = null;
	    } catch (Exception e) {
	        Log.e("tag", e.getMessage());
	    }
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) 
	{
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) 
	{
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		
		if (id == R.id.action_settings) 
		{
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
	
	protected void onPause()
	{
		super.onPause();
		glView.onPause();
		
		// Pause camera capture and pause tracker
		javaCamTracker.pauseCamera();
	}
	
	protected void onResume()
	{
		super.onResume();
		glView.onResume();
		
		// Restart camera capture and tracker
		javaCamTracker.startCamera();
	}
	
	protected void onDestroy() 
	{
		super.onStop();
		
		// Stop camera completely and destroy tracker
		javaCamTracker.stopCamera();
	}
	
	// Checks if application is running on emulator
	private boolean isProbablyEmulator() 
	{
	    return Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1
	            && (Build.FINGERPRINT.startsWith("generic")
	                    || Build.FINGERPRINT.startsWith("unknown")
	                    || Build.MODEL.contains("google_sdk")
	                    || Build.MODEL.contains("Emulator")
	                    || Build.MODEL.contains("Android SDK built for x86"));
	}
}
