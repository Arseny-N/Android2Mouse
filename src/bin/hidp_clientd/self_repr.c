#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "self_repr.h"
//#include "log.h"

#define _error(f, a...) fprintf(stderr," ERR: %16s:    " f ,__func__, ##a)
#define _info(f, a...) fprintf(stderr,"INFO: %16s:    " f ,__func__, ##a)
#define _dbg(f, a...) fprintf(stderr," DBG: %16s:    " f, __func__, ##a)

#define error(f, a...)  _error(f "\n", ##a)
#define info(f, a...) _info(f "\n", ##a)
#define dbg(f, a...) _dbg(f "\n", ##a)
int get_dev_class(uint32_t *class, int dev_id)
{
	uint8_t cls[3];
	int hci = hci_open_dev(dev_id);
	if(hci < 0) {
		error("hci_open_dev");
		return -1;
	}
	if(hci_read_class_of_dev(hci, cls, 1000)< 0) {
		error("hci_read_class_of_dev");
		hci_close_dev(hci);		
		return -1;
	}
	*class = 0;
	*class = cls[2] << 16 |  cls[1] << 8 | cls[0];
	hci_close_dev(hci);
	return 0;
}
static int set_dev_class(uint32_t class, int dev_id)
{
	int hci = hci_open_dev(dev_id);
	if(hci < 0) {
		error("hci_open_dev");
		return -1;
	}
	if(hci_write_class_of_dev(hci, class, 2000)< 0) {
		error("hci_read_class_of_dev");
		hci_close_dev(hci);
		return -1;
	}
	hci_close_dev(hci);
	return 0;
}

int apply_repr(struct self_repr *rep)
{
	sdp_session_t *sess;
		
	if(get_dev_class(&rep->old_cod,rep->dev_id))
		return -1;

	if(set_dev_class(rep->cod, rep->dev_id))
		return -1;
		
	if(rep->build_record) {
		memset(&rep->record, 0, sizeof(sdp_record_t));
		if(rep->build_record(&rep->record, rep->arg))
			return -1;

		sess = sdp_connect(&rep->interface, BDADDR_LOCAL, SDP_RETRY_IF_BUSY);
		if(!sess) {
			error( "No local sdp server!\n");
			return -1;
		}

		if (sdp_record_register(sess, &(rep->record), SDP_RECORD_PERSIST) < 0) {
			error( "Service Record registration failed\n");
			sdp_close(sess);
			return -1;
		}
		sdp_close(sess);
	}
	return 0;
	
}
int unapply_repr(struct self_repr *rep)
{
	sdp_session_t *sess;
	if(set_dev_class(rep->old_cod, rep->dev_id))
		return -1;		

	sess = sdp_connect(&rep->interface, BDADDR_LOCAL, SDP_RETRY_IF_BUSY);
	if(!sess) {
		error( "No local sdp server!\n");
		return -1;
	}


		
	if (sdp_device_record_unregister_binary(sess, BDADDR_ANY, rep->record.handle)) {
		error( "Service Record registration failed(%s)\n", strerror(errno));
		sdp_close(sess);
		return -1;
	}

	sdp_close(sess);

	return 0;
	
}

