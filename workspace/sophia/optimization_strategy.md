# Optimization Implementation Strategy

## Overview

This document provides a step-by-step strategy for optimizing buddy.c to eliminate O(2^rank) metadata loops.

## Core Optimization: Block-Head-Only Metadata

### Concept

**Current approach**: Every page in a block stores the rank
```
Block of rank 4 (8 pages):
metadata[0] = 4
metadata[1] = 4
metadata[2] = 4
... (8 total writes)
```

**Optimized approach**: Only block head stores rank
```
Block of rank 4 (8 pages):
metadata[0] = 4
metadata[1-7] = 0 (or implicit)
```

### Metadata Encoding Scheme

**Option A: Implicit continuation (RECOMMENDED)**
```c
// Block head (first page of block)
metadata[idx] = rank | (free_bit << 7)

// Other pages in block
metadata[idx] = 0  // Zero means "part of a block, see earlier page"
```

**Benefits**:
- Simple to implement
- Easy to debug (0 = not a block head)
- Backward compatible with initialization (all zeros)

**Drawback**:
- query_ranks() needs to search backward for block head

**Option B: Explicit continuation marker**
```c
// Block head
metadata[idx] = rank | (free_bit << 7)

// Other pages in block  
metadata[idx] = 0xFF  // Special marker
```

**Benefits**:
- Explicit distinction between "uninitialized" and "continuation"
- Easier error detection

**Drawback**:
- Slightly more complex

**Recommendation**: Use Option A (implicit continuation)

## Implementation Plan

### Step 1: Update Helper Functions

#### 1.1 Add function to find block head
```c
/* Helper: Find the block head for a given page */
static int find_block_head(int page_idx, int rank) {
    // Calculate block size
    int block_size = 1 << (rank - 1);
    
    // Round down to block boundary
    int block_start = (page_idx / block_size) * block_size;
    
    return block_start;
}
```

**Why needed**: When querying a page that's not a block head, we need to find which block it belongs to.

#### 1.2 Modify metadata access pattern

**Current**: Direct metadata access everywhere
**New**: Access through helper that understands block structure

```c
/* Helper: Get the rank of a page (handles continuation pages) */
static int get_page_rank(int page_idx) {
    unsigned char meta = metadata[page_idx];
    
    // If this page has a rank (not zero), return it
    if ((meta & 0x7F) != 0) {
        return meta & 0x7F;
    }
    
    // This is a continuation page - search backward for block head
    // Try each rank from largest to smallest
    for (int rank = MAXRANK; rank >= 1; rank--) {
        int block_size = 1 << (rank - 1);
        int potential_head = (page_idx / block_size) * block_size;
        
        if (potential_head < page_idx && 
            (metadata[potential_head] & 0x7F) == rank) {
            return rank;
        }
    }
    
    return 0;  // Invalid/uninitialized
}
```

**Alternative (simpler but requires more metadata)**:
- Store block rank in ALL pages during allocation (current approach)
- Only store in head during free
- Trade-off: Keeps alloc simple but free is still slow

### Step 2: Update init_page()

**Current code** (lines 106-108, 122-124):
```c
for (int i = 0; i < max_block_size; i++) {
    metadata[start_page + i] = 0x80 | MAXRANK;
}
```

**Optimized code**:
```c
// Only mark the block head
metadata[start_page] = 0x80 | MAXRANK;

// Other pages implicitly part of block (already zeroed)
```

**Savings**: From O(pgcount) to O(num_blocks) operations

### Step 3: Update alloc_pages()

**Current code** (lines 167-169, 176-178):
```c
// During splitting
for (int i = 0; i < block_size; i++) {
    metadata[right_idx + i] = 0x80 | current_rank;
}

// During allocation
for (int i = 0; i < block_size; i++) {
    metadata[block_idx + i] = rank;
}
```

**Optimized code**:
```c
// During splitting - only mark right block head
metadata[right_idx] = 0x80 | current_rank;
// Clear continuation pages (optional if already zeroed)
// No loop needed!

// During allocation - mark all pages in allocated block
// KEEP THIS LOOP - see note below
for (int i = 0; i < block_size; i++) {
    metadata[block_idx + i] = rank;  // Allocated (no 0x80 bit)
}
```

