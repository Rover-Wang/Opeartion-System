#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MEMORY_SIZE 1024 //内存总大小为1024
#define PROCESS_NUM 10 //有10个进程

//内存大小随机在[100, 200]之间产生
#define MIN_SIZE 100 
#define MAX_SIZE 200 

typedef struct Block {
    int id;             //分区的序号
    int size;           //分区的大小
    int startAddr;      //分区的开始位置
    bool status;        //分区的状态：true为空闲，false为被占用
    int pid;            //如果该分区被占用，则存放占用进程的id; 否则为 - 1
    struct Block* prev;  //指向前面一块内存分区
    struct Block* next;   //指向后面一块内存分区
}Block;


typedef struct PCB {
    int pid;            //进程的序号
    int neededMem;      //需要的内存分区大小
    int status;         //1: 内存分配成功；-1：分配失败
    int blockID;        //如果分配成功，保存占用分区的id,否则为 - 1
    struct PCB* next;   //指向下一个PCB
}PCB;

Block* memory = NULL;
int process_size[PROCESS_NUM];
Block* allocated[PROCESS_NUM]; // 记录每个进程分配到的块


//内存大小随机产生
void randSize()
{
    srand((unsigned)time(NULL));
    for (int i = 0; i < PROCESS_NUM; i++)
    {
        process_size[i] = rand() % (MAX_SIZE - MIN_SIZE + 1) + MIN_SIZE;
    }
}


// 初始化内存
void init_memory()
{
    memory = (Block*)malloc(sizeof(Block));
    memory->startAddr = 0;
    memory->size = MEMORY_SIZE;
    memory->status = 1;//1空闲 
    memory->pid = -1;
    memory->next = NULL;
    for (int i = 0; i < PROCESS_NUM; i++) allocated[i] = NULL;
}


//初始化NF标记
int sign = 0;

// 打印内存分区情况
void print_memory()
{
    Block* p = memory;
    printf("分区情况：\n");
    while (p)
    {
        printf("[%d-%d] 大小:%d %s", p->startAddr, p->startAddr + p->size - 1, p->size, p->status ? "空闲" : "占用");
        if (!p->status) printf(" (进程%d)", p->pid + 1);
        printf("\n");
        p = p->next;
    }
    printf("\n");
}

// 在block内随机选择起始地址
int random_start(Block* block, int size)
{
    if (block->size == size) //如果分配大小刚好等于分区大小
    {
        return block->startAddr;
    }
    int max_offset = block->size - size;//最大偏移量
    return block->startAddr + (rand() % (max_offset + 1));
}

//首次适应算法(FF)
int alg_FF(int pid)
{
    Block* p = memory;
    while (p)
    {
        if (p->status == 1 && p->size >= process_size[pid])//如果当前分区为空且大小足够
        {
            int alloc_start = random_start(p, process_size[pid]);
            //若是分配地址高于分块起始地址：分割前段
            if (alloc_start > p->startAddr)
            {
                Block* pre = (Block*)malloc(sizeof(Block));
                pre->startAddr = p->startAddr;//前段起始==分块起始
                pre->size = alloc_start - p->startAddr;
                pre->status = 1;
                pre->pid = -1;
                pre->next = p;

                //将前驱的next指向pre
                Block* prev = memory;
                if (prev == p) memory = pre;//若p即是链表头，更新头指针为pre
                else
                {
                    while (prev->next != p) prev = prev->next;
                    prev->next = pre;
                }
                p->startAddr = alloc_start;
                p->size -= pre->size;
            }
            // 更新后的p->size若是还大于进程大小：分割后段
            if (p->size > process_size[pid])
            {
                Block* post = (Block*)malloc(sizeof(Block));
                post->startAddr = p->startAddr + process_size[pid];
                post->size = p->size - process_size[pid];
                post->status = 1;
                post->pid = -1;
                post->next = p->next;
                p->next = post;
                p->size = process_size[pid];
            }
            // 分配
            p->status = 0;
            p->pid = pid;
            allocated[pid] = p;
            return 1;
        }

        p = p->next;//否则向后继续查找
    }
    return 0;
}


//循环首次适应算法(NF)
int alg_NF(int pid, Block** last)
{
    Block* p = *last;
    Block* start = p;
    do
    {
        if (p->status == 1 && p->size >= process_size[pid])//如果当前分区为空且大小足够
        {
            int alloc_start = random_start(p, process_size[pid]);
            //若是分配地址高于分块起始地址：分割前段
            if (alloc_start > p->startAddr)
            {
                Block* pre = (Block*)malloc(sizeof(Block));
                pre->startAddr = p->startAddr;//前段起始==分块起始
                pre->size = alloc_start - p->startAddr;
                pre->status = 1;
                pre->pid = -1;
                pre->next = p;

                //将前驱的next指向pre
                Block* prev = memory;
                if (prev == p) memory = pre;//若p即是链表头，更新头指针为pre
                else
                {
                    while (prev->next != p) prev = prev->next;
                    prev->next = pre;
                }
                p->startAddr = alloc_start;
                p->size -= pre->size;
            }
            // 更新后的p->size若是还大于进程大小：分割后段
            if (p->size > process_size[pid])
            {
                Block* post = (Block*)malloc(sizeof(Block));
                post->startAddr = p->startAddr + process_size[pid];
                post->size = p->size - process_size[pid];
                post->status = 1;
                post->pid = -1;
                post->next = p->next;
                p->next = post;
                p->size = process_size[pid];
            }
            // 分配
            p->status = 0;
            p->pid = pid;
            allocated[pid] = p;
            *last = p->next ? p->next : memory; // 更新last指针
            return 1;
        }
        p = p->next ? p->next : memory;//否则向后继续查找
    } while (p != start);//再次回到起点，分配失败，返回0
    return 0;
}

// 合并空闲分区
void merge_free()
{
    Block* p = memory;
    while (p && p->next)
    {
        if (p->status == 1 && p->next->status == 1 && (p->startAddr + p->size) == p->next->startAddr)//前后两块均为空闲且相邻
        {
            Block* tmp = p->next;
            p->size += tmp->size;
            p->next = tmp->next;
            free(tmp);
        }
        else
        {
            p = p->next;
        }
    }
}

// 回收内存
void recycle(int pid)
{
    if (allocated[pid])
    {
        allocated[pid]->status = 1;
        allocated[pid]->pid = -1;
        allocated[pid] = NULL;
        merge_free();
    }
}


int main()
{
    printf("请选择分配算法：1. 首次适应(FF)  2. 循环首次适应(NF)\n");
    int choice;
    scanf_s("%d", &choice);

    init_memory();
    randSize();

    printf("进程所需内存：\n");
    for (int i = 0; i < PROCESS_NUM; i++)
    {
        printf("进程#%d: %2d\n", i + 1, process_size[i]);
    }
    printf("\n");

    Block* nf_last = memory;
    for (int i = 0; i < PROCESS_NUM; i++)
    {
        int success = 0;
        if (choice == 1)
            success = alg_FF(i);
        else
            success = alg_NF(i, &nf_last);

        printf("为进程%d分配内存%s\n", i + 1, success ? "成功" : "失败");
        print_memory();
    }

    printf("开始回收内存：\n");
    for (int i = 0; i < PROCESS_NUM; i++)
    {
        if (allocated[i]) //已分配
        {
            printf("回收进程%d的内存\n", i + 1);
            recycle(i);
            print_memory();
        }
    }
    return 0;
}