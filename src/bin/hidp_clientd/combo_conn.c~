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


/* -------- Multi conn_info implementation ----------- */
#define SHIFT 0x80

static const uint8_t kbd_asciimap[] =
{
	0x00,             // NUL
	0x00,             // SOH
	0x00,             // STX
	0x00,             // ETX
	0x00,             // EOT
	0x00,             // ENQ
	0x00,             // ACK  
	0x00,             // BEL
	0x2a,			// BS	Backspace
	0x2b,			// TAB	Tab
	0x28,			// LF	Enter
	0x00,             // VT 
	0x00,             // FF 
	0x00,             // CR 
	0x00,             // SO 
	0x00,             // SI 
	0x00,             // DEL
	0x00,             // DC1
	0x00,             // DC2
	0x00,             // DC3
	0x00,             // DC4
	0x00,             // NAK
	0x00,             // SYN
	0x00,             // ETB
	0x00,             // CAN
	0x00,             // EM 
	0x00,             // SUB
	0x00,             // ESC
	0x00,             // FS 
	0x00,             // GS 
	0x00,             // RS 
	0x00,             // US 

	0x2c,		   //  ' '
	0x1e|SHIFT,	   // !
	0x34|SHIFT,	   // "
	0x20|SHIFT,    // #
	0x21|SHIFT,    // $
	0x22|SHIFT,    // %
	0x24|SHIFT,    // &
	0x34,          // '
	0x26|SHIFT,    // (
	0x27|SHIFT,    // )
	0x25|SHIFT,    // *
	0x2e|SHIFT,    // +
	0x36,          // ,
	0x2d,          // -
	0x37,          // .
	0x38,          // /
	0x27,          // 0
	0x1e,          // 1
	0x1f,          // 2
	0x20,          // 3
	0x21,          // 4
	0x22,          // 5
	0x23,          // 6
	0x24,          // 7
	0x25,          // 8
	0x26,          // 9
	0x33|SHIFT,      // :
	0x33,          // ;
	0x36|SHIFT,      // <
	0x2e,          // =
	0x37|SHIFT,      // >
	0x38|SHIFT,      // ?
	0x1f|SHIFT,      // @
	0x04|SHIFT,      // A
	0x05|SHIFT,      // B
	0x06|SHIFT,      // C
	0x07|SHIFT,      // D
	0x08|SHIFT,      // E
	0x09|SHIFT,      // F
	0x0a|SHIFT,      // G
	0x0b|SHIFT,      // H
	0x0c|SHIFT,      // I
	0x0d|SHIFT,      // J
	0x0e|SHIFT,      // K
	0x0f|SHIFT,      // L
	0x10|SHIFT,      // M
	0x11|SHIFT,      // N
	0x12|SHIFT,      // O
	0x13|SHIFT,      // P
	0x14|SHIFT,      // Q
	0x15|SHIFT,      // R
	0x16|SHIFT,      // S
	0x17|SHIFT,      // T
	0x18|SHIFT,      // U
	0x19|SHIFT,      // V
	0x1a|SHIFT,      // W
	0x1b|SHIFT,      // X
	0x1c|SHIFT,      // Y
	0x1d|SHIFT,      // Z
	0x2f,          // [
	0x31,          // bslash
	0x30,          // ]
	0x23|SHIFT,    // ^
	0x2d|SHIFT,    // _
	0x35,          // `
	0x04,          // a
	0x05,          // b
	0x06,          // c
	0x07,          // d
	0x08,          // e
	0x09,          // f
	0x0a,          // g
	0x0b,          // h
	0x0c,          // i
	0x0d,          // j
	0x0e,          // k
	0x0f,          // l
	0x10,          // m
	0x11,          // n
	0x12,          // o
	0x13,          // p
	0x14,          // q
	0x15,          // r
	0x16,          // s
	0x17,          // t
	0x18,          // u
	0x19,          // v
	0x1a,          // w
	0x1b,          // x
	0x1c,          // y
	0x1d,          // z
	0x2f|SHIFT,    // 
	0x31|SHIFT,    // |
	0x30|SHIFT,    // }
	0x35|SHIFT,    // ~
	0				// DEL
};
#define KBD_MOD_CTRL		1 //0x80
#define KBD_MOD_SHIFT		2 //0x81
#define KBD_MOD_ALT		4 //0x82
#define KBD_MOD_GUI		8 //0x83

#define KBD_GETCH_MODS(c) ( c=='c' ? KBD_MOD_CTRL : c=='a' ? KBD_MOD_ALT : c=='s' ? KBD_MOD_SHIFT : c=='g' ? KBD_MOD_GUI : 0 )



