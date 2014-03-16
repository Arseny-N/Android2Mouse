/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.example.bluetoothmouse;

import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.widget.TextView;

import com.example.common.hidp.HidpBcaster;


public class MouseGestureListener extends GestureDetector.SimpleOnGestureListener {

    public static final String TAG = "GestureListener";
    static final int NOT_PUSHED = 0;
    static final int PUSHED = 1;
    static float sensivity = 1.0f;
    static int bstate = NOT_PUSHED;
    
    static final int NOT_MOVED = 2;
    static final int MOVED = 3;
    TextView text;
    static int moved = MOVED;
    private int adjSensivity(float val) {
    	return (int)( val * sensivity);
    }
    @Override
    public void onLongPress(MotionEvent e) {
        // Touch has been long enough to indicate a long press.
        // Does not indicate motion is complete yet (no up event necessarily)
        Log.i(TAG, "Long Press");
        this.echo("Right Click");
        HidpBcaster.PushMouseButton(HidpBcaster.MEV_BTN_RIGHT);
   	 	HidpBcaster.ReleaseMouseButton(HidpBcaster.MEV_BTN_RIGHT);
        
    }
    public void init(TextView mtext) {
    	this.text = mtext;
    }
    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX,
    float distanceY) {
        // User attempted to scroll
    	this.echo("");
    	distanceX = -distanceX;
    	distanceY = -distanceY;
    	HidpBcaster.ReportMouse(adjSensivity(distanceX), adjSensivity(distanceY), 0);
        Log.i(TAG, "Scroll x:" + distanceX + " y:" + distanceY);
        moved = MOVED;
        return false;
    }
    public void echo(String msg) {    	        
        //Sets the new text to TextView (runtime click event)
        text.setText(msg);
    }
    
    @Override
    public boolean onDoubleTap(MotionEvent e) {
        // User tapped the screen twice.
    	
    	if(bstate == NOT_PUSHED) 
    		moved = NOT_MOVED;
    	
    	if(bstate == PUSHED && moved == NOT_MOVED) {
    		this.echo("Double Click");
       	 	HidpBcaster.ReleaseMouseButton(HidpBcaster.MEV_BTN_LEFT);
    		HidpBcaster.PushMouseButton(HidpBcaster.MEV_BTN_LEFT);
       	 	HidpBcaster.ReleaseMouseButton(HidpBcaster.MEV_BTN_LEFT);
       	     		
    		return false;
    	}
    	
    	if(bstate == NOT_PUSHED)
    		HidpBcaster.PushMouseButton(HidpBcaster.MEV_BTN_LEFT);
    	else
    		HidpBcaster.ReleaseMouseButton(HidpBcaster.MEV_BTN_LEFT);
    	    	   	     	
    	this.echo("Grasp Mode [" +(bstate != PUSHED ? "On": "Off")+ "]");
        Log.i(TAG, "Double tap");
        bstate = bstate == PUSHED ? NOT_PUSHED : PUSHED;
        return false;
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e) {
        // A confirmed single-tap event has occurred.  Only called when the detector has
        // determined that the first tap stands alone, and is not part of a double tap.
    	 HidpBcaster.PushMouseButton(HidpBcaster.MEV_BTN_LEFT);
    	 HidpBcaster.ReleaseMouseButton(HidpBcaster.MEV_BTN_LEFT); 
    	 this.echo("Left Click");
    	 Log.i(TAG, "Single tap confirmed");
    	 return false;
    }

}
