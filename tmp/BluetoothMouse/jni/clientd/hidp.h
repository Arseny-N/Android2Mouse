typedef uint8_t u8;

#define HIDP_MSGT_HANDSHAKE  0

#	define HIDP_RCHSH_SUCCESFULL 0
#	define HIDP_RCHSH_NOT_READY 1
#	define HIDP_RCHSH_ERR_INVALID_REPORT_ID 2
#	define HIDP_RCHSH_ERR_UNSUPPORTED_REQUEST 3
#	define HIDP_RCHSH_ERR_INVALID_PARAMETER 4
#	define HIDP_RCHSH_ERR_UNKNOWN 0xe
#	define HIDP_RCHSH_ERR_FATAL 0xf

#define	HIDP_MSGT_HID_CONTROL 1

#	define HIDP_RCHSH_SUCCESFULL 0

#define	HIDP_MSGT_GET_REPORT  4
#define	HIDP_MSGT_SET_REPORT  5
#define	HIDP_MSGT_GET_PROTOCOL  6
#define	HIDP_MSGT_SET_PROTOCOL  7
#define	HIDP_MSGT_DATA  10

#define GET_HIDP_TYPE(hdr)  (hdr>>4)
/*
#define MK_RES_CODE(name, pref) {.name = #name, .val = HIDP_RC##pref##_##name}
struct hidp_res_codes {
	char *name;
	u8 val;
} *hsh_codes[] = {
	MK_RES_CODE(SUCCESSFULL, HSH),
	MK_RES_CODE(NOT_READY, HSH),
	MK_RES_CODE(ERR_INVALID_REPORT_ID, HSH),
	MK_RES_CODE(ERR_UNSUPPORTED_REQUEST, HSH),
	MK_RES_CODE(ERR_UNKNOWN, HSH),
	MK_RES_CODE(ERR_FATAL, HSH),
};
#undef MK_RES_CODE
*/
struct hidp_msg_type{
	char *name;
	struct hidp_res_codes *res_codes;
} hidp_name_lookup[] = {
#define MK_MSG_TYPE(name, rcd) { #name, rcd, }	
	MK_MSG_TYPE(HANDSHAKE,  NULL),
	MK_MSG_TYPE(HID_CONTROL,  NULL),
	{ NULL, NULL },
	{ NULL, NULL },
	MK_MSG_TYPE(GET_REPORT, NULL),
	MK_MSG_TYPE(SET_REPORT,NULL),
	MK_MSG_TYPE(GET_PROTOCOL, NULL),
	MK_MSG_TYPE(SET_PROTOCOL, NULL),
	{ NULL, NULL },
	{ NULL, NULL },
	MK_MSG_TYPE(DATA, NULL),
#undef MK_MSG_TYPE
};


#define hidp_hdr_build(t, p) (((t<<4)&0xf0) | (p&0x0f))

static inline void hidp_hdr_prase(u8 hdr, u8 *msg_type, u8 *param)
{
	*msg_type = (hdr & 0xf0) >> 4;
	*param = (hdr & 0xf);
}


struct hidp_pkt {
	u8 hidp_pkt_hdr; /* use hidp_hdr* for manipulations on .hidp_pkt_hdr */
	u8 data[];
};

static inline struct hidp_pkt *hidp_alloc_pkt(u8 size)
{
	struct hidp_pkt *pkt = malloc(sizeof(struct hidp_pkt)+size);
	memset(pkt, 0, sizeof(struct hidp_pkt)+size);
	return pkt;
}
static inline void hidp_free_pkt(struct hidp_pkt *pkt)
{
	free(pkt);
}



