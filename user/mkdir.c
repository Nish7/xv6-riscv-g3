#include "kernel/types.h"
#include "kernel/stat.h"
#include "Kernel/fcntl.h"
#include "user/user.h"

int makeDirectory (char *path){
  char *p;
  char temp;
  int status = 0;

  // Skip leading slashes
  p = path;
  while (*p == '/')
    p++;

  while (*p != 0) {
    // Find the next component
    while (*p != '/' && *p != 0)
      p++;

    temp = *p;
    *p = 0; // Temporarily clear string value

    if (mkdir(path) < 0) {
      // Atempt to open the directory
      int fd = open(path, O_RDONLY); // fd = file descriptor
      if (fd > 0) {
        fprintf(2, "mkdir: cannot create directory '%s'\n", path);
        status = -1;
        break;
      }
      close(fd);
    }

    *p = temp; // Restore string value
    if (temp == 0)
      break;

    while (*p == '/') // Ignore consecutive slashes
      p++;
  }

  return status;
}

int main(int argc, char *argv[])
{
  int i;
  int parentFlag = 0;

  if (argc < 2) {
    fprintf(2, "Usage: mkdir files...\n");
    exit(1);
  }

  // Checks number of arguments when parent flag is present
  if (strcmp(argv[1], "-p") == 0) {
    parentFlag = 1;
    if (argc < 3) {
      fprintf(2, "Not enough arguments");
      exit(1);
    }
  }

  for (i = 1 + parentFlag; i < argc; i++) {
    if (parentFlag) {
      if (makeDirectory(argv[i]) < 0)
        exit(1);
    } else {
      if (makeDirectory(argv[i]) < 0) {
        fprintf(2, "mkdir: %s failed to create\n", argv[i]);
        exit(1);
      }
    }
  }

  exit(1);
}