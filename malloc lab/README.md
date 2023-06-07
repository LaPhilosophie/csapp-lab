# lab说明

> 下面是对lab说明文档的总结

## 评测

在`malloclab-handout.tar`发行版中的驱动程序`mdriver.c`将测试`mm.c`包的正确性、空间利用率和吞吐量。驱动程序由一组跟踪文件控制，这些文件包含在`malloclab-handout.tar`分发版中。每个跟踪文件都包含一系列分配、重新分配和释放指令，指示驱动程序以某种顺序调用您的`mm_malloc`、`mm_realloc`和`mm_free`例程。驱动程序和跟踪文件与我们为您的提交`mm.c`文件评分时将使用的相同。

驱动程序`mdriver.c`接受以下命令行参数：

- `-t <tracedir>`：在目录`tracedir`而不是`config.h`中定义的默认目录中查找默认跟踪文件。
- `-f <tracefile>`：使用一个特定的跟踪文件代替默认的跟踪文件集进行测试。
- `-h`：打印命令行参数的摘要。
- `-l`：除了学生的`malloc`包之外，还运行和测量`libc malloc`。
- `-v`：详细输出。在紧凑表中打印每个跟踪文件的性能分解。
- `-V`：更详细的输出。在处理每个跟踪文件时打印附加诊断信息。对于确定哪个跟踪文件导致您的`malloc`包失败很有用。

运行`./mdriver -t ./traces -v`，即可使用指定的trace文件夹下的文件进行评测，-v打印性能输出



```shell
make clean

make 

./mdriver -t ./traces -v
```



原始代码的评测输出：

```c
$ ./mdriver -t ./traces -v
Team Name:ateam
Member 1 :Harry Bovik:bovik@cs.cmu.edu
Using default tracefiles in ./traces/
Measuring performance with gettimeofday().
ERROR: mem_sbrk failed. Ran out of memory...
ERROR [trace 4, line 7673]: mm_malloc failed.
ERROR: mem_sbrk failed. Ran out of memory...
ERROR [trace 5, line 1662]: mm_malloc failed.
ERROR: mem_sbrk failed. Ran out of memory...
ERROR [trace 6, line 1780]: mm_malloc failed.
ERROR: mem_sbrk failed. Ran out of memory...
ERROR [trace 9, line 1705]: mm_realloc failed.
ERROR: mem_sbrk failed. Ran out of memory...
ERROR [trace 10, line 6562]: mm_realloc failed.

Results for mm malloc:
trace  valid  util     ops      secs  Kops
 0       yes   23%    5694  0.000021268585
 1       yes   19%    5848  0.000018332273
 2       yes   30%    6648  0.000020339184
 3       yes   40%    5380  0.000015349351
 4        no     -       -         -     -
 5        no     -       -         -     -
 6        no     -       -         -     -
 7       yes   55%   12000  0.000031382166
 8       yes   51%   24000  0.000070340909
 9        no     -       -         -     -
10        no     -       -         -     -
Total            -       -         -     -
```

看一下short1-bal.rep文件的内容：

```
20000
6
12
1
a 0 2040
a 1 2040
f 1
a 2 48
a 3 4072
f 3
a 4 4072
f 0
f 2
a 5 4072
f 4
f 5

```

## 函数说明

mm.c中有下面几个函数：

mm_init：执行任何必要的初始化

mm_malloc：mm_malloc 例程返回指向至少 size 个字节的已分配块有效载荷的指针。整个分配的块应该位于堆区域内并且不应该与任何其他分配的块重叠。由于 libc malloc 始终返回与 8 字节对齐的有效负载指针，因此您的 malloc 实现也应该这样做并始终返回 8 字节对齐的指针。

mm_free：mm_free 例程释放 ptr 指向的块。它什么都不返回。只有在先前调用 mm_malloc 或 mm_realloc 返回传递的指针 (ptr) 且尚未释放时，此例程才能保证工作。

mm_realloc：mm_realloc(void \*ptr, size_t size) 例程返回一个指向至少 size 个字节的已分配区域的指针

memlib.c中有我们可以使用的函数