static int press_kbd_char(struct keyboard_report *kbd, uint8_t keycode, uint8_t modifier)
{		

	kbd->modifier |= modifier;
	if(kbd->keycodes[0] != keycode && kbd->keycodes[1] != keycode &&
	   kbd->keycodes[2] != keycode && kbd->keycodes[3] != keycode &&
	   kbd->keycodes[4] != keycode && kbd->keycodes[5] != keycode ) {
		int i;
		for(i=0; i<6; ++i) {
			if(kbd->keycodes[i] == 0x00) {
				kbd->keycodes[i] = keycode;
				break;
			} 
		}
		if(i == 6) {
			info("Pressed more then 6 buttons");
			return 1;
		}				
	}
	dbg("processing keyboard (%d, %d)",kbd->reserved, kbd->modifier);
	dbg("processing keycodes (0x%x 0x%x 0x%x 0x%x 0x%x 0x%x)",kbd->keycodes[0],kbd->keycodes[1],kbd->keycodes[2],
								  kbd->keycodes[3],kbd->keycodes[4],kbd->keycodes[5]);
	if(send_input_rep(kbd) == -1) {
		error("send_input_rep()");
		return -1;
	}
	
	return 0;
}
int release_kbd_char(struct keyboard_report *kbd, uint8_t keycode, uint8_t modifier)
{		

	kbd->modifier &= ~modifier;
	
	if(keycode){
		int i;
		for(i=0; i<6; ++i) {
			if(kbd->keycodes[i] == keycode) {
				kbd->keycodes[i] = 0x00;				
				break;
			} 
		}			
	}
	dbg("processing keyboard %d (%d, %d)", kbd->report_id, kbd->reserved, kbd->modifier);
	dbg("processing keycodes (0x%x 0x%x 0x%x 0x%x 0x%x 0x%x)",kbd->keycodes[0],kbd->keycodes[1],kbd->keycodes[2],
								  kbd->keycodes[3],kbd->keycodes[4],kbd->keycodes[5]);
	if(send_input_rep(kbd) == -1) {
		error("send_input_rep()");
		return -1;
	}
	
	return 0;
}

