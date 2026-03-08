# Code Examples - Performance Optimization

## Quick Reference: Before and After

### Global State Changes

**ADD THIS** after line 20:
```c
/* Cached free block counts for O(1) query_page_counts */
static int free_counts[MAXRANK + 1];
```

### Function: add_to_free_list() - Add counter

**CURRENT** (lines 38-46):
```c
static void add_to_free_list(int rank, void *p) {
    free_node_t *node = (free_node_t *)p;
    node->prev = NULL;
    node->next = free_lists[rank];
    if (free_lists[rank] != NULL) {
        free_lists[rank]->prev = node;
    }
    free_lists[rank] = node;
}
```

**OPTIMIZED**:
```c
static void add_to_free_list(int rank, void *p) {
    free_node_t *node = (free_node_t *)p;
    node->prev = NULL;
    node->next = free_lists[rank];
    if (free_lists[rank] != NULL) {
        free_lists[rank]->prev = node;
    }
    free_lists[rank] = node;
    free_counts[rank]++;  // ADD THIS LINE
}
```

### Function: remove_from_free_list() - Add counter

**CURRENT** (lines 49-62):
```c
static void remove_from_free_list(int rank, void *p) {
    free_node_t *node = (free_node_t *)p;
    
    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        /* This is the head of the list */
        free_lists[rank] = node->next;
    }
    
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }
}
```

**OPTIMIZED**:
```c
static void remove_from_free_list(int rank, void *p) {
    free_node_t *node = (free_node_t *)p;
    
    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        /* This is the head of the list */
        free_lists[rank] = node->next;
    }
    
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }
    
    free_counts[rank]--;  // ADD THIS LINE
}
```

### Function: init_page() - Remove metadata loops

**CURRENT** (lines 86-130):
```c
int init_page(void *p, int pgcount) {
    base_ptr = p;
    total_pages = pgcount;
    metadata = metadata_storage;
    
    /* Initialize free lists */
    for (int i = 0; i <= MAXRANK; i++) {
        free_lists[i] = NULL;
    }
    
    /* Initialize metadata to 0 */
    for (int i = 0; i < pgcount; i++) {
        metadata[i] = 0;
    }
    
    /* Create maximum-rank blocks from all available memory */
    int max_block_size = 1 << (MAXRANK - 1);
    int remaining_pages = pgcount;
    int start_page = 0;
    
    while (remaining_pages >= max_block_size) {
        void *block = get_page_ptr(start_page);
        add_to_free_list(MAXRANK, block);
        
        /* Mark all pages in this block */           // <-- REMOVE THIS LOOP
        for (int i = 0; i < max_block_size; i++) {
            metadata[start_page + i] = 0x80 | MAXRANK;
        }
        
        start_page += max_block_size;
        remaining_pages -= max_block_size;
    }
    
    /* Handle remaining pages by creating smaller blocks */
    for (int rank = MAXRANK - 1; rank >= 1; rank--) {
        int block_size = 1 << (rank - 1);
        if (remaining_pages >= block_size) {
            void *block = get_page_ptr(start_page);
            add_to_free_list(rank, block);
            
            /* Mark all pages in this block */       // <-- REMOVE THIS LOOP
            for (int i = 0; i < block_size; i++) {
                metadata[start_page + i] = 0x80 | rank;
            }
            
            start_page += block_size;
            remaining_pages -= block_size;
        }
    }
    
    return OK;
}
```

