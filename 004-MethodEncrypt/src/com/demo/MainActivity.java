package com.demo;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends Activity
{
    /** Called when the activity is first created. */

    public native String getText();

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        System.loadLibrary("demo");

        
        TextView tv = (TextView) findViewById(R.id.text);
        tv.setText(getText());

    }
}
