#include <stdbool.h>
#include "types.h"



#define HCR_SHUTDOWN 	01

#define METHOD_SHUTDOWN 	0x44
#define METHOD_SUCCESS 		0
#define METHOD_ERROR 		-1
extern int int_sock, cnt_sock, src_fd;
extern bdaddr_t daddr, laddr;
#define FIFO_PATH "/data/hidp_clientd.fifo"
//#define FIFO_PATH "fifo.mouse"
#define FIFO_MODE 0777

struct conn_info {
	struct hdr_lookup_funcs *hdr_lookup;

	int (*process_src) (int fd, struct conn_info *info);
	
	void *data;
};

int start_session(char *addr);
int end_session(void);

int send_mouse_report(int x,int y, int b, int f);
int send_shutdown(void);

