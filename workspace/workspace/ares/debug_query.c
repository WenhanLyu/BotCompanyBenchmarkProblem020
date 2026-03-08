#include <stdio.h>
#include <stdlib.h>
#include "buddy.h"

// External access to metadata for debugging
extern unsigned char *metadata;
extern void *base_ptr;
extern int total_pages;

int main() {
    // Initialize with a small pool
    void *pool = malloc(32 * 4096); // 32 pages
    if (!pool) {
        printf("Failed to allocate pool\n");
        return 1;
    }
    
    if (init_page(pool, 32) != 0) {
        printf("Failed to initialize buddy system\n");
        return 1;
    }
    
    printf("Debug query_ranks() Bug\n");
    printf("========================\n\n");
    
    // Allocate two rank-1 pages
    void *page0 = alloc_pages(1);
    void *page1 = alloc_pages(1);
    
    printf("After allocation:\n");
    long idx0 = ((char*)page0 - (char*)base_ptr) / 4096;
    long idx1 = ((char*)page1 - (char*)base_ptr) / 4096;
    printf("  page0 idx=%ld, metadata=0x%02x (rank=%d, free=%d)\n", 
           idx0, metadata[idx0], metadata[idx0] & 0x7F, (metadata[idx0] & 0x80) ? 1 : 0);
    printf("  page1 idx=%ld, metadata=0x%02x (rank=%d, free=%d)\n", 
           idx1, metadata[idx1], metadata[idx1] & 0x7F, (metadata[idx1] & 0x80) ? 1 : 0);
    
    // Free page0
    return_pages(page0);
    printf("\nAfter freeing page0:\n");
    printf("  page0 idx=%ld, metadata=0x%02x (rank=%d, free=%d)\n", 
           idx0, metadata[idx0], metadata[idx0] & 0x7F, (metadata[idx0] & 0x80) ? 1 : 0);
    printf("  page1 idx=%ld, metadata=0x%02x (rank=%d, free=%d)\n", 
           idx1, metadata[idx1], metadata[idx1] & 0x7F, (metadata[idx1] & 0x80) ? 1 : 0);
    
    // Free page1 - should coalesce to rank-2
    return_pages(page1);
    printf("\nAfter freeing page1 (coalesce to rank-2):\n");
    printf("  page0 idx=%ld, metadata=0x%02x (rank=%d, free=%d)\n", 
           idx0, metadata[idx0], metadata[idx0] & 0x7F, (metadata[idx0] & 0x80) ? 1 : 0);
    printf("  page1 idx=%ld, metadata=0x%02x (rank=%d, free=%d)\n", 
           idx1, metadata[idx1], metadata[idx1] & 0x7F, (metadata[idx1] & 0x80) ? 1 : 0);
    
    printf("\nquery_ranks results:\n");
    printf("  query_ranks(page0) = %d (expected: 2)\n", query_ranks(page0));
    printf("  query_ranks(page1) = %d (expected: 2)\n", query_ranks(page1));
    
    // Let's manually trace what query_ranks would do for page0
    printf("\nManual trace for query_ranks(page0) at idx=%ld:\n", idx0);
    printf("  metadata[%ld] = 0x%02x, rank from metadata = %d\n", 
           idx0, metadata[idx0], metadata[idx0] & 0x7F);
    
    // Check if it's a free block head
    if (metadata[idx0] & 0x80) {
        printf("  -> This IS a free block head! Should return %d immediately\n", metadata[idx0] & 0x7F);
    } else {
        printf("  -> Not a free block head, searching for containing blocks...\n");
    }
    
    // Trace the search loop
    for (int r = 16; r >= 1; r--) {
        int block_size = 1 << (r - 1);
        int head_idx = (idx0 / block_size) * block_size;
        printf("  r=%d: block_size=%d, head_idx=%d", r, block_size, head_idx);
        
        if (head_idx < idx0 && idx0 < head_idx + block_size) {
            printf(", checking head_idx < idx0 < head_idx+block_size");
            if (metadata[head_idx] & 0x80) {
                if ((metadata[head_idx] & 0x7F) == r) {
                    printf(" -> MATCH! Would return %d\n", r);
                    break;
                } else {
                    printf(" (free but wrong rank)\n");
                }
            } else {
                printf(" (not free)\n");
            }
        } else {
            printf(" (range check failed)\n");
        }
    }
    
    free(pool);
    return 0;
}
