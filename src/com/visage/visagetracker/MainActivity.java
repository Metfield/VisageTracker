package com.visage.visagetracker;

import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;

public class MainActivity extends Activity 
{
	@Override
	protected void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState); 
		setContentView(R.layout.activity_main);
		
		TextView textView1 = (TextView)findViewById(R.id.textView1);
		
		textView1.setText(getJNIString());
	}
	
	public native String getJNIString();
	
	static
	{
		System.loadLibrary("VisageTracker");
	}
}
