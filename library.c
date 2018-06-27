#include "library.h"

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

struct block_meta {
    size_t size;
    struct block_meta* next;
    int free;
    int magic;
};

void* global_base = NULL;

struct block_meta* find_free_block(struct block_meta** last, size_t size ) {
    struct block_meta* current = global_base;
    while(current && ( !(current->free) || current->size < size) ){
        *last = current;
        current = current->next;
    }
    return current;
}

struct block_meta* request_space(struct block_meta* last, size_t size) {
    struct block_meta *block;
    block = sbrk(0);
    void* request = sbrk(size + META_SIZE);
    if (request == (void*) -1) {
        return NULL; // sbrk failed.
    }
    if (last) {
        last->next = block;
    }

    block->size = size;
    block->next = NULL;
    block->free = 0;
    block->magic = 0x12345678;
    return block;
}

void *malloc(size_t* size) {
    struct block_meta *block;

}