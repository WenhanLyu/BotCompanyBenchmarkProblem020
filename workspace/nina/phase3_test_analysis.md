# Phase 3 Test Analysis

## Test Code (main.c lines 43-50)

```c
{
    printf("Phase 3: query_page_counts test #1(empty)\n");
    tCnt = 0;
    for (pgIdx = 1; pgIdx <= MAXRANK; ++pgIdx) {
        dotOk(query_page_counts(pgIdx) == 0);
    }
    dotDone();
}
```

## Test Context

### Pre-conditions
- Phase 1 completed: `init_page()` initialized 32,768 pages (128 MB / 4K)
- Phase 2 completed: All 32,768 pages allocated using `alloc_pages(1)` in a loop
- Memory state: **All pages allocated, no free blocks**

### What Phase 3 Tests
For each rank from 1 to 16, verify that `query_page_counts(rank)` returns 0.

This tests that:
1. The function correctly counts free blocks
2. After allocating all available memory, all free lists are empty
3. The function works for all valid rank values (1-16)

### Expected Behavior
Each call should return 0 because:
- All 32,768 individual pages (rank 1) have been allocated
- There are no free blocks at any rank
- All free lists should be empty

## Test Execution

### Output
```
Phase 3: query_page_counts test #1(empty)
................
Total: 16 Ok
```

### Breakdown
Each dot represents one successful assertion:
- `.` = dotOk(query_page_counts(1) == 0)  ✅
- `.` = dotOk(query_page_counts(2) == 0)  ✅
- `.` = dotOk(query_page_counts(3) == 0)  ✅
- `.` = dotOk(query_page_counts(4) == 0)  ✅
- `.` = dotOk(query_page_counts(5) == 0)  ✅
- `.` = dotOk(query_page_counts(6) == 0)  ✅
- `.` = dotOk(query_page_counts(7) == 0)  ✅
- `.` = dotOk(query_page_counts(8) == 0)  ✅
- `.` = dotOk(query_page_counts(9) == 0)  ✅
- `.` = dotOk(query_page_counts(10) == 0) ✅
- `.` = dotOk(query_page_counts(11) == 0) ✅
- `.` = dotOk(query_page_counts(12) == 0) ✅
- `.` = dotOk(query_page_counts(13) == 0) ✅
- `.` = dotOk(query_page_counts(14) == 0) ✅
- `.` = dotOk(query_page_counts(15) == 0) ✅
- `.` = dotOk(query_page_counts(16) == 0) ✅

### Result
✅ **All 16 checks PASS**

## Implementation Verification

The `query_page_counts()` function correctly:

1. **Accepts valid ranks** (1-16)
2. **Iterates through free list** for each rank:
   ```c
   free_node_t *node = free_lists[rank];
   while (node != NULL) {
       count++;
       node = node->next;
   }
   ```
3. **Returns 0** when free list is empty (node == NULL from start)

## Why This Test Matters

Phase 3 is critical because it:
- Validates that `query_page_counts()` works correctly
- Confirms that allocation tracking is accurate (all free lists emptied)
- Tests the function across all valid rank values
- Serves as a baseline for later tests (Phase 5 tests the opposite - all free)

## Related Tests

- **Phase 5**: Tests `query_page_counts()` after deallocation (expects non-zero for rank 16)
- **Phase 8A**: Tests during progressive deallocation/coalescing pattern
- **Phase 8B**: Tests during alternating free/alloc pattern

All related tests also pass, confirming `query_page_counts()` works correctly in all scenarios.
