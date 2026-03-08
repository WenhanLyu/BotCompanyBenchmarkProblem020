#include <stdio.h>
#include <stdlib.h>
#include "buddy.h"

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
    
    printf("Diagnostic Test for query_ranks()\n");
    printf("==================================\n\n");
    
    // Allocate two rank-1 pages
    void *page0 = alloc_pages(1);
    void *page1 = alloc_pages(1);
    
    printf("Step 1: Allocated two rank-1 pages\n");
    printf("  page0 = %p\n", page0);
    printf("  page1 = %p\n", page1);
    printf("  query_ranks(page0) = %d\n", query_ranks(page0));
    printf("  query_ranks(page1) = %d\n", query_ranks(page1));
    
    // Free page0
    return_pages(page0);
    printf("\nStep 2: Freed page0\n");
    printf("  query_ranks(page0) = %d\n", query_ranks(page0));
    printf("  query_ranks(page1) = %d (still allocated)\n", query_ranks(page1));
    
    // Free page1 - should coalesce
    return_pages(page1);
    printf("\nStep 3: Freed page1 (should coalesce to rank-2)\n");
    printf("  query_ranks(page0) = %d (expected: 2)\n", query_ranks(page0));
    printf("  query_ranks(page1) = %d (expected: 2)\n", query_ranks(page1));
    
    free(pool);
    return 0;
}
