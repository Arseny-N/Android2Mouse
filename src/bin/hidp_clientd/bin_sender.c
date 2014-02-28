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


#include "log.h"

#include "hidp.h"

#include "hidp_client.h"
#include "combo_conn.h"
struct bin_packet packet;

static char *bpkt_lookup[] = { "null","kbd", "mouse", "gpad", "sys","test" };
static inline int lookup_bpkt(char *name) 
{
#define ARR_SIZE(a) sizeof(a)/sizeof(a[0])
	int i;
	for(i=0; i<ARR_SIZE(bpkt_lookup); ++i) 
		if(strcmp(name, bpkt_lookup[i]) == 0)
			return i;
	return -1;
}
void send_packet(struct bin_packet *p)
{
	int fd = open(FIFO_PATH,O_WRONLY);
	if(fd==-1) {
		error("open() " FIFO_PATH);
		exit(EXIT_FAILURE);
	}
	write(fd, (void*)p, sizeof(*p));
	info("%d packet(%d) -> %s", sizeof(*p), p->report_id, FIFO_PATH);
}
static void send_string_frag(char *s, size_t size)
{
//	struct bin_packet *p = get_packet(KBD_REP_ID);
//	p->flags = BRPT_STRING;	
	char arr[MAX_STRING+1];
	memset(arr, '\0', sizeof(arr));
	strncpy(arr, s, size);
	arr[MAX_STRING] = '\0';
	printf("(%.2d)-->%s\n", size,arr);
}
int send_str_report(char *s, size_t size)
{
	int i, frags = size / MAX_STRING /*+ (size % MAX_STRING != 0)*/, s_ind=0;

	for(i=0; i<frags; ++i) {
		send_string_frag(&s[s_ind], MAX_STRING);
		s_ind += MAX_STRING;
	}
	if(size % MAX_STRING) {
		send_string_frag(&s[s_ind], size-s_ind);
	}

}
int main(int argc, char *argv[])
{
	if(start_log("hidp_clientd.log") == -1) {
		printf("failed to init logs...\n");
		exit(EXIT_FAILURE);
	}
	if(argc < 3) {
usage:
		info("Usage: %s rid flgs ....", argv[0]);
		exit(EXIT_FAILURE);
	}
	int i;
	u8 *p;
	
	packet.report_id = lookup_bpkt(argv[1]);
	packet.flags = strtol(argv[2], NULL, 0);
	info (" %d %d", packet.report_id,	packet.flags);
	switch(packet.report_id) {
        		case KBD_REP_ID: /* Keyboard */	        	

        			if(packet.flags & BRPT_STRING) {
					snprintf(packet.report.string.s, MAX_STRING, "%s", argv[3]);
				
				} else if(packet.flags & BRPT_CHAR)  {
					packet.report.letter.c = strtol(argv[3] , NULL, 0);
					packet.report.letter.mods = strtol(argv[4] , NULL, 0);
				} else {
				        if(argc < 10)
        					goto usage;
	        			packet.report.keyboard.modifier = strtol(argv[3] , NULL, 0);
					for(i=0; i< 6; i++)
						packet.report.keyboard.keycodes[i] = strtol(argv[4+i] ,NULL, 0);			
				}			
				break;        		
        		case  GPAD_REP_ID: /* Gamepad */     
        		        if(argc < 10)
        				goto usage;
	       			packet.report.gamepad.buttons = strtol(argv[3] , NULL, 0);	 
        			p = &(packet.report.gamepad.x);
				for(i=0; i< 6; i++) 
					 p[i] = strtol(argv[4+i] ,NULL, 0);				
				break;		        	
			case  MOUSE_REP_ID: /* Mouse */ 
			        if(argc < 7)
        				goto usage;
        			p = &(packet.report.mouse.buttons);
				for(i=0; i< 4; i++) 
					 p[i] = strtol(argv[3+i] ,NULL, 0);				
				break;		        		
			case SYS_REP_ID: /* for IPC */	        				

        			break;
        		case 5:
        			send_str_report(argv[3], strlen(argv[3]));
        			break;
				
        }
 	send_packet(&packet);
	return 0;
        	
}

