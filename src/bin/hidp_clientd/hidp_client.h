
#include <stdbool.h>

struct hcr_mouse_report {
	int x, y, b;
	int flags; /* Shutdown */
};

struct connection_header {
	char addr[24];
//	int profile;
};

//#define PROFILE_MOUSE 	01


#define HCR_SHUTDOWN 	01


// #define FIFO_PATH "/data/hidp_clientd.fifo"
#define FIFO_MODE 0777

int start_session(char *addr);
int end_session(void);

int send_mouse_report(int x,int y, int b, int f);
int send_shutdown(void);