- `void *mem_sbrk(int incr)`: 将堆扩展`incr`字节，其中`incr`是正非零整数，并返回指向新分配的堆区域的第一个字节的通用指针。语义与Unix'sbrk函数相同，除了`mem_sbrk`只接受正的非零整数参数。
- `void *mem_heap_lo(void)`: 返回指向堆中第一个字节的通用指针。
- `void *mem_heap_hi(void)`: 返回指向堆中最后一个字节的通用指针。
- `size_t mem_heapsize(void)`: 返回堆的当前大小(以字节为单位)。
- `size_t mem_pagesize(void)`: 返回系统的页面大小(以字节为单位)(Linux系统为4K)。



## 编程规则

- 您不应更改`mm.c`中的任何接口。
- 您不应调用任何与内存管理相关的库调用或系统调用。这不包括在您的代码中使用`malloc`、`calloc`、`free`、`realloc`、`sbrk`、`brk`或这些调用的任何变体。
- 您不得在`mm.c`程序中定义任何全局或静态复合数据结构，例如数组、结构、树或列表。但是，您可以在`mm.c`中声明全局标量变量，例如整数、浮点数和指针。
- 为了与返回8字节边界对齐的块的`libc malloc`包保持一致，您的分配器必须始终返回与8字节边界对齐的指针。驱动程序会为您强制执行此要求。



> 在实施时，建议分阶段进行。前9个跟踪包含对`malloc`和`free`的请求。最后2个跟踪包含对`realloc`、`malloc`和`free`的请求。我们建议您首先让您的`malloc`和`free`例程在前9个跟踪上正确有效地工作。只有这样，您才应该将注意力转移到`realloc`实现上。初学者可以在现有的`malloc`和`free`实现之上构建`realloc`。但是，要获得真正好的性能，您需要构建一个独立的`realloc`。最后，使用分析器(如gprof)可以帮助您优化性能。



# 隐式空闲链表实现

## 宏

为了降低复杂性并减少可能的错误，建议将指针算法封装在C预处理器宏中。正如书中提到的，内存管理器中的指针算法令人困惑且容易出错，可以通过为指针操作编写宏来显着降低复杂性

| 宏名称    | 宏定义                                              | 作用                                                         |
| --------- | --------------------------------------------------- | ------------------------------------------------------------ |
| GET       | `(*(unsigned int *)(p))`                            | 读取地址p处的unsigned int类型的值                            |
| PUT       | `(*(unsigned int *)(p) = (val))`                    | 将unsigned int类型的值val存储到地址p处                       |
| GET_SIZE  | `(GET(p) & ~0x7)`                                   | 从地址p处读取unsigned int类型的值，并忽略最后三位，得到块的大小 |
| GET_ALLOC | `(GET(p) & 0x1)`                                    | 从地址p处读取unsigned int类型的值，并获取最后一位，判断该块是否已分配 |
| HDRP      | `((char *)(bp) - WSIZE)`                            | 给定块指针bp，计算出该块的头部地址                           |
| FTRP      | `((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)`       | 给定块指针bp，计算出该块的脚部地址                           |
| NEXT_BLKP | `((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))` | 给定块指针bp，计算出下一个块的地址                           |
| PREV_BLKP | `((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))` | 给定块指针bp，计算出前一个块的地址                           |

理解上面的宏的前提是理解分配的堆的内存块的结构

