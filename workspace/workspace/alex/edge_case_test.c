#include <stdio.h>
#include <stdlib.h>
#include "buddy.h"

int main() {
    printf("=== Edge Case Testing ===\n\n");
    
    // Test 1: Initialize with small page count
    printf("Test 1: Small initialization (1 page)\n");
    void *p1 = malloc(4096);
    int ret = init_page(p1, 1);
    printf("  init_page with 1 page: %s\n", ret == OK ? "OK" : "FAILED");
    
    void *r1 = alloc_pages(1);
    printf("  alloc_pages(1): %s\n", !IS_ERR(r1) ? "OK" : "FAILED");
    
    void *r2 = alloc_pages(1);
    printf("  alloc_pages(1) again (should fail): %s\n", 
           IS_ERR(r2) && PTR_ERR(r2) == -ENOSPC ? "OK (correctly failed)" : "FAILED");
    
    ret = return_pages(r1);
    printf("  return_pages: %s\n", ret == OK ? "OK" : "FAILED");
    
    // Test 2: Invalid rank testing
    printf("\nTest 2: Invalid rank tests\n");
    void *p2 = malloc(65536);
    init_page(p2, 16);
    
    void *r_invalid1 = alloc_pages(0);
    printf("  alloc_pages(0): %s\n", 
           IS_ERR(r_invalid1) && PTR_ERR(r_invalid1) == -EINVAL ? "OK (correctly rejected)" : "FAILED");
    
    void *r_invalid2 = alloc_pages(17);
    printf("  alloc_pages(17): %s\n", 
           IS_ERR(r_invalid2) && PTR_ERR(r_invalid2) == -EINVAL ? "OK (correctly rejected)" : "FAILED");
    
    void *r_invalid3 = alloc_pages(-1);
    printf("  alloc_pages(-1): %s\n", 
           IS_ERR(r_invalid3) && PTR_ERR(r_invalid3) == -EINVAL ? "OK (correctly rejected)" : "FAILED");
    
    int qc_invalid1 = query_page_counts(0);
    printf("  query_page_counts(0): %s\n", 
           qc_invalid1 == -EINVAL ? "OK (correctly rejected)" : "FAILED");
    
    int qc_invalid2 = query_page_counts(17);
    printf("  query_page_counts(17): %s\n", 
           qc_invalid2 == -EINVAL ? "OK (correctly rejected)" : "FAILED");
    
    // Test 3: Return invalid pointers
    printf("\nTest 3: Invalid return_pages tests\n");
    void *p3 = malloc(65536);
    init_page(p3, 16);
    
    ret = return_pages(NULL);
    printf("  return_pages(NULL): %s\n", 
           ret == -EINVAL ? "OK (correctly rejected)" : "FAILED");
    
    ret = return_pages((void*)0x1234);
    printf("  return_pages(invalid ptr): %s\n", 
           ret == -EINVAL ? "OK (correctly rejected)" : "FAILED");
    
    void *alloc1 = alloc_pages(2);
    ret = return_pages(alloc1);
    printf("  return_pages(valid): %s\n", ret == OK ? "OK" : "FAILED");
    
    ret = return_pages(alloc1);
    printf("  return_pages(same ptr twice): %s\n", 
           ret == -EINVAL ? "OK (correctly rejected double free)" : "FAILED");
    
    // Test 4: Query invalid pointers
    printf("\nTest 4: Invalid query_ranks tests\n");
    void *p4 = malloc(65536);
    init_page(p4, 16);
    
    int qr = query_ranks(NULL);
    printf("  query_ranks(NULL): %s\n", 
           qr == -EINVAL ? "OK (correctly rejected)" : "FAILED");
    
    qr = query_ranks((void*)((char*)p4 + 1));  // Unaligned
    printf("  query_ranks(unaligned): %s\n", 
           qr == -EINVAL ? "OK (correctly rejected)" : "FAILED");
    
    qr = query_ranks((void*)((char*)p4 + 65536));  // Out of bounds
    printf("  query_ranks(out of bounds): %s\n", 
           qr == -EINVAL ? "OK (correctly rejected)" : "FAILED");
    
    // Test 5: Large allocation and fragmentation
    printf("\nTest 5: Fragmentation test\n");
    void *p5 = malloc(262144);  // 64 pages
    init_page(p5, 64);
    
    // Allocate all as rank 1
    void *allocs[64];
    int i;
    for (i = 0; i < 64; i++) {
        allocs[i] = alloc_pages(1);
        if (IS_ERR(allocs[i])) {
            printf("  Failed to allocate page %d\n", i);
            break;
        }
    }
    printf("  Allocated %d rank-1 pages: %s\n", i, i == 64 ? "OK" : "FAILED");
    
    // Free every other one
    for (i = 0; i < 64; i += 2) {
        return_pages(allocs[i]);
    }
    
    int free_rank1 = query_page_counts(1);
    printf("  Free rank-1 pages after freeing half: %d (expected 32)\n", free_rank1);
    
    // Try to allocate rank 2 (should fail due to fragmentation)
    void *r_rank2 = alloc_pages(2);
    printf("  alloc_pages(2) with fragmentation: %s\n", 
           IS_ERR(r_rank2) && PTR_ERR(r_rank2) == -ENOSPC ? "OK (correctly failed due to fragmentation)" : "FAILED");
    
    // Free the rest to coalesce
    for (i = 1; i < 64; i += 2) {
        return_pages(allocs[i]);
    }
    
    int free_rank6 = query_page_counts(6);  // 64 pages = 2^6
    printf("  Free rank-6 pages after full coalesce: %d (expected 1)\n", free_rank6);
    
    printf("\n=== All Edge Case Tests Complete ===\n");
    
    free(p1);
    free(p2);
    free(p3);
    free(p4);
    free(p5);
    
    return 0;
}
