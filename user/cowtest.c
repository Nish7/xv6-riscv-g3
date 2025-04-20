#include "kernel/types.h"
#include "user/user.h"
#include "kernel/memlayout.h"
#define PGSIZE 4096

// Test 1: Basic COW functionality
void test_cow() {
  char *mem = sbrk(PGSIZE);
  if (mem == (char *)-1) {
    printf("sbrk failed\n");
    exit(1);
  }
  *mem = 'A';

  int pid = fork();
  if (pid < 0) {
    printf("fork failed\n");
    exit(1);
  } else if (pid == 0) {
    // Child process
    printf("Child reads before write: %s \n", mem); // Should be 'A'
    *mem = 'B'; // Write to shared page
    printf("Child wrote: B \n");
    printf("Memory address: %p\n", mem);
    exit(0);
  } else {
    // Parent process
    wait(0);
    if (*mem == 'A') {
        printf("Parent reads: A \n");
    } else {
        printf("Parent reads something that is wrong");
    }
  }
}

// Test 2: Multiple writes in child
void test_multiple_writes() {
  char *mem = sbrk(PGSIZE);
  if (mem == (char *)-1) {
    printf("sbrk failed\n");
    exit(1);
  }
  *mem = 'A';

  int pid = fork();
  if (pid < 0) {
    printf("fork failed\n");
    exit(1);
  } else if (pid == 0) {
    // Child process
    printf("Child reads before writes: %s \n", mem);
    *mem = 'B'; // First write
    *(mem + 1) = 'C'; // Second write
    printf("Child wrote: %s %s \n", mem, (char *)(mem + 1));
    exit(0);
  } else {
    // Parent process
    wait(0);
    printf("Parent reads: %s %s \n", mem, (char *)(mem + 1)); // Should be unchanged
  }
}

// Test 3: Page fault handling
void test_page_fault_handling() {
  int pid = fork();
  if (pid < 0) {
    printf("fork failed\n");
    exit(1);
  } else if (pid == 0) {
    // Child process
    char *mem = sbrk(PGSIZE);
    *mem = 'E'; // Write to shared page
    printf("Child wrote: %s\n", mem);
    exit(0);
  } else {
    // Parent process
    wait(0);
    char *mem = sbrk(PGSIZE);
    printf("Parent reads: %s\n", mem); // Should be unchanged
  }
}

int main() {
  printf("Running COW tests...\n");

  printf("First test....\n");
  test_cow();

  printf("Second test....\n");
  test_multiple_writes();

  printf("Third test....\n");
  test_page_fault_handling();

  printf("All tests completed.\n");
  exit(0);
}
