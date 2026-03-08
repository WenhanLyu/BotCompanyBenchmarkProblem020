# Project Roadmap - Buddy Algorithm Implementation

## Project Goal
Implement a high-quality Buddy memory allocation algorithm in C that passes ACMOJ evaluation (Problem ID: 1848).

## Milestone Progress

### M1: Project Setup & Foundation ✓ (In Progress)
**Status**: Starting
**Cycles Budget**: 2
**Cycles Used**: 0

**Objective**: Establish project infrastructure and understand requirements

**Tasks**:
- Create .gitignore for OJ compatibility
- Initial git commit with boilerplate
- Document problem requirements clearly
- Set up build system validation

**Success Criteria**:
- Code compiles successfully with `make`
- `.gitignore` includes CMakeFiles/ and CMakeCache.txt
- Initial commit pushed to GitHub
- Build produces `code` executable

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
(To be updated as project progresses)

---

**Last Updated**: Cycle 1 (Athena)
