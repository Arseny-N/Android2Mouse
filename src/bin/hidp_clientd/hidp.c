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
#include "combo_conn.h"

/* -------- Lightweight cache ----------- */

struct send_cache common_cache, drop_cache;
static void inline zero_cache(struct send_cache *c)
{
	memset(c->buf, 0, c->size);
	c->p = c->buf;
}

int start_cache(struct send_cache *cache, size_t size)
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
int l2cap_connect(int psm)
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

void print_sockopts(char *prefix, int sk)
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
int peek_hidp_hdr(int sk, u8 *hdr)
{
	info("Peeking hdr sk=%d", sk);
	return  recv(sk, hdr, 1, MSG_PEEK); 
}
int send_hidp_pkt(int sk, u8 hdr, u8 *data, int size, int flags, struct send_cache *cache)
{
	ssize_t sent;
	info("Sk: %d Pkt: (%.2d) (cache=%d) ", sk ,size, cache->size);
	int i;
	
	/*printf("data: [");
	for(i=0; i<size; ++i){
		printf(" 0x%x,", data[i]);
	}
	printf("]\n");*/
	check_cache(cache, size+1);
	
	zero_cache(cache);
	push_cache(cache,&hdr, 1);
	push_cache(cache,data, size);				

	sent = send(sk, get_cache(cache), size+1, flags);
	dbg("sent=%d",sent);
	return sent;
}



int recv_hidp_pkt(int sk, u8 *data, int size, int flags)
{
	return recv(sk, data, size, flags);
}
int drop_hidp_pkt(int sk, int size, int flags, struct send_cache *cache)
{
	check_cache(cache, size+1);
	int rv;

	rv = recv_hidp_pkt(sk, get_cache(cache), size, flags);
/*	int i;
	info("Dropping this ----------V");
	for(i=0; i<size; ++i){
		printf(" 0x%x", ((u8*) buf)[i]);
	}
	*/
	info("Reciving pkt on %d, size=%d, falgs=%d", sk, size, flags);
	return rv;
}

int send_einval(int sk, int flags)
{
	return send_hidp_pkt(sk, hidp_hdr_build(0, 0x4), NULL, 0, flags, &common_cache);
}

int send_hidp_report(u8 type, u8 *data, int size, int flags)
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
	return drop_hidp_pkt(sk, 1, 0, &drop_cache) == -1 ? METHOD_ERROR : METHOD_SUCCESS;
}
struct hdr_lookup_funcs hdr_lookup_dropall[] = {
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

