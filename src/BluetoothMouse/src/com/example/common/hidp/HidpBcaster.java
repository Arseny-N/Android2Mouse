package com.example.common.hidp;

import java.io.IOException;

import android.util.Log;

public class HidpBcaster {
	public static final String TAG = "HidpBcaster";
	public static final int EV_BTN_LEFT = 1;
	public static final int EV_BTN_RIGHT = 2;
	public static final int EV_BTN_MIDDLE = 3;
	private static Process server = null;
	private static boolean isServerRunning() {
		int rv;
		try {
			rv = server.exitValue();
		} catch(IllegalThreadStateException e) {
			return true;
		}
		Log.e(TAG, "Server exited with value "+ rv);
		return false;
	}
	public static boolean StartSession(String address) {
		
		 try {
           server = Runtime.getRuntime().exec("su -c /data/hidp_clientd");
        } catch (Exception e) {
        	e.printStackTrace();
        	return false;
        } 
		
		return isServerRunning() ? InitBcast(address) : false;
		//return InitBcast(address);
		 
		
	}
	
	public static boolean EndSession() {
		boolean r = EndBcast();
		
		try {
			Runtime.getRuntime().exec("su -c busybox pkill hidp_clientd");
		} catch (IOException e) {			
			e.printStackTrace();
		}
		
		return r;
		
	}
	public static boolean ReportEvent(int x, int y, int b, int t) {
		return EventBcast(x, y, b, t);
		
	}
	
	public native static boolean InitBcast(String s);
	public native static boolean EndBcast();
	public native static boolean EventBcast(int x, int y, int b, int t);

	 static {
	       System.loadLibrary("JNI_hidp");
	 }

}
