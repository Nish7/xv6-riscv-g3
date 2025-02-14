#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int is_positive_int(const char *s) {
	// Check negative
	if (*s == '-') return 0;
	// Check non-digit
	for (; *s; s++) {
		if (*s < '0' || *s > '9') return 0;
	}
	return 1;
}


int main(int argc, char *argv[]) {
	// Sleep requires at least 2 arguments 
	// with the first being sleep itself and the second being the sleep ticks
	if(argc < 2){
		fprintf(2, "missing operand. Usage: sleep <ticks>\n");
		exit(1);
	}

	// If there are multiple tick arguments, they are added together
	int total_ticks = 0;
	for (int i = 1; i<argc; i++) {
		if (!is_positive_int(argv[i])) {
			fprintf(2, "Sleep: invalid ticks '%s'\n", argv[i]);
			exit(1);
		}
		total_ticks += atoi(argv[i]);
	}
	
	// Sleep for total_ticks amount of time
	if (sleep(total_ticks) < 0) { // sys_sleep return -1 if failed
		fprintf(2, "Sleep failed\n");
		exit(1);
	}
	exit(0);
}
