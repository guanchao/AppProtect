package com.demo;

import android.app.Activity;
import android.os.Bundle;
import android.widget.Toast;

public class MainActivity extends Activity
{
    /** Called when the activity is first created. */

    public native void test();

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        System.loadLibrary("demo1");

        test();

        Toast.makeText(this, new Test().add(2,5) + "", Toast.LENGTH_LONG).show();
        

    }
}
