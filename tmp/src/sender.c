#include <stdlib.h>
#include <stdio.h>
#include "hidp_client.h"
#include "log.h"

int main(int argc, char *argv[])
{
	char *line;
	if(!argv[1]) {
		error("baddr?");
		exit(EXIT_FAILURE);		
	}
	
	if(start_session(argv[1]) == -1) {
		error("failed to start session %s", argv[1]);
		exit(EXIT_FAILURE);
	}
	while(getline(&line, NULL, stdin) != -1) {
		int x, y, b;
		
		x = y = b = 0;
		switch(*line) {
			case 'x':
				x = strtol(&line[1], NULL, 0);
				break;
			case 'y':
				y = strtol(&line[1], NULL, 0);
				break;
			case 'b':
				b = strtol(&line[1], NULL, 0);
				break;
			case 's':
				goto out;
		}
		
	}
out:
	if(end_session()) {
		error("Failed to end session");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}
