

#include <linux/types.h>
#include "bluetooth/sdp.h"
#include "bluetooth/sdp_lib.h"
#include "bluetooth/bluetooth.h"


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
extern struct self_repr hid_mouse;

int unapply_repr(struct self_repr *rep);
int apply_repr(struct self_repr *rep);


static inline int apply_mouse(void)
{
	return apply_repr(&hid_mouse);
}
static inline int unapply_mouse(void)
{
	return unapply_repr(&hid_mouse);
}
