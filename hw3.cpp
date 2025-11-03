#include <stdlib.h>     //包含随机数产生函数
#include <stdio.h >     //标准输入输出函数库
#include <time.h >     //与时间有关的函数头文件
#include <string.h>    //字符串操作函数头文件

typedef struct PCB {
    char pid[64];       //进程标识符，即进程的名字

    //以下部分为用于进程调度的信息
    char state;         //‘r’: 运行状态；‘w’:就绪状态
    int priority;       //进程优先级
    int neededTime;     //进程需要的运行时间 
    int totalWaitTime;  //进程累计已等待的CPU时间 
	int arrivalTime;   //进程到达时间
	int startTime;     //进程开始时间
	int endTime;       //进程结束时间
	float responseRatio; //响应比（仅HRRN调度算法使用）
    //以下部分为进程的控制信息
    struct PCB* next;   //指向下一个PCB的链表指针

}PCB;

struct PCB* processList = NULL;  // 进程链表
int currentTime = 0;             // 当前模拟时间
int totalWaitTime = 0;           // 总等待时间

void calculateWaitTime();
void createProcess();
void printProcesses();
struct PCB* getNextProcessSJF();
void SJF_Scheduling();
struct PCB* getNextProcessHRRN();

void createProcess()
{
    srand((unsigned)time(NULL));
    for (int i = 0; i < 10; i++)
    {
        PCB* newProcess = (PCB*)malloc(sizeof(PCB));
        sprintf_s(newProcess->pid, "P%d", i + 1);
        newProcess->state = 'w'; // 初始状态为就绪
        newProcess->priority = 0;
        newProcess->neededTime = rand() % 41 + 10;
        newProcess->totalWaitTime = 0;
        newProcess->arrivalTime = rand() % 20 + 1;
        newProcess->next = NULL;

        // 将新进程添加到链表
        if (processList == NULL)
        {
            processList = newProcess;
        }
        else
        {
            struct PCB* temp = processList;
            while (temp->next != NULL)
            {
                temp = temp->next;
            }
            temp->next = newProcess;
        }
    }
}


// 打印所有进程信息
void printProcesses() 
{
    printf("进程信息:\n");
    printf("PID\t到达时间\t运行时间\t状态\n");
    printf("------------------------------------------------\n");

    struct PCB* temp = processList;
    while (temp != NULL) 
    {
        printf("%s\t%d\t\t%d\t\t%c\n",
            temp->pid, temp->arrivalTime, temp->neededTime, temp->state);
        temp = temp->next;//遍历链表
    }
    printf("\n");
}

// SJF排队器
struct PCB* getNextProcessSJF() 
{
    struct PCB* selected = NULL;
    struct PCB* temp = processList;

    while (temp != NULL) 
    {
        // 考虑已到达且未完成的进程
        if (temp->arrivalTime <= currentTime && temp->state != 'f') 
        {
            if (selected == NULL || temp->neededTime < selected->neededTime) 
            {
                selected = temp;
            }
        }
        temp = temp->next;
    }

