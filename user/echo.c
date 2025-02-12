#include "kernel/types.h"
#include "kernel/stat.h"
#include <stddef.h>
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int i;
  void echo_function_enhanced(int argc, char *argv[], int start_val){
    for(i = start_val; i < argc; i++){
      int length_of_string = strlen(argv[i]);
      for(int j = 0; j < length_of_string + 1; j++){
        if(argv[i][j] == '\\'){
          if(j + 1 != length_of_string && argv[i][j+1] == 'n'){
            write(1, "\n", 1);
            j += 1;
          }
          else if(j + 1 != length_of_string && argv[i][j+1] == 't'){
            write(1, "\t", 1);
            j += 1;
          }
          else {
            write(1, &argv[i][j], 1);
          }
        }
        else {
          write(1, &argv[i][j], 1);   
        }
          //printf("%c", argv[i][j]);
      }
      if (i + 1 < argc){
        write(1, " ", 1);
      }
      else {
        write(1, "\n", 1);
      }
    }
  }
  if(strcmp(argv[1], "-e") == 0){
    echo_function_enhanced(argc, argv, 2);
  }
  else {
    for(i = 1; i < argc; i++){
      write(1, argv[i], strlen(argv[i]));
      if(i + 1 < argc){
        write(1, " ", 1);
    } 
      else {
        write(1, "\n", 1);
      }
    }
  }
  
  exit(0);
}
