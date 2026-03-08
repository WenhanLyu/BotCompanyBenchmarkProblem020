# Kai's Work Notes - Cycle 1

## Assignment
Working on issues #4 and #5 (interpreting as M2.1 tasks from roadmap):
- Implement init_page()
- Implement query_page_counts()

## Context Review

### Key Requirements:
- Valid ranks: 1-16 (page size = 4K × 2^(rank-1))
- Minimum allocation unit: 4K page
- Total pages in test: 32,768 (MAXRANK0PAGE = 128MB / 4KB)
- Must use buddy algorithm with coalescing

### Test Phases Relevant to M2.1:
- **Phase 1**: Tests init_page() returns OK
- **Phase 3**: Tests query_page_counts() returns 0 for all ranks after full allocation
- **Phase 5**: Tests query_page_counts() after deallocation (should have blocks at MAXRANK)

### Data Structure Design Needed:
1. Free lists for each rank (1-16)
2. Metadata to track each page's status and rank
3. Base pointer and total page count storage

## Implementation Plan

### Step 1: Design Data Structures
- Array of 16 free lists (one per rank)
- Doubly-linked list nodes embedded in free pages
- Metadata array: one byte per page for rank tracking
- Global state: base pointer, total pages

### Step 2: Implement init_page()
- Store base pointer and page count
- Initialize all free lists to NULL
- Calculate how many max-rank blocks fit
- Create max-rank blocks and add to free list
- Mark metadata for all pages

### Step 3: Implement query_page_counts()
- Validate rank (1-16)
- Walk free list for that rank and count nodes
- Return count or -EINVAL
