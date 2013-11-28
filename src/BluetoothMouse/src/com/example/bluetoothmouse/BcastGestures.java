package com.example.bluetoothmouse;
import android.annotation.TargetApi;
import android.app.Activity;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;
import com.example.common.hidp.HidpBcaster;




@TargetApi(Build.VERSION_CODES.CUPCAKE)
public class BcastGestures extends Activity {
	GestureDetector gd;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.bcast_gestures);
        String addr = (savedInstanceState == null) ? null :
            (String) savedInstanceState.getString(DeviceSelection.KEY_ADDR);
		if (addr == null) {
			Bundle extras = getIntent().getExtras();
			addr = extras != null ? extras.getString(DeviceSelection.KEY_ADDR) : null;
			
		}
		setTitle(addr);
		GestureDetector.SimpleOnGestureListener gestureListener = getGestureListener();
        gd = new GestureDetector(this, gestureListener);
        
        HidpBcaster.StartSession(addr);
    }
    
    @Override
	protected void onDestroy() {
		super.onDestroy();
		HidpBcaster.EndSession();
		
	}

	@Override
    public boolean onTouchEvent(MotionEvent event)
    {       
        return gd.onTouchEvent(event);
    }
    public GestureDetector.SimpleOnGestureListener getGestureListener() {
        return new GestureListener();
    }
}
