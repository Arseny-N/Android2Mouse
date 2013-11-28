#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


#include <sys/types.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <poll.h>
#include <pthread.h>

#include <string.h>
#include <jni.h>
#define ANDROID_LOG
#include "log.h"



  /* 
   com.example.common.hidp.HidpBcaster.InitBcast 
   com.example.common.hidp.HidpBcaster.InitBcast
   */
JNIEXPORT jboolean JNICALL Java_com_example_common_hidp_HidpBcaster_InitBcast( JNIEnv* env,
                                                  jobject thiz,
                                                  jstring javaString )
{
	jboolean rv = 1;
	 /*Get the native string from javaString*/
	const char *nativeString = (*env)->GetStringUTFChars(env, javaString, 0);
	
	dbg("addr:%s", nativeString);
	

	
	if( start_session(nativeString) == -1) {
		error("Failed to init_hidp with address(%s)", nativeString);
		goto r;
	}
	
	rv = 0;
r:
	(*env)->ReleaseStringUTFChars(env, javaString, nativeString);
    	return rv;
}
JNIEXPORT jboolean JNICALL Java_com_example_common_hidp_HidpBcaster_EndBcast( JNIEnv* env, jobject thiz )
{
	dbg("exitting");
	end_session();
    	return 0;
}

#define EV_BTN_LEFT   1
#define EV_BTN_RIGHT  2
#define EV_BTN_MIDDLE 3

JNIEXPORT jboolean JNICALL Java_com_example_common_hidp_HidpBcaster_EventBcast( JNIEnv* env, jobject thiz, jint x, jint y, jint b_event, jint b_type)
{
	static int b = 0;	
	dbg("x:%d y:%d, b_event:%d, b_type:%d", x, y, b_event, b_type);
	
	switch (b_event) {	
		case EV_BTN_LEFT:  if(b_type) b |= 1; else b &= ~1;  break;
		case EV_BTN_RIGHT: if(b_type) b |= 2; else b &= ~2;  break;
		case EV_BTN_MIDDLE:if(b_type) b |= 4; else b &= ~4;  break; 
	}
	if(send_mouse_report(x, y, b, 0) == -1) {
		error("send_mouse_rep()");
		return;
	}		
	
}

