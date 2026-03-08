#include <stdio.h>

int main() {
    printf("Rank to pages mapping:\n");
    for (int rank = 1; rank <= 16; rank++) {
        int pages = 1 << (rank - 1);
        printf("  Rank %2d = 2^%2d = %6d pages = %7d KB\n", 
               rank, rank-1, pages, pages * 4);
    }
    
    printf("\n512 pages = 2MB\n");
    printf("Rank 10 = 2^9 = 512 pages\n");
    printf("So allocating rank 10 from 512-page pool uses all memory.\n");
    
    return 0;
}
