# Project Roadmap - Buddy Algorithm Implementation

## Project Goal
Implement a high-quality Buddy memory allocation algorithm in C that passes ACMOJ evaluation (Problem ID: 1848).

## Milestone Progress

### M0: Initial Planning ✓ COMPLETE
**Status**: Complete
**Cycles Used**: 2 (Athena + 2 workers)

**Completed**:
- ✓ Created .gitignore with required entries
- ✓ Initial git commit with boilerplate  
- ✓ Comprehensive requirements analysis (Marcus)
- ✓ Independent code review (Elena)
- ✓ Roadmap established

**Key Findings**:
- CRITICAL: Makefile produces `test` not `code` (OJ will fail)
- CRITICAL: Build has compilation errors (main.c lines 55-56)
- All 5 functions are empty stubs - full implementation needed
- Estimated 14-17 cycles total for complete implementation

---

### M1: Fix Build System ✓ COMPLETE
**Status**: Complete
**Cycles Budget**: 1
**Cycles Used**: 1 (Ares team)

**Objective**: Fix critical build blockers

**Completed**:
1. ✓ Fixed Makefile: changed output from `test` to `code`
2. ✓ Fixed main.c lines 55-56: removed PTR_ERR wrapper
3. ✓ Verified clean build with no errors/warnings
4. ✓ Committed and pushed (commit fad711c)

**Success Criteria**: All met
- ✓ `make` produces executable named `code`
- ✓ Build completes with zero errors
- ✓ Build completes with zero warnings
- ✓ Changes pushed to GitHub

**Lesson Learned**: With N-cycle budget, need N-1 for workers + 1 for manager to verify, OR schedule verification in same cycle with workers.

---

### M2.1: Data Structures and Basic Init ← NEXT
**Status**: Ready to start
**Cycles Budget**: 3
**Assigned to**: Ares team

**Objective**: Design and implement core data structures + init_page()

**Tasks**:
1. Design data structures:
   - Free list structure (doubly-linked for O(1) removal)
   - Metadata array to track page ranks
   - Global allocator state
2. Implement `init_page()`:
   - Initialize free lists for ranks 1-16
   - Organize initial memory as max-rank blocks
   - Store base pointer and total pages
3. Implement `query_page_counts()`:
   - Count free blocks at specified rank
   - Validate rank input (1-16)

**Success Criteria**:
- Data structures documented and clear
- `init_page()` creates correct initial state
- Phase 1 test passes (initialization)
- Phase 3 can verify all ranks empty after allocation
- Code compiles without warnings
- Changes committed and pushed

**Why This Scope**:
- Foundation must be solid before building allocation logic
- query_page_counts is simple and validates free list tracking
- 3 cycles allows careful design + implementation + testing

---

### M2.2: Block Allocation with Splitting (Planned)
**Status**: Planned  
**Cycles Budget**: 4

**Objective**: Implement alloc_pages() with buddy splitting algorithm

**Tasks**:
1. Implement `alloc_pages()`:
   - Validate rank (1-16, return -EINVAL if invalid)
   - Search free list for requested rank
   - If not available, split larger blocks recursively
   - Mark allocated blocks and update metadata
   - Return -ENOSPC when out of memory
2. Update metadata tracking for ALL pages in block
3. Ensure sequential allocation order (left-to-right)

**Success Criteria**:
- Phase 2 passes: allocate all 32,768 pages sequentially
- Out-of-memory error handling works
- Invalid rank handling works
- All allocated pages tracked correctly
- Code compiles without warnings

---

### M2.3: Block Deallocation and Queries (Planned)
**Status**: Planned
**Cycles Budget**: 3

**Objective**: Implement return_pages() (basic) + query_ranks()

**Tasks**:
1. Implement `return_pages()` WITHOUT coalescing:
   - Validate pointer (NULL check, bounds check)
   - Return -EINVAL for invalid pointers
   - Add block back to free list at its rank
   - Mark as deallocated
2. Implement `query_ranks()`:
   - Look up rank from metadata
   - Handle allocated vs free blocks
   - Validate pointer

