#include "buddy.h"
#define NULL ((void *)0)

/* Constants */
#define MAXRANK 16
#define PAGE_SIZE 4096

/* Free list node - embedded in free pages */
typedef struct free_node {
    struct free_node *prev;
    struct free_node *next;
} free_node_t;

/* Global state */
static void *base_ptr = NULL;           /* Start of managed memory */
static int total_pages = 0;             /* Total number of pages */
static free_node_t *free_lists[MAXRANK + 1];  /* Free lists for ranks 1-16 */
static int free_counts[MAXRANK + 1];    /* Cached count of free blocks per rank */

/* Metadata stored at end of address space - we'll use malloc to allocate it separately */
static unsigned char *metadata = NULL;

/* Helper: Get page index from pointer */
static inline int get_page_index(void *p) {
    if (p < base_ptr) return -1;
    long offset = (char *)p - (char *)base_ptr;
    if (offset % PAGE_SIZE != 0) return -1;
    int idx = offset / PAGE_SIZE;
    if (idx >= total_pages) return -1;
    return idx;
}

/* Helper: Get pointer from page index */
static inline void *get_page_ptr(int idx) {
    return (char *)base_ptr + (idx * PAGE_SIZE);
}

/* Helper: Add page to free list */
static void add_to_free_list(int rank, void *p) {
    free_node_t *node = (free_node_t *)p;
    node->prev = NULL;
    node->next = free_lists[rank];
    if (free_lists[rank] != NULL) {
        free_lists[rank]->prev = node;
    }
    free_lists[rank] = node;
    free_counts[rank]++;  /* Increment cached count */
}

/* Helper: Remove page from free list */
static void remove_from_free_list(int rank, void *p) {
    free_node_t *node = (free_node_t *)p;
    
    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        /* This is the head of the list */
        free_lists[rank] = node->next;
    }
    
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }
    
    free_counts[rank]--;  /* Decrement cached count */
}

/* Helper: Calculate buddy index */
static inline int get_buddy_index(int idx, int rank) {
    return idx ^ (1 << (rank - 1));
}

/* Helper: Check if buddy is free and same rank */
static int is_buddy_free(int buddy_idx, int rank) {
    if (buddy_idx >= total_pages) return 0;
    return metadata[buddy_idx] == (0x80 | rank);  /* 0x80 marks free */
}

/* We need external memory for metadata - use static array */
/* Max possible pages: assume up to 1GB / 4KB = 256K pages */
#define MAX_POSSIBLE_PAGES (256 * 1024)
static unsigned char metadata_storage[MAX_POSSIBLE_PAGES];

/* Initialize pages */
int init_page(void *p, int pgcount) {
    base_ptr = p;
    total_pages = pgcount;
    metadata = metadata_storage;
    
    /* Initialize free lists and counts */
    for (int i = 0; i <= MAXRANK; i++) {
        free_lists[i] = NULL;
        free_counts[i] = 0;
    }
    
    /* Initialize metadata to 0 */
    for (int i = 0; i < pgcount; i++) {
        metadata[i] = 0;
    }
    
    /* Create maximum-rank blocks from all available memory */
    int max_block_size = 1 << (MAXRANK - 1);  /* 2^15 = 32768 pages */
    int remaining_pages = pgcount;
    int start_page = 0;
    
    while (remaining_pages >= max_block_size) {
        void *block = get_page_ptr(start_page);
        add_to_free_list(MAXRANK, block);
        
        /* OPTIMIZATION: Only mark the block head (first page) */
        metadata[start_page] = 0x80 | MAXRANK;  /* Free + rank 16 */
        /* Continuation pages remain 0 (implicit) */
        
        start_page += max_block_size;
        remaining_pages -= max_block_size;
    }
    
    /* Handle remaining pages by creating smaller blocks */
    for (int rank = MAXRANK - 1; rank >= 1; rank--) {
        int block_size = 1 << (rank - 1);
        if (remaining_pages >= block_size) {
            void *block = get_page_ptr(start_page);
            add_to_free_list(rank, block);
            
            /* OPTIMIZATION: Only mark the block head (first page) */
            metadata[start_page] = 0x80 | rank;
            /* Continuation pages remain 0 (implicit) */
            
            start_page += block_size;
            remaining_pages -= block_size;
        }
    }
    
    return OK;
}

