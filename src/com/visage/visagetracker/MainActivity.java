package com.visage.visagetracker;

import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

public class MainActivity extends ActionBarActivity 
{
	// OpenGL Surface view
	private GLSurfaceView glView;
	private int glVersion = 2;
	
	// Initialize JNI stuff
	public native void trackerInit(String configFilename);
	public native void setupBinding(String bindFilename);
	
	static
	{
		System.loadLibrary("VisageTracker");
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		// Handle openGL stuff
		ActivityManager activityManager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
	    ConfigurationInfo configurationInfo = activityManager.getDeviceConfigurationInfo();
	 
	    final boolean supportsGLES2 = configurationInfo.reqGlEsVersion >= 0x20000 || isProbablyEmulator();
		
	    // Check if device is compatible
	    if(supportsGLES2)
	    {
			glView = new GLSurfaceView(this);
			glView.setEGLContextClientVersion(glVersion);
			
			glView.setRenderer(new TrackerRenderer());
			setContentView(glView);
	    }
	    else
	    {
	    	Toast.makeText(this, "Sorry, your device doesn't support OpenGL 2.0 :'(", Toast.LENGTH_LONG).show();
	    	return;
	    }
		
		// Initialize the tracker
		trackerInit(getFilesDir().getAbsolutePath() + "/Facial Features Tracker - High.cfg");
		
		// Load and setup the binding file
		setupBinding(getFilesDir().getAbsolutePath() + "/jones.bind");
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
	}
	
	protected void onResume()
	{
		super.onResume();
		glView.onResume();
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
