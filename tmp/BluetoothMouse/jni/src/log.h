

#include <errno.h>
#include <string.h>


#ifdef ANDROID_LOG
#include <android/log.h>
#define _error(f, a...) ((void)__android_log_print\
	(ANDROID_LOG_ERROR,"JNI_hidp","%16s:%s (%.2d): " f ,__func__,strerror(errno),errno, ##a))
	
#define _info(f, a...) ((void)__android_log_print(ANDROID_LOG_INFO,"JNI_hidp","%16s:    " f ,__func__, ##a))
#define _dbg(f, a...) ((void)__android_log_print(ANDROID_LOG_DEBUG,"JNI_hidp","%16s:    " f, __func__, ##a))

#else

#define _error(f, a...) fprintf(stderr," ERR: %16s:    " f ,__func__, ##a)
#define _info(f, a...) fprintf(stderr,"INFO: %16s:    " f ,__func__, ##a)
#define _dbg(f, a...) fprintf(stderr," DBG: %16s:    " f, __func__, ##a)

#endif



#define error(f, a...)  _error(f "\n", ##a)
#define info(f, a...) _info(f "\n", ##a)
#define dbg(f, a...) _dbg(f "\n", ##a)
