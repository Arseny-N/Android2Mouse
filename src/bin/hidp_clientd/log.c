#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "log.h"
static int __open_log(struct log *l) 
{
	setbuf(l->stream, NULL);		
	l->counter = 0;
	l->buf = malloc(l->line_size+6);
	if(!l->buf) 
		return -1;
	return 0;
}
static int _open_log(struct log *l,size_t line_size, size_t file_size, FILE *stream, char *path) 
{
	l->stream = stream;
	l->line_size = line_size - 6;
	l->path = path;
	l->file_size = ((file_size / line_size) * line_size);		
	return __open_log(l);
}

int add_arg_to_log(struct log **l, char *arg, char *val) 
{
	
}
int open_log(struct log *l,size_t line_size, size_t file_size, char *path) 
{
	FILE *stream;
	
	if(strcmp(path, "stdout")==0) {
		stream = stdout;
		l->notFile = true;
	} else if(strcmp(path, "sterr")==0) {
		stream = stderr;
		l->notFile = true;
	} else {
		stream = fopen(path, "w");
		if(!stream)
			return -1;	
		l->notFile = false;
	}
	
	return _open_log(l,line_size, file_size, stream, path);	


}

int close_log(struct log *l) 
{
	if(l->notFile)
		return 0;
	if(fclose(l->stream) == -1)
		return -1;
	return 0;
}


void push_string(struct log *l, int level, char *s)
{
	if(!l->notFile && ftell(l->stream) >= l->file_size)	
		rewind(l->stream);
	// printf("%s\n", s);   /* COMMENT ME IFF RUNNING UNDER ANDROID - OR I WILL FILL ALL THE CACHES AND HUNG THE PROG. */
	(void) fprintf(l->stream, "[%lu]:%s: %.*s\n", l->counter,GET_LV(level),(int)l->line_size, s);
	fflush(l->stream);
	l->counter += 1;
	
}
void print_string(struct log *l, int level, char *fmt, ...)
{
	va_list ap;
        va_start(ap, fmt);
       
	(void) vsprintf(l->buf, fmt, ap);
	
	va_end(ap);
	push_string(l, level, l->buf);
	
}

