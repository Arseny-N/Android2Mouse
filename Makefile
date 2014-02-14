
ANT ?= ant
ADB ?= adb
MAKE ?= make
CFALGS ?= -Wall
ANT_MODE ?= debug
#CC = arm-linux-gnueabi-gcc
CC = gcc
install: InstallGui InstallBin
build:  BuildBin BuildGui
test: testSu testElf

PullLog:
	adb pull /data/hidp_clientd.log tools/logs/hidp_clientd.log.`date +"%T"`
testSu:
	$(ADB) shell su -c id
testElf: InstallBin
	$(ADB) shell su -c /data/bmouse cod
BuildBin:
	cd ./src/bin/bmouse; $(MAKE) CC=$(CC) CFALGS=$(CFLAGS); #cp bmouse ../../../build 
	cd ./src/bin/hidp_clientd; $(MAKE) CC=$(CC) CFALGS=$(CFLAGS); #cp hidp_clientd ../../../build
BuildGui:
	cd ./src/becomeAMouse; $(ANT) $(ANT_MODE); cd jni; ndk-build; cd ..; cp ./bin/becomeAMouse.apk ../../build;
	cd ./src/BluetoothMouse; $(ANT) $(ANT_MODE);cd jni; ndk-build; cd ..; cp ./bin/BluetoothMouse.apk ../../build;
InstallGui:
	$(ADB) install -r build/becomeAMouse.apk
	$(ADB) install -r build/BluetoothMouse.apk
InstallBin:
	$(ADB) push build/bmouse /sdcard/bmouse; \
	$(ADB) shell su -c 'cat /sdcard/bmouse > /data/bmouse; chmod 0777 /data/bmouse; chown root /data/bmouse'
	$(ADB) push build/hidp_clientd /sdcard/hidp_clientd; \
	$(ADB) shell su -c 'cat /sdcard/hidp_clientd > /data/hidp_clientd; chmod 0777 /data/hidp_clientd; chown root /data/hidp_clientd'


	
