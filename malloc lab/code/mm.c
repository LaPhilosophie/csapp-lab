/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7) // ~0x7是0xFFFFFFF8，这里向上舍入到 8 的倍数

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Basic constants and macros */
#define WSIZE 4             /* Word and header/footer size (bytes) */
#define DSIZE 8             /* Double word size (bytes) 或者，header+footer的大小*/
#define CHUNKSIZE (1 << 12) /* Extend heap by this amount (bytes) 4096字节*/

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* 获取p地址处保存的unsigned int的值 Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
/* 将val的值以unsigned int存储到p地址处*/
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* 读取p地址处保存的unsigned int的值，忽略最后三位 Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
/* 获取p地址处保存的unsigned int的值的最后一位，也即该块是否分配*/
#define GET_ALLOC(p) (GET(p) & 0x1)

/* 获取bp地址处对应的块的头部的指针，char *类型，bp：块指针  减：往头部走 加：往脚部走 Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
/* 获取bp地址处对应的块的脚部的指针，char *类型，要注意，头/脚部保存的大小是一整个块，包含头脚部和payload的所有的大小 */
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* 获取bp地址处对应的块的下一个块的块指针 Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
/* 获取bp地址处对应的块的前一个块的块指针 */
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

char *heap_listp;

static void *coalesce(void *);

static void *extend_heap(size_t words) {
    char *bp;
    size_t size;

    /* 为了8字节对齐，传入的words应该是2的整数倍，比如2,4,8 以满足8字节倍数的要求 Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    // 调用mem_sbrk 申请size字节的内存，如果失败，返回NULL，bp是指向新申请的内存的底部的指针
    if ((long)(bp = mem_sbrk(size)) == -1) return NULL;

    /* Initialize free block header/footer and the epilogue header */
    /* 这里bp被当做指向块的指针，put宏将会覆盖尾声块*/
    /* 给新开辟的空间设置头部和脚部，并标记为未分配状态*/
    PUT(HDRP(bp), PACK(size, 0));         /* Free block header */
    PUT(FTRP(bp), PACK(size, 0));         /* Free block footer */
    /* 由于尾声块被覆盖，这里将尾声块放到新开辟空间的后面，作为结束标志*/
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header*/
    /* 这里分配了一个新的块，假设前一个块是未分配状态，那么前后两个块需要进行合并 */
    /* coalesce函数返回合并后的空闲块的指针*/
    return coalesce(bp);
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
        /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1) {
        return -1;
    }
    PUT(heap_listp, 0);                            /* Alignment padding */
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); /* 序言块 头部 8/1 Prologue header */
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); /* 序言块 脚部 8/1 Prologue footer */
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));     /* 尾声块 头部 0/1 Epilogue header */
    heap_listp += (2 * WSIZE);

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL) {
        return -1;
    }
    return 0;
}

static void * find_fit(ssize_t asize)
{
    void *bp;
    /* 遍历空闲链表，寻找合适大小的空闲块 */
    for(bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        /* 如果找到合适大小的空闲块，则返回指向该空闲块的指针 */
        if(!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
            return bp;
        }
    }
    return NULL; /* No fit */
}

