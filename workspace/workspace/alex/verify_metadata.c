/*
 * Metadata Behavior Verification
 * 
 * This test verifies the actual metadata behavior for allocated vs free blocks
 */

#include <stdio.h>
#include <stdlib.h>
#include "buddy.h"

#define PAGE_SIZE 4096
#define TEST_PAGES 65536

/* External metadata access for verification */
extern unsigned char *metadata;

int main() {
    printf("=== Metadata Behavior Verification ===\n\n");
    
    void *pool = malloc(TEST_PAGES * PAGE_SIZE);
    if (!pool) {
        printf("ERROR: Failed to allocate test memory\n");
        return 1;
    }
    
    init_page(pool, TEST_PAGES);
    
    /* Test 1: Allocate a rank-5 block */
    printf("Test 1: Allocated Block Metadata\n");
    void *block = alloc_pages(5);
    if ((long)block < 0) {
        printf("ERROR: Failed to allocate\n");
        return 1;
    }
    
    int block_size = 1 << (5 - 1);  /* 16 pages */
    int block_idx = ((char *)block - (char *)pool) / PAGE_SIZE;
    
    printf("  Block at index %d, rank 5, size %d pages\n", block_idx, block_size);
    printf("  Metadata for allocated block:\n");
    for (int i = 0; i < block_size && i < 5; i++) {
        printf("    Page %d: metadata[%d] = 0x%02x (rank=%d, free=%d)\n",
               i, block_idx + i, metadata[block_idx + i],
               metadata[block_idx + i] & 0x7F,
               (metadata[block_idx + i] & 0x80) ? 1 : 0);
    }
    
    /* Query ranks on allocated block */
    printf("  query_ranks() on allocated block:\n");
    for (int i = 0; i < block_size && i < 5; i++) {
        void *page = (char *)block + (i * PAGE_SIZE);
        int rank = query_ranks(page);
        printf("    Page %d: rank = %d\n", i, rank);
    }
    
    /* Test 2: Free the block */
    printf("\nTest 2: Freed Block Metadata (before any coalescing)\n");
    return_pages(block);
    
    printf("  Metadata after freeing:\n");
    for (int i = 0; i < block_size && i < 5; i++) {
        printf("    Page %d: metadata[%d] = 0x%02x (rank=%d, free=%d)\n",
               i, block_idx + i, metadata[block_idx + i],
               metadata[block_idx + i] & 0x7F,
               (metadata[block_idx + i] & 0x80) ? 1 : 0);
    }
    
    /* Query ranks on freed block */
    printf("  query_ranks() on freed block:\n");
    for (int i = 0; i < block_size && i < 5; i++) {
        void *page = (char *)pool + ((block_idx + i) * PAGE_SIZE);
        int rank = query_ranks(page);
        printf("    Page %d: rank = %d\n", i, rank);
    }
    
    /* Test 3: Check what happens with coalescing */
    printf("\nTest 3: Coalescing Behavior\n");
    
    /* Allocate two buddy blocks of rank 5 */
    void *b1 = alloc_pages(5);
    void *b2 = alloc_pages(5);
    
    if ((long)b1 < 0 || (long)b2 < 0) {
        printf("  Could not allocate buddy blocks for coalescing test\n");
    } else {
        int b1_idx = ((char *)b1 - (char *)pool) / PAGE_SIZE;
        int b2_idx = ((char *)b2 - (char *)pool) / PAGE_SIZE;
        
        printf("  Allocated b1 at index %d and b2 at index %d\n", b1_idx, b2_idx);
        
        /* Free both - they should coalesce */
        return_pages(b1);
        return_pages(b2);
        
        printf("  After freeing both blocks (should coalesce):\n");
        
        /* Check metadata of the coalesced block */
        int lower_idx = (b1_idx < b2_idx) ? b1_idx : b2_idx;
        printf("  Metadata at coalesced block head (idx=%d):\n", lower_idx);
        printf("    metadata[%d] = 0x%02x (rank=%d, free=%d)\n",
               lower_idx, metadata[lower_idx],
               metadata[lower_idx] & 0x7F,
               (metadata[lower_idx] & 0x80) ? 1 : 0);
        
        /* Check continuation pages */
        printf("  Metadata of continuation pages:\n");
        for (int i = 1; i < 10 && i < 32; i++) {
            printf("    metadata[%d] = 0x%02x (rank=%d, free=%d)\n",
                   lower_idx + i, metadata[lower_idx + i],
                   metadata[lower_idx + i] & 0x7F,
                   (metadata[lower_idx + i] & 0x80) ? 1 : 0);
        }
        
        /* Query ranks */
        printf("  query_ranks() results:\n");
        for (int i = 0; i < 10; i++) {
            void *page = (char *)pool + ((lower_idx + i) * PAGE_SIZE);
            int rank = query_ranks(page);
            printf("    Page %d: rank = %d\n", i, rank);
        }
    }
    
    free(pool);
    return 0;
}
