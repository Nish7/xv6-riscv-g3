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
  int pids[5];      // Array to store child PIDs
  struct procstat stats[5]; // Array to store process statistics
  int workloads[5] = {50, 40, 30, 20, 10}; // Different work sizes
  
  printf("Starting benchmark with %d processes...\n", n_procs);
  sleep(5);
  // Fork test processes
  for(i = 0; i < n_procs; i++) {
    pid = fork();
    if(pid < 0) {
      printf("Fork failed\n");
      exit(1);
    }
    
    if(pid == 0) {  // Child process
      sleep(5);
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
    
    // Store child PID in parent
    pids[i] = pid;
  }
  
  // Set up tracking for process statistics
  int collected[n_procs];
  for(i = 0; i < n_procs; i++) {
    collected[i] = 0;  // Initialize - no stats collected yet
  }
  
  // Wait for each child, collecting statistics right before the wait
  int remaining = n_procs;
  
  while(remaining > 0) {
    // Check all children for completion first (they'll be in ZOMBIE state)
    for(i = 0; i < n_procs; i++) {
      if(!collected[i]) {
        if(getprocstat(pids[i], &stats[i]) == 0) {
          // Check if process has completed (has a non-zero completion_time)
          if(stats[i].state == 6) {  // ZOMBIE = 6
            // Process has completed, mark it as collected
            collected[i] = 1;
          }
        }
      }
    }
    
    // Now wait for one child to fully clean it up
    int status;
    int pid = wait(&status);
    
    // Record that we've processed one more child
    if(pid > 0) {
      remaining--;
    } else {
      // If no child was ready, sleep briefly
      sleep(1);
    }
  }
  
  // Print collected statistics
  printf("\nBENCHMARK RESULTS:\n");
  printf("PID  TYPE     CS      RUNTIME     CREATION     COMPLETION  TURNAROUND  CPU%%\n");
  
  for(i = 0; i < n_procs; i++) {
    uint64 turnaround = 0;
    int cpu_util = 0;
    
    if(stats[i].creation_time > 0) {
      if(stats[i].completion_time > 0) {
        turnaround = stats[i].completion_time - stats[i].creation_time;
      } else {
        // Process might still be running or we couldn't get completion time
        // Use current time as an approximation
        turnaround = uptime() - stats[i].creation_time;
      }
      
      // Calculate CPU utilization percentage
      if(turnaround > 0) {
        cpu_util = (stats[i].run_time * 100) / turnaround;
      }
    }
    
    printf("%d    %s    %lu      %lu      %lu      %lu      %lu       %d%%\n",
           pids[i], 
           (i % 2 == 0) ? "CPU" : "MIX",
           stats[i].context_switches, 
           stats[i].run_time,
           stats[i].creation_time, 
           stats[i].completion_time, 
           turnaround, 
           cpu_util);
  }
  
  return 0;
}
