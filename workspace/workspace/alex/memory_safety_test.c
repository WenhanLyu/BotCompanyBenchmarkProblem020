#include <stdio.h>
#include <stdlib.h>
#include "buddy.h"

int main() {
    printf("=== Memory Safety Tests ===\n\n");
    
    // Test boundary conditions
    printf("Test 1: Boundary alignment checks\n");
    void *p = malloc(1024 * 1024);  // 256 pages
    init_page(p, 256);
    
    // Get first page
    void *page = alloc_pages(1);
    printf("  Allocated page: %p\n", page);
    
    // Query the exact page
    int rank = query_ranks(page);
    printf("  query_ranks(exact page): %d %s\n", rank, rank == 1 ? "OK" : "FAILED");
    
    // Query page + 1 byte (should fail - unaligned)
    rank = query_ranks((char*)page + 1);
    printf("  query_ranks(page + 1): %d %s\n", rank, rank == -EINVAL ? "OK" : "FAILED");
    
    // Query page + 4095 (should fail - unaligned)  
    rank = query_ranks((char*)page + 4095);
    printf("  query_ranks(page + 4095): %d %s\n", rank, rank == -EINVAL ? "OK" : "FAILED");
    
    // Query page + 4096 (next page, should work)
    rank = query_ranks((char*)page + 4096);
    printf("  query_ranks(next page): %d (should be valid rank or -EINVAL if not allocated)\n", rank);
    
    return_pages(page);
    
    // Test large allocations
    printf("\nTest 2: Maximum rank allocation\n");
    init_page(p, 32768);  // 2^15 pages
    
    void *max_block = alloc_pages(16);  // Rank 16 = 2^15 pages
    printf("  alloc_pages(16): %s\n", !IS_ERR(max_block) ? "OK" : "FAILED");
    
    if (!IS_ERR(max_block)) {
        rank = query_ranks(max_block);
        printf("  query_ranks(rank 16 block): %d %s\n", rank, rank == 16 ? "OK" : "FAILED");
        
        int ret = return_pages(max_block);
        printf("  return_pages(rank 16 block): %s\n", ret == OK ? "OK" : "FAILED");
    }
    
    // Test multiple allocations of different ranks
    printf("\nTest 3: Mixed rank allocations\n");
    init_page(p, 128);
    
    void *r1 = alloc_pages(3);  // 4 pages
    void *r2 = alloc_pages(2);  // 2 pages  
    void *r3 = alloc_pages(1);  // 1 page
    
    printf("  Allocated rank 3: %s\n", !IS_ERR(r1) ? "OK" : "FAILED");
    printf("  Allocated rank 2: %s\n", !IS_ERR(r2) ? "OK" : "FAILED");
    printf("  Allocated rank 1: %s\n", !IS_ERR(r3) ? "OK" : "FAILED");
    
    if (!IS_ERR(r1)) {
        int q = query_ranks(r1);
        printf("  query_ranks(rank 3 block): %d %s\n", q, q == 3 ? "OK" : "FAILED");
    }
    if (!IS_ERR(r2)) {
        int q = query_ranks(r2);
        printf("  query_ranks(rank 2 block): %d %s\n", q, q == 2 ? "OK" : "FAILED");
    }
    if (!IS_ERR(r3)) {
        int q = query_ranks(r3);
        printf("  query_ranks(rank 1 block): %d %s\n", q, q == 1 ? "OK" : "FAILED");
    }
    
    // Test partial coalescing
    printf("\nTest 4: Partial coalescing\n");
    init_page(p, 16);
    
    void *pages[16];
    for (int i = 0; i < 16; i++) {
        pages[i] = alloc_pages(1);
    }
    
    // Free first 4 pages
    for (int i = 0; i < 4; i++) {
        return_pages(pages[i]);
    }
    
    // Should have 1 rank-3 block (4 pages)
    int count = query_page_counts(3);
    printf("  After freeing first 4 pages, rank-3 count: %d %s\n", 
           count, count == 1 ? "OK" : "FAILED");
    
    // Free next 4 pages
    for (int i = 4; i < 8; i++) {
        return_pages(pages[i]);
    }
    
    // Should have 1 rank-4 block (8 pages)
    count = query_page_counts(4);
    printf("  After freeing first 8 pages, rank-4 count: %d %s\n", 
           count, count == 1 ? "OK" : "FAILED");
    
    printf("\n=== Memory Safety Tests Complete ===\n");
    
    free(p);
    return 0;
}
