#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Define a large array that should trigger demand paging
#define ARRAY_SIZE (4*1024*1024)  // 4MB of data
char large_array[ARRAY_SIZE];

void test_sequential_access() {
  printf("Testing sequential memory access...\n");
  
  // Access each page of the array sequentially
  // This should trigger demand paging one page at a time
  for (int i = 0; i < ARRAY_SIZE; i += 4096) {
    large_array[i] = i & 0xFF;
    printf("Accessed memory at offset %d (page %d)\n", i, i/4096);
    // sleep(20); // Sleep a bit to see paging in the kernel logs
  }
  
  // Verify the data was written correctly
  int errors = 0;
  for (int i = 0; i < ARRAY_SIZE; i += 4096) {
    if (large_array[i] != (i & 0xFF)) {
      printf("Error at index %d: expected %d, got %d\n", 
             i, i & 0xFF, large_array[i]);
      errors++;
    }
  }
  
  if (errors == 0)
    printf("Sequential access test passed!\n");
  else
    printf("Sequential access test failed with %d errors\n", errors);
}

void test_random_access() {
  printf("\nTesting random memory access...\n");
  
  // Access some random pages to demonstrate non-sequential demand paging
  int indices[] = {
    1048576,  // 1MB mark
    2097152,  // 2MB mark 
    3145728,  // 3MB mark
    512000,   // ~0.5MB mark
    2621440   // ~2.5MB mark
  };
  
  for (int i = 0; i < 5; i++) {
    int idx = indices[i];
    large_array[idx] = (idx & 0xFF);
    printf("Accessed memory at offset %d (page %d)\n", idx, idx/4096);
    sleep(20); // Sleep a bit to see paging in the kernel logs
  }
  
  // Verify the data
  int errors = 0;
  for (int i = 0; i < 5; i++) {
    int idx = indices[i];
    if (large_array[idx] != (idx & 0xFF)) {
      printf("Error at index %d: expected %d, got %d\n", 
             idx, idx & 0xFF, large_array[idx]);
      errors++;
    }
  }
  
  if (errors == 0)
    printf("Random access test passed!\n");
  else
    printf("Random access test failed with %d errors\n", errors);
}

void test_bss_section() {
  printf("\nTesting BSS section (should be demand-paged and zero-initialized)...\n");
  
  // The large_array is in the BSS section and should be demand-paged
  // Check that it's properly zero-initialized
  int errors = 0;
  for (int i = 10000; i < 20000; i += 1000) {
    if (large_array[i] != 0) {
      printf("Error: BSS section not zeroed at index %d (value: %d)\n", 
             i, large_array[i]);
      errors++;
    } else {
      printf("Verified zero at index %d\n", i);
    }
    sleep(10);
  }
  
  if (errors == 0)
    printf("BSS section test passed!\n");
  else
    printf("BSS section test failed with %d errors\n", errors);
}

int main() {
  printf("Starting demand paging test...\n");
  printf("Large array size: %d bytes (%d pages)\n", 
         ARRAY_SIZE, ARRAY_SIZE/4096);
  
  test_bss_section();
  test_sequential_access();
  test_random_access();
  
  printf("\nAll demand paging tests completed\n");
  return 0;
}
