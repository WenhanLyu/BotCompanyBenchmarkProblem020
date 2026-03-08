#include <stdio.h>
#include <stdlib.h>
#include "buddy.h"

int main() {
    // Allocate a larger pool to control fragmentation
    void *pool = malloc(256 * 4096); // 256 pages
    if (!pool) {
        printf("Failed to allocate pool\n");
        return 1;
    }
    
    if (init_page(pool, 256) != 0) {
        printf("Failed to initialize buddy system\n");
        return 1;
    }
    
    printf("Test: Stale Metadata After Coalescing\n");
    printf("=======================================\n\n");
    
    // Allocate a rank-3 block (4 pages)
    void *block = alloc_pages(3);
    if (IS_ERR(block)) {
        printf("Failed to allocate rank-3 block\n");
        return 1;
    }
    
    printf("Step 1: Allocated rank-3 block (4 pages)\n");
    printf("  block = %p\n", block);
    
    // Check all 4 pages
    for (int i = 0; i < 4; i++) {
        void *page = (char*)block + i * 4096;
        int rank = query_ranks(page);
        printf("  page[%d] rank = %d (expected: 3)\n", i, rank);
        if (rank != 3) {
            printf("  ERROR: Expected 3, got %d\n", rank);
        }
    }
    
    // Free the block
    int ret = return_pages(block);
    if (ret != 0) {
        printf("\nFailed to free block: %d\n", ret);
        return 1;
    }
    
    printf("\nStep 2: Freed rank-3 block\n");
    printf("  (Should coalesce with buddies if they're free)\n");
    
    // Now query all 4 pages - they should all return the same rank
    // (the rank of the coalesced free block)
    int ranks[4];
    for (int i = 0; i < 4; i++) {
        void *page = (char*)block + i * 4096;
        ranks[i] = query_ranks(page);
        printf("  page[%d] rank = %d\n", i, ranks[i]);
    }
    
    // Verify all pages return the same rank
    int all_same = 1;
    for (int i = 1; i < 4; i++) {
        if (ranks[i] != ranks[0]) {
            all_same = 0;
            break;
        }
    }
    
    if (all_same) {
        printf("\n✓ SUCCESS: All pages return the same rank (%d)\n", ranks[0]);
        printf("  This proves stale metadata bug is fixed!\n");
    } else {
        printf("\n✗ FAILURE: Pages return different ranks!\n");
        printf("  page[0] = %d\n", ranks[0]);
        printf("  page[1] = %d\n", ranks[1]);
        printf("  page[2] = %d\n", ranks[2]);
        printf("  page[3] = %d\n", ranks[3]);
        printf("  Stale metadata bug still exists!\n");
        free(pool);
        return 1;
    }
    
    free(pool);
    return 0;
}
