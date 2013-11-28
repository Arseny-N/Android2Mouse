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

import com.example.common.hidp.HidpBcaster;


public class GestureListener extends GestureDetector.SimpleOnGestureListener {

    public static final String TAG = "GestureListener";
    static final int NOT_PUSHED = 0;
    static final int PUSHED = 1;
    
    static int bstate = NOT_PUSHED;
    
    static final int NOT_MOVED = 2;
    static final int MOVED = 3;
    
    static int moved = MOVED;
    @Override
    public void onLongPress(MotionEvent e) {
        // Touch has been long enough to indicate a long press.
        // Does not indicate motion is complete yet (no up event necessarily)
        Log.i(TAG, "Long Press");
        HidpBcaster.ReportEvent(0, 0, HidpBcaster.EV_BTN_RIGHT, 1);
        HidpBcaster.ReportEvent(0, 0, HidpBcaster.EV_BTN_RIGHT, 0);
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX,
    float distanceY) {
        // User attempted to scroll
    	distanceX = -distanceX;
    	distanceY = -distanceY;
    	HidpBcaster.ReportEvent((int) distanceX, (int) distanceY, 0, 0);
        Log.i(TAG, "Scroll x:" + distanceX + " y:" + distanceY);
        moved = MOVED;
        return false;
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
    float velocityY) {
        // Fling event occurred.  Notification of this one happens after an "up" event.
        Log.i(TAG, "Fling");
        return false;
    }
    
  
    
    @Override
    public boolean onDoubleTap(MotionEvent e) {
        // User tapped the screen twice.
    	if(bstate == NOT_PUSHED) 
    		moved = NOT_MOVED;
    	
    	if(bstate == PUSHED && moved == NOT_MOVED) {
    		HidpBcaster.ReportEvent(0, 0, HidpBcaster.EV_BTN_LEFT, 0);
    		HidpBcaster.ReportEvent(0, 0, HidpBcaster.EV_BTN_LEFT, 1);
    		HidpBcaster.ReportEvent(0, 0, HidpBcaster.EV_BTN_LEFT, 0);
    		return false;
    	}
    	
    	bstate = bstate == PUSHED ? NOT_PUSHED : PUSHED;
    	HidpBcaster.ReportEvent(0, 0, HidpBcaster.EV_BTN_LEFT, bstate);
        Log.i(TAG, "Double tap");
        return false;
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e) {
        // A confirmed single-tap event has occurred.  Only called when the detector has
        // determined that the first tap stands alone, and is not part of a double tap.
    	 HidpBcaster.ReportEvent(0, 0, HidpBcaster.EV_BTN_LEFT, 1);
         HidpBcaster.ReportEvent(0, 0, HidpBcaster.EV_BTN_LEFT, 0);
        Log.i(TAG, "Single tap confirmed");
        return false;
    }

}