static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));  // 获取空闲块的大小
    /* 如果空闲块的大小大于请求的大小，则将空闲块分割为两部分，一部分分配给请求，另一部分仍然是空闲块 */
    if((csize - asize) >= (2 * DSIZE)) {
        /* 将空闲块分割为两部分，一部分分配给请求，另一部分仍然是空闲块 */
        PUT(HDRP(bp), PACK(asize, 1));  // 设置分配块的头部
        PUT(FTRP(bp), PACK(asize, 1));  // 设置分配块的脚部
        bp = NEXT_BLKP(bp);  // 将指针指向剩余的空闲块
        PUT(HDRP(bp), PACK(csize - asize, 0));  // 设置剩余空闲块的头部
        PUT(FTRP(bp), PACK(csize - asize, 0));  // 设置剩余空闲块的脚部
    } else {  // 否则，将整个空闲块分配给请求
        PUT(HDRP(bp), PACK(csize, 1));  // 设置分配块的头部
        PUT(FTRP(bp), PACK(csize, 1));  // 设置分配块的脚部
    }
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize; /* Adjust block size */  // 调整后的块大小
    size_t extendsize; /* Amount to extend heap if no fit */  // 如果没有合适的块，则需要扩展堆的大小
    char *ptr;

    /* Ignore spurious requests */
    if(size == 0){  // 如果请求的大小为0，则返回NULL
        return NULL;
    }

    /* Adjust block size to include overhead and alignment reqs. */
    if(size <= DSIZE){  // 如果请求的大小小于等于DSIZE，则将块大小设置为2 * DSIZE
        asize = 2 * DSIZE; 
    } else {  // 否则，将块大小设置为DSIZE的倍数，满足对齐要求
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE); 
    }

    /* Search the free list for a fit */
    if((ptr = (char *)find_fit(asize)) != NULL) {  // 在空闲链表中查找合适大小的空闲块
        place(ptr, asize);  // 将空闲块分配给请求
        return ptr;  // 返回指向分配的空间的指针
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);  // 如果没有找到合适大小的空闲块，则需要扩展堆的大小，扩展的大小为请求大小和一个默认的 CHUNKSIZE 中的较大值
    if((ptr = extend_heap(extendsize/WSIZE)) == NULL) {  // 扩展堆的大小，并将新分配的空间划分为一个新的空闲块
        return NULL;  // 如果扩展失败，则返回NULL
    }
    place(ptr, asize);  // 将新分配的空间分配给请求
    return ptr;  // 返回指向分配的空间的指针
}



/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{   /*获取ptr指向的块的大小*/
    size_t size = GET_SIZE(HDRP(ptr));
    /*将块的头脚部的标志位都改为0，也即标记为未分配*/
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    /*free操作伴随着四种合并的可能*/
    coalesce(ptr);
}

/* 块的合并操作 */
static void *coalesce(void *ptr) {
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(ptr)));//获取前一个块的分配状态
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));// 获取后一个块的分配状态
    size_t size = GET_SIZE(HDRP(ptr));//获取当前块的大小
    
    // 根据前后块的分配状态，进行四种合并操作
    // 1.前后块都已分配
    if (prev_alloc && next_alloc) {
        return ptr;
    }
    // 2.前块已分配，后块未分配
    else if (prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(ptr)));// 当前块的大小+后块的大小
        PUT(HDRP(ptr), PACK(size, 0));
        PUT(FTRP(ptr), PACK(size, 0));
    }
    // 3.前块未分配，后块已分配
    else if (!prev_alloc && next_alloc) {
        size += GET_SIZE(HDRP(PREV_BLKP(ptr)));// 当前块的大小+前块的大小
        PUT(FTRP(ptr), PACK(size, 0));        // 合并当前块+前块，更新当前块的头部、脚部、分配状态
        PUT(HDRP(PREV_BLKP(ptr)),PACK(size, 0));
        ptr = PREV_BLKP(ptr);
    }
    // 4.前后块都未分配
    else {
        size += GET_SIZE(HDRP(PREV_BLKP(ptr))) + GET_SIZE(FTRP(NEXT_BLKP(ptr)));
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(size, 0));
        ptr = PREV_BLKP(ptr);
    }
    return ptr;//返回合并后的块的指针
}
/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
    size_t oldsize;
    void *newptr;

    /* If size == 0 then this is just free, and we return NULL. */
    if (size == 0) {
        mm_free(ptr);
        return 0;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if (ptr == NULL) {
        return mm_malloc(size);
    }

    newptr = mm_malloc(size);

    /* If realloc() fails the original block is left untouched  */
    if (!newptr) {
        return 0;
    }

    /* Copy the old data. */
    oldsize = GET_SIZE(HDRP(ptr)) - DSIZE;
    if (size < oldsize) oldsize = size;
    memcpy(newptr, ptr, oldsize);

    /* Free the old block. */
    mm_free(ptr);

    return newptr;
}












