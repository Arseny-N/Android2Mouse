call tools\adb.exe install -r build\becomeAMouse.apk
call tools\adb.exe install -r build\BluetoothMouse.apk
call tools\adb.exe push build\bmouse /sdcard/bmouse; 
call tools\adb.exe shell su -c 'cat /sdcard/bmouse > /data/bmouse; chmod 0777 /data/bmouse; chown root /data/bmouse'
call tools\adb.exe push build\hidp_clientd /sdcard/hidp_clientd; 
call tools\adb.exe shell su -c 'cat /sdcard/hidp_clientd > /data/hidp_clientd; chmod 0777 /data/hidp_clientd; chown root /data/hidp_clientd'