    return selected;
}
//SJF
void SJF_Scheduling() 
{
    printf("SJF执行过程\n");
    //初始化
    currentTime = 0;
    int completed = 0;
    int totalProcesses = 0;

    // 计算总进程数
    struct PCB* temp = processList;
    while (temp != NULL) 
    {
        totalProcesses++;
        temp = temp->next;
    }

    printf("执行顺序: ");
    while (completed < totalProcesses) 
    {
        struct PCB* nextProcess = getNextProcessSJF();

        if (nextProcess != NULL) 
        {
            // 更新当前时间
            if (currentTime < nextProcess->arrivalTime) 
            {
                currentTime = nextProcess->arrivalTime;
            }

            // 设置进程开始时间
            nextProcess->startTime = currentTime;
            nextProcess->state = 'r';

            // 输出执行信息
            printf("%s ", nextProcess->pid);

            currentTime += nextProcess->neededTime;
            nextProcess->endTime = currentTime;
            nextProcess->state = 'f';  // 完成状态

            completed++;
        }
        else 
        {
            currentTime++;  // 如果没有就绪进程，时间前进
        }
    }
    printf("\n\n");

    // 计算等待时间并输出结果
    calculateWaitTime();
}
//HRRN调度算法
void HRRN_Scheduling() 
{
    printf("HRRN调度算法执行过程\n");
    currentTime = 0;
    int completed = 0;
    int totalProcesses = 0;

    // 重置所有进程状态
    struct PCB* temp = processList;
    while (temp != NULL) 
    {
        temp->state = 'w';
        temp->startTime = -1;
        temp->endTime = -1;
        temp->totalWaitTime = 0;
        totalProcesses++;
        temp = temp->next;
    }

    printf("执行顺序: ");
    while (completed < totalProcesses) 
    {
        struct PCB* nextProcess = getNextProcessHRRN();

        if (nextProcess != NULL) {
            // 更新当前时间
            if (currentTime < nextProcess->arrivalTime) 
            {
                currentTime = nextProcess->arrivalTime;
            }

            // 设置进程开始时间
            nextProcess->startTime = currentTime;
            nextProcess->state = 'r';

            // 输出执行信息
            printf("%s ", nextProcess->pid);

            // 模拟进程执行
            currentTime += nextProcess->neededTime;
            nextProcess->endTime = currentTime;
            nextProcess->state = 'f';

            completed++;
        }
        else {
            currentTime++;
        }
    }
    printf("\n\n");

    // 计算等待时间并输出结果
    calculateWaitTime();
}

// HRRN排队器
struct PCB* getNextProcessHRRN() 
{
    struct PCB* selected = NULL;
    float maxRatio = -1.0;
    struct PCB* temp = processList;

    // 计算所有就绪进程的响应比
    while (temp != NULL) 
    {
        if (temp->arrivalTime <= currentTime && temp->state != 'f') 
        {
            // 响应比 = (等待时间 + 需要运行时间) / 需要运行时间
            int waitTime = currentTime - temp->arrivalTime;
            temp->responseRatio = (float)(waitTime + temp->neededTime) / temp->neededTime;

            if (temp->responseRatio > maxRatio) 
            {
                maxRatio = temp->responseRatio;
                selected = temp;
            }
        }
        temp = temp->next;
    }

    return selected;
}

// 计算等待时间和平均等待时间
void calculateWaitTime() 
{
    printf("进程执行详情:\n");
    printf("PID\t到达时间\t运行时间\t开始时间\t结束时间\t等待时间\n");
    printf("------------------------------------------------------------------------\n");

    totalWaitTime = 0;
    struct PCB* temp = processList;

    while (temp != NULL) 
    {
        if (temp->startTime != -1) 
        {
            int waitTime = temp->startTime - temp->arrivalTime;
            temp->totalWaitTime = waitTime;
            totalWaitTime += waitTime;

            printf("%s\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n",
                temp->pid, temp->arrivalTime, temp->neededTime,
                temp->startTime, temp->endTime, waitTime);
        }
        temp = temp->next;
    }

    float avgWaitTime = (float)totalWaitTime / 10.0;
    printf("\n总等待时间: %d\n", totalWaitTime);
    printf("平均等待时间: %.2f\n\n", avgWaitTime);
}


// 释放进程链表内存
void freeProcessList() 
{
    struct PCB* temp = processList;
    while (temp != NULL) 
    {
        struct PCB* toDelete = temp;
        temp = temp->next;
        free(toDelete);
    }
    processList = NULL;
}

int main() 
{
	createProcess();
    printProcesses();
    

    // 执行SJF调度算法
    SJF_Scheduling();

    // 重置进程状态以进行HRRN调度
    struct PCB* temp = processList;
    while (temp != NULL) 
    {
        temp->state = 'w';
        temp->startTime = -1;
        temp->endTime = -1;
        temp->totalWaitTime = 0;
        temp = temp->next;
    }

    // 执行HRRN调度算法
    HRRN_Scheduling();

    // 释放内存
    freeProcessList();

    return 0;
}