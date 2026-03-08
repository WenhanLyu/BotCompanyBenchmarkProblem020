#include <stdio.h>
#include <stdlib.h>
#include "buddy.h"

int main() {
    printf("=== Detailed Coalescing Test ===\n\n");
    
    // Test with 64 pages (2^6 pages)
    void *p = malloc(262144);  // 64 * 4096
    init_page(p, 64);
    
    printf("Initial state after init:\n");
    for (int r = 1; r <= 16; r++) {
        int count = query_page_counts(r);
        if (count > 0) {
            printf("  Rank %d: %d blocks\n", r, count);
        }
    }
    
    // Allocate all as rank 1
    printf("\nAllocating all 64 pages as rank-1...\n");
    void *allocs[64];
    for (int i = 0; i < 64; i++) {
        allocs[i] = alloc_pages(1);
    }
    
    printf("After allocating all:\n");
    for (int r = 1; r <= 16; r++) {
        int count = query_page_counts(r);
        if (count > 0) {
            printf("  Rank %d: %d blocks\n", r, count);
        }
    }
    
    // Free in order
    printf("\nFreeing all 64 pages in order...\n");
    for (int i = 0; i < 64; i++) {
        return_pages(allocs[i]);
    }
    
    printf("After freeing all in order:\n");
    for (int r = 1; r <= 16; r++) {
        int count = query_page_counts(r);
        if (count > 0) {
            printf("  Rank %d: %d blocks\n", r, count);
        }
    }
    
    // Now test with a different pattern - allocate and free in reverse order
    printf("\n--- Second test: reverse order ---\n");
    init_page(p, 64);
    
    for (int i = 0; i < 64; i++) {
        allocs[i] = alloc_pages(1);
    }
    
    printf("Freeing all 64 pages in reverse order...\n");
    for (int i = 63; i >= 0; i--) {
        return_pages(allocs[i]);
    }
    
    printf("After freeing all in reverse order:\n");
    for (int r = 1; r <= 16; r++) {
        int count = query_page_counts(r);
        if (count > 0) {
            printf("  Rank %d: %d blocks\n", r, count);
        }
    }
    
    // Test with exact power of 2 allocations
    printf("\n--- Third test: buddy-aligned allocations ---\n");
    init_page(p, 64);
    
    // Allocate 2 rank-5 blocks (32 pages each)
    void *b1 = alloc_pages(6);  // rank 6 = 32 pages
    void *b2 = alloc_pages(6);
    
    printf("Allocated 2 rank-6 blocks\n");
    printf("Pointer 1: %p\n", b1);
    printf("Pointer 2: %p\n", b2);
    
    printf("Free counts before returning:\n");
    for (int r = 1; r <= 16; r++) {
        int count = query_page_counts(r);
        if (count > 0) {
            printf("  Rank %d: %d blocks\n", r, count);
        }
    }
    
    // Return them
    return_pages(b1);
    printf("\nAfter returning first block:\n");
    for (int r = 1; r <= 16; r++) {
        int count = query_page_counts(r);
        if (count > 0) {
            printf("  Rank %d: %d blocks\n", r, count);
        }
    }
    
    return_pages(b2);
    printf("\nAfter returning second block:\n");
    for (int r = 1; r <= 16; r++) {
        int count = query_page_counts(r);
        if (count > 0) {
            printf("  Rank %d: %d blocks\n", r, count);
        }
    }
    
    free(p);
    return 0;
}