**Success Criteria**:
- Phase 4 passes: return all pages with validation
- Phase 6-7 pass: query ranks correctly
- Invalid pointer handling works
- Code compiles without warnings

**Note**: This milestone does NOT include coalescing - that's M2.4

---

### M2.4: Buddy Coalescing (Planned)
**Status**: Planned
**Cycles Budget**: 6

**Objective**: Implement buddy coalescing in return_pages()

**Tasks**:
1. Implement buddy address calculation: `buddy_idx = idx ^ (1 << (rank-1))`
2. Check if buddy is free AND same rank
3. If yes: remove buddy from free list, merge into rank+1, recurse
4. If no: add block to free list at current rank
5. Handle edge cases:
   - Buddy at boundary (out of total pages range)
   - Termination at MAXRANK
   - Proper metadata updates during coalescing

**Success Criteria**:
- Phase 5 passes: all pages coalesce back to MAXRANK
- Phase 8A passes: progressive coalescing pattern
- Phase 8B passes: alternating free pattern
- All 131,246 test assertions pass
- Code compiles without warnings

**Why 6 Cycles**:
- Coalescing is the hardest part (most edge cases)
- Need multiple iterations to debug complex patterns
- Phase 8A and 8B are integration tests with tricky scenarios

---

### M3: Final Code Review and OJ Readiness (Planned)
**Status**: Planned
**Cycles Budget**: 2

**Objective**: Final verification and polish before marking complete

**Tasks**:
- Independent verification: all 8 test phases pass
- Code review for edge cases and error handling
- Performance check (time/space complexity)
- Final build verification
- Documentation check

**Success Criteria**:
- All 131,246 test assertions pass
- No compiler warnings
- Build produces `code` executable correctly
- Code ready for external OJ evaluation
- All requirements from README.md verified

---

## Constraints & Notes

### Technical Constraints
- Must use C language
- Minimum allocation unit: 4K page
- Valid ranks: 1-16 (page size = 4K × 2^(rank-1))
- Time limit: 10000ms per test case
- Memory limit: 244 MiB per test case

### Submission Requirements
- Repository URL: https://github.com/WenhanLyu/BotCompanyBenchmarkProblem020.git
- Build process: cmake (optional) → make → produces `code` executable
- Max submission attempts: 3 (counted)
- Git submission type

### Risk Factors
- Buddy algorithm coalescing logic is tricky - needs careful testing
- Off-by-one errors in address calculations
- Memory fragmentation edge cases
- Limited submission budget requires high confidence before submission

## Lessons Learned

### Cycle 1-2 (Athena Planning Phase)
- **Worker reports critical**: Marcus and Elena caught build blockers immediately
- **Independent verification works**: Both workers working blind found same critical issues
- **Small milestones better**: Breaking M1 into build-fix-only keeps focus tight
- **Build must work first**: Cannot test anything until compilation succeeds

### Cycle 3-7 (M1 Implementation + Research)
- **M1 technically complete**: Leo fixed build in 1 cycle (commit fad711c)
- **Procedural issue**: With 1/1 budget, no cycle left for Ares to formally complete
- **Lesson**: Need buffer cycle for manager OR schedule verification with workers
- **Research investment pays off**: 4 workers did deep research (Sophia, Oliver, Elena, Marcus)
  - Comprehensive algorithm understanding (Sophia: 9000+ words)
  - Complete test analysis (Oliver: all 8 phases documented)
  - Independent code assessment (Elena: current state + gaps)
  - Requirements validation (Marcus: edge cases, constraints)
- **Breaking M2 into sub-milestones**: Original M2 (8 cycles) too large
  - M2.1: Data structures + init (3 cycles)
  - M2.2: Allocation + splitting (4 cycles)
  - M2.3: Deallocation + queries (3 cycles)  
  - M2.4: Coalescing (6 cycles - the hard part)
  - Total: 16 cycles, more realistic given complexity
- **Coalescing is the critical risk**: Phases 5, 8A, 8B all depend on correct buddy merging

---

**Last Updated**: Cycle 8 (Athena - after M1 deadline miss)