static int send_char(struct keyboard_report *kbd, uint8_t keycode, uint8_t modifier) 
{
	if(keycode < sizeof(kbd_asciimap)) {
		int key = kbd_asciimap[keycode];
		
		if (key & SHIFT ) {
			modifier |= 0x02;
			key &= ~SHIFT;
		}
		int r = press_kbd_char(kbd, key, modifier);
		(long) release_kbd_char(kbd, key, modifier);
		return r;
	}
	return 0;
}
static int send_string(struct keyboard_report *kbd, char *string, size_t len)
{
	int i;
	for(i=0; i<len; ++i) {	
		if(send_char(kbd, string[i], 0)) {
			break;
		}
		
	}
	return i;
}
static int process_multi_char(int fd, struct conn_info *info) 
{
	struct keyboard_report kbd; //= { .report_id = KBD_REP_ID }; BUGS in gcc ?!
	struct mouse_report    mouse;// = { .report_id = MOUSE_REP_ID };
	struct gamepad_report  gpad;// = { .report_id = GPAD_REP_ID };
	int i, f, t;
	char c, mod;
        char *line = NULL, *tmp_line, *next;
        size_t dummy, len;
        ssize_t read;
        static int8_t st = 0;
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
	__ZERO(gpad, GPAD_REP_ID);
	
        while ((read = getline(&line, &dummy, (FILE*) info->data)) != -1) {

		dbg("-----[%c]", line[0]);
        	switch(line[0]) {
        		case 'f':
      				f=0;
		        	sscanf(&line[1], "%d", &f);
		        	       	
				if(f & HCR_SHUTDOWN) {
					dbg("receved SHUTDOWN rq");
					return SHUTDOWN;
				}
        			break;
        			
        		case 'm': /* Mouse */
		        	sscanf(&line[1], "%d %d %d %d", &mouse.x, &mouse.y, &mouse.buttons, &mouse.wheel);
   			   	mouse.x = mouse.y = 22;
		   		dbg("processing mouse (%d, %d, 0x%x, %d)", mouse.x, mouse.y, mouse.buttons, mouse.wheel);
		   		dbg("%s", &line[1]);

				if(send_input_rep(&mouse) == -1) {
					error("send_input_rep()");
					return -1;
				}
        			__ZERO(mouse, MOUSE_REP_ID);	
				break;		
			case 'c': /* Keyboard */	
				c = mod = t = 0;
				sscanf(&line[1], " %c %c %d", &c, &mod, &t);
				dbg("%c %c %d", c, mod, t); 
				if(t)
					(long)press_kbd_char(&kbd, kbd_asciimap[c], KBD_GETCH_MODS(mod));
				else 
					(long)release_kbd_char(&kbd, kbd_asciimap[c], KBD_GETCH_MODS(mod));			
				break;
			case 'C':			
				c = mod = 0;
				sscanf(&line[1], " %c %c", &c, &mod);
				dbg("%c %c", c, mod); 
				send_char(&kbd,c, KBD_GETCH_MODS(mod));
				break;
			case 'S':			
				len = strlen(&line[2]);		
				(&line[2])[--len]='\0';         
				dbg("%d -> '%s'",len, &line[2]); 
				send_string(&kbd,&line[2],len);
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
				if(send_input_rep(&kbd) == -1) {
					error("send_input_rep()");
					return -1;
				}
				__ZERO(kbd, KBD_REP_ID);
				break;
			case 'g': /* Gamepad */
				sscanf(&line[1], "%d %d %d %d %d %d %x", &gpad.x,&gpad.y,&gpad.z,&gpad.rx,&gpad.ry,&gpad.rz,
									 &gpad.buttons);
									 
  				gpad.x=gpad.y=gpad.z=gpad.rx=gpad.ry=gpad.rz = st++;					 
		   		dbg("processing gamepad (%d %d %d %d %d %d %x)", gpad.x,gpad.y,gpad.z,gpad.rx,gpad.ry,gpad.rz,
									 gpad.buttons);
				if(send_input_rep(&gpad) == -1) {
					error("send_mouse_rep()");
					return -1;
				}
				__ZERO(gpad, GPAD_REP_ID);
        	}
#undef __ZERO
        	
        }
        
	free(line);
	
	return 0;
}
struct conn_info conn_multi_char = {
	.hdr_lookup = hdr_lookup_dropall,
	.process_src = process_multi_char,
};
static int process_multi_bin(int fd, struct conn_info *info) 
{
	struct keyboard_report kbd; //= { .report_id = KBD_REP_ID }; BUGS in gcc ?!
	struct mouse_report    mouse;
	struct gamepad_report  gpad;
        ssize_t numRead, i;

       	uint8_t report_id;
       	struct bin_packet packet;
	dbg("Hello");        
#define __ZERO(r, R) \
	memset(&r, 0, sizeof(r));\
	r.report_id = R;
	__ZERO(mouse, MOUSE_REP_ID);	
	__ZERO(kbd, KBD_REP_ID);
	__ZERO(gpad, GPAD_REP_ID);
	
        while ((numRead = read(fd, &packet , sizeof(packet)))>0) {

		dbg("-----[%d]", packet.report_id);
		
        	switch(packet.report_id) {
        		case KBD_REP_ID: /* Keyboard */	        				
				if(packet.flags & BRPT_STRING) {
					packet.report.string.s[MAX_STRING-1] = 0;
					send_string(&kbd,packet.report.string.s,strlen(packet.report.string.s));
				} else if(packet.flags & BRPT_CHAR) {
					send_char(&kbd,packet.report.letter.c,packet.report.letter.mods);
				} else {
					for(i=0; i < 6;++i)
						kbd.keycodes[i] = packet.report.keyboard.keycodes[i];
					kbd.modifier = packet.report.keyboard.modifier;
					
					dbg("processing keyboard (%d, %d)",kbd.reserved, kbd.modifier);
			   		dbg("processing keycodes (0x%x 0x%x 0x%x 0x%x 0x%x 0x%x)",kbd.keycodes[0],
		   									kbd.keycodes[1],kbd.keycodes[2],kbd.keycodes[3],kbd.keycodes[4], kbd.keycodes[5]);
					if(send_input_rep(&kbd) == -1) {
						error("send_input_rep()");
						return -1;
					}
				}	
				__ZERO(kbd, KBD_REP_ID);
				break;        		
        		case  GPAD_REP_ID: /* Gamepad */       									 
			#define S(x) gpad.x = packet.report.gamepad.x
				S(x);S(y);S(z);
				S(rx);S(ry);S(rz);
				S(buttons);
			#undef S
		   		dbg("processing gamepad (%d %d %d %d %d %d %x)", gpad.x,gpad.y,gpad.z,gpad.rx,gpad.ry,gpad.rz,
									 gpad.buttons);
				if(send_input_rep(&gpad) == -1) {
					error("send_mouse_rep()");
					return -1;
				}
				__ZERO(gpad, GPAD_REP_ID);
				break;
		        	
			case  MOUSE_REP_ID: /* Mouse */ 

   			   	mouse.x = packet.report.mouse.x;
   			   	mouse.y = packet.report.mouse.y;
  			   	mouse.wheel = packet.report.mouse.wheel;
   			   	mouse.buttons = packet.report.mouse.buttons;
   			   	
		   		dbg("processing mouse (%d, %d, 0x%x, %d)", mouse.x, mouse.y, mouse.buttons, mouse.wheel);


				if(send_input_rep(&mouse) == -1) {
					error("send_input_rep()");
					return -1;
				}
        			__ZERO(mouse, MOUSE_REP_ID);	
				break;					
			case SYS_REP_ID: /* for IPC */	        				
				if(packet.flags & HCR_SHUTDOWN){				
					dbg("receved SHUTDOWN rq");
					return SHUTDOWN;
				}
        			break;
				
        	}
#undef __ZERO
        	
        }
        	
	return 0;
}

struct conn_info conn_multi_bin = {
	.hdr_lookup = hdr_lookup_dropall,
	.process_src = process_multi_bin,
};
