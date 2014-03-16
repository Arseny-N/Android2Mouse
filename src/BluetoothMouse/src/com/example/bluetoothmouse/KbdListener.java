package com.example.bluetoothmouse;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.widget.TextView;

public class KbdListener extends TextView{
	Context context;
	public KbdListener(Context context) {
		super(context);
		this.context = context;
		// TODO Auto-generated constructor stub
	}
	public KbdListener(Context context, AttributeSet attrs) {
		super(context, attrs);
		// TODO Auto-generated constructor stub
	}
	public KbdListener(Context context, AttributeSet attrs, int defStyle) {
		super( context, attrs,  defStyle);
		// TODO Auto-generated constructor stub
	}
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		Log.d("Listener","keyCode "+keyCode);
		return super.onKeyDown(keyCode, event);			
	}
	
		
}
