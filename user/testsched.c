#include "kernel/types.h"
#include "user/user.h"

void busy_loop() {
    for (int j = 0; j < 1000; j++) {}
}

int main() {
    int pid1, pid2, pid3;

    printf("Starting priority scheduling test...\n");

    if ((pid1 = fork()) == 0) {
        printf("Child 1 PID: %d\n", getpid());
        while(1) busy_loop();
        printf("Child 1 exiting\n");
        exit(0);
    }

    if ((pid2 = fork()) == 0) {
        printf("Child 2 PID: %d\n", getpid());
        while(1) busy_loop();
        printf("Child 2 exiting\n");
        exit(0);
    }

    if ((pid3 = fork()) == 0) {
        printf("Child 3 PID: %d\n", getpid());
        while(1) busy_loop();
        printf("Child 3 exiting\n");
        exit(0);
    }
    
    if (setpriority(pid1, 3) < 0) printf("Failed to set PID %d to priority 0\n", pid1);
    else printf("Set PID %d to priority 0\n", pid1);
    if (setpriority(pid2, 3) < 0) printf("Failed to set PID %d to priority 2\n", pid2);
    else printf("Set PID %d to priority 2\n", pid2);
    if (setpriority(pid3, 4) < 0) printf("Failed to set PID %d to priority 4\n", pid3);
    else printf("Set PID %d to priority 4\n", pid3);

    for (int i = 0; i < 3; i++) wait(0);
    printf("Test complete, check scheduler output.\n");
    exit(0);
}