/* Allocate pages */
void *alloc_pages(int rank) {
    /* Validate rank */
    if (rank < 1 || rank > MAXRANK) {
        return (void *)(long)(-EINVAL);
    }
    
    /* Find a free block of requested rank or larger */
    int current_rank = rank;
    while (current_rank <= MAXRANK && free_lists[current_rank] == NULL) {
        current_rank++;
    }
    
    /* No free block found */
    if (current_rank > MAXRANK) {
        return (void *)(long)(-ENOSPC);
    }
    
    /* Get the free block */
    void *block = (void *)free_lists[current_rank];
    remove_from_free_list(current_rank, block);
    
    /* Split the block down to requested rank */
    while (current_rank > rank) {
        current_rank--;
        int block_size = 1 << (current_rank - 1);
        
        /* Split: keep left half, add right half to free list */
        int left_idx = get_page_index(block);
        int right_idx = left_idx + block_size;
        void *right_block = get_page_ptr(right_idx);
        
        /* OPTIMIZATION: Only mark right block head as free */
        metadata[right_idx] = 0x80 | current_rank;
        /* Continuation pages remain 0 (implicit) */
        add_to_free_list(current_rank, right_block);
    }
    
    /* Mark allocated block */
    int block_idx = get_page_index(block);
    int block_size = 1 << (rank - 1);
    for (int i = 0; i < block_size; i++) {
        metadata[block_idx + i] = rank;  /* Allocated (no 0x80 bit) */
    }
    
    return block;
}

/* Return pages */
int return_pages(void *p) {
    /* Validate pointer */
    if (p == NULL) {
        return -EINVAL;
    }
    
    int idx = get_page_index(p);
    if (idx < 0) {
        return -EINVAL;
    }
    
    /* Get rank of block */
    int rank = metadata[idx] & 0x7F;  /* Mask off free bit */
    if (rank < 1 || rank > MAXRANK) {
        return -EINVAL;
    }
    
    /* Check if already free */
    if (metadata[idx] & 0x80) {
        return -EINVAL;
    }
    
    /* Coalesce with buddy */
    while (rank < MAXRANK) {
        int buddy_idx = get_buddy_index(idx, rank);
        
        /* Check if buddy is free and same rank */
        if (!is_buddy_free(buddy_idx, rank)) {
            break;  /* Cannot coalesce */
        }
        
        /* Remove buddy from free list */
        void *buddy_ptr = get_page_ptr(buddy_idx);
        remove_from_free_list(rank, buddy_ptr);
        
        /* Merge: use the lower index */
        if (buddy_idx < idx) {
            idx = buddy_idx;
        }
        
        /* Increase rank */
        rank++;
    }
    
    /* Add coalesced block to free list */
    void *block = get_page_ptr(idx);
    /* OPTIMIZATION: Only mark block head as free */
    metadata[idx] = 0x80 | rank;
    /* Continuation pages don't need updating for free blocks */
    add_to_free_list(rank, block);
    
    return OK;
}

/* Query ranks */
int query_ranks(void *p) {
    int idx = get_page_index(p);
    if (idx < 0) {
        return -EINVAL;
    }
    
    unsigned char meta = metadata[idx];
    int rank = meta & 0x7F;
    
    /* If rank is 0, this is a continuation page of a free block */
    /* Search backward to find the block head */
    if (rank == 0) {
        /* Try each rank from largest to smallest */
        for (int r = MAXRANK; r >= 1; r--) {
            int block_size = 1 << (r - 1);
            int head_idx = (idx / block_size) * block_size;
            
            /* Check if there's a free block head at this position */
            if (head_idx < idx && (metadata[head_idx] & 0x80) && 
                (metadata[head_idx] & 0x7F) == r) {
                return r;
            }
        }
        return -EINVAL;  /* Shouldn't happen for valid pages */
    }
    
    return rank;
}

/* Query page counts */
int query_page_counts(int rank) {
    /* Validate rank */
    if (rank < 1 || rank > MAXRANK) {
        return -EINVAL;
    }
    
    /* OPTIMIZATION: Return cached count (O(1) instead of O(n)) */
    return free_counts[rank];
}
