# Leo's Workspace Note - Issue #12

## Assignment
Issue #12: Unable to access tracker database, but as build engineer, investigating compiler warnings

## Investigation

### Current Build State
- Makefile exists but has minimal compiler flags
- Current: `gcc -o code main.c buddy.c`
- Build succeeds with no warnings using default flags
- Tests pass (98,547+ assertions)

### Discovered Issues
When compiling with strict warnings (`-Wall -Wextra`):
```
main.c:94:19: warning: unused variable 'r' [-Wunused-variable]
   94 |             void *r = alloc_pages(1);
main.c:119:19: warning: unused variable 'r' [-Wunused-variable]
  119 |             void *r = alloc_pages(1);
```

### Analysis
- Lines 94 and 119 in main.c allocate pages but don't use the return value
- This is intentional (exhausting memory for test), but creates compiler warnings
- Variable `r` assigned but never used

### Proposed Fix
Two options:
1. Use `(void)r;` to suppress warning
2. Change to `(void)alloc_pages(1);` 

Option 2 is cleaner - directly casting to void shows intent.

## Work Plan
1. Fix unused variable warnings in main.c (lines 94, 119)
2. Optionally: Add -Wall to Makefile for better code quality
3. Verify build succeeds with no warnings
4. Run tests to ensure no regression
5. Commit and push

## Status
Ready to implement fixes
