package com.example.bluetoothmouse;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import com.example.common.hidp.HidpBcaster;

import android.annotation.TargetApi;
import android.app.ListActivity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;

public class JoystickSelection extends ListActivity {
    
    private static final int ACTIVITY_EDIT=1;

    List<String> addrs = new ArrayList<String>();
    

    public void onCreate(Bundle savedInstanceState) {
	   super.onCreate(savedInstanceState);
	    String[] values = new String[] { "Type 1","Type 2","Type 3" };
	    ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
	        android.R.layout.simple_list_item_1, values);
	    setListAdapter(adapter);
	  }

	  @Override
	  protected void onListItemClick(ListView l, View v, int position, long id) {
	    String item = (String) getListAdapter().getItem(position);	    	        	
	    Intent i = new Intent(this, Joystick.class);
	    i.putExtra(DeviceSelection.KEY_ADDR, item);
	    startActivityForResult(i, ACTIVITY_EDIT);
	    		    	    	    
	  }
}