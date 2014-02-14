#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "bluetooth.h"
#include "l2cap.h"

#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <poll.h>

#include "log.h"

#include "hidp.h"

#include "hidp_client.h"

/* linux/uhid.h ? */
static bdaddr_t daddr, laddr;
static int int_sock = -1, cnt_sock = -1, src_fd = -1;

// static int (*process_evfd) (int fd);	




/* -------- Lightweight cache ----------- */
struct send_cache {
	u8 *buf, *p;
	size_t size;
};
static struct send_cache common_cache;
static void inline zero_cache(struct send_cache *c)
{
	memset(c->buf, 0, c->size);
	c->p = c->buf;
}
static int inline start_cache(struct send_cache *cache, size_t size)
{
	cache-> size = size;
	cache->buf = (u8*) malloc(size);
	if(!cache->buf) {
		error("Failed to malloc(%d)",size);
		return -1;
	}
	zero_cache(cache);
	return 0;
}
static int inline check_cache(struct send_cache *cache, size_t size)
{
	if(size > cache->size) {
		free(cache->buf);
		info("Reallocing cache to %d buytes", size);
		return start_cache(cache, size);
	}
	return 0;
}

static void inline push_cache(struct send_cache *c, u8 *v, size_t size)
{
	memcpy(c->p, v, size);
	c->p += size;
}
static inline u8 *get_cache(struct send_cache *c)
{
	return c->buf;
}
/* -------- Connection related functions ----------- */
static int l2cap_connect(int psm)
{
	struct sockaddr_l2 addr;
	int sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if(sk == -1) {
		error("socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP)");
		return -1;
	}
		


	memset(&addr, 0, sizeof(addr));	
	addr.l2_psm = htobs(psm);
	bacpy(&addr.l2_bdaddr, &daddr);
	addr.l2_family = PF_BLUETOOTH;

	if(connect(sk, (struct sockaddr *) &addr,  sizeof(struct sockaddr_l2)) == -1) {
		error("connect()");		
		goto fail;
	}
	info("Connection enstablished.\n");
	return sk;
fail:
#if 0
	{
	char *str_addr = batostr(&daddr);
#define P(a) errno == a ? #a
	error("psm=%x, bdaddr=%s sk=%d errno=%d(%s)\n", psm, str_addr, sk, errno, 
			 P(EAFNOSUPPORT) :
			 P(EADDRINUSE) : 
			 P(EINVAL) :
			 "?");

	free(str_addr);
#endif	
	close(sk);
	return -1;		
	
}
void close_sk(void)
{
	/* The order matters (see the spec.) */
	dbg("int_sock=%d, cnt_sock=%d", int_sock, cnt_sock);
	if(int_sock != -1)
		close(int_sock);
	if(cnt_sock != -1)
		close(cnt_sock);

}

static void print_sockopts(char *prefix, int sk)
{
	struct l2cap_options opts;
	socklen_t optlen;
	
	memset(&opts, 0, sizeof(opts));
	optlen = sizeof(opts);
	
	if (getsockopt(sk, SOL_L2CAP, L2CAP_OPTIONS, &opts, &optlen) < 0) {
		error( "Can't get default L2CAP options: %s (%d)",
							strerror(errno), errno);
		return;
	}
	if(prefix)
		info("%s", prefix);
	
	info("\tIMTU(%d) OMTU(%d)",opts.imtu,opts.omtu);
}
/* -------- Packet handeling ----------- */
static int peek_hidp_hdr(int sk, u8 *hdr)
{
	info("Peeking hdr sk=%d", sk);
	return  recv(sk, hdr, 1, MSG_PEEK); 
}
static int send_hidp_pkt(int sk, u8 hdr, u8 *data, int size, int flags, struct send_cache *cache)
{
	ssize_t sent;
	info("Sk: %d Pkt: (%.2d) (cache=%d,data=%p)", sk ,size, cache->size);
	
	check_cache(cache, size+1);
	
	zero_cache(cache);
	push_cache(cache,&hdr, 1);
	push_cache(cache,data, size);				

	sent = send(sk, get_cache(cache), size+1, flags);
	dbg("sent=%d",sent);
	return sent;
}



static int recv_hidp_pkt(int sk, u8 *data, int size, int flags)
{
	return recv(sk, data, size, flags);
}
static int drop_hidp_pkt(int sk, int size, int flags)
{
	void *buf = malloc(size);
	int rv;
	if(!buf)
		return -1;
	rv = recv_hidp_pkt(sk, buf, size, flags);
	free(buf);
	info("Reciving pkt on %d, size=%d, falgs=%d", sk, size, flags);
	return rv;
}

static int send_einval(int sk, int flags)
{
	return send_hidp_pkt(sk, hidp_hdr_build(0, 0x4), NULL, 0, flags, &common_cache);
}

#define REPT_OTHER 0
#define REPT_INPUT 1
#define REPT_OUTPUT 2
#define REPT_FEATURE 3

#define send_input_rep(rep) send_hidp_report(REPT_INPUT,(u8 *) &rep, sizeof(rep), 0)


static int send_hidp_report(u8 type, u8 *data, int size, int flags)
{
	return send_hidp_pkt(int_sock,hidp_hdr_build(HIDP_MSGT_DATA, type & 0x3), data, size, flags ,&common_cache);
}

int process_hdr_dull(u8 hdr, int sk)
{
	info("!Processing!\n");
	return 0;
}
int pkt_drop(u8 hdr, int sk)
{
	info("Dropping: (%x)", hdr);
	return drop_hidp_pkt(sk, 1, 0);
}
static struct hdr_lookup_funcs {
	int (*process)(u8 hdr, int sk);
}  hdr_lookup_dropall[] = {
	{ pkt_drop }, /* 0 */
	{ pkt_drop }, /* 1 */
	NULL, /* 2 */
	NULL, /* 3 */
	{ pkt_drop }, /* 4 */
	{ pkt_drop }, /* 5 */
	{ pkt_drop }, /* 6 */
	{ pkt_drop }, /* 7 */
	NULL,
	NULL,
	{ pkt_drop }, /* 10 */
};
struct conn_info {
	struct hdr_lookup_funcs *hdr_lookup;

	int (*process_src) (int fd, struct conn_info *info);
	
	void *data;
};



int process_sk(int sk, struct conn_info *info) 
{
	u8 hdr;
	int err = peek_hidp_hdr(sk, &hdr);
	info("Got pkt (%x)", hdr);
	
	if(GET_HIDP_TYPE(hdr) <= 0xa && info->hdr_lookup[GET_HIDP_TYPE(hdr)].process) {
		dbg("processing pkt with method");
		info->hdr_lookup[GET_HIDP_TYPE(hdr)].process(hdr, sk);
	} else {
		dbg("no method, dropping pkt, sending einval");
		int len = recv_hidp_pkt(sk, &hdr, 1, 0);
		if(len == -1) {
			error("errno=%d",errno);
			error("recv");
			return -1;
		}
			
		if(send_einval(sk, 0) == -1) {
			error("errno=%d",errno);
			error("send");
			return -1;
		}
			
	}
	return 0;
		

}
#define SHUTDOWN 1
/* -------- Mouse conn_info implementation ----------- */


	
static int send_mouse_rep(u8 b, u8 x, u8 y, u8 w)
{
	u8 m[5] = {1,b, x, y, w};
	
	return send_hidp_report(REPT_INPUT, m, sizeof(m), 0);
}
int process_mouse(int fd, struct conn_info *info) 
{
	struct hcr_mouse_report m;
	
	while(read(fd, &m, sizeof(m)) == sizeof(m)) {
		if(m.flags & HCR_SHUTDOWN) {
			dbg("receved SHUTDOWN rq");
			return SHUTDOWN;
		}
		dbg("processing mouse (%d, %d, %x)", m.x, m.y, m.b);
		if(send_mouse_rep(m.b, m.x, m.y, 0) == -1) {
			error("send_mouse_rep()");
			return -1;
		}
	
	}
	return 0;
}
int process_mouse_nice(int fd, struct conn_info *info) 
{
        char *line = NULL;
        size_t dummy;
        ssize_t read;
	dbg("Hello");        
	if(!info->data) {
		info->data = fdopen(fd, "r");
		if(!info->data) {
			error("fdopen(\"r\")");
			return -1;
		} 			
	}
	
        while ((read = getline(&line, &dummy, info->data)) != -1) {
        	int x, y, b , f; 
        	x=y=b=f=0;
        	sscanf(line, "%d %d %d %d", &x, &y, &b, &f); 
        	dbg("Hello");       	
		if(f & HCR_SHUTDOWN) {
			dbg("receved SHUTDOWN rq");
			return SHUTDOWN;
		}
		dbg("processing mouse (%d, %d, %x)", x, y, b);
		if(send_mouse_rep(b, x, y, 0) == -1) {
			error("send_mouse_rep()");
			return -1;
		}
        }

        /* ----- Not freeing the line when exit()'ing, I know ------ */

	return 0;
}
struct conn_info conn_mouse = {
	.hdr_lookup = hdr_lookup_dropall,
	.process_src = process_mouse_nice,
};

/* -------- Multi conn_info implementation ----------- */
#define KBD_REP_ID     1
#define MOUSE_REP_ID   2
#define GPAD_REP_ID    3
struct keyboard_report {
	uint8_t report_id;
	uint8_t modifier;
	uint8_t reserved;
	uint8_t keycodes[6];
};

struct mouse_report {
	uint8_t report_id;
	uint8_t buttons;
	int8_t x;
	int8_t y;	
	int8_t wheel;
};

struct gamepad_report {
	uint8_t report_id;
	uint16_t buttons;
	int8_t lx;
	int8_t ly;
	int8_t rx;
	int8_t ry;
};

static struct keyboard_report kbd_static; 

int press_kbd_char(int rid, uint8_t keycode, uint8_t modifier)
{		

	kbd_static.report_id = rid;
	kbd_static.modifier |= modifier;
	if(kbd_static.keycodes[0] != keycode && kbd_static.keycodes[1] != keycode &&
	   kbd_static.keycodes[2] != keycode && kbd_static.keycodes[3] != keycode &&
	   kbd_static.keycodes[4] != keycode && kbd_static.keycodes[5] != keycode ) {
		int i;
		for(i=0; i<6; ++i) {
			if(kbd_static.keycodes[i] == 0x00) {
				kbd_static.keycodes[i] = keycode;				
				break;
			} 
		}
		if(i == 6) {
			info("Pressed more then 6 buttons");
			return 1;
		}				
	}

	
	if(send_input_rep(kbd) == -1) {
		error("send_input_rep()");
		return -1;
	}
	
	return 0;
}
int release_kbd_char(int rid, uint8_t keycode, uint8_t modifier)
{		

	kbd_static.report_id = rid;
	kbd_static.modifier &= ~modifier;
	
	if(keycode){
		int i;
		for(i=0; i<6; ++i) {
			if(kbd_static.keycodes[i] == keycode) {
				kbd_static.keycodes[i] = 0x00;				
				break;
			} 
		}			
	}
	
	if(send_input_rep(kbd) == -1) {
		error("send_input_rep()");
		return -1;
	}
	
	return 0;
}

int process_multi(int fd, struct conn_info *info) 
{
	struct keyboard_report kbd; //= { .report_id = KBD_REP_ID }; BUGS in gcc ?!
	struct mouse_report    mouse;// = { .report_id = MOUSE_REP_ID };
	struct gamepad_report  gpad = { .report_id = GPAD_REP_ID };
	int i, f;
        char *line = NULL, *tmp_line, *next;
        size_t dummy;
        ssize_t read;
	dbg("Hello");        
	if(!info->data) {
		info->data = fdopen(fd, "r");
		if(!info->data) {
			error("fdopen(\"r\")");
			return -1;
		} 			
	}
#define __ZERO(r, R) \
	memset(&r, 0, sizeof(r));\
	r.report_id = R;
	__ZERO(mouse, MOUSE_REP_ID);	
	__ZERO(kbd, KBD_REP_ID);
        while ((read = getline(&line, &dummy, (FILE*) info->data)) != -1) {


        	switch(line[0]) {
        		case 'f':
      				f=0;
		        	sscanf(&line[1], "%d", &f);
		        	dbg("Hello");       	
				if(f & HCR_SHUTDOWN) {
					dbg("receved SHUTDOWN rq");
					return SHUTDOWN;
				}
        			break;
        		case 'm':
		        	sscanf(&line[1], "%d %d %d %d", &mouse.x, &mouse.y, &mouse.buttons, &mouse.wheel);
   			   	mouse.x = mouse.y = 22;
		   		dbg("processing mouse (%d, %d, 0x%x, %d)", mouse.x, mouse.y, mouse.buttons, mouse.wheel);
		   		dbg("%s", &line[1]);

				if(send_input_rep(mouse) == -1) {
					error("send_input_rep()");
					return -1;
				}
        			__ZERO(mouse, MOUSE_REP_ID);	
				break;		
			case 'k':
				sscanf(&line[1], "%d %d", &kbd.reserved, &kbd.modifier);         			
				
		   		dbg("processing keyboard (%d, %d)",kbd.reserved, kbd.modifier);
		   		break;
			case 'K':
				i=0; tmp_line = &line[1];
				
				while( i < 6){					
					kbd.keycodes[i++] = strtol(tmp_line , &next, 0);			
					tmp_line = next;
				} 
				dbg("processing keyboard (%d, %d)",kbd.reserved, kbd.modifier);
		   		dbg("processing keycodes (0x%x 0x%x 0x%x 0x%x 0x%x 0x%x)",kbd.keycodes[0],
		   									kbd.keycodes[1],kbd.keycodes[2],kbd.keycodes[3],kbd.keycodes[4], kbd.keycodes[5]);
				if(send_input_rep(kbd) == -1) {
					error("send_input_rep()");
					return -1;
				}
				__ZERO(kbd, KBD_REP_ID);
				break;
//			case 'g':
//				sscanf(&line[1], "%d %d %d %d", &x, &y, &b, &f);         			
//		   		dbg("processing mouse (%d, %d, %x)", x, y, b);
//				if(send_input_rep(gpad) == -1) {
//					error("send_mouse_rep()");
//					return -1;
//				}
//				__ZERO(gpad, GPAD_REP_ID);
        	}
#undef __ZERO
        	
        }
        
	free(line);
	
	return 0;
}
struct conn_info conn_multi = {
	.hdr_lookup = hdr_lookup_dropall,
	.process_src = process_multi,
};

static struct {
	char *name;
	struct conn_info *info;
} conn_lookup[] = {
	{ .name = "mouse", .info = &conn_mouse },
	{ .name = "multi", .info = &conn_multi }
};
static inline struct conn_info *lookup_conn_type(char *name) 
{
#define ARR_SIZE(a) sizeof(a)/sizeof(a[0])
	int i;
	for(i=0; i<ARR_SIZE(conn_lookup); ++i) 
		if(strcmp(name, conn_lookup[i].name) == 0)
			return conn_lookup[i].info;
	return NULL;
}

struct {
	char *addr;
	char *type;
} cmd_line;

int init_conn(struct conn_info **info)
{
	dbg("addr:%s",cmd_line.addr);
	if(str2ba(cmd_line.addr, &daddr) == -1) {	
		error( "Failed to get dest adress (%s)", cmd_line.addr);
		bacpy(&daddr, BDADDR_ANY);
		return -1;
	}
	*info = lookup_conn_type(cmd_line.type);
	if(!*info) {
		error( "Failed to get connection type (%s)", cmd_line.type);
		return -1;
	}
	info("Starting with %s conn type.",cmd_line.type);
	return 0;
}

void hipd_cleanup(void)
{
	close_sk();
	end_log();
}

static void print_signal(int s, siginfo_t *t, void *v)
{
	info("Signal %d(%s) caught.", s, strsignal(s));
}
static int global_sighandler(void (*sighandler)(int , siginfo_t *, void *))
{
	struct sigaction sa;
	int i;
	
	memset(&sa, 0, sizeof(sa));
	
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = sighandler;
	
	for(i=0; i< _NSIG; i++) 
		if(sigaction(i, &sa, NULL) == -1 && errno != EINVAL)
			return -1;
	return 0;
}
#define FIFO_PATH "fifo.mouse"
int main ( int argc, char * argv [] )
{
	struct pollfd p_fds[3];
	struct conn_info *conn_info;
	int (*process[3]) (int fd, struct conn_info *info);

	
	
	if(start_log("hidp_clientd.log") == -1) {
		exit(EXIT_FAILURE);
	}
	if(!argv[1] || !argv[2]) {
usage:
		info("Usage: %s <conn-type> <bluetooth-address>");
		exit(EXIT_FAILURE);		
	}
	cmd_line.type = argv[1];
	cmd_line.addr = argv[2];
	
	if(mkfifo(FIFO_PATH, FIFO_MODE) == -1 && errno != EEXIST) {
		error("mkfifo %s %o", FIFO_PATH, FIFO_MODE);
		exit(EXIT_FAILURE);
	}

	src_fd = open(FIFO_PATH, O_RDONLY);
	if(src_fd == -1) {
		error("open()");
		exit(EXIT_FAILURE);
	
	}

	
	printf("Hello\n");
	dbg("reading connection header");
	if(init_conn(&conn_info) == -1 ) {
		error("Failed to init connection.");
		goto usage;
	}
	
	/* Assign the header somewhere */
	
	if(start_cache(&common_cache, 16)) {
		error("Failed to start_cache(16)");
		exit(EXIT_FAILURE);
	}

	if(atexit(hipd_cleanup) != 0) {
		error("atexit");
		exit(EXIT_FAILURE);
	}	
	info("Start");
	
	cnt_sock = l2cap_connect(0x11); /* Control channel */
	if(cnt_sock == -1) {
		error("Failed to open cnt socket");
		exit(EXIT_FAILURE);
	}
	
	int_sock = l2cap_connect(0x13); /* Interrupt channel */
	if(int_sock == -1) {
		error("Failed to open int socket");
		exit(EXIT_FAILURE);
	}

	print_sockopts("INT:",int_sock);
	print_sockopts("CNT:",cnt_sock);
	
	info("cnt_sock(%i),int_sock(%i),src_fd(%i)",cnt_sock,int_sock,src_fd);	
		
	memset(p_fds, 0, sizeof(p_fds));
	
	p_fds[0].fd = cnt_sock;
	p_fds[1].fd = int_sock;
	p_fds[2].fd = src_fd;
	
	p_fds[0].events = p_fds[1].events = p_fds[2].events = POLLIN;
	
	process[0] = process[1] = process_sk;
	process[2] = conn_info->process_src;

	
	info("Started polling.\n");		
	
	for(;;) {
		int i, rv = poll(p_fds, 3, -1);
		if(rv == -1) {
			error("poll");
			exit(EXIT_FAILURE);
		}
	
		
		for(i=0 ;rv && i <3; ++i) {
			if(p_fds[i].revents & POLLIN) {
				info("========================> POLLIN:%d",i);
				switch(process[i](p_fds[i].fd, conn_info)) {
					case SHUTDOWN:
						info("shutdown");
						exit(EXIT_SUCCESS);
					case -1:
						error("Error processing fd %d", i);
				}
				info("POLL-OK\n");
				p_fds[i].revents = 0;
				rv --;
			} 
		}
			
			
	}
		
}
	
	
	