**OPTIMIZED**:
```c
int init_page(void *p, int pgcount) {
    base_ptr = p;
    total_pages = pgcount;
    metadata = metadata_storage;
    
    /* Initialize free lists AND counters */
    for (int i = 0; i <= MAXRANK; i++) {
        free_lists[i] = NULL;
        free_counts[i] = 0;  // ADD THIS LINE
    }
    
    /* Initialize metadata to 0 */
    for (int i = 0; i < pgcount; i++) {
        metadata[i] = 0;
    }
    
    /* Create maximum-rank blocks from all available memory */
    int max_block_size = 1 << (MAXRANK - 1);
    int remaining_pages = pgcount;
    int start_page = 0;
    
    while (remaining_pages >= max_block_size) {
        void *block = get_page_ptr(start_page);
        add_to_free_list(MAXRANK, block);
        
        /* Mark ONLY the block head */              // <-- CHANGED
        metadata[start_page] = 0x80 | MAXRANK;     // <-- ONE LINE ONLY
        
        start_page += max_block_size;
        remaining_pages -= max_block_size;
    }
    
    /* Handle remaining pages by creating smaller blocks */
    for (int rank = MAXRANK - 1; rank >= 1; rank--) {
        int block_size = 1 << (rank - 1);
        if (remaining_pages >= block_size) {
            void *block = get_page_ptr(start_page);
            add_to_free_list(rank, block);
            
            /* Mark ONLY the block head */          // <-- CHANGED
            metadata[start_page] = 0x80 | rank;    // <-- ONE LINE ONLY
            
            start_page += block_size;
            remaining_pages -= block_size;
        }
    }
    
    return OK;
}
```

### Function: alloc_pages() - Optimize split metadata

**CURRENT** (lines 156-171):
```c
/* Split the block down to requested rank */
while (current_rank > rank) {
    current_rank--;
    int block_size = 1 << (current_rank - 1);
    
    /* Split: keep left half, add right half to free list */
    int left_idx = get_page_index(block);
    int right_idx = left_idx + block_size;
    void *right_block = get_page_ptr(right_idx);
    
    /* Mark right block as free at current_rank */     // <-- REMOVE LOOP
    for (int i = 0; i < block_size; i++) {
        metadata[right_idx + i] = 0x80 | current_rank;
    }
    add_to_free_list(current_rank, right_block);
}
```

**OPTIMIZED**:
```c
/* Split the block down to requested rank */
while (current_rank > rank) {
    current_rank--;
    int block_size = 1 << (current_rank - 1);
    
    /* Split: keep left half, add right half to free list */
    int left_idx = get_page_index(block);
    int right_idx = left_idx + block_size;
    void *right_block = get_page_ptr(right_idx);
    
    /* Mark right block head as free */                // <-- CHANGED
    metadata[right_idx] = 0x80 | current_rank;        // <-- ONE LINE ONLY
    add_to_free_list(current_rank, right_block);
}
```

**KEEP THE ALLOCATED BLOCK LOOP** (lines 173-178) - unchanged:
```c
/* Mark allocated block */
int block_idx = get_page_index(block);
int block_size = 1 << (rank - 1);
for (int i = 0; i < block_size; i++) {
    metadata[block_idx + i] = rank;  /* Allocated (no 0x80 bit) */
}
```

**WHY**: query_ranks() needs to work on ANY page in allocated block. Keeping this ensures O(1) queries.

### Function: return_pages() - Remove metadata loop

**CURRENT** (lines 228-234):
```c
/* Add coalesced block to free list */
void *block = get_page_ptr(idx);
int block_size = 1 << (rank - 1);
for (int i = 0; i < block_size; i++) {        // <-- REMOVE LOOP
    metadata[idx + i] = 0x80 | rank;
}
add_to_free_list(rank, block);
```

**OPTIMIZED**:
```c
/* Add coalesced block to free list */
void *block = get_page_ptr(idx);
metadata[idx] = 0x80 | rank;                  // <-- ONE LINE ONLY
add_to_free_list(rank, block);
```

### Function: query_ranks() - Handle continuation pages

**CURRENT** (lines 240-247):
```c
int query_ranks(void *p) {
    int idx = get_page_index(p);
    if (idx < 0) {
        return -EINVAL;
    }
    
    return metadata[idx] & 0x7F;
}
```

