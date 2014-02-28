#include <stdint.h>

#define KBD_REP_ID     1
#define MOUSE_REP_ID   2
#define GPAD_REP_ID    3
#define SYS_REP_ID    14 // JUST FOR IPC (multi_bin)
struct keyboard_report {
	uint8_t report_id;
	uint8_t modifier;
	uint8_t reserved;
	uint8_t keycodes[6];
}  __attribute__((__packed__));;

struct mouse_report {
	uint8_t report_id;
	uint8_t buttons;
	int8_t x;
	int8_t y;	
	int8_t wheel;
} __attribute__((__packed__));;

struct gamepad_report {
	uint8_t report_id;
	uint16_t buttons;
		
	int8_t x;
	int8_t y;
	int8_t z;
	int8_t rx;
	int8_t ry;
	int8_t rz;
	
	
}  __attribute__((__packed__));;



#define SIZE_CHECK(type, size)					\
static inline void check_##type(void)				\
{								\
	if(sizeof(struct type) != (size)) {			\
		info("failure struct " #type " is not %d bytes but %d.",size,sizeof(struct type));\
		 exit(EXIT_FAILURE);				\
	}							\
}
SIZE_CHECK(keyboard_report, 9)
SIZE_CHECK(mouse_report,5)
SIZE_CHECK(gamepad_report, 9)

extern struct conn_info conn_multi_char;
extern struct conn_info conn_multi_bin;

/*---------  process_multi_bin stuff ---------------*/
#define MAX_STRING 16

struct bin_mouse {
	uint8_t buttons;
	int8_t x;
	int8_t y;	
	int8_t wheel;
};

struct bin_keyboard {
	uint8_t modifier;
	uint8_t keycodes[6];
};
/* p.flags Bits */
#define BRPT_STRING 1
#define BRPT_CHAR 2
/* p.flags2 Bits for BRPT_CHAR */
#	define CHAR_ASCII 1
#	define CHAR_RAW   2


struct bin_string {
	char s[MAX_STRING];
};
struct bin_char {
	char c;
	int mods;
};
struct bin_gamepad {
	uint16_t buttons;
		
	int8_t x;
	int8_t y;
	int8_t z;
	int8_t rx;
	int8_t ry;
	int8_t rz;
	
	
};
struct bin_packet {
	uint8_t report_id;
	uint8_t flags;
	uint8_t flags2;
	union {
		struct bin_mouse       mouse;
		struct bin_keyboard keyboard; 	
		struct bin_gamepad   gamepad;
		struct bin_string     string;
		struct bin_char       letter;
		
	} report;
};
