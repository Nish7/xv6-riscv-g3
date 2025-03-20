#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// CPU-bound workload with configurable intensity
void cpu_work(int iterations) {
  volatile int i, j;
  for(i = 0; i < iterations; i++)
    for(j = 0; j < 1000000; j++);
}

// Mixed workload - some CPU, some sleep
void mixed_work(int iterations, int sleep_ms) {
  for(int i = 0; i < iterations; i++) {
    cpu_work(10);
    sleep(sleep_ms);
  }
}

int main(int argc, char *argv[]) {
  int n_procs = 5;  // Number of processes to create
  int i, pid;
  int workloads[5] = {50, 40, 30, 20, 10}; // Different work sizes
  
  printf("Starting benchmark with %d processes...\n", n_procs);
  
  // Fork test processes
  for(i = 0; i < n_procs; i++) {
    pid = fork();
    if(pid < 0) {
      printf("Fork failed\n");
      exit(1);
    }
    
    if(pid == 0) {  // Child process
      printf("Process %d (PID %d) starting work\n", i, getpid());
      
      // Do some work based on process number
      if(i % 2 == 0) {
        // CPU-bound process
        cpu_work(workloads[i]);
      } else {
        // Mixed CPU/IO process
        mixed_work(workloads[i], 10);
      }
      
      printf("Process %d (PID %d) completed\n", i, getpid());
      exit(0);
    }
  }
  
  // Parent waits for all children
  for(i = 0; i < n_procs; i++) {
    wait(0);
  }
  
  printf("All processes completed. Run 'procstat' to see results.\n");
  procstat(); // Display stats right after completion
  
  return 0;
}
