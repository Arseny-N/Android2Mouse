#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "log.h"

int open_log(struct log *l,size_t line_size, size_t file_size, char *path) 
{
	l->stream = fopen(path, "w");
	if(!l->stream)
		return -1;
	setbuf(l->stream, NULL);	
	l->line_size = line_size - 6;
	l->file_size = ((file_size / line_size) * line_size);
	l->path = path;
	l->counter = 0;
	l->buf = malloc(line_size);
	if(!l->buf) 
		return -1;
	
	return 0;
}
int close_log(struct log *l) 
{
	if(fclose(l->stream) == -1)
		return -1;
	return 0;
}


void push_string(struct log *l, int level, char *s)
{
	if(ftell(l->stream) >= l->file_size)	
		rewind(l->stream);
	printf("%s\n", s);
	(void) fprintf(l->stream, "[%lu]:%s: %.*s\n", l->counter,GET_LV(level),(int)l->line_size, s);
	fflush(l->stream);
	l->counter += 1;
	
}
void print_string(struct log *l, int level, char *fmt, ...)
{
	va_list ap;
        va_start(ap, fmt);
        if(ftell(l->stream) >= l->file_size)	
		rewind(l->stream);
	(void) vsprintf(l->buf, fmt, ap);
	
	va_end(ap);
	push_string(l, level, l->buf);
	
}

