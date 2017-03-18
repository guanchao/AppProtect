package com.demo;

import android.app.Activity;
import android.os.Bundle;
import android.widget.Toast;

public class MainActivity extends Activity
{
    /** Called when the activity is first created. */
    static{
        System.loadLibrary("hello");
        
    }

    public native String test();

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        Toast.makeText(getApplicationContext(), test(), Toast.LENGTH_LONG).show();

    }
}
