package com.example.bluetoothmouse;
import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.view.MotionEventCompat;
import android.text.Editable;
import android.text.method.KeyListener;
import android.util.Log;
import android.view.GestureDetector;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;

import com.example.common.hidp.HidpBcaster;




@SuppressLint("NewApi")
@TargetApi(Build.VERSION_CODES.CUPCAKE)
public class Mouse extends Activity {
	GestureDetector gd;
	//KbdListener textView;
	boolean graspMode;
	float wheel_sensivity = (float) 0.2;
	SeekBar seekBar;
	public static float sensivity = 1.0f;
	public static float MaxSensivity = 2.0f;
	
	TextView textView;
	Button textButton, buttonEnter,buttonLeft,buttonRight, buttonDel,buttonTab;
	
	SeekBar.OnSeekBarChangeListener SeekBarActivity = new SeekBar.OnSeekBarChangeListener() {

	    @Override
	    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
	        //  Notify that the progress level has changed.
	    	sensivity = (MaxSensivity*progress)/100;
	        Log.d("SeekBar", "SeekBar now at the value of:"+progress);

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

	private Button setUpButton(int id, final char ch, final String s) {
	    	Button b = (Button) findViewById(id);
	    	if(b!= null)
	    		b.setOnClickListener(new View.OnClickListener() {
	    			public void onClick(View v) {	            	
	    				HidpBcaster.ReportRawChar(ch);	            	
	    			}
	    		});
	    	else
	    		Log.e("setUpButton","Failed to init "+s);
	    	return b;
	}
	@Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.mouse);
        //textView = (KbdListener) findViewById(R.id.inputText);
        textView = (TextView) findViewById(R.id.inputText);
        
        textButton = (Button) findViewById(R.id.inputButton); 
        
        seekBar=(SeekBar)findViewById(R.id.seekBar1);        
        buttonEnter = setUpButton(R.id.buttonEnter, HidpBcaster.KEV_BTN_ENTER,	"Enter");
        buttonLeft 	= setUpButton(R.id.buttonLeft, 	HidpBcaster.KEV_BTN_LEFT, 	"Left");
        buttonRight = setUpButton(R.id.buttonRight, HidpBcaster.KEV_BTN_RIGHT,	"Right");
        buttonDel	= setUpButton(R.id.buttonBs, 	HidpBcaster.KEV_BTN_DEL, 	"Del");
        buttonTab	= setUpButton(R.id.buttonTab, 	HidpBcaster.KEV_BTN_TAB, 	"Tab");
        
        textButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
            	String s = textView.getText().toString();
            	HidpBcaster.ReportString(s);
            	textView.setText(null);
            }
        });

        String addr = (savedInstanceState == null) ? null :
            (String) savedInstanceState.getString(DeviceSelection.KEY_ADDR);
		if (addr == null) {
			Bundle extras = getIntent().getExtras();
			addr = extras != null ? extras.getString(DeviceSelection.KEY_ADDR) : null;
			
		}
		//setDislayOptions(DISPLAY_SHOW_TITLE);
		//setSubtitle(addr);
		setTitle("Mouse");
		MouseGestureListener listener = new MouseGestureListener();
		listener.init((TextView) findViewById(R.id.textView1));
        gd = new GestureDetector(this, listener);              
    }
    
    int oldPos = 0;

	@Override
    public boolean onTouchEvent(MotionEvent event) { 
			int actionMasked = event.getActionMasked();
			if(actionMasked == MotionEvent.ACTION_POINTER_DOWN) {
				int index = MotionEventCompat.getActionIndex(event);
				int xPos = (int)MotionEventCompat.getX(event, index);
			    int yPos = (int)MotionEventCompat.getY(event, index);
			    oldPos = yPos;
			    Log.d("Pos", "Y -->> "+yPos);
			}
			if (actionMasked == MotionEvent.ACTION_MOVE && event.getPointerCount() > 1) {
				int index = MotionEventCompat.getActionIndex(event);
				int xPos = (int)MotionEventCompat.getX(event, index);
			    int yPos = (int)MotionEventCompat.getY(event, index);
			    
			    Log.d("Pos", "Y --> "+yPos);
			    HidpBcaster.ReportMouse(0, 0, (int)((oldPos-yPos)*wheel_sensivity));
			    oldPos = yPos;
				return true;
			}else {			
				return gd.onTouchEvent(event);
			}
			
    }
    
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu items for use in the action bar
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.mouse_actions, menu);
        boolean result = super.onCreateOptionsMenu(menu);
        menu.add(0, 11, 0, "Tune Sensivity");
        return result;
    }
    

    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle presses on the action bar items
        switch (item.getItemId()) {
            case R.id.action_search:
            	if(textView.getVisibility() == View.VISIBLE) {
            		String s = textView.getText().toString();
            		//HidpBcaster.ReportString(s);
            	
            		textView.setText(null);
            		textView.setVisibility(View.GONE);
            		textButton.setVisibility(View.GONE);
            		
                    buttonEnter.setVisibility(View.GONE); 
                    buttonLeft.setVisibility(View.GONE);
                    buttonRight.setVisibility(View.GONE);
                    buttonDel.setVisibility(View.GONE);
                    buttonTab.setVisibility(View.GONE);
                    
            	} else {
            		textView.setVisibility(View.VISIBLE);
            		textButton.setVisibility(View.VISIBLE);
            		
            		 buttonEnter.setVisibility(View.VISIBLE); 
                     buttonLeft.setVisibility(View.VISIBLE);
                     buttonRight.setVisibility(View.VISIBLE);
                     buttonDel.setVisibility(View.VISIBLE);
                     buttonTab.setVisibility(View.VISIBLE);
            	}
                return true;
            case 11:
         		if(seekBar.getVisibility() == View.VISIBLE) 
            		seekBar.setVisibility(View.GONE);
            	else 
            		seekBar.setVisibility(View.VISIBLE);	        	
            	return true;	        
                 
            default:
                return super.onOptionsItemSelected(item);
        }
    }
   
}