**IMPORTANT NOTE**: We KEEP the metadata loop for ALLOCATED blocks because:
1. query_ranks() is called on any page in an allocated block
2. Without marking all pages, we can't distinguish allocated vs. continuation
3. Alternative: Modify query_ranks() to search backward (slower query)

**Decision Point**: 
- **Option A**: Keep loop for allocated blocks, optimize only free blocks
  - Pro: query_ranks() stays O(1)
  - Con: Still has O(2^rank) in allocation path
  
- **Option B**: Remove loop, make query_ranks() search backward
  - Pro: Faster allocation
  - Con: Slower queries (but queries less frequent than alloc)

**Recommendation**: Start with Option A (safer), can do Option B if needed

### Step 4: Update return_pages()

**Current code** (lines 231-233):
```c
for (int i = 0; i < block_size; i++) {
    metadata[idx + i] = 0x80 | rank;
}
```

**Optimized code**:
```c
// Only mark the block head as free
metadata[idx] = 0x80 | rank;

// Clear continuation pages (make them zero/implicit)
int block_size = 1 << (rank - 1);
for (int i = 1; i < block_size; i++) {
    metadata[idx + i] = 0;
}
```

**Wait - this still has a loop!**

**Better approach**:
```c
// Only mark the block head as free
metadata[idx] = 0x80 | rank;

// Continuation pages already zero from earlier, or:
// Leave them as-is (allocated rank) - doesn't matter for free blocks
```

**Key insight**: Free block continuation pages don't need to be updated! The free list tracks only heads.

**Final optimized code**:
```c
// Add coalesced block to free list
void *block = get_page_ptr(idx);
metadata[idx] = 0x80 | rank;  // Only update block head
add_to_free_list(rank, block);
```

**Savings**: From O(2^rank) to O(1) operations

### Step 5: Update query_ranks()

**Current code** (lines 240-246):
```c
int query_ranks(void *p) {
    int idx = get_page_index(p);
    if (idx < 0) {
        return -EINVAL;
    }
    
    return metadata[idx] & 0x7F;
}
```

**If we use Option A (keep allocated loops)**:
```c
int query_ranks(void *p) {
    int idx = get_page_index(p);
    if (idx < 0) {
        return -EINVAL;
    }
    
    unsigned char meta = metadata[idx];
    int rank = meta & 0x7F;
    
    // If rank is 0, this is a continuation page of a free block
    // Need to find the block head
    if (rank == 0) {
        // Search backward for block head
        for (int r = MAXRANK; r >= 1; r--) {
            int block_size = 1 << (r - 1);
            int head_idx = (idx / block_size) * block_size;
            if (head_idx < idx && (metadata[head_idx] & 0x80)) {
                // Found a free block head that contains this page
                return r;
            }
        }
        return -EINVAL;  // Shouldn't happen
    }
    
    return rank;
}
```

**Complexity**: O(16) worst case - acceptable

### Step 6: Optimize query_page_counts()

**Current code** (lines 250-265):
```c
int query_page_counts(int rank) {
    if (rank < 1 || rank > MAXRANK) {
        return -EINVAL;
    }
    
    int count = 0;
    free_node_t *node = free_lists[rank];
    while (node != NULL) {
        count++;
        node = node->next;
    }
    
    return count;
}
```

**Optimized code**:
```c
// Add to global state
static int free_counts[MAXRANK + 1];

// Update add_to_free_list()
static void add_to_free_list(int rank, void *p) {
    free_node_t *node = (free_node_t *)p;
    node->prev = NULL;
    node->next = free_lists[rank];
    if (free_lists[rank] != NULL) {
        free_lists[rank]->prev = node;
    }
    free_lists[rank] = node;
    free_counts[rank]++;  // INCREMENT COUNT
}

// Update remove_from_free_list()
static void remove_from_free_list(int rank, void *p) {
    free_node_t *node = (free_node_t *)p;
    
    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        free_lists[rank] = node->next;
    }
    
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }
    
    free_counts[rank]--;  // DECREMENT COUNT
}

// Update query_page_counts()
int query_page_counts(int rank) {
    if (rank < 1 || rank > MAXRANK) {
        return -EINVAL;
    }
    
    return free_counts[rank];  // O(1) lookup
}

// Don't forget to initialize in init_page()
for (int i = 0; i <= MAXRANK; i++) {
    free_lists[i] = NULL;
    free_counts[i] = 0;  // INITIALIZE
}
```