**OPTIMIZED**:
```c
int query_ranks(void *p) {
    int idx = get_page_index(p);
    if (idx < 0) {
        return -EINVAL;
    }
    
    unsigned char meta = metadata[idx];
    int rank = meta & 0x7F;
    
    /* If rank is non-zero, return it directly */
    if (rank != 0) {
        return rank;
    }
    
    /* rank=0 means this is a continuation page of a free block */
    /* Search backward for the block head */
    for (int r = MAXRANK; r >= 1; r--) {
        int block_size = 1 << (r - 1);
        int head_idx = (idx / block_size) * block_size;
        
        /* Check if head_idx is before our page AND is marked as free rank r */
        if (head_idx < idx && (metadata[head_idx] & 0x7F) == r && (metadata[head_idx] & 0x80)) {
            return r;
        }
    }
    
    /* If we get here, page is uninitialized or invalid */
    return -EINVAL;
}
```

**NOTE**: This only applies to FREE blocks (continuation pages). Allocated blocks still have rank in every page.

### Function: query_page_counts() - Use cached count

**CURRENT** (lines 250-265):
```c
int query_page_counts(int rank) {
    /* Validate rank */
    if (rank < 1 || rank > MAXRANK) {
        return -EINVAL;
    }
    
    /* Count nodes in free list */        // <-- REMOVE THIS
    int count = 0;
    free_node_t *node = free_lists[rank];
    while (node != NULL) {
        count++;
        node = node->next;
    }
    
    return count;
}
```

**OPTIMIZED**:
```c
int query_page_counts(int rank) {
    /* Validate rank */
    if (rank < 1 || rank > MAXRANK) {
        return -EINVAL;
    }
    
    /* Return cached count */             // <-- CHANGED
    return free_counts[rank];             // <-- ONE LINE ONLY
}
```

## Summary of Changes

### Lines to ADD:
1. Line ~21: `static int free_counts[MAXRANK + 1];`
2. Line ~46: `free_counts[rank]++;` in add_to_free_list()
3. Line ~62: `free_counts[rank]--;` in remove_from_free_list()
4. Line ~89: `free_counts[i] = 0;` in init_page()

### Lines to CHANGE:
1. Lines 106-108: Replace loop with `metadata[start_page] = 0x80 | MAXRANK;`
2. Lines 122-124: Replace loop with `metadata[start_page] = 0x80 | rank;`
3. Lines 167-169: Replace loop with `metadata[right_idx] = 0x80 | current_rank;`
4. Lines 231-233: Replace loop with `metadata[idx] = 0x80 | rank;`
5. Lines 240-247: Expand to handle continuation pages
6. Lines 257-262: Replace loop with `return free_counts[rank];`

### Lines to KEEP UNCHANGED:
1. Lines 176-178: Allocated block metadata loop (needed for query_ranks)

## Testing Verification

After changes, verify:
```bash
make clean
make
./code < test_input.txt
```

All 98K+ assertions must still pass.

## Performance Test

Create stress_test.c:
```c
#include <stdio.h>
#include <time.h>
#include "buddy.h"

#define TEST_PAGES 65536

int main() {
    static char memory[TEST_PAGES * 4096];
    
    clock_t start = clock();
    
    // Initialize
    init_page(memory, TEST_PAGES);
    
    // Stress test: many large allocations
    for (int i = 0; i < 1000; i++) {
        void *p = alloc_pages(12);
        if ((long)p > 0) {
            return_pages(p);
        }
    }
    
    clock_t end = clock();
    double ms = (double)(end - start) / CLOCKS_PER_SEC * 1000;
    
    printf("Stress test completed in %.2f ms\n", ms);
    printf("Status: %s\n", ms < 10000 ? "PASS" : "FAIL");
    
    return ms < 10000 ? 0 : 1;
}
```

Expected result: < 100ms (well under 10 second limit)

## Diff Summary

Total changes: ~15 lines modified, ~5 lines added
Complexity: LOW-MEDIUM
Risk: MEDIUM (careful testing required)
Benefit: HIGH (eliminates O(2^rank) bottleneck)
