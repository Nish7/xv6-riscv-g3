#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  // Sleep requires 2 arguments with the first being sleep itself and the second being the sleep ticks
  if(argc != 2){
    fprintf(2, "Usage: sleep <ticks>\n");
    exit(1);
  }

  // Sleep for specified number of ticks
  int ticks = atoi(argv[1]);
  if (sleep(ticks) < 0) { // sys_sleep return -1 if failed
    fprintf(2, "Sleep failed\n");
    exit(1);
  }
  exit(0);
}
