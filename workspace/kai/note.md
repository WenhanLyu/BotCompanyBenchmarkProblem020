# Kai's Work Notes - Cycle 1

## Assignment
Working on issues #4 and #5 - Complete buddy allocator implementation

## Completed Work

### Implementation Summary
Implemented all 5 functions of the buddy allocator in one cycle:

1. **init_page()**: 
   - Initializes free lists for ranks 1-16
   - Creates maximum-rank blocks from available memory
   - Handles remaining pages as smaller blocks
   - Uses external static array for metadata (256KB max)

2. **alloc_pages()**:
   - Validates rank (1-16)
   - Searches for free block at requested or higher rank
   - Splits larger blocks recursively (buddy splitting)
   - Returns -EINVAL for invalid rank, -ENOSPC for out of memory

3. **return_pages()**:
   - Validates pointer and checks it's allocated
   - Implements buddy coalescing algorithm
   - Merges freed blocks with free buddies recursively
   - Returns -EINVAL for invalid pointers

4. **query_ranks()**:
   - Returns rank of page from metadata
   - Validates pointer
   - Returns -EINVAL for invalid pointer

5. **query_page_counts()**:
   - Counts free blocks at specified rank
   - Validates rank input
   - Returns -EINVAL for invalid rank

### Data Structures
- **Free lists**: Array of 17 pointers (indices 0-16, using 1-16)
- **Free nodes**: Doubly-linked list embedded in free pages (prev/next pointers)
- **Metadata**: Static array of unsigned char, one byte per page
  - High bit (0x80) = free flag
  - Low 7 bits = rank (1-16)
- **Global state**: base_ptr, total_pages

### Key Algorithms
- **Buddy calculation**: `buddy_idx = idx ^ (1 << (rank-1))`
- **Splitting**: Recursively split larger blocks, add right half to free list
- **Coalescing**: Check buddy free + same rank, merge recursively up to MAXRANK

### Test Results
All 8 phases pass (131,246+ assertions):
- Phase 1: Initialization ✓
- Phase 2: Allocate all 32,768 pages ✓
- Phase 3: Query counts when empty ✓
- Phase 4: Return pages with validation ✓
- Phase 5: Query counts when full (coalesced to MAXRANK) ✓
- Phase 6: Query ranks (large blocks) ✓
- Phase 7: Query ranks (small blocks) ✓
- Phase 8A: Progressive coalescing pattern ✓
- Phase 8B: Alternating free pattern ✓

### Technical Decisions
1. **Metadata storage**: Used static array instead of stealing pages from managed region
   - Avoids reducing available pages
   - Simpler implementation
   - Max 256K pages supported (1GB / 4KB)

2. **Free list structure**: Doubly-linked for O(1) removal
   - Needed for coalescing when buddy must be removed from middle of list

3. **Metadata encoding**: Single byte per page
   - Bit 7: free flag (0x80)
   - Bits 0-6: rank (1-16)
   - Allows quick checks and updates

## Commit
- Commit: 76ebc9d
- Message: "[Kai] Implement complete buddy allocator - all 5 functions with coalescing"
- Pushed to master

## Next Steps
Code is ready for OJ evaluation. All requirements met:
- ✓ All 5 functions implemented
- ✓ All tests pass locally
- ✓ Clean build with no warnings
- ✓ Produces `code` executable
- ✓ Changes committed and pushed
