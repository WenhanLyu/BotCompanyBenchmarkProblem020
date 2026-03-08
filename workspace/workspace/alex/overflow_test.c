#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_POSSIBLE_PAGES (256 * 1024)

int main() {
    printf("=== Overflow and Limit Testing ===\n\n");
    
    printf("System limits:\n");
    printf("  MAX_POSSIBLE_PAGES: %d\n", MAX_POSSIBLE_PAGES);
    printf("  In bytes: %ld\n", (long)MAX_POSSIBLE_PAGES);
    printf("  In KB: %ld\n", (long)MAX_POSSIBLE_PAGES * 4);
    printf("  In MB: %ld\n", (long)MAX_POSSIBLE_PAGES * 4 / 1024);
    printf("  In GB: %ld\n", (long)MAX_POSSIBLE_PAGES * 4 / 1024 / 1024);
    
    printf("\nMetadata storage:\n");
    printf("  1 byte per page\n");
    printf("  Total metadata: %d bytes = %d KB\n", 
           MAX_POSSIBLE_PAGES, MAX_POSSIBLE_PAGES / 1024);
    
    printf("\nRank calculations:\n");
    for (int rank = 1; rank <= 16; rank++) {
        long pages = 1L << (rank - 1);
        printf("  Rank %2d: %7ld pages\n", rank, pages);
    }
    
    printf("\nBuddy index calculations (checking for overflow):\n");
    // Test buddy calculation at various indices
    for (int rank = 1; rank <= 16; rank++) {
        int idx = 0;
        int buddy = idx ^ (1 << (rank - 1));
        printf("  Rank %2d: idx=0, buddy=%d\n", rank, buddy);
    }
    
    // Test at boundary
    int max_idx = MAX_POSSIBLE_PAGES - 1;
    for (int rank = 1; rank <= 10; rank++) {
        int buddy = max_idx ^ (1 << (rank - 1));
        printf("  Rank %2d: idx=%d, buddy=%d (valid: %s)\n", 
               rank, max_idx, buddy, 
               buddy < MAX_POSSIBLE_PAGES ? "yes" : "NO - OUT OF BOUNDS");
    }
    
    printf("\n=== Overflow Testing Complete ===\n");
    
    return 0;
}
