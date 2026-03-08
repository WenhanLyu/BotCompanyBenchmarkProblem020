/*
 * Continuation Page Verification Test for query_ranks()
 * 
 * This test verifies that query_ranks() correctly handles continuation pages
 * (pages with rank=0) after the metadata optimization that only marks block heads.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buddy.h"

#define PAGE_SIZE 4096
#define MAXRANK 16
#define TEST_PAGES 65536  /* 256 MB worth of pages */

int test_count = 0;
int pass_count = 0;

void assert_equal(int actual, int expected, const char *msg) {
    test_count++;
    if (actual == expected) {
        pass_count++;
        printf("✓ Test %d: %s (expected=%d, actual=%d)\n", test_count, msg, expected, actual);
    } else {
        printf("✗ Test %d: %s (expected=%d, actual=%d)\n", test_count, msg, expected, actual);
    }
}

void test_continuation_pages_for_free_blocks() {
    printf("\n=== Test 1: Continuation Pages for Free Blocks ===\n");
    
    /* Allocate memory pool */
    void *pool = malloc(TEST_PAGES * PAGE_SIZE);
    if (!pool) {
        printf("ERROR: Failed to allocate test memory\n");
        return;
    }
    
    /* Initialize */
    int ret = init_page(pool, TEST_PAGES);
    assert_equal(ret, 0, "init_page should succeed");
    
    /* Test large free blocks - should have continuation pages */
    /* After init, we should have max-rank blocks */
    
    /* Query the first page of the first block */
    int rank = query_ranks(pool);
    assert_equal(rank, MAXRANK, "First page should have rank MAXRANK");
    
    /* Query continuation pages within the first max-rank block */
    /* Block size for MAXRANK = 2^15 = 32768 pages */
    int block_size = 1 << (MAXRANK - 1);
    
    for (int offset = 1; offset < block_size && offset < 100; offset++) {
        void *continuation = (char *)pool + (offset * PAGE_SIZE);
        int cont_rank = query_ranks(continuation);
        assert_equal(cont_rank, MAXRANK, "Continuation page should return MAXRANK");
    }
    
    /* Test the last page of the first block */
    void *last_page = (char *)pool + ((block_size - 1) * PAGE_SIZE);
    rank = query_ranks(last_page);
    assert_equal(rank, MAXRANK, "Last page of block should return MAXRANK");
    
    free(pool);
}

void test_continuation_pages_after_split() {
    printf("\n=== Test 2: Continuation Pages After Block Split ===\n");
    
    void *pool = malloc(TEST_PAGES * PAGE_SIZE);
    if (!pool) {
        printf("ERROR: Failed to allocate test memory\n");
        return;
    }
    
    init_page(pool, TEST_PAGES);
    
    /* Allocate a rank-10 block, which will cause splitting */
    void *allocated = alloc_pages(10);
    if ((long)allocated < 0) {
        printf("ERROR: Failed to allocate rank-10 block\n");
        free(pool);
        return;
    }
    
    /* After splitting a MAXRANK block, we should have:
     * - One allocated rank-10 block
     * - One free rank-MAXRANK-1 block (or smaller free blocks)
     */
    
    /* The allocated block should have rank 10 on ALL pages */
    int block_size_10 = 1 << (10 - 1);  /* 2^9 = 512 pages */
    
    for (int offset = 0; offset < block_size_10 && offset < 50; offset++) {
        void *page = (char *)allocated + (offset * PAGE_SIZE);
        int rank = query_ranks(page);
        assert_equal(rank, 10, "Allocated block pages should have rank 10");
    }
    
    free(pool);
}

void test_continuation_pages_all_ranks() {
    printf("\n=== Test 3: Continuation Pages for All Ranks ===\n");
    
    void *pool = malloc(TEST_PAGES * PAGE_SIZE);
    if (!pool) {
        printf("ERROR: Failed to allocate test memory\n");
        return;
    }
    
    init_page(pool, TEST_PAGES);
    
    /* Test different ranks */
    for (int rank = 1; rank <= MAXRANK && rank <= 12; rank++) {
        void *allocated = alloc_pages(rank);
        if ((long)allocated < 0) {
            printf("Warning: Could not allocate rank %d\n", rank);
            continue;
        }
        
        int block_size = 1 << (rank - 1);
        
        /* Test first page */
        int r = query_ranks(allocated);
        assert_equal(r, rank, "First page should match allocated rank");
        
        /* Test a continuation page (if block has more than 1 page) */
        if (block_size > 1) {
            void *cont_page = (char *)allocated + PAGE_SIZE;
            r = query_ranks(cont_page);
            assert_equal(r, rank, "Continuation page should match allocated rank");
        }
        
        /* Test last page */
        if (block_size > 1) {
            void *last_page = (char *)allocated + ((block_size - 1) * PAGE_SIZE);
            r = query_ranks(last_page);
            assert_equal(r, rank, "Last page should match allocated rank");
        }
    }
    
    free(pool);
}

