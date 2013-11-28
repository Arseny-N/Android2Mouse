#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include "hidp_client.h"

#include "log.h"
static int fd;
int start_session(char *addr) 
{
	int rv;
	fd = open(FIFO_PATH, O_WRONLY);
//	fd = STDOUT_FILENO;

	if(fd == -1) {
		error("open()");
		return -1;
	}

	rv = send_conn_header(addr, 0);
	info("Starting session with fifo %s, rv=%d", FIFO_PATH, rv);
	return rv;
}
int end_session(void) 
{
	send_shutdown();
	return close(fd);
}

int send_report(void *data, size_t size)
{
	char buf[sizeof(struct hcr_mouse_report) + 20];
	int rq_size = 0;
	memset(buf, 0, sizeof(buf));
	if(data) {
		memcpy(buf + rq_size, data, size);
		rq_size += size;
	}
	
	return write(fd, buf, rq_size);
}
int send_shutdown(void)
{
	return send_mouse_report(0, 0,0, HCR_SHUTDOWN);
}
int send_mouse_report(int x,int y, int b, int flags)
{
	struct hcr_mouse_report r = { .x = x, .y = y, .b = b , .flags= flags};
	return send_report(&r, sizeof(r));
}
int send_conn_header(char *addr, int p)
{
	struct connection_header h;
	snprintf(h.addr, 24,"%s", addr);
	return send_report(&h, sizeof(h));
}
/*
int main(int argc, char *argv[])
{
	char *line = NULL, *next;

	if(start_session()) {
		perror("start_session");
		return -1;
	}
#define ERR(call) if(call == -1) {perror( #call ); exit(EXIT_FAILURE); }

	ERR(send_baddr(argv[1], 0));
	
	while(getline(&line, NULL, stdin) != -1) {
		switch(*line) {	

			case 'b': case 'B':
				ERR(send_baddr(&line[1], 1));
				break;
			case 'r': case 'R':
			{
				char *p = &line[1];		
				int x = strtol(p, &next, 0); p = next;
				int y = strtol(p, &next, 0); p = next;
				int b = strtol(p, NULL, 0);
		
				ERR(send_mouse_report(x, y, b));
			}
				break;
			case 's': case 'S':
				ERR(send_shutdown());
				break;
			case 'e': case 'E':
				ERR(end_session());
				return 0;
		}
	}
	return 0;
}
*/
