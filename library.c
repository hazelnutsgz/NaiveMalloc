#include <stdio.h>
#include <unistd.h>

#define BLOCK_SIZE 20
#define align4(x)  (((((x)-1)>>2)<<2)+4)

void* base = NULL;

struct block_meta{
    size_t          size;
    struct block_meta* next;
    struct block_meta* prev;
    int             free;
    void*           ptr;
    char            data[1];
};

struct block_meta* get_block(void *p){
    char *tmp;
    tmp = p;
    tmp -= BLOCK_SIZE;
    return tmp;
}


int valid_addr(void *p) {
    if (base && p > base && p < sbrk(0)){
        return (p == (get_block(p)->ptr));
    }
    return 0;
}

void split_block(struct block_meta* b, size_t size) {
    struct block_meta* new;
    new = (struct block_meta* )(b->data + size);
    new->size = b->size - BLOCK_SIZE;
    new->next = b->next;
    if (new->next) {
        new->next->prev = new;
    }
    new->prev = b;
    new->free = 1;
    new->ptr  = new->data;
    b->size   = size;
    b->next   = new;
}

struct block_meta* find_block(struct block_meta** last, size_t size) {
    struct block_meta* traverse = base;
    while(traverse &&!(traverse->free &&traverse->size >= size)){
        *last = traverse;
        traverse = traverse->next;
    }
    return traverse;
}

struct block_meta* extend_heap(struct block_meta* last, size_t size) {
    struct block_meta* meta;
    meta = sbrk(0);  //Not allocated yet, just mark the meta as the top of the heap
    void* test = sbrk(BLOCK_SIZE+size);
    if (test == (void*)-1) {
        return NULL;
    }
    meta->size = size;
    meta->next = NULL;
    meta->prev = last;
    meta->ptr = meta->data;
    if (last)
        last->next = meta;
    meta->free = 0;
    return meta;
}

void *malloc(size_t size) {
    struct block_meta* b;
    struct blocl_meta* last;
    size_t aligned_size = align4(size);
    if (base) {
        last = base;
        b = find_block(&last, aligned_size);
        if(b) {
            /*If we can split it*/
            if((b->size - aligned_size) >= (BLOCK_SIZE+4))
                split_block(b, aligned_size);
            b->free = 0;
        }else{
            /* No available space is found in user space*/
            b = extend_heap(last, aligned_size);
            if (!b)
                return NULL;
        }
    }else{
        b = extend_heap(NULL, aligned_size);
        if( !b)
            return NULL;
        base = b;
    }
    return (b->data);
}

struct block_meta* fusion(struct block_meta* meta) {
    if(meta->next && meta->next->free) {
        meta->size += BLOCK_SIZE + meta->next->size;
        meta->next = meta->next->next;
        if (meta->next)
            meta->next->prev = meta;
    }
    return meta;
}

void free(void* p) {
    struct block_meta* b;
    if(valid_addr(p)){
        b = get_block(p);
        b->free = 1;

        if (b->prev && b->prev->free)
            b= fusion(b->prev);

        if (b->next)
            fusion(b);
        else{
            if(b->prev)
                b->prev->next = NULL;
            else
                base = NULL;

            //When we reach the end of the block list, we should deallocate it to OS
            brk(b);
        }
    }
}
