package com.example.bluetoothmouse;

import com.example.common.hidp.HidpBcaster;

import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

public class SelectBcastType extends ListActivity {
    private static final int ACTIVITY_EDIT=1;
    private static String addr;
    private static boolean end = false;
    public void onDestroy() {
    	super.onDestroy();
    	//if(end)
    		HidpBcaster.EndSession();
    		
    }
    
	public void onCreate(Bundle savedInstanceState) {
	    super.onCreate(savedInstanceState);
	    addr = (savedInstanceState == null) ? null :
            (String) savedInstanceState.getString(DeviceSelection.KEY_ADDR);
		if (addr == null) {
			Bundle extras = getIntent().getExtras();
			addr = extras != null ? extras.getString(DeviceSelection.KEY_ADDR) : null;
			
		}
		HidpBcaster.StartSession(addr);
        
	    String[] values = new String[] { "Mouse", "Joystick" };
	    ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
	        android.R.layout.simple_list_item_1, values);
	    setListAdapter(adapter);
	  }

	  @Override
	  protected void onListItemClick(ListView l, View v, int position, long id) {
	    String item = (String) getListAdapter().getItem(position);	    
	    if(item.equals("Mouse")) {
	    	end = false;
	    	Intent i = new Intent(this, Mouse.class);
	    	i.putExtra(DeviceSelection.KEY_ADDR, addr);
	    	startActivityForResult(i, ACTIVITY_EDIT);
	    	
	    }
	    if(item.equals("Joystick")) {
	    	end = false;
	    	Intent i = new Intent(this, JoystickSelection.class);
	    	i.putExtra(DeviceSelection.KEY_ADDR, addr);
	    	startActivityForResult(i, ACTIVITY_EDIT);
	    	
	    }
	    
	    
	  }
}
