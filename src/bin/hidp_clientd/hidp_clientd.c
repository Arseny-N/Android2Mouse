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

static int (*process_evfd) (int fd);	



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






static int peek_hidp_hdr(int sk, u8 *hdr)
{
	info("Peeking hdr sk=%d", sk);
	return  recv(sk, hdr, 1, MSG_PEEK); 
}
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



static int send_hidp_report(u8 type, u8 *data, int size, int flags)
{
	return send_hidp_pkt(int_sock,hidp_hdr_build(HIDP_MSGT_DATA, type & 0x3), data, size, flags ,&common_cache);
}
static int send_mouse_rep(u8 b, u8 x, u8 y, u8 w)
{
	u8 m[5] = {1,b, x, y, w};
	
	return send_hidp_report(REPT_INPUT, m, sizeof(m), 0);
}
int process_hdr(u8 hdr, int sk)
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
}  hdr_lookup[] = {
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


int process_sk(int sk) 
{
	u8 hdr;
	int err = peek_hidp_hdr(sk, &hdr);
	info("Got pkt (%x)", hdr);
	
	if(GET_HIDP_TYPE(hdr) <= 0xa && hdr_lookup[GET_HIDP_TYPE(hdr)].process) {
		dbg("processing pkt with method");
		hdr_lookup[GET_HIDP_TYPE(hdr)].process(hdr, sk);
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


int process_mouse(int fd) 
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

int read_connection_header(int fd)
{
	struct connection_header a;		
	if(read(fd, &a, sizeof(a)) != sizeof(a)) {
		error("read");
		return -1;
	}
	a.addr[23] = 0;
	dbg("addr:%s",a.addr);
	if(str2ba(a.addr, &daddr) == -1) {	
		error( "Failed to get dest adress (%s)", a.addr);
		bacpy(&daddr, BDADDR_ANY);
		return -1;
	}
	/* In case of profiles implementation */
	process_evfd = process_mouse;
	
	return 0;
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
void hipd_cleanup(void)
{
	close_sk();
	end_log();
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
int main ( int argc, char * argv [] )
{
	struct pollfd p_fds[3];
	int (*process[3]) (int fd);

	if(start_log("/data/hidp_clientd.log") == -1) {
		exit(EXIT_FAILURE);
	}
	if(mkfifo(FIFO_PATH, FIFO_MODE) == -1 && errno != EEXIST) {
		error("mkfifo %s %o", FIFO_PATH, FIFO_MODE);
		exit(EXIT_FAILURE);
	}
		
	src_fd = open(FIFO_PATH, O_RDONLY);
	if(src_fd == -1) {
		error("open()");
		exit(EXIT_FAILURE);
	}
	dbg("reading connection header");
	if(read_connection_header(src_fd) == -1 ) {
		error("Failed to read connection header");
		exit(EXIT_FAILURE);
	}
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
	process[2] = process_evfd;

	
	info("Started polling.\n");		
	
	for(;;) {
		int i, rv = poll(p_fds, 3, -1);
		if(rv == -1) {
			error("poll");
			exit(EXIT_FAILURE);
		}
	
		
		for(i=0 ;rv && i <3; ++i) {
			if(p_fds[i].revents & POLLIN) {
				info("POLLIN:%d",i);
				switch(process[i](p_fds[i].fd)) {
					case SHUTDOWN:
						info("shutdown");
						exit(EXIT_SUCCESS);
					case -1:
						error("Error processing fd %d", i);
				}
				info("POLL-OK");
				p_fds[i].revents = 0;
				rv --;
			} 
		}
			
			
	}
		
}
	
	
	

