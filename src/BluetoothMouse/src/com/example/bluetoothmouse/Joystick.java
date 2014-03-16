package com.example.bluetoothmouse;



import java.lang.reflect.Array;
import java.util.ArrayList;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.GestureDetector;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.View;
import android.widget.SeekBar;
//@SuppressLint("ViewConstructor")
//public class BcastGesturesJoystick extends View{
	

//}

















import android.view.WindowManager;

import com.example.common.hidp.HidpBcaster;
import com.zerokol.views.JoystickView;
import com.zerokol.views.JoystickView.OnJoystickMoveListener;

import android.app.Activity;
import android.os.Bundle;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

public class Joystick extends Activity implements SensorEventListener {
	
	SensorManager sensorManager = null;
	public static float sensivity = 1.0f;
	public static float MaxSensivity = 1.22f;
	
	public static final String TAG = "Jooystick";
	SeekBar seekBar;
	
	Button b1, b2, b3, b4;
	Button a1, a2, a3, a4;
	WindowManager mWindowManager;
	Display mDisplay;
	private float mLastX, mLastY, mLastZ;
	private boolean mInitialized;	
    private final float NOISE =  2.0f;
    
    SeekBar.OnSeekBarChangeListener SeekBarActivity = new SeekBar.OnSeekBarChangeListener() {

	    @Override
	    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
	        //  Notify that the progress level has changed.
	    	sensivity = (MaxSensivity*progress)/100;
	        Log.d("SeekBar", "SeekBar now at the value of:"+progress + " Sensivity: "+ sensivity);

	    }

