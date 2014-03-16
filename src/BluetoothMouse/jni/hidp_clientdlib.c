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
#include "combo_conn.h"

#include "log.h"


static struct bin_packet packet;
static int fd;
void send_packet( struct bin_packet *p)
{
	int numWritten = write(fd, (void*)p, sizeof(*p));
	if( numWritten == sizeof(*p))
		return;
	error("Failed to send packet: report_id=0x%d wrote %d of %d", p->report_id, numWritten, sizeof(*p));
}

struct bin_packet *get_packet(int rid)
{	
	memset(&packet, 0, sizeof(packet));
	packet.report_id = rid;
	return &packet;
}


int start_session(const char *addr) 
{

	fd = open(FIFO_PATH, O_WRONLY);
	if(fd == -1) {
		error("open()");
		return -1;
	}
	info("Starting session with fifo %s", FIFO_PATH);
	return 0;
}
int end_session(void) 
{
	struct bin_packet *p = get_packet(SYS_REP_ID);
	p->flags = HCR_SHUTDOWN;
	send_packet(p);
	
	return close(fd);
}

void send_mouse_report(int x, int y,int buttons, int wheel)
{
	struct bin_packet *p = get_packet(MOUSE_REP_ID);
	p->report.mouse.buttons = buttons;
	p->report.mouse.x = x;
	p->report.mouse.y = y;
	p->report.mouse.wheel = wheel;
	send_packet(p);	
}
 void _send_char_report(int ch, int mod, int flg)
{	
	struct bin_packet *p = get_packet(KBD_REP_ID);
	p->flags = BRPT_CHAR;
	p->flags2 = flg;
	p->report.letter.c = ch;
	p->report.letter.mods = mod;
	send_packet(p);	
}

static void send_string_frag(const char *s, size_t size)
{
	struct bin_packet *p = get_packet(KBD_REP_ID);
	p->flags = BRPT_STRING;
	strncpy(p->report.string.s, s, size);
	send_packet(p);	
}
void send_str_report(const char *s)
{
	size_t size = strlen(s);
	int i, frags = size / MAX_STRING, s_ind=0;
	for(i=0; i<frags; ++i) {
		send_string_frag(&s[s_ind], MAX_STRING);
		s_ind += MAX_STRING;
	}
	if(size % MAX_STRING) {
		send_string_frag(&s[s_ind], size-s_ind);
	}
}
void send_gpad_report(int x,int y,int z,int rx,int ry,int rz,int buttons)
{
	struct bin_packet *p = get_packet(GPAD_REP_ID);
	
#define S(a) p->report.gamepad.a = a
	S(x);S(y);
	S(z);S(rx);
	S(rz);S(ry);
	S(buttons);
#undef S	
	send_packet(p);	
}
