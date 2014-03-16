package com.example.common.hidp;

import java.io.IOException;

import android.util.Log;

public class HidpBcaster {
	public static final String TAG = "HidpBcaster";
	
	public static final String LOG_FILE = "/data/hidp_clientd.log";
	
	public static final int MEV_BTN_LEFT = 1;
	public static final int MEV_BTN_RIGHT = 2;
	public static final int MEV_BTN_MIDDLE = 4;
	
	
	public static final int JEV_BTN_A = 1;
	public static final int JEV_BTN_B = 2;
	public static final int JEV_BTN_C = 4;
	public static final int JEV_BTN_D = 8;
	
	public static final int JEV_BTN_1 = 16;
	public static final int JEV_BTN_2 = 32;
	public static final int JEV_BTN_3 = 64;
	public static final int JEV_BTN_4 = 128;
	
	public static final int JEV_BTN_A1 = 256;
	public static final int JEV_BTN_A2 = 512;
	public static final int JEV_BTN_A3 = 1024;
	public static final int JEV_BTN_A4 = 2048;
	
	public static final int JEV_BTN_B1 = 4096;
	public static final int JEV_BTN_B2 = 4096*2;
	public static final int JEV_BTN_B3 = 4096*2*2;
	public static final int JEV_BTN_B4 = 4096*2*2*2;
	
	public static final char KEV_BTN_ENTER	= 	0x28;			
	public static final char KEV_BTN_RIGHT 	= 	0x4f;
	public static final char KEV_BTN_LEFT 	= 	0x50;
	public static final char KEV_BTN_DEL	=	0x2a;
	public static final char KEV_BTN_TAB 	=	0x2b;
	
	
	public static int mouse_buttons = 0;
	public static void PushMouseButton(int btn) {
		mouse_buttons |= btn;
		ReportMouse(0, 0, 0);
	}
	public static void ReleaseMouseButton(int btn) {
		mouse_buttons &= ~btn;
		ReportMouse(0, 0, 0);
	}
	
	
	public static int joystick_buttons = 0;
	public static void PushJoystickButton(int btn) {
		joystick_buttons |= btn;
		ReportJoystick(0, 0, 0, 0, 0, 0);
	}
	public static void ReleaseJoystickButton(int btn) {
		joystick_buttons &= ~btn;
		ReportJoystick(0, 0, 0, 0, 0, 0);
	}
	public static void ReleaseAllButtons() {
		joystick_buttons = 0;
		mouse_buttons = 0;
		ReportJoystick(0, 0, 0, 0, 0, 0);
		ReportMouse(0, 0, 0);
	}
	private static Process server = null;
	private static boolean isServerRunning() {
		int rv;
		try {
			rv = server.exitValue();
		} catch(IllegalThreadStateException e) {
			Log.i(TAG, "Server Running");
			return true;
		}
		Log.e(TAG, "Server exited with value "+ rv);
		
		return false;
	}
	public static boolean StartSession(String address) {
		//KillServer();
		Log.d(TAG, "Execing: su -c /data/hidp_clientd multi "+address);
		 try {
           server = Runtime.getRuntime().exec("su -c /data/hidp_clientd --logfile="+LOG_FILE+" multi "+address);           
        } catch (Exception e) {
        	e.printStackTrace();
        	Log.e(TAG, "Exeption");
        	return false;
        } 
		
		return isServerRunning() ? InitBcast(address) : false;
	}
	public static void KillServer() {
		try {
			Runtime.getRuntime().exec("su -c busybox pkill hidp_clientd");
		} catch (IOException e) {			
			e.printStackTrace();
		}
	}
	public static boolean EndSession() {
		boolean r = EndBcast();	
		ReleaseAllButtons();
		KillServer();
		Log.i("--------->","Ending Session");		
		return r;
		
	}
	public static void ReportMouse(int x, int y, int w) {
			
			reportMouse(x, y, mouse_buttons, w);
			
	}
	public static void ReportJoystick(int x, int y, int z,int rx, int ry, int rz) {
		
		reportGpad(x, y, z,rx, ry, rz, joystick_buttons);
		
	}
	public static void ReportString(String s) {
		
		reportString(s);
		
	}
	public static void ReportAsciiChar(char c) {		
		reportAsciiChar(c, 0);		
	}
	public static void ReportRawChar(char c) {		
		reportRawChar(c, 0);
	}
	public native static boolean InitBcast(String s);
	public native static boolean EndBcast();
	
	
	public native static void reportMouse(int x, int y, int b, int w);
	public native static void reportGpad(int x, int y, int z,int rx, int ry, int rz, int b );
	public native static void reportString(String s);
	public native static void reportAsciiChar(char c, int mod);
	public native static void reportRawChar(char c, int mod);

	 static {
	       System.loadLibrary("JNI_hidp");
	 }

}
