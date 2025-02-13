#include "kernel/types.h"
#include "kernel/stat.h"
#include <stddef.h>
#include "user/user.h"


void echo_function_enhanced(int argc, char *argv[], int start_val){
  for(int i = start_val; i < argc; i++){
    int length_of_string = strlen(argv[i]);
    for(int j = 0; j < length_of_string + 1; j++){
      if(argv[i][j] == '\\'){
        if(j + 1 != length_of_string) {
          switch (argv[i][j + 1]) {
              case 'n':
                  write(1, "\n", 1);
                  j++; 
                  break;
              case 't':
                  write(1, "\t", 1);
                  j++; 
                  break;
              case 'e':
                j = j + 3;
              default:
                  write(1, &argv[i][j], 1); // if \ just a part of the sentence, no escape sequences 
                  break;
          }
        }
      }
      else {
        write(1, &argv[i][j], 1); // input next character 
      }
    }
    if (i + 1 < argc){
      write(1, " ", 1); // put space between WORDS
    }
    else {
      write(1, "\n", 1); // last character, so that next command can be inputted on next line 
    }
  }
}

int
main(int argc, char *argv[])
{
  int i; 
  if(argc > 1){  
    if(strcmp(argv[1], "-e") == 0){
      echo_function_enhanced(argc, argv, 2);
  }
  else { 
    // this is original code for echo 
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
}
  
  exit(0);
}
