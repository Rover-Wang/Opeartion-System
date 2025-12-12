#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>

#define MEMORY_SIZE 1024 //内存总大小为1024
#define PROCESS_NUM 8 //有8个进程,针对（2）

#define MIN_EXP 3    // 最小块大小2^3=8
#define MAX_EXP 10   // 最大块大小2^10=1024（内存总大小）
#define MAX_LEVEL (MAX_EXP - MIN_EXP) // 层级数：10-3=7层

//进程所需内存大小随机在[2^3, 2^8]之间产生
#define MIN_SIZE 3 
#define MAX_SIZE 8


typedef struct BuddyBlock {
    int level;             //层级
    int size;           
    int startAddr;      
    bool status;        //分区的状态：1为空闲，0为被占用
    int pid;            //如果该分区被占用，则存放占用进程的id; 否则为 - 1
    struct BuddyBlock* next;  
    struct BuddyBlock* prev;
}BuddyBlock;

BuddyBlock* freelist[MAX_LEVEL + 1]; //每层的空闲链表头指针
int process_size[PROCESS_NUM];
BuddyBlock* allocated[PROCESS_NUM]; // 记录每个进程分配到的块


//内存大小随机产生,用于（2）
void randSize(int n)
{
    srand((unsigned)time(NULL));
    for (int i = 0; i < n; i++)
    {
        process_size[i] = (int)pow ( 2 , rand() % (MAX_SIZE - MIN_SIZE + 1) + MIN_SIZE );
    }
}


// 初始化内存
void init_memory()
{
    for (int i = 0; i <= MAX_LEVEL; i++)
    {
		freelist[i] = NULL;//初始化每层空闲链表，表头置空
    }
    BuddyBlock* memory = (BuddyBlock*)malloc(sizeof(BuddyBlock));
	if (!memory) 
	{
		printf("内存分配失败！\n");
		exit(1);
	}		
    memory->startAddr = 0;
    memory->size = MEMORY_SIZE;
    memory->level = MAX_LEVEL;
    memory->status = 1;//1空闲 
    memory->pid = -1;
    memory->next = NULL;
	memory->prev = NULL;
	freelist[MAX_LEVEL] = memory;//将整个内存块加入最高层空闲链表
    for (int i = 0; i < PROCESS_NUM; i++) allocated[i] = NULL;
	printf("初始化内存：[0-1023](层级%d,大小%d) 空闲\n", MAX_LEVEL, MEMORY_SIZE);
}



// 打印分区情况
void print_buddy() 
{
    printf("分区情况：\n");
    for (int i = MAX_LEVEL; i >= 0; i--) 
    {
        BuddyBlock* p = freelist[i];
        while (p) 
        {
            printf("[%d-%d] 大小:%d %s", p->startAddr, p->startAddr + p->size - 1, p->size, "空闲");
            if (!p->status) printf(" (进程%d)", p->pid + 1);
            printf("\n");
            p = p->next;
        }
    }
	for (int i = 0; i < PROCESS_NUM; i++)
	{
		BuddyBlock* block = allocated[i];
		if (block != NULL && block->status == 0) // 仅打印已分配且未回收的块
		{
			printf("[%d-%d] 大小:%d 占用 (进程%d)\n",
				block->startAddr, block->startAddr + block->size - 1,
				block->size, block->pid + 1);
		}
	}
    printf("\n");
}

// 找到合适层级
int get_level(int size) 
{
	if (size <= 0) return -1;
	// 找到大于等于size的最小2的幂次指数
	int exp = 0;
	while ((1 << exp) < size) exp++;
	if (exp > MAX_EXP) return -1;
	return exp - MIN_EXP;
}

//块划分
void split_block(int level) 
{
	BuddyBlock* block = freelist[level];//获取上一级空闲链表的第一个空闲块
    if (!block) return;
	freelist[level] = block->next;//将上一级空闲链表的头指针指向下一个块
	if (freelist[level])
	{
		freelist[level]->prev = NULL;//空闲链表后一块的前驱置空
	}
	int new_exp = (MIN_EXP + level) - 1; 
	int new_size = 1 << new_exp;         
	int new_level = level - 1;           
    BuddyBlock* b1 = (BuddyBlock*)malloc(sizeof(BuddyBlock));
    BuddyBlock* b2 = (BuddyBlock*)malloc(sizeof(BuddyBlock));
	if (!b1 || !b2) 
	{
		printf("内存分配失败！\n");
		if (b1) free(b1);
		if (b2) free(b2);
		// 将未分配的块重新加入空闲链表
		block->next = freelist[level];
		freelist[level] = block;
		return;
	}
    b1->startAddr = block->startAddr;
    b1->size = new_size;
    b1->level = new_level;
    b1->status = 1;
    b1->pid = -1;
	b1->prev = NULL;   
    b1->next = b2;
    b2->startAddr = block->startAddr + new_size;
    b2->size = new_size;
    b2->level = new_level;
    b2->status = 1;
    b2->pid = -1;
	b2->prev = b1;
    b2->next = freelist[new_level];//将b1b2插入表头
	
	if (freelist[new_level])
	{
		freelist[new_level]->prev = b2;
	}
	freelist[new_level] = b1;//更新表头指针

    printf("划分：[%d-%d] -> [%d-%d] 和 [%d-%d]\n", block->startAddr, block->startAddr + block->size - 1, b1->startAddr, b1->startAddr + b1->size - 1, b2->startAddr, b2->startAddr + b2->size - 1);
    free(block);
}

