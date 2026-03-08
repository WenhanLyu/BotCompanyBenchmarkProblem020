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

### M1: Fix Build System ← NEXT
**Status**: Ready to start
**Cycles Budget**: 1
**Assigned to**: Ares team

**Objective**: Fix critical build blockers

**Tasks**:
1. Fix Makefile: change output from `test` to `code`
2. Fix main.c lines 55-56: remove PTR_ERR wrapper (return_pages returns int not void*)
3. Verify clean build with no errors/warnings
4. Commit and push

**Success Criteria**:
- `make` produces executable named `code`
- Build completes with zero errors
- Build completes with zero warnings
- Changes pushed to GitHub

---

### M2: Core Buddy Algorithm Implementation (Planned)
**Status**: Planned
**Cycles Budget**: 8

**Objective**: Implement the five required functions with correct Buddy algorithm logic

**Key Components**:
1. Data structures for managing free lists per rank (1-16)
2. `init_page()`: Initialize memory pool and free lists
3. `alloc_pages()`: Split blocks as needed, allocate from appropriate rank
4. `return_pages()`: Return blocks and coalesce buddies
5. `query_ranks()`: Track allocated/free block ranks
6. `query_page_counts()`: Count available blocks per rank

**Success Criteria**:
- All functions implemented with proper error handling
- Basic logic for splitting and coalescing works
- Code compiles without warnings

---

### M3: Testing & Bug Fixes (Planned)
**Status**: Planned
**Cycles Budget**: 6

**Objective**: Test against provided test suite and fix issues

**Tasks**:
- Run local tests with provided main.c
- Debug failures in each test phase
- Verify boundary conditions
- Handle edge cases (NULL, invalid ranks, out of memory)

**Success Criteria**:
- All 8 test phases pass locally
- No segmentation faults or memory errors
- Output matches expected format

---

### M4: Code Quality & OJ Submission Prep (Planned)
**Status**: Planned
**Cycles Budget**: 3

**Objective**: Polish code and ensure OJ compatibility

**Tasks**:
- Code review for efficiency and correctness
- Verify time/space complexity requirements
- Final build system check
- Documentation review

**Success Criteria**:
- Code follows C best practices
- No unnecessary complexity
- Ready for external OJ evaluation
- All requirements from README.md met

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

---

**Last Updated**: Cycle 2 (Athena)
