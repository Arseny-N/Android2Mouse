package com.example.bluetoothmouse;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

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

public class DeviceSelection extends ListActivity {
    
    private static final int ACTIVITY_EDIT=1;

    List<String> addrs = new ArrayList<String>();
    

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.device_list);
        fillData();
        
    }

	@TargetApi(Build.VERSION_CODES.ECLAIR)
	private void fillData() {
    	 ArrayAdapter<String> devs = new ArrayAdapter<String>(this, R.layout.device_row);
 	    
 	    
 	    BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
 	    if(mBluetoothAdapter == null) {
 	    	Log.d("Choice", " No bluetooth");
 	    }
 	    
 	    if (!mBluetoothAdapter.isEnabled()) {
 	    	Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
 	        startActivity(enableBtIntent);
 	    }
 	        
 	    Set<BluetoothDevice> pairedDevices = mBluetoothAdapter.getBondedDevices();
 	    if (pairedDevices.size() > 0) {
 	    	for (BluetoothDevice device : pairedDevices) {
 	    		devs.add(device.getName());
 	    		addrs.add(device.getAddress());
 	         }
 	    }
 	   setListAdapter(devs);
    }

	public static final String KEY_ADDR= "_addr";

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        super.onListItemClick(l, v, position, id);
        Intent i = new Intent(this, BcastGestures.class);
        i.putExtra(KEY_ADDR, addrs.get(position));
        startActivityForResult(i, ACTIVITY_EDIT);
    }

    @Override
    
    public boolean onCreateOptionsMenu(Menu menu) {
        boolean result = super.onCreateOptionsMenu(menu);
        menu.add(0, 1, 0, "Update List");
        return result;
    }
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case 1:
           fillData();
           return true;
        case 2:
        	
        	return true;
        }
       
        return super.onOptionsItemSelected(item);
    }
}