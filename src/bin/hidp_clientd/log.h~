

#include <errno.h>
#include <string.h>
#include <stdbool.h>


#define L_DBG  0x0
#define L_ERR  0x1
#define L_INFO 0x2

#define GET_LV(i) (i== L_DBG ? "DBG" : i == L_ERR ? "ERR" : i== L_INFO ? "INF" : "UNK" )

struct log {
	size_t line_size;
	size_t file_size;
	FILE *stream;
	char *path;
	char *buf;
	
	bool notFile;
	unsigned long counter;
} __log;

#ifdef ANDROID_LOG
#include <android/log.h>
#define start_log(path) 0
#define end_log() 0

#define _error(f, a...) ((void)__android_log_print(ANDROID_LOG_ERROR,"JNI_hidp","%16s:%s (%.2d): " f ,__func__,strerror(errno),errno, ##a))
#define _info(f, a...) ((void)__android_log_print(ANDROID_LOG_INFO,"JNI_hidp","%16s:    " f ,__func__, ##a))
#define _dbg(f, a...)  ((void)__android_log_print(ANDROID_LOG_DEBUG,"JNI_hidp","%16s:    " f, __func__, ##a))

#define error(f, a...)  _error(f "\n", ##a)
#define info(f, a...) _info(f "\n", ##a)
#define dbg(f, a...) _dbg(f "\n", ##a)
#elif defined STDOUT_LOG

#define _error(f, a...) fprintf (stderr," ERR: %16s:%s (%.2d): " f ,__func__,strerror(errno),errno, ##a))
#define _info(f, a...)  fprintf(stderr,"INFO: %16s:    " f ,__func__, ##a)
#define _dbg(f, a...)   fprintf(stderr," DBG: %16s:    " f, __func__, ##a)

#define start_log(path) 0
#define end_log() 0

#define error(f, a...)  _error(f "\n", ##a)
#define info(f, a...) _info(f "\n", ##a)
#define dbg(f, a...) _dbg(f "\n", ##a)
#else

void print_string(struct log *l, int level, char *fmt, ...);
void push_string(struct log *l, int level, char *s);
int close_log(struct log *l);
int open_log(struct log *l,size_t line_size, size_t file_size, char *path);

#define start_log(path) open_log(&__log, 64,1024*64, path)
#define end_log() close_log(&__log)


#define error(f, a...) print_string(&__log, L_ERR, "%s:%s(%.2d):" f ,__func__,strerror(errno),errno, ##a)
#define info(f, a...)  print_string(&__log, L_INFO,"%s:" f ,__func__, ##a)
#define dbg(f, a...)   print_string(&__log, L_DBG, "%s:" f, __func__, ##a)

#endif







