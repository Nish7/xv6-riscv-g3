#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "kernel/spinlock.h"
#include "kernel/riscv.h"
#include "kernel/proc.h"

#define MAXCOLS 5
#define MAXPIDS 32

static char *procstatenames[] = {
  [UNUSED] = "unused",
  [USED] = "used",
  [SLEEPING] = "sleep",
  [RUNNABLE] = "runble",
  [RUNNING] = "running",
  [ZOMBIE] = "zombie"
};

static int parse_pids(char *str, int pids[], int max_count) {
  int count = 0;
  char *p = str;
  while (*p && count < max_count) {
    pids[count++] = atoi(p);
    char *comma = strchr(p, ',');
    if (!comma) break;
    p = comma + 1;
  }
  return count;
}

// ps: print all processes
// ps [-p pidlist] [-o col1[,col2,...]]
int main(int argc, char *argv[]) 
{
  int pids[MAXPIDS];
  int pid_count = 0;

  char *columns[MAXCOLS];
  int num_cols = 0;

  // parse arguments
  // -p pidlist
  // -o col1[,col2,...]
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-p") == 0) {
      if (i + 1 >= argc) {
        printf("ps: missing argument after -p\n");
        exit(1);
      }
      pid_count = parse_pids(argv[++i], pids, MAXPIDS);
    } else if (strcmp(argv[i], "-o") == 0) {
      if (i + 1 >= argc) {
        printf("ps: missing argument after -p\n");
        exit(1);
      }

      char *colstr = argv[++i];
      char *field = colstr;

      while (num_cols < MAXCOLS && *field) {
        // Split the string by comma by replacing it with null terminator.
        char *comma = strchr(field, ',');

        if (comma) {
          *comma = '\0';
          columns[num_cols++] = field;
          field = comma + 1;
        } else {
          columns[num_cols++] = field;
          break;
        }
      }
    } else {
      printf("Usage: ps [-p pidlist] [-o col1[,col2,...]]\n");
      exit(1);
    }
  }

  if (num_cols == 0) {
    columns[0] = "pid";
    columns[1] = "state";
    columns[2] = "name";
    num_cols = 3;
  }

  struct uproc up[NPROC];
  int n = ps(up, NPROC);
  if (n < 0) {
    printf("ps: error in sys_ps\n");
  }

  // print header
  for (int i = 0; i < num_cols; i++) {
    printf("%s\t", columns[i]);
  }
  printf("\n");

  // print process
  for (int i = 0; i < n; i++) {
    if (pid_count > 0) {
      int match = 0;
      for (int j = 0; j < pid_count; j++) {
        if (up[i].pid == pids[i]) {
          match = 1;
          break;
        }
      }

      if (!match) {
        continue;
      }
    }

    // Print each column in order.
    for (int c = 0; c < num_cols; c++) {
      if (strcmp(columns[c], "pid") == 0) {
        printf("%d\t", up[i].pid);
      } else if (strcmp(columns[c], "name") == 0) {
        printf("%s\t", up[i].name);
      } else if (strcmp(columns[c], "state") == 0) {
        int s = up[i].state;
        char *st = (s >= 0 && s < NELEM(procstatenames) && procstatenames[s])
                       ? procstatenames[s]
                       : "???";
        printf("%s\t", st);
      } else {
        printf("? \t");
      }
    }

    printf("\n");
  }

  exit(0);
}