void test_continuation_pages_after_free() {
    printf("\n=== Test 4: Continuation Pages After Free (Coalescing) ===\n");
    
    void *pool = malloc(TEST_PAGES * PAGE_SIZE);
    if (!pool) {
        printf("ERROR: Failed to allocate test memory\n");
        return;
    }
    
    init_page(pool, TEST_PAGES);
    
    /* Allocate multiple small blocks */
    void *block1 = alloc_pages(5);
    void *block2 = alloc_pages(5);
    
    if ((long)block1 < 0 || (long)block2 < 0) {
        printf("ERROR: Failed to allocate test blocks\n");
        free(pool);
        return;
    }
    
    /* Check allocated blocks */
    int r1 = query_ranks(block1);
    int r2 = query_ranks(block2);
    assert_equal(r1, 5, "Block1 should have rank 5");
    assert_equal(r2, 5, "Block2 should have rank 5");
    
    /* Free the blocks - they should coalesce */
    return_pages(block1);
    return_pages(block2);
    
    /* After freeing, query_ranks should work on the freed pages */
    /* The coalesced block should have a higher rank */
    int freed_rank = query_ranks(block1);
    if (freed_rank >= 5) {
        printf("✓ After freeing, block has rank %d (>= 5, coalescing worked)\n", freed_rank);
        pass_count++;
    } else {
        printf("✗ After freeing, block has rank %d (expected >= 5)\n", freed_rank);
    }
    test_count++;
    
    /* Test continuation pages of the freed block */
    int block_size = 1 << (freed_rank - 1);
    if (block_size > 1) {
        void *cont_page = (char *)block1 + PAGE_SIZE;
        int cont_rank = query_ranks(cont_page);
        assert_equal(cont_rank, freed_rank, "Continuation page should match freed block rank");
    }
    
    free(pool);
}

void test_edge_cases() {
    printf("\n=== Test 5: Edge Cases ===\n");
    
    void *pool = malloc(TEST_PAGES * PAGE_SIZE);
    if (!pool) {
        printf("ERROR: Failed to allocate test memory\n");
        return;
    }
    
    init_page(pool, TEST_PAGES);
    
    /* Test invalid pointers */
    int rank = query_ranks(NULL);
    assert_equal(rank, -22, "NULL pointer should return -EINVAL (-22)");
    
    /* Test unaligned pointer */
    void *unaligned = (char *)pool + 100;
    rank = query_ranks(unaligned);
    assert_equal(rank, -22, "Unaligned pointer should return -EINVAL");
    
    /* Test out-of-bounds pointer */
    void *out_of_bounds = (char *)pool + (TEST_PAGES * PAGE_SIZE);
    rank = query_ranks(out_of_bounds);
    assert_equal(rank, -22, "Out-of-bounds pointer should return -EINVAL");
    
    free(pool);
}

void test_backward_search_logic() {
    printf("\n=== Test 6: Backward Search Logic Verification ===\n");
    
    void *pool = malloc(TEST_PAGES * PAGE_SIZE);
    if (!pool) {
        printf("ERROR: Failed to allocate test memory\n");
        return;
    }
    
    init_page(pool, TEST_PAGES);
    
    /* Allocate and free a specific rank to create a known free block */
    void *block = alloc_pages(8);
    if ((long)block < 0) {
        printf("ERROR: Failed to allocate test block\n");
        free(pool);
        return;
    }
    
    /* All pages in the allocated block should have rank 8 */
    int block_size = 1 << (8 - 1);  /* 2^7 = 128 pages */
    
    /* Test middle page */
    void *middle = (char *)block + ((block_size / 2) * PAGE_SIZE);
    int rank = query_ranks(middle);
    assert_equal(rank, 8, "Middle page of allocated block should have rank 8");
    
    /* Free the block */
    return_pages(block);
    
    /* After freeing, the continuation pages should find the block head */
    /* The block might have coalesced to a higher rank */
    rank = query_ranks(middle);
    if (rank >= 8 && rank <= MAXRANK) {
        printf("✓ After freeing, continuation page found block head (rank=%d)\n", rank);
        pass_count++;
    } else {
        printf("✗ After freeing, continuation page has invalid rank=%d\n", rank);
    }
    test_count++;
    
    free(pool);
}

int main() {
    printf("=================================================\n");
    printf("  query_ranks() Continuation Page Verification\n");
    printf("=================================================\n");
    
    test_continuation_pages_for_free_blocks();
    test_continuation_pages_after_split();
    test_continuation_pages_all_ranks();
    test_continuation_pages_after_free();
    test_edge_cases();
    test_backward_search_logic();
    
    printf("\n=================================================\n");
    printf("  Test Results: %d/%d tests passed\n", pass_count, test_count);
    printf("=================================================\n");
    
    if (pass_count == test_count) {
        printf("\n✓ ALL TESTS PASSED - query_ranks() continuation page handling verified!\n");
        return 0;
    } else {
        printf("\n✗ SOME TESTS FAILED - see details above\n");
        return 1;
    }
}
