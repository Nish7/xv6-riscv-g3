#include "kernel/types.h"
#include "user/user.h"

#define MAX_CHILDREN 10
#define DEFAULT_LOOPS 1000

void busy_loop() {
    for (volatile int j = 0; j < DEFAULT_LOOPS; j++) {
        // Volatile to prevent compiler optimization
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <priority1> <priority2> <priority3> ...\n", argv[0]);
        printf("Example: %s 4 3 3 1\n", argv[0]);
        exit(1);
    }

    int num_children = argc - 1;  // Number of priorities provided
    if (num_children > MAX_CHILDREN) {
        printf("Error: Maximum %d children allowed\n", MAX_CHILDREN);
        exit(1);
    }

    int priorities[MAX_CHILDREN];
    
    for (int i = 0; i < num_children; i++) {
        priorities[i] = atoi(argv[i + 1]);
        if (priorities[i] < 0 || priorities[i] > 10) {
            printf("Error: Priority %d must be between 0 and 10\n", priorities[i]);
            exit(1);
        }
    }

    printf("Starting priority scheduling test with %d children...\n", num_children);

    // Create children
    for (int i = 0; i < num_children; i++) {
        int pid = fork();
        
        if (pid < 0) {
            printf("Fork failed for child %d\n", i + 1);
            exit(1);
        }
        
        if (pid == 0) {
            int current_pid = getpid();
            printf("Child %d PID: %d\n", i + 1, current_pid);
            
            if (setpriority(current_pid, priorities[i]) < 0) {
                printf("Child %d failed to set priority to %d\n", 
                       i + 1, priorities[i]);
                exit(1);
            }
            
            printf("Child %d set priority to %d\n", i + 1, priorities[i]);
            
            // First child runs finite loops, others run infinitely
            if (i == 0) {
                busy_loop();
                printf("Child %d exiting\n", i + 1);
            } else {
                while (1) {
                    busy_loop();
                }
                printf("Child %d exiting\n", i + 1);  // Unreachable, but kept for consistency
            }
            exit(0);
        }
    }

    // Parent waits for all children
    for (int i = 0; i < num_children; i++) {
        wait(0);
    }
    
    printf("Test complete, check scheduler output.\n");
    exit(0);
}
