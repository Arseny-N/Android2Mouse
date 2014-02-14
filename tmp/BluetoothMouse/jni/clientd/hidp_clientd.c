#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "bluetooth.h"
#include "l2cap.h"

#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <poll.h>

#include "log.h"

#include "hidp.h"

#include "hidp_client.h"
#include "self_repr.h"

/* linux/uhid.h ? */
static bdaddr_t daddr, laddr;
static int int_sock = -1, cnt_sock = -1, src_fd = -1;
static int (*process_evfd) (int fd) = NULL;

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
/* TODO: implement cahing :) */
static int send_hidp_pkt(int sk, u8 hdr, u8 *data, int size, int flags)
{
	ssize_t sent;
	u8 *p = &hdr;
	if(size) {
		p = (u8*) malloc(size+1);
		if(!p) 
			return -1;
		
		memcpy(p, &hdr, 1);
		memcpy(p+1, data, size);
	}
	int i;
	info("Sk: %d Pkt: (%.2d) ", sk ,size);

	sent = send(sk, p, size+1, flags);
	if(size) 
		free(p);
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
	return send_hidp_pkt(sk, hidp_hdr_build(0, 0x4), NULL, 0, flags);
}

#define REPT_OTHER 0
#define REPT_INPUT 1
#define REPT_OUTPUT 2
#define REPT_FEATURE 3



static int send_hidp_report(u8 type, u8 *data, int size, int flags)
{
	return send_hidp_pkt(int_sock,hidp_hdr_build(HIDP_MSGT_DATA, type & 0x3), data, size, flags );
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
	info("Dropping: (%x)\n", hdr);
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
	info("Got pkt (%x)\n", GET_HIDP_TYPE(hdr));
	
	if(GET_HIDP_TYPE(hdr) <= 0xa && hdr_lookup[GET_HIDP_TYPE(hdr)].process)
		hdr_lookup[GET_HIDP_TYPE(hdr)].process(hdr, sk);
	else {
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
		if(m.flags & HCR_SHUTDOWN)
			return SHUTDOWN;
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
	if(int_sock != -1)
		close(int_sock);
	if(cnt_sock != -1)
		close(cnt_sock);
	dbg("Here");
}
void hipd_cleanup(void)
{
	close_sk();
//	unapply_mouse();
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
		info("%s\n", prefix);
	
	info("\tIMTU(%d) OMTU(%d)\n",opts.imtu,opts.omtu);
}


int main ( int argc, char * argv [] )
{
	struct pollfd p_fds[3];
	int (*process[3]) (int fd);
	
	dbg("Here");
	if(mkfifo(FIFO_PATH, FIFO_MODE) == -1 && errno != EEXIST) {
		error("mkfifo %s %o", FIFO_PATH, FIFO_MODE);
		exit(EXIT_FAILURE);
	}
		dbg("Here");
	src_fd = open(FIFO_PATH, O_RDONLY);
	if(src_fd == -1) {
		error("open()");
		exit(EXIT_FAILURE);
	}
	dbg("Here");
	if(read_connection_header(src_fd) == -1 ) {
		error("Failed to read bdaddr");
		exit(EXIT_FAILURE);
	}
	
//	if(apply_mouse()) {
//		error("apply_mouse");
//		exit(EXIT_FAILURE);
//	}
	if(atexit(hipd_cleanup) != 0) {
		error("atexit");
		exit(EXIT_FAILURE);
	}	
	info("Start");
	
	cnt_sock = l2cap_connect(0x11); /* Control channel */
	if(cnt_sock == -1)
		exit(EXIT_FAILURE);
	
	int_sock = l2cap_connect(0x13); /* Interrupt channel */
	if(int_sock == -1)
		exit(EXIT_FAILURE);
	

	print_sockopts("INT:",int_sock);
	print_sockopts("CNT:",cnt_sock);
	
	info("cnt_sock(%i),int_sock(%i),src_fd(%i)\n",cnt_sock,int_sock,src_fd);	
		
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
				info("POLLIN:%d\n",i);
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
	
	
	

