

#include <linux/types.h>
#include "sdp.h"
#include "sdp_lib.h"
#include "bluetooth.h"

/*0x5a020c -- a phone */
typedef int u32;

struct self_repr {
	u32 cod;
	u32 old_cod;
	int dev_id;
	bdaddr_t interface;
		
	sdp_record_t record;
	int (*build_record)(sdp_record_t *record,void* arg);
	void *arg;
	
	char *name;
};
extern struct self_repr hid_mouse, mobile_phone;

int unapply_repr(struct self_repr *rep);
int apply_repr(struct self_repr *rep);

int get_dev_class(uint32_t *class, int dev_id);
static inline int apply_mouse(void)
{
	return apply_repr(&hid_mouse);
}
static inline int unapply_mouse(void)
{
	return unapply_repr(&hid_mouse);
}
