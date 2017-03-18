package com.dexencrypt;

import android.app.Activity;
import android.os.Bundle;
import android.widget.Toast;

public class MainActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		Security s = new Security(this.getApplicationContext());
		Toast.makeText(getApplication(), s.invokeHiddenMethod(), Toast.LENGTH_SHORT).show();
	}

}
