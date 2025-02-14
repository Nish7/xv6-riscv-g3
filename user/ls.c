#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

#define MAX_ENTRIES 100  // Limit to avoid excessive memory use

int show_hidden = 0;        // **Flag for -a: Show hidden files**
int append_symbols = 0;     // **Flag for -F: Append file type symbols**
int long_format = 0;        // **Flag for -l: Long listing format**
int stream_output = 0;      // **Flag for -m: Comma-separated output**

// **Function to append symbols based on file type (-F flag implementation)**
void append_file_symbol(char *name, short type) { 
  switch (type) {
    case T_DIR:
      printf("%s/\n", name);
      break;
    case T_FILE:
      printf("%s\n", name);
      break;
    case T_DEVICE:
      printf("%s|\n", name);
      break;
    default:
      printf("%s\n", name);
  }
}

// **Function to extract the filename from a given path**
char* fmtname(char *path) {
  static char buf[DIRSIZ + 1];
  char *p;

  // Find the last `/` in the path to extract the filename
  for (p = path + strlen(path); p >= path && *p != '/'; p--);
  p++;

  // **If -m flag is enabled, return the filename without padding**
  if (stream_output) {  
    return p;  
  }

  // Copy and pad the name with spaces for normal display
  if (strlen(p) >= DIRSIZ)
    return p;

  memmove(buf, p, strlen(p));
  memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
  buf[DIRSIZ] = '\0'; // Ensure null termination
  return buf;
}

// **Function to list directory contents or display file information**
void ls(char *path) {
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;
  int first_entry = 1; // **Used for formatting comma-separated output (-m flag)**

  if ((fd = open(path, O_RDONLY)) < 0) {
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if (fstat(fd, &st) < 0) {
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch (st.type) {
  case T_DEVICE:
  case T_FILE:
    if (stream_output) { // **-m flag: Print as a single entry**
      printf("%s\n", fmtname(path));
    } else if (long_format) { // **-l flag: Long format output**
      char mode[11] = "----------";  // Dummy file mode for simplicity
      mode[0] = (st.type == T_DIR) ? 'd' : '-';
      
      // **Print POSIX-style long format**
      printf("%s %u ? ? %u ??? %s\n", mode, 1, (uint)st.size, fmtname(path));
    } else if (append_symbols) { // **-F flag: Append file type symbols**
      append_file_symbol(fmtname(path), st.type);
    } else {
      printf("%s %d %d %d\n", fmtname(path), st.type, st.ino, (int)st.size);
    }
    close(fd);
    return;

  case T_DIR:
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
      printf("ls: path too long\n");
      break;
    }

    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';

    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      if (de.inum == 0)
        continue;

      if (!show_hidden && de.name[0] == '.') { 
        continue; // **-a flag: Skip hidden files unless enabled**
      }

      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;

      if (stat(buf, &st) < 0) {
        printf("ls: cannot stat %s\n", buf);
        continue;
      }

      if (stream_output) { // **-m flag: Format as comma-separated list**
        if (!first_entry) {
          printf(", "); // **Add comma before entries except the first one**
        }
        printf("%s", fmtname(buf));
        first_entry = 0; // **Mark that at least one entry has been printed**
      } else if (long_format) { // **-l flag: Long format output**
        char mode[11] = "----------";  
        mode[0] = (st.type == T_DIR) ? 'd' : '-';
        
        printf("%s %u ? ? %u ??? %s\n", mode, 1, (uint)st.size, fmtname(buf));
      } else if (append_symbols) { // **-F flag: Append file type symbols**
        append_file_symbol(fmtname(buf), st.type);
      } else {
        printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, (int)st.size);
      }
    }

    if (stream_output) { // **-m flag: End output with a newline**
      printf("\n");
    }
    break;
  }
  close(fd);
}

// **Main function to parse flags and list files/directories**
int main(int argc, char *argv[]) {
  int i = 1;

  // **Parse flags**
  for (; i < argc; i++) {
    if (argv[i][0] == '-') {
      for (int j = 1; argv[i][j] != '\0'; j++) {
        if (argv[i][j] == 'a') show_hidden = 1; // **Enable -a flag**
        else if (argv[i][j] == 'F') append_symbols = 1; // **Enable -F flag**
        else if (argv[i][j] == 'l' && stream_output == 0) long_format = 1; // **Enable -l flag**
        else if (argv[i][j] == 'm') { // **Enable -m flag**
          stream_output = 1;
          long_format = 0; // **-m flag disables -l flag**
        }
      }
    } else {
      break; // finish with flag mode setup, move on to the ls operation
    }
  }

  // **If no directory is specified, list the current directory**
  if (i == argc) {
    ls(".");
  } else {
    for (; i < argc; i++) {
      ls(argv[i]);
    }
  }

  exit(0);
}
