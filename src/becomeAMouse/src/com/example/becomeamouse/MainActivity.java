package com.example.becomeamouse;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;

import android.os.Bundle;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.ToggleButton;

public class MainActivity extends Activity {
	public static final String PREFS_NAME = "MyPrefsFile";

	@SuppressWarnings("deprecation")
	protected String becameA(String who, String old) {
		try {
			
			DataInputStream dis = new DataInputStream(Runtime.getRuntime().exec("su -c /data/bmouse " + who +" "+ old).getInputStream());
			String line = "", ret = "";
			while ( (line = dis.readLine()) != null) {
				Log.d("", line);
				ret= ret+line;
			}
			dis.close();
			return ret;
			
		} catch (IOException e) {
			e.printStackTrace();
		}
		return null;
	}
	String old = null;
	public void onToggleClicked(View view) {   
	    boolean on = ((ToggleButton) view).isChecked();
	    if (on) {
	    	old = becameA("mouse","");
			Log.d("BecomeAMouse", "OLD == " + old);
			SharedPreferences sharedPref = getSharedPreferences(PREFS_NAME, 0);

			SharedPreferences.Editor editor = sharedPref.edit();
			
			editor.putString("sdp-record", old);
			editor.putBoolean("mouce-mode", true);
			
			editor.commit();
			
	    } else {
	    	SharedPreferences sharedPref = getSharedPreferences(PREFS_NAME, 0);

		
	    	String old = sharedPref.getString("sdp-record", "");
	    	Log.d("BecomeAMouse", "OLD ==>> " + old);
	    	becameA("phone",old);
	    }
	}
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		  BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
	 	    if (!mBluetoothAdapter.isEnabled()) {
	 	    	Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
	 	        startActivity(enableBtIntent);
	 	    }
	 	    
		ToggleButton toggle = (ToggleButton) findViewById(R.id.toggleButton);
		toggle.setChecked(becameA("cod", "").equals("0x10590"));

		
	}
}
