

#虚拟内存

[TOC]

## 从物理内存到虚拟内存

**物理地址**	主存被组织成一个由M个连续的字节大小的单元组成的数组，每个字节都有一个唯一的物理地址

**物理寻址** (Physical addressing)	CPU访问内存采用物理地址

**虚拟寻址** (Virtual addressing)	CPU访问内存采用虚拟地址



## 地址翻译

### 多层页面 Multi-Level Page Table

​	虽然页表是一个表，但因为往往虚拟地址的位数比物理内存的位数要大得多，所以保存页表项(PTE) 所需要的空间也是一个问题。eg.假设每个页的大小是 4KB（2 的 12 次方），每个地址有 48 位，一条 PTE 记录有 8 个字节，那么要全部保存下来，需要的大小是：

$2^{48} \times 2^{-12} \times 2^3 = 2^{39} bytes$

整整 512 GB！所以我们采用多层页表，第一层的页表中的条目指向第二层的页表，一个一个索引下去，最终寻找具体的物理地址，整个翻译过程如下：

![img](http://wdxtub.com/images/14619365087673.jpg)

eg.构造一个两级的页表层次结构。其中一级页表中每个PTE负责映射虚拟空间中的一个4MB的片（chunk），每一片都是由1024个连续页面组成。eg.PTE0映射第一片，PTE1映射接下来的一片。所以假设地址空间4GB，1024个PTE足够覆盖整个空间。（页面大小4KB）

​	二级页表中的每个PTE都负责映射一个4KB的虚拟内存页面。



![CAASP9-17](C:\Users\Kevin\Desktop\虚拟内存\CSAPP9-17.jpg)



### 地址翻译实例

内存系统设定如下：

- 14 位的虚拟地址
- 12 位的物理地址
- 页大小为 64 字节

TLB 的配置为：

- 能够存储 16 条记录
- 每个集合有 4 条记录

系统本身缓存（对应于物理地址）：

- 16 行，每个块 4 个字节
- 直接映射（即 16 个集合）

![img](http://wdxtub.com/images/14619386956596.jpg)

TLB 中的数据为

![img](http://wdxtub.com/images/14619392574046.jpg)

页表中的数据为（一共有 256 条记录，这里列出前 16 个）

![img](http://wdxtub.com/images/14619394874702.jpg)

缓存中的数据为

![img](http://wdxtub.com/images/14619399183944.jpg)



> 若我们需要虚拟地址为 `0x03D4`

具体的转换过程如下图所示：

![img](http://wdxtub.com/images/14619404450222.jpg)

具体流程：

先看 TLB 中有没有对应的条目，所以先看虚拟地址的第 6-13 位，在前面的 TLB 表中，根据 TLBI 为 3 这个信息，去看这个 set 中有没有 tag 为 3 的项目，发现有，并且对应的 PPN 是 0x0D，所以对应到物理地址，就是 PPN 加上虚拟地址的 0-5 位，而具体的物理地址又可以在缓存中找到（利用 cache memory 的机制），就可以获取到对应的数据了。

下面的例子同样可以按照这个方法来进行分析

> 虚拟地址为 `0x0020`

![img](http://wdxtub.com/images/14619405964352.jpg)



## 动态内存分配

程序员通过动态内存分配（例如 `malloc`）来让程序在运行时得到虚拟内存。动态内存分配器会管理一个虚拟内存区域，称为堆(heap)。

分配器以块为单位来维护堆，可以进行分配或释放。有两种类型的分配器：

- 显式分配器：应用分配并且回收空间（C 语言中的 `malloc` 和 `free`）
- 隐式分配器：应用只负责分配，但是不负责回收（Java 中的垃圾收集）

> 垃圾收集 (garbage collection)：自动释放未使用的已分配的块的过程

先来看看一个简单的使用 `malloc` 和 `free` 的例子

```C++
#include <stdio.h>
#include <stdlib.h>
void foo(int n) {
    int i, *p;
    
    /* Allocate a block of n ints */
    p = (int *) malloc(n * sizeof(int));
    if (p == NULL) {
        perror("malloc");
        exit(0);
    }
    
    /* Initialize allocated block */
    for (i=0; i<n; i++)
        p[i] = i;
    /* Return allocated block to the heap */
    free(p);
}
```

为了讲述方便，我们做如下假设：

- 内存地址按照字来编码
- 每个字的大小和整型一致

例如：

![img](http://wdxtub.com/images/14619422087381.jpg)

程序可以用任意的顺序发送 `malloc` 和 `free` 请求，`free` 请求必须作用与已被分配的 block。

分配器有如下的限制：

- 不能控制已分配块的数量和大小
- 必须立即响应 `malloc` 请求（不能缓存或者给请求重新排序）
- 必须在未分配的内存中分配
- 不同的块需要对齐（32 位中 8 byte，64 位中 16 byte）
- 只能操作和修改未分配的内存
- 不能移动已分配的块



### 性能指标

现在我们可以来看看如何去评测具体的分配算法了。假设给定一个 `malloc` 和 `free` 的请求的序列：

$R_0, R_1, ..., R_k, ..., R_{n-1}$

目标是尽可能提高吞吐量以及内存利用率（注意，这两个目标常常是冲突的）

吞吐量是在单位时间内完成的请求数量。假设在 10 秒中之内进行了 5000 次 `malloc` 和 5000 次 `free` 调用，那么吞吐量是 1000 operations/second

另外一个目标是 Peak Memory Utilization，就是最大的内存利用率。

影响内存利用率的主要因素就是『内存碎片』（fragmentation），分为内部碎片和外部碎片两种。

**内部碎片** （internalfragmentation）	内部碎片指的是对于给定的块，如果需要存储的数据(payload)小于块大小，就会因为对齐和维护堆所需的数据结构的缘故而出现无法利用的空间，例如：

![img](http://wdxtub.com/images/14619426495995.jpg)

内部碎片只依赖于上一个请求的具体模式，所以比较容易测量。

**外部碎片**（external fragmentation）指的是内存中没有足够的连续空间，如下图所示，内存中有足够的空间，但是空间不连续，所以成为了碎片：

![img](http://wdxtub.com/images/14619429933039.jpg)



### 实现



## 垃圾回收



## 内存陷阱

关于内存的使用需要注意避免以下问题：

- 解引用错误指针
- 读取未初始化的内存
- 覆盖内存
- 引用不存在的变量
- 多次释放同一个块
- 引用已释放的块
- 释放块失败

### 解引用错误指针 Dereferencing Bad Pointers

这是非常常见的例子，没有引用对应的地址，少了 `&`

```C++
int val;
...
scanf("%d", val);
```

### 读取未初始化的内存 Reading Uninitialized Memory

不能假设堆中的数据会自动初始化为 0。下面的代码就会出现奇怪的问题

```c++
/* return y = Ax */
int *matvec(int **A, int *x) {
    int *y = malloc(N * sizeof(int));
    int i, j;
    
    for (i = 0; i < N; i++)
        for (j = 0; j < N; j++)
            y[i] += A[i][j] * x[j];
    return y;
}
```

这个示例中，不正确地假设数组y被初始化为0

### 覆盖内存 Overwriting Memory

这里有挺多问题，第一种是分配了错误的大小，下面的例子中，一开始不能用 `sizeof(int)`，因为指针的长度不一定和 int 一样。

```C++
int **p;
p = malloc(N * sizeof(int));

for (i = 0; i < N; i++) 
    p[i] = malloc(M * sizeof(int));
```

第二个问题是超出了分配的空间，下面代码的 for 循环中，因为使用了 `<=`，会写入到其他位置

```C++
int **p;

p = malloc(N * sizeof (int *));

for (i = 0; i <= N; i++)
    p[i] = malloc(M * sizeof(int));
```

第三种是因为没有检查字符串的长度，超出部分就写到其他地方去了（经典的缓冲区溢出攻击也是利用相同的机制）

```C++
char s[8];
int i;
gets(s); /* reads "123456789" from stdin */
```

第四种是没有正确理解指针的大小以及对应的操作，应该使用 `sizeof(int *)`

```C++
int *search(int *p, int val) {
    while (*p && *p != null)
        p += sizeof(int);
    return p;
}
```

第五种是引用了指针，而不是其指向的对象，下面的例子中，`*size--` 一句因为 `--` 的优先级比较高，所以实际上是对指针进行了操作，正确的应该是 `(*size)--`

```C++
int *BinheapDelete(int **binheap, int *size) {
    int *packet;
    packet = binheap[0];
    binheap[0] = binheap[*size - 1];
    *size--;
    Heapify(binheap, *size, 0);
    return (packet);
}
```

### Referencing Nonexistent Variables

下面的情况中，没有注意到局部变量会在函数返回的时候失效（所以对应的指针也会无效），这是传引用和返回引用需要注意的，传值的话则不用担心

```C++
int *foo() {
    int val;
    
    return &val;
}
```

### Freeing Blocks Multiple Times

这个不用多说，不能重复搞两次

```c++
x = malloc(N * sizeof(int));
//  <manipulate x>
free(x);

y = malloc(M * sizeof(int));
//  <manipulate y>
free(x);
```

### Referencing Freed Blocks

同样是很明显的错误，不要犯

```c++
x = malloc(N * sizeof(int));
//  <manipulate x>
free(x);
//  ....

y = malloc(M * sizeof(int));
for (i = 0; i < M; i++)
    y[i] = x[i]++;
```

### Memory Leaks

用完没有释放，就是内存泄露啦

```C++
foo() {
    int *x = malloc(N * sizeof(int));
    // ...
    return ;
}
```

或者只释放了数据结构的一部分：

```C++
struct list {
    int val;
    struct list *next;
};

foo() {
    struct list *head = malloc(sizeof(struct list));
    head->val = 0;
    head->next = NULL;
    //...
    free(head);
    return;
}
```

## 总结