//内存分配
int buddy_alloc(int pid, int size)
{
	if (size <= 0 || size > MEMORY_SIZE) 
	{
		printf("无效的分配大小！\n");
		return 0;
	}
	int level = get_level(size);//确定合适层级
	for (int i = level; i <= MAX_LEVEL; i++)
	{
		if (freelist[i])
		{
			int current_level = i;
			while (current_level > level)//做块划分，直到到达合适层级
			{
				split_block(current_level);
				current_level--;
			}
			BuddyBlock* block = freelist[level];//b1用于分配
			if (!block)
			{
				continue;//直接找下一层级
			}
			freelist[level] = block->next;//b2做表头
			if (freelist[level]) 
			{
				freelist[level]->prev = NULL; //从空闲链表移除该块
			}
			block->status = 0;
			block->pid = pid;
			block->next = NULL;
			block->prev = NULL;
			allocated[pid] = block;
			printf("进程%d分配成功：[%d-%d] 大小:%d\n", pid + 1, block->startAddr, block->startAddr + block->size - 1, block->size);
			return 1; // 分配成功
		}
	}
	printf("进程%d分配失败：内存不足！\n", pid + 1); 
	return 0; // 分配失败
}

// 合并伙伴
void merge_free(BuddyBlock* block) 
{
    if (!block || block->status == 0) 
    {
        return; // 空分区或已占用，不处理
    }

    int level = block->level;
    // 最高层级直接加入空闲链表
    if (level >= MAX_LEVEL) 
	{
        block->prev = NULL;
        block->next = freelist[level];
        if (freelist[level]) 
        {
            freelist[level]->prev = block;
        }
        freelist[level] = block;
        return;
    }

    // 伙伴地址计算
    int buddy_addr = block->startAddr ^ block->size;
    BuddyBlock* current = freelist[level];
    BuddyBlock* found_buddy = NULL;

    //先遍历
    while (current) 
    {
        if (current->startAddr == buddy_addr && current->status == 1) 
        {
            found_buddy = current;
            break;
        }
        current = current->next;
    }

    //找到伙伴则合并，否则直接加入链表
    if (found_buddy)
    {
        // 移除伙伴块
        if (found_buddy->prev) 
        {
            found_buddy->prev->next = found_buddy->next;
        }
        else 
        {
            freelist[level] = found_buddy->next;
        }
        if (found_buddy->next) 
        {
            found_buddy->next->prev = found_buddy->prev;
        }

        // 合并块并更新层级
        BuddyBlock* merged_block = block;
        if (block->startAddr > found_buddy->startAddr) 
        {
            merged_block = found_buddy;
        }
        merged_block->size *= 2;
        merged_block->level += 1;

        // 释放被合并的块
        if (merged_block == block) 
        {
            free(found_buddy);
        }
        else 
        {
            free(block);
        }

        // 递归合并
        merge_free(merged_block);
    }
    else
    {
        // 无伙伴，加入当前层级空闲链表
        block->prev = NULL;
        block->next = freelist[level];
        if (freelist[level]) 
        {
            freelist[level]->prev = block;
        }
        freelist[level] = block;
    }
}

// 回收内存
void recycle(int pid)
{
    if (allocated[pid])
    {
        printf("回收进程%d的内存[%d-%d] 大小:%d\n",pid + 1, allocated[pid]->startAddr, allocated[pid]->startAddr + allocated[pid]->size - 1, allocated[pid]->size);
        allocated[pid]->status = 1;
        allocated[pid]->pid = -1;
		BuddyBlock* block = allocated[pid];
        allocated[pid] = NULL;
        merge_free(block);
        print_buddy();
    }
    else return;
}

// 测试用例1：固定分配
void test_case1()
{
	int sizes[3];
	sizes[0] = (int)pow(2, 7);
	sizes[1] = (int)pow(2, 4);
	sizes[2] = (int)pow(2, 8);
	for (int i = 0; i < 3; i++)
	{
		printf("分配进程%d 大小:%d\n", i + 1, sizes[i]);
		if (buddy_alloc(i, sizes[i]))
		{
			print_buddy();
		}
		else
		{
			printf("分配失败！\n");
		}
	}
	// 回收所有进程内存
	for (int i = 0; i < 3; i++)
	{
		recycle(i);
	}
}

// 测试用例2：随机分配
void test_case2()
{
	randSize(PROCESS_NUM);
	for (int i = 0; i < PROCESS_NUM; i++)
	{
		printf("分配进程%d 大小:%d\n", i + 1, process_size[i]);
		if (buddy_alloc(i, process_size[i]))
		{
			print_buddy();
		}
		else
		{
			printf("分配失败！\n");
		}
	}
	// 回收所有进程内存
	for (int i = 0; i < PROCESS_NUM; i++)
	{
		recycle(i);
	}
}

int main() 
{
    printf("Buddy System内存分配演示\n");
    init_memory();
    printf("请选择测试用例：1. 固定分配 2. 随机分配\n");
    int choice;
    scanf_s("%d", &choice);
    if (choice == 1) test_case1();
    else test_case2();
    return 0;
}