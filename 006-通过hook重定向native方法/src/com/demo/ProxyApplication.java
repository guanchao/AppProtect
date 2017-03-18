package com.demo;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Map;

import android.app.Application;
import android.app.Instrumentation;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.util.Log;
import android.widget.Toast;

public class ProxyApplication extends Application{

	static{
		System.loadLibrary("substrate");
		System.loadLibrary("hackcodejiagu");
	}

	
	@Override
	public void onCreate() {
		super.onCreate();
	}
	

	@Override
	public void attachBaseContext(Context base) {
		super.attachBaseContext(base);		
	}
	

}
