#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf(
        "Not enough arguments! Argument 1 == pid and Argument 2 == priority.\n");
    exit(1);
  }

  int pid = atoi(argv[1]);
  int priority = atoi(argv[2]);

  if (setpriority(pid, priority) == 0) {
    printf("PID %d's priority was set to %d successfully\n", pid, priority);
    exit(0);
  } else {
    printf("An error occured!\n");
    exit(1);
  }
}
