#include <stdio.h>
#include <stdlib.h>

#include "self_repr.h"


int main ( int argc, char * argv [] )
{
	if(!argv[1]) {
usage:
		fprintf(stderr, "Usage: %s cod|mouse|phone sdp-handle-to-remove\n", argv[0]);
		exit(EXIT_FAILURE);
	}
		
	if(strcmp(argv[1], "cod") == 0) {	
		uint32_t cod;
print_cod:			
		if(get_dev_class(&cod)) {
			error("Failed to get device class");
			exit(EXIT_FAILURE);
		}
		printf("0x%x\n", cod);		
		exit(EXIT_SUCCESS);
	}
	if(strcmp(argv[1], "cod-mouse") == 0) {
		if(set_dev_class(DC_MOUSE)) {
			error("failed to set dev class");
			exit(EXIT_FAILURE);
		}
		goto print_cod;
	}
	if(strcmp(argv[1], "mouse") == 0) {
		int handle;
		if(add_sdp_record(&hid_mouse, NULL, &handle)) {
			error("failed to apply repr");
			exit(EXIT_FAILURE);
		}
		if(set_dev_class(DC_MOUSE)) {
			error("failed to set dev class");
			exit(EXIT_FAILURE);
		}
		printf("0x%x\n", handle);
	}
	if(strcmp(argv[1], "multi") == 0) {
		int handle;
		if(add_sdp_record(&hid_multi, NULL, &handle)) {
			error("failed to apply repr");
			exit(EXIT_FAILURE);
		}
		if(set_dev_class(DC_MOUSE)){
			error("failed to set dev class");
			exit(EXIT_FAILURE);
		}
		printf("0x%x\n", handle);
	}
	
	if(strcmp(argv[1], "phone") == 0) {
		if(!argv[2])
			goto usage;
		
		int handle = strtol(argv[2], NULL, 0);
		if(remove_sdp_record(handle) == -1) {
			error("failed to remove %x record\n", handle);
			exit(EXIT_FAILURE);
		}
		if(set_dev_class(DC_PHONE)) {
			error("failed to set dev class");
			exit(EXIT_FAILURE);
		}
	}

	exit(EXIT_SUCCESS);
}