int build_hid_mouse(sdp_record_t *record, void *arg)
{

	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, hidkb_uuid, l2cap_uuid, hidp_uuid;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto, *proto[3];
	sdp_data_t *psm, *lang_lst, *lang_lst2, *hid_spec_lst, *hid_spec_lst2;
	unsigned int i;
	uint8_t dtd = SDP_UINT16;
	uint8_t dtd2 = SDP_UINT8;
	uint8_t dtd_data = SDP_TEXT_STR8;
	void *dtds[2];
	void *values[2];
	void *dtds2[2];
	void *values2[2];
	int leng[2];
	uint8_t hid_spec_type = 0x22;
	uint16_t hid_attr_lang[] = { 0x409, 0x100 };
	static const uint16_t ctrl = 0x11;
	static const uint16_t intr = 0x13;
	static const uint16_t hid_attr[] = { 
		0x100,     // SDP_ATTR_HID_DEVICE_RELEASE_NUMBER		
		0x111,     // SDP_ATTR_HID_PARSER_VERSION		
		0x80,      // SDP_ATTR_HID_DEVICE_SUBCLASS	(0x90)
		0x00,      // SDP_ATTR_HID_COUNTRY_CODE
		0x01,      // SDP_ATTR_HID_VIRTUAL_CABLE		
		0x01       // SDP_ATTR_HID_RECONNECT_INITIATE		
		};
	static const uint16_t hid_attr2[] = { 
		0x0,       // SDP_ATTR_HID_SDP_DISABLE
 		0x01,      // SDP_ATTR_HID_REMOTE_WAKEUP
		0x100, 	   // SDP_ATTR_HID_PROFILE_VERSION
		0x1f40,    // SDP_ATTR_HID_SUPERVISION_TIMEOUT	
		0x01,  	   // SDP_ATTR_HID_NORMALLY_CONNECTABLE	
		0x01       // SDP_ATTR_HID_BOOT_DEVICE	
		};
	const uint8_t hid_spec[] = {
		0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)	// 54
		0x09, 0x02,                    // USAGE (Mouse)
    		0xa1, 0x01,                    // COLLECTION (Application)
    		0x09, 0x01,                    //   USAGE (Pointer)
    		0xa1, 0x00,                    //   COLLECTION (Physical)
		0x85, 0x01,                    //     REPORT_ID (1)
		0x05, 0x09,                    //     USAGE_PAGE (Button)
    		0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    		0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
    		0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    		0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    		0x95, 0x03,                    //     REPORT_COUNT (3)
	    	0x75, 0x01,                    //     REPORT_SIZE (1)
	    	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	    	0x95, 0x01,                    //     REPORT_COUNT (1)
	    	0x75, 0x05,                    //     REPORT_SIZE (5)
	    	0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
	    	0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
	    	0x09, 0x30,                    //     USAGE (X)
	    	0x09, 0x31,                    //     USAGE (Y)
	    	0x09, 0x38,                    //     USAGE (Wheel)
	    	0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
	    	0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
	    	0x75, 0x08,                    //     REPORT_SIZE (8)
	    	0x95, 0x03,                    //     REPORT_COUNT (3)
	    	0x81, 0x06,                    //     INPUT (Data,Var,Rel)
	    	0xc0,                          //   END_COLLECTION
	    	0xc0,       
	};


	memset(record, 0, sizeof(sdp_record_t));
	record->handle = 0xffffffff;//si->handle;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(record, root);

	sdp_add_lang_attr(record);

	sdp_uuid16_create(&hidkb_uuid, HID_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &hidkb_uuid);
	sdp_set_service_classes(record, svclass_id);

	sdp_uuid16_create(&profile[0].uuid, HID_PROFILE_ID);
	profile[0].version = 0x0100;
	pfseq = sdp_list_append(0, profile);
	sdp_set_profile_descs(record, pfseq);

	/* protocols */
	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[1] = sdp_list_append(0, &l2cap_uuid);
	psm = sdp_data_alloc(SDP_UINT16, &ctrl);
	proto[1] = sdp_list_append(proto[1], psm);
	apseq = sdp_list_append(0, proto[1]);

	sdp_uuid16_create(&hidp_uuid, HIDP_UUID);
	proto[2] = sdp_list_append(0, &hidp_uuid);
	apseq = sdp_list_append(apseq, proto[2]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(record, aproto);

	/* additional protocols */
	proto[1] = sdp_list_append(0, &l2cap_uuid);
	psm = sdp_data_alloc(SDP_UINT16, &intr);
	proto[1] = sdp_list_append(proto[1], psm);
	apseq = sdp_list_append(0, proto[1]);

	sdp_uuid16_create(&hidp_uuid, HIDP_UUID);
	proto[2] = sdp_list_append(0, &hidp_uuid);
	apseq = sdp_list_append(apseq, proto[2]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_add_access_protos(record, aproto);

	sdp_set_info_attr(record, "HID Mouse", NULL, NULL);

	for (i = 0; i < sizeof(hid_attr) / 2; i++)
		sdp_attr_add_new(record,
					SDP_ATTR_HID_DEVICE_RELEASE_NUMBER + i,
					SDP_UINT16, &hid_attr[i]);

	dtds[0] = &dtd2;
	values[0] = &hid_spec_type;
	dtds[1] = &dtd_data;
	values[1] = (uint8_t *) hid_spec;
	leng[0] = 0;
	leng[1] = sizeof(hid_spec);
	hid_spec_lst = sdp_seq_alloc_with_length(dtds, values, leng, 2);
	hid_spec_lst2 = sdp_data_alloc(SDP_SEQ8, hid_spec_lst);
	sdp_attr_add(record, SDP_ATTR_HID_DESCRIPTOR_LIST, hid_spec_lst2);

	for (i = 0; i < sizeof(hid_attr_lang) / 2; i++) {
		dtds2[i] = &dtd;
		values2[i] = &hid_attr_lang[i];
	}

	lang_lst = sdp_seq_alloc(dtds2, values2, sizeof(hid_attr_lang) / 2);
	lang_lst2 = sdp_data_alloc(SDP_SEQ8, lang_lst);
	sdp_attr_add(record, SDP_ATTR_HID_LANG_ID_BASE_LIST, lang_lst2);

	sdp_attr_add_new(record, SDP_ATTR_HID_SDP_DISABLE, SDP_UINT16, &hid_attr2[0]);

	for (i = 0; i < sizeof(hid_attr2) / 2 - 1; i++)
		sdp_attr_add_new(record, SDP_ATTR_HID_REMOTE_WAKEUP + i,
						SDP_UINT16, &hid_attr2[i + 1]);


	info("HID Mouse built\n");

	return 0;
}
struct self_repr hid_mouse = {
	.cod = 0x10590, 
	.build_record = build_hid_mouse, 
	
	.interface = *BDADDR_ANY, 
	.dev_id = 0 
} , mobile_phone = {
	.cod = 0x5a020c, 
	.build_record = NULL, 
	
	.interface = *BDADDR_ANY, 
	.dev_id = 0 	
};