![](https://cdn.jsdelivr.net/gh/LaPhilosophie/image/img/202305312110226.png)



```c
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

```

## mm_init



```c
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
```

mm_init函数负责堆的初始化工作

mm_init函数先调用mem_sbrk函数分配堆的初始空间，大小为4个字，依次是

- padding
- 序言块 头部 8/1 Prologue header
- 序言块 脚部 8/1 Prologue footer
- 尾声块 头部 0/1 Epilogue header

下面看一下mem_sbrk函数

```c
/* 
 * mem_sbrk - simple model of the sbrk function. Extends the heap 
 *    by incr bytes and returns the start address of the new area. In
 *    this model, the heap cannot be shrunk.
 */
void *mem_sbrk(int incr) 
{
    char *old_brk = mem_brk;

    if ( (incr < 0) || ((mem_brk + incr) > mem_max_addr)) {
	errno = ENOMEM;
	fprintf(stderr, "ERROR: mem_sbrk failed. Ran out of memory...\n");
	return (void *)-1;
    }
    mem_brk += incr;
    return (void *)old_brk;
}
```

可以看出，堆的范围是通过mem_brk限制的，堆的增长本质上是将全局变量mem_sbrk增加incr个字节

```c
static char *mem_brk;        /* points to last byte of heap */
```

mem_sbrk函数返回old_brk，old_brk是堆增长前的的最后一个字节的指针

 

之后，在mm_init函数中，使用了四个PUT，对堆的初始空间的四个字进行了初始化工作

对空堆的初始化完成之后，使用extend_heap函数拓展堆的大小

```c
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
```

- 首先对传入的字进行处理，为了保持8字节对齐，传入的words应该是2的整数倍
- 调用mem_sbrk进行堆空间的申请
- 给新分配的大小为size的空间进行初始化，将头部和足部设置为size大小，未分配
  - 注意到，这里分配新块时将原先的尾声块覆盖了
- 设置新的尾声块
- 调用coalesce函数

![](https://cdn.jsdelivr.net/gh/LaPhilosophie/image/img/202305312011844.png)



## mm_free

该函数释放ptr指向的块的空间

- 获取块的大小
- 将块的头脚部的标志位都改为0，也即标记为未分配
- 将这个块置为空闲，可能会导致合并操作，因此需要调用coalesce函数

```c
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
```



## coalesce

块的合并操作，建议配合书上的四个合并情况的配图观赏

- 前后块都已分配
- 前块已分配，后块未分配
- 前块未分配，后块已分配
- 前后块都未分配

```c
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
```







## mm_malloc

看一下lab提供的原始版本的mm_malloc函数，~~发现这个实现不就是扯淡吗~~。mm_malloc函数仅仅使用mem_sbrk函数申请对齐后的size大小的堆空间，然后对块写入size值

```c
/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}
```

下面的mm_malloc实现增加了：

- 特殊size值（0）的处理
- 使用find函数在空闲链表中查找合适大小的空闲块
- 使用place函数将空闲块分配给请求
- 如果没有找到合适大小的空闲块，则需要使用extend_heap函数扩展堆的大小

```c
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
    if((ptr = find_fit(asize)) != NULL) {  // 在空闲链表中查找合适大小的空闲块
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

```

### find_fit

遍历堆空间，找到一个合适大小的空闲块，并返回指向该空闲块的指针。如果找不到就返回null

```c
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
```

### place

给定空闲块的指针和需要分配的堆空间的大小，对块进行分配

- 如果空闲块的大小减去请求的大小大于等于16个字节（头部4字节、脚部4字节+8字节作为填充），那么就将空闲块分割为两部分
- 否则，如果空闲块的大小减去请求的大小小于两个字，那么将整个空闲块分配给请求

```c
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
```



## mm_realloc

根据文档，该函数具有以下约束。

- 如果 ptr 为 NULL，调用等同于 mm_malloc(size)；
- 如果 size 等于零，则调用等同于 mm_free(ptr)；
- 如果 ptr 不为 NULL，则它必须由先前对 mm malloc 或 mm realloc 的调用返回。调用 mm realloc 将 ptr（旧块）指向的内存块的大小更改为 size bytes 并返回新块的地址。请注意，新块的地址可能与旧块的地址相同，也可能不同，具体取决于您的实现、旧块中的内部碎片量以及 realloc 请求的大小。新块的内容与旧 ptr 块的内容相同，直到新旧大小中的最小值。其他一切都未初始化。
- 如果旧块为 8 个字节，新块为 12 个字节，则新块的前 8 个字节与旧块的前 8 个字节相同，最后 4 个字节未初始化。类似地，如果旧块是 8 个字节而新块是 4 个字节，那么新块的内容与旧块的前 4 个字节相同。



# 显示空闲链表实现

//todo