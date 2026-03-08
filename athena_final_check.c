/* Athena's Final Verification Test - Cycle 28
 * Testing the specific stale metadata bug scenario
 */

#include <stdio.h>
#include <stdlib.h>
#include "buddy.h"

#define PAGE_SIZE 4096

int main() {
    int failures = 0;
    int tests = 0;
    
    /* Allocate a large pool to ensure coalescing works */
    void *pool = malloc(PAGE_SIZE * 1024);  /* 1024 pages = 4MB */
    if (!pool) {
        printf("Failed to allocate test pool\n");
        return 1;
    }
    
    init_page(pool, 1024);
    
    /* Test 1: Allocate small blocks, then free to cause coalescing */
    printf("Test 1: Stale metadata after coalescing\n");
    
    /* Allocate 4 rank-1 blocks (4 pages) */
    void *b1 = alloc_pages(1);
    void *b2 = alloc_pages(1);
    void *b3 = alloc_pages(1);
    void *b4 = alloc_pages(1);
    
    tests++;
    if (!b1 || !b2 || !b3 || !b4) {
        printf("FAIL: Could not allocate rank-1 blocks\n");
        failures++;
    } else {
        printf("PASS: Allocated 4 rank-1 blocks\n");
    }
    
    /* Free them to trigger coalescing */
    return_pages(b1);
    return_pages(b2);
    return_pages(b3);
    return_pages(b4);
    
    /* After coalescing, these 4 pages should be part of a larger free block
     * Query each page - they should all return the same (coalesced) rank
     */
    int r1 = query_ranks(b1);
    int r2 = query_ranks(b2);
    int r3 = query_ranks(b3);
    int r4 = query_ranks(b4);
    
    tests++;
    if (r1 != r2 || r2 != r3 || r3 != r4) {
        printf("FAIL: Inconsistent ranks after coalescing: %d, %d, %d, %d\n", r1, r2, r3, r4);
        failures++;
    } else {
        printf("PASS: All 4 pages return consistent rank %d after coalescing\n", r1);
    }
    
    tests++;
    if (r1 < 1 || r1 > 16) {
        printf("FAIL: Invalid rank %d\n", r1);
        failures++;
    } else {
        printf("PASS: Rank is valid (%d)\n", r1);
    }
    
    /* Test 2: Allocate larger block, free it, query continuation pages */
    printf("\nTest 2: Continuation pages in free blocks\n");
    
    void *big = alloc_pages(5);  /* rank 5 = 16 pages */
    tests++;
    if (!big) {
        printf("FAIL: Could not allocate rank-5 block\n");
        failures++;
    } else {
        printf("PASS: Allocated rank-5 block at %p\n", big);
        
        /* Free it */
        return_pages(big);
        
        /* Query all 16 pages in the block */
        int consistent = 1;
        int first_rank = query_ranks(big);
        
        for (int i = 0; i < 16; i++) {
            void *page = (char *)big + i * PAGE_SIZE;
            int rank = query_ranks(page);
            
            if (rank != first_rank) {
                printf("FAIL: Page %d has rank %d, expected %d\n", i, rank, first_rank);
                consistent = 0;
                failures++;
                tests++;
            }
        }
        
        tests++;
        if (consistent) {
            printf("PASS: All 16 pages in freed block return consistent rank %d\n", first_rank);
        }
        
        tests++;
        if (first_rank >= 5) {
            printf("PASS: Coalesced rank %d >= original rank 5\n", first_rank);
        } else {
            printf("FAIL: Coalesced rank %d < original rank 5\n", first_rank);
            failures++;
        }
    }
    
    /* Test 3: Block head vs continuation page distinction */
    printf("\nTest 3: Block heads return correct rank immediately\n");
    
    void *b = alloc_pages(3);  /* rank 3 = 4 pages */
    tests++;
    if (!b) {
        printf("FAIL: Could not allocate rank-3 block\n");
        failures++;
    } else {
        int rank = query_ranks(b);
        tests++;
        if (rank == 3) {
            printf("PASS: Allocated block head returns rank 3\n");
        } else {
            printf("FAIL: Allocated block head returns rank %d, expected 3\n", rank);
            failures++;
        }
        
        /* Query continuation pages */
        for (int i = 1; i < 4; i++) {
            void *page = (char *)b + i * PAGE_SIZE;
            int r = query_ranks(page);
            tests++;
            if (r == 3) {
                printf("PASS: Continuation page %d returns rank 3\n", i);
            } else {
                printf("FAIL: Continuation page %d returns rank %d, expected 3\n", i, r);
                failures++;
            }
        }
        
        return_pages(b);
    }
    
    free(pool);
    
    printf("\n========================================\n");
    printf("Tests: %d, Failures: %d\n", tests, failures);
    if (failures == 0) {
        printf("✅ ALL TESTS PASSED - Code ready for OJ submission\n");
    } else {
        printf("❌ FAILURES DETECTED - DO NOT submit to OJ\n");
    }
    printf("========================================\n");
    
    return failures > 0 ? 1 : 0;
}