		@Override
		public void onStartTrackingTouch(SeekBar arg0) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void onStopTrackingTouch(SeekBar arg0) {
			// TODO Auto-generated method stub
			
		}


	};
	 private Button setUpButton(int id, final int ev, final String desc) {
	    	Button b = (Button) findViewById(id);
	    	Log.d("B", "SUccess to init " + id +  " " + ev + " " + desc);
	    	if(b==null) {
	    			Log.e("B", "Failed to init " + id +  " " + ev + " " + desc);
	    			return null;
	    	}
	    	b.setOnTouchListener(new View.OnTouchListener() {
	            public boolean onTouch(View v, MotionEvent e) {
	            	if(e.getAction() == MotionEvent.ACTION_DOWN) {
	            		Log.d("BcastJoystick", "Down " + desc);	
	            		HidpBcaster.PushJoystickButton(ev);
	               	 	
	            	}            	
	            	if(e.getAction() == MotionEvent.ACTION_UP) {
	            		Log.d("BcastJoystick", "Up "+ desc);
	            		HidpBcaster.ReleaseJoystickButton(ev);
	            	}            	            	            	
					return false;
	            }
	        });
	    	return b;
	    }
	 
	
	 				
	public void initNormalJs() {
		 setContentView(R.layout.joystick);
		 setTitle("js-normal");
		 setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
		 b1 = setUpButton(R.id.button1,HidpBcaster.JEV_BTN_A, "button A");
	     b2 = setUpButton(R.id.button2,HidpBcaster.JEV_BTN_B, "button C");
	     b3 = setUpButton(R.id.button3,HidpBcaster.JEV_BTN_C, "button B");
	     b4 = setUpButton(R.id.button4,HidpBcaster.JEV_BTN_D, "button D");
	        
	     a1 = setUpButton(R.id.axis1,HidpBcaster.JEV_BTN_1, "button 1");
	     a2 = setUpButton(R.id.axis2,HidpBcaster.JEV_BTN_2, "button 2");
	     a3 = setUpButton(R.id.axis3,HidpBcaster.JEV_BTN_3, "button 3");
	     a4 = setUpButton(R.id.axis4,HidpBcaster.JEV_BTN_4, "button 4");			 
	 }
	public void initNormalJs2() {
		 setContentView(R.layout.joystick2);
		 setTitle("js-normal2");
		 setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
		 setUpButton(R.id.button1,HidpBcaster.JEV_BTN_A, "button A");
	     setUpButton(R.id.button2,HidpBcaster.JEV_BTN_B, "button B");
	     setUpButton(R.id.Button01,HidpBcaster.JEV_BTN_D, "button C");
	     setUpButton(R.id.Button02,HidpBcaster.JEV_BTN_C, "button B");
	     setUpButton(R.id.Button03,HidpBcaster.JEV_BTN_1, "button B");
	     setUpButton(R.id.Button04,HidpBcaster.JEV_BTN_2, "button B");
	     setUpButton(R.id.Button05,HidpBcaster.JEV_BTN_3, "button B");
	     setUpButton(R.id.Button06,HidpBcaster.JEV_BTN_4, "button B");
	     
	     setUpButton(R.id.Button07,HidpBcaster.JEV_BTN_A1, "button B");
	     setUpButton(R.id.Button08,HidpBcaster.JEV_BTN_A2, "button B");
	     setUpButton(R.id.Button09,HidpBcaster.JEV_BTN_A3,"button B");
	     setUpButton(R.id.Button10,HidpBcaster.JEV_BTN_A4, "button B");
	     
	     setUpButton(R.id.Button11,HidpBcaster.JEV_BTN_B1, "button B");
	     setUpButton(R.id.Button12,HidpBcaster.JEV_BTN_B2, "button B");
	     setUpButton(R.id.Button13,HidpBcaster.JEV_BTN_B3, "button B");
	     setUpButton(R.id.Button14,HidpBcaster.JEV_BTN_B4, "button B");	     
	     			 
	 }
	public void initNormalJs3() {
		 setContentView(R.layout.joystick3);
		 setTitle("js-normal3");
		 setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);		 
	     setUpButton(R.id.Button08,HidpBcaster.JEV_BTN_A, "button C");
	     setUpButton(R.id.Button09,HidpBcaster.JEV_BTN_B, "button B");
	     setUpButton(R.id.Button02,HidpBcaster.JEV_BTN_C, "button B");
	     setUpButton(R.id.Button01,HidpBcaster.JEV_BTN_A4, "button B");
	     setUpButton(R.id.Button18,HidpBcaster.JEV_BTN_D, "button B");
	     setUpButton(R.id.Button16,HidpBcaster.JEV_BTN_3, "button B");
	     setUpButton(R.id.Button04,HidpBcaster.JEV_BTN_4, "button B");
	     
	     setUpButton(R.id.Button17,HidpBcaster.JEV_BTN_A1, "button B");
	     setUpButton(R.id.Button03,HidpBcaster.JEV_BTN_A2, "button B");
	     setUpButton(R.id.Button05,HidpBcaster.JEV_BTN_A3,"button B");
	    
	     			 
	 }
	 public boolean onCreateOptionsMenu(Menu menu) {
	        boolean result = super.onCreateOptionsMenu(menu);
	        menu.add(0, 1, 0, "Tune Sensivity");
	        return result;
	    }
	 public boolean onOptionsItemSelected(MenuItem item) {
		 switch (item.getItemId()) {
	     	case 1:
	     		if(seekBar.getVisibility() == View.VISIBLE) 
	        		seekBar.setVisibility(View.GONE);
	        	else 
	        		seekBar.setVisibility(View.VISIBLE);	        	
	        	return true;	        
	        }	       
	        return super.onOptionsItemSelected(item);
	 }
	 @Override
	 protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
                 
        
        WindowManager mWindowManager = (WindowManager) getSystemService(WINDOW_SERVICE);
        mDisplay = mWindowManager.getDefaultDisplay();
        

        String addr = (savedInstanceState == null) ? null :
            (String) savedInstanceState.getString(DeviceSelection.KEY_ADDR);
		if (addr == null) {
			Bundle extras = getIntent().getExtras();
			addr = extras != null ? extras.getString(DeviceSelection.KEY_ADDR) : null;
			
		}
		if(addr.equals("Type 1")){
			initNormalJs();
		}
		if(addr.equals("Type 2")){
			initNormalJs2();
		}
		if(addr.equals("Type 3")){
			initNormalJs3();
		}
		 		        
        sensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);

        seekBar=(SeekBar)findViewById(R.id.seekBar1);
        

       //Add a listener to listen SeekBar events
        seekBar.setOnSeekBarChangeListener(SeekBarActivity);
    }
    @Override
    protected void onResume() {
       super.onResume();
       sensorManager.registerListener(this, sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER), 
    		   	SensorManager.SENSOR_DELAY_GAME);
       sensorManager.registerListener(this, sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE),
    		   	SensorManager.SENSOR_DELAY_GAME);
    }
    @Override
    protected void onStop() {
       super.onStop();
       sensorManager.unregisterListener(this, sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER));
       sensorManager.unregisterListener(this, sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE));
    }

    @Override
  	protected void onDestroy() {
  		super.onDestroy();
  		sensorManager.unregisterListener(this, sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER));
        sensorManager.unregisterListener(this, sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE));  		
  		
  	}
          
    
    
	@Override
	public void onAccuracyChanged(Sensor arg0, int arg1) {
		// TODO Auto-generated method stub
		
	}
	private int adjSensivity(float val) {
	    	float a = ( val * sensivity);
	    	int b = ((int) a) * 10;	    	
	    	return b; 
	}
	public void onSensorChanged(SensorEvent event) {		
		float x = event.values[0];
		float y = event.values[1];
		float z = event.values[2];
		if (!mInitialized) {
			mLastX = x;
			mLastY = y;
			mLastZ = z;			
			mInitialized = true;
		} else {
			float deltaX = Math.round(Math.abs(mLastX - x));
			float deltaY = Math.round(Math.abs(mLastY - y));
			float deltaZ = 0.0f;//Math.round(Math.abs(mLastZ - z));
			if (deltaX < NOISE) deltaX = (float)0.0;
			if (deltaY < NOISE) deltaY = (float)0.0;
			if (deltaZ < NOISE) deltaZ = (float)0.0;
				
			HidpBcaster.ReportJoystick(
						 adjSensivity(mLastX+deltaX),
						 adjSensivity(mLastY+deltaY),
						 adjSensivity(mLastZ+deltaZ)*0, 0, 0, 0);			
			mLastX = Math.round(x);
			mLastY = Math.round(y);
			mLastZ = Math.round(z);			
		}
	}
}
	