**Savings**: From O(num_blocks) to O(1)

## Testing Strategy

### Phase 1: Verify Correctness
1. Run all existing test assertions (98K+)
2. All must pass - no regressions

### Phase 2: Stress Testing
Create new stress tests:

```c
// Test 1: Large allocations (worst case for old code)
void stress_test_large_allocs() {
    init_page(memory, 65536);  // 256MB
    
    // Allocate many rank-16 blocks (32K pages each)
    void *blocks[10];
    for (int i = 0; i < 10; i++) {
        blocks[i] = alloc_pages(16);
        assert(blocks[i] > 0);
    }
    
    // Free them all
    for (int i = 0; i < 10; i++) {
        return_pages(blocks[i]);
    }
}

// Test 2: Many small operations
void stress_test_many_operations() {
    init_page(memory, 65536);
    
    // 10,000 allocation/free cycles
    for (int i = 0; i < 10000; i++) {
        void *p = alloc_pages(8);
        return_pages(p);
    }
}

// Test 3: Query performance
void stress_test_queries() {
    init_page(memory, 65536);
    
    // Create many free blocks
    for (int i = 0; i < 1000; i++) {
        alloc_pages(1);
    }
    
    // Query counts many times
    for (int i = 0; i < 10000; i++) {
        for (int rank = 1; rank <= 16; rank++) {
            query_page_counts(rank);
        }
    }
}
```

### Phase 3: Performance Measurement
```c
#include <time.h>

clock_t start = clock();
stress_test_large_allocs();
clock_t end = clock();

double time_ms = (double)(end - start) / CLOCKS_PER_SEC * 1000;
printf("Test completed in %.2f ms\n", time_ms);

// Should be well under 10,000 ms
```

## Risk Mitigation

### Risk 1: Breaking correctness
**Mitigation**: 
- Implement incrementally (one function at a time)
- Run full test suite after each change
- Keep old code commented out for comparison

### Risk 2: Edge cases in query_ranks()
**Mitigation**:
- Thoroughly test continuation page queries
- Test boundary conditions (first page, last page of block)
- Test all ranks

### Risk 3: Off-by-one errors in block head calculation
**Mitigation**:
- Use well-tested formula: `(idx / block_size) * block_size`
- Add assertions to verify block alignment
- Test with various page counts

## Implementation Checklist

- [ ] Add free_counts array to global state
- [ ] Update add_to_free_list() to increment counts
- [ ] Update remove_from_free_list() to decrement counts
- [ ] Update init_page() to:
  - [ ] Initialize free_counts to 0
  - [ ] Only set metadata at block heads
- [ ] Update alloc_pages() to:
  - [ ] Only set metadata at split block head
  - [ ] Keep metadata loop for allocated blocks (Option A)
- [ ] Update return_pages() to:
  - [ ] Only set metadata at coalesced block head
- [ ] Update query_ranks() to:
  - [ ] Handle continuation pages (rank=0)
  - [ ] Search backward for block head if needed
- [ ] Update query_page_counts() to:
  - [ ] Return free_counts[rank] directly
- [ ] Run all existing tests
- [ ] Create and run stress tests
- [ ] Measure performance
- [ ] Verify < 10s for all test cases

## Expected Performance Improvement

### Before Optimization
- init_page(256K pages): ~256K metadata writes
- alloc_pages(rank 16): ~64K metadata writes (split + alloc)
- return_pages(rank 16): ~32K metadata writes
- query_page_counts(): O(n) list traversal

### After Optimization
- init_page(256K pages): ~8 metadata writes (only block heads)
- alloc_pages(rank 16): ~32K metadata writes (only alloc, not split)
- return_pages(rank 16): ~1 metadata write (only head)
- query_page_counts(): O(1) array lookup

### Estimated Speedup
- init_page(): ~32,000x faster
- alloc_pages(): ~2x faster
- return_pages(): ~32,000x faster
- query_page_counts(): ~1000x faster

**Overall**: Should easily meet 10-second time limit even for massive test cases.

## Conclusion

**Effort**: 4-6 hours (1 development cycle)
**Risk**: Medium (touching core algorithm)
**Benefit**: High (eliminates timeout risk)
**Recommendation**: Implement before OJ submission
