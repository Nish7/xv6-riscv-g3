#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void print_help() {
	fprintf(2, "Usage: sleep <TICKS>\n");
	fprintf(2, "       sleep OPTION\n");
	fprintf(2, "Pause for number of TICKS.\n");
	fprintf(2, "TICKS must be an integer or is interpreted as integer if it is float.\n");
	fprintf(2, "Given two or more arguments, pause for the amount of time specified by\n");
	fprintf(2, "the sum of their values.\n");
	fprintf(2, "--help\n");
	fprintf(2, "display help and exit\n");
	fprintf(2, "--version\n");
	fprintf(2, "display version number\n");
	exit(0);
}

void print_version() {
	fprintf(2, "Version 1.0\n");
	exit(0);
}

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
	// Check for help or version info
	if (strcmp(argv[1], "--help") == 0) print_help();
	else if (strcmp(argv[1], "--version") == 0) print_version();

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
			fprintf(2, "sleep: invalid ticks '%s'\n", argv[i]);
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
