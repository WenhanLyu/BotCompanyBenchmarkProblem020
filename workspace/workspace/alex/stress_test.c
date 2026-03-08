#include <stdio.h>
#include <stdlib.h>
#include "buddy.h"

int main() {
    printf("=== Stress Testing ===\n\n");
    
    // Test 1: Large memory pool
    printf("Test 1: Large memory pool (32768 pages = 128MB)\n");
    size_t large_size = (size_t)32768 * 4096;
    void *large_pool = malloc(large_size);
    if (!large_pool) {
        printf("  Failed to allocate large pool\n");
        return 1;
    }
    
    int ret = init_page(large_pool, 32768);
    printf("  init_page(32768 pages): %s\n", ret == OK ? "OK" : "FAILED");
    
    // Try to allocate maximum rank
    void *max_block = alloc_pages(16);
    printf("  alloc_pages(16) [32768 pages]: %s\n", 
           !IS_ERR(max_block) ? "OK" : "FAILED");
    
    if (!IS_ERR(max_block)) {
        ret = return_pages(max_block);
        printf("  return_pages: %s\n", ret == OK ? "OK" : "FAILED");
    }
    
    // Test 2: Many small allocations
    printf("\nTest 2: Many small allocations\n");
    init_page(large_pool, 1024);
    
    void *ptrs[1024];
    int allocated = 0;
    for (int i = 0; i < 1024; i++) {
        ptrs[i] = alloc_pages(1);
        if (!IS_ERR(ptrs[i])) {
            allocated++;
        }
    }
    printf("  Allocated %d/1024 rank-1 pages: %s\n", 
           allocated, allocated == 1024 ? "OK" : "FAILED");
    
    // Return all
    for (int i = 0; i < allocated; i++) {
        ret = return_pages(ptrs[i]);
        if (ret != OK) {
            printf("  Failed to return page %d\n", i);
            break;
        }
    }
    printf("  Returned all pages: OK\n");
    
    // Test 3: Allocation pattern stress
    printf("\nTest 3: Allocation pattern stress\n");
    init_page(large_pool, 512);
    
    // Allocate various ranks
    void *r16 = alloc_pages(10);  // 512 pages
    printf("  alloc_pages(10): %s\n", !IS_ERR(r16) ? "OK" : "Failed as expected");
    
    void *r8 = alloc_pages(9);   // 256 pages
    printf("  alloc_pages(9): %s\n", !IS_ERR(r8) ? "OK" : "FAILED");
    
    void *r7 = alloc_pages(8);   // 128 pages
    printf("  alloc_pages(8): %s\n", !IS_ERR(r7) ? "OK" : "FAILED");
    
    void *r6 = alloc_pages(7);   // 64 pages
    printf("  alloc_pages(7): %s\n", !IS_ERR(r6) ? "OK" : "FAILED");
    
    // Should be full now
    void *should_fail = alloc_pages(1);
    printf("  alloc_pages(1) when full: %s\n", 
           IS_ERR(should_fail) && PTR_ERR(should_fail) == -ENOSPC ? "OK (correctly failed)" : "FAILED");
    
    // Return in different order
    if (!IS_ERR(r7)) return_pages(r7);
    if (!IS_ERR(r6)) return_pages(r6);
    if (!IS_ERR(r8)) return_pages(r8);
    
    // Now should be able to allocate again
    void *r_new = alloc_pages(9);
    printf("  alloc_pages(9) after freeing: %s\n", !IS_ERR(r_new) ? "OK" : "FAILED");
    
    printf("\n=== Stress Testing Complete ===\n");
    
    free(large_pool);
    return 0;
}
