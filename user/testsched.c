#include "kernel/types.h"
#include "user/user.h"

void busy_loop() {
    for (int j = 0; j < 1000; j++) {}
}

int main() {
    int pid1, pid2, pid3, pid4;

    printf("Starting priority scheduling test...\n");

    if ((pid1 = fork()) == 0) {
        printf("Child 1 PID: %d\n", getpid());
        if (setpriority(getpid(), 4) < 0) {
            printf("Child 1 failed to set priority to 0\n");
            exit(1);
        }
        busy_loop();
        printf("Child 1 exiting\n");
        exit(0);
    }

    if ((pid2 = fork()) == 0) {
        printf("Child 2 PID: %d\n", getpid());
        if (setpriority(getpid(), 3) < 0) {
            printf("Child 1 failed to set priority to 0\n");
            exit(1);
        }
        printf("Child 2 set priority to 3\n");
        while(1) busy_loop();
        printf("Child 2 exiting\n");
        exit(0);
    }

    if ((pid3 = fork()) == 0) {
        printf("Child 3 PID: %d\n", getpid());
        if (setpriority(getpid(), 3) < 0) {
            printf("Child 1 failed to set priority to 0\n");
            exit(1);
        }
        printf("Child 3 set priority to 3\n");
        while(1) busy_loop();
        printf("Child 3 exiting\n");
        exit(0);
    }

    if ((pid4 = fork()) == 0) {
        printf("Child 4 PID: %d\n", getpid());
        if (setpriority(getpid(), 1) < 0) {
            printf("Child 4 failed to set priority to 0\n");
            exit(1);
        }
        printf("Child 3 set priority to 3\n");
        while(1) busy_loop();
        printf("Child 3 exiting\n");
        exit(0);
    }

    for (int i = 0; i < 3; i++) wait(0);
    printf("Test complete, check scheduler output.\n");
    exit(0);
}
