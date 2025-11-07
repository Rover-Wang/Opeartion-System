#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <time.h>

#define NUM_QUEUES 5        
#define MAX_PROCESSES 10    
#define SEMAPHORE_MAX 100   


HANDLE sema_a;             // 保护队列0的入队互斥
HANDLE sema_b;             // 通知调度器有进程可调度
HANDLE queueMutex;         // 保护队列并发访问
HANDLE outputMutex;        // 保护输出同步

struct PCB {
    char pid[64];           
    char state;             
    int priority;           
    int neededTime;         
    int usedTime;           
    int arrivalTime;        
    int totalWaitTime;      
    struct PCB* next;       
};

// 队列
struct Queue {
    struct PCB* front;      // 队头
    struct PCB* rear;       // 队尾
    int timeQuantum;        // 时间片大小
    int priority;           // 队列优先级
};

// 全局变量
struct Queue queues[NUM_QUEUES];  // 多级反馈队列数组
int completedProcesses = 0;       // 已完成进程数
int processCount = 0;             // 已创建进程数
DWORD startTime;                  // 程序开始时间


void initQueues();
void enqueue(struct Queue* queue, struct PCB* process);
struct PCB* dequeue(struct Queue* queue);
int hasProcesses();
void printAllQueues();
void updateProcessWaitTimes(int timeSlice, struct PCB* runningProcess);
void executor(int queueIndex, struct PCB* process);
DWORD WINAPI generator(LPVOID lpParam);
DWORD WINAPI scheduler(LPVOID lpParam);

// 初始化所有队列
void initQueues() 
{
    for (int i = 0; i < NUM_QUEUES; i++) 
    {
        queues[i].front = NULL;
        queues[i].rear = NULL;
        queues[i].timeQuantum = 10 * (1 << i);  // 10,20,40,80,160 ms
        queues[i].priority = i + 1;             // 优先级1-5
    }
}

// 检查是否有进程在队列中
int hasProcesses() 
{
    for (int i = 0; i < NUM_QUEUES; i++) 
    {
        if (queues[i].front != NULL) 
        {
            return 1;
        }
    }
    return 0;
}

// 入队
void enqueue(struct Queue* queue, struct PCB* process) 
{
    process->state = 'w'; // 就绪状态

    if (queue->rear == NULL) 
    {
        queue->front = queue->rear = process;
    }
    else {
        queue->rear->next = process;
        queue->rear = process;
    }
    process->next = NULL;
}

// 出队操作
struct PCB* dequeue(struct Queue* queue) 
{
    if (queue->front == NULL) return NULL;

    struct PCB* process = queue->front;
    queue->front = queue->front->next;
    if (queue->front == NULL) queue->rear = NULL;

    process->state = 'r'; // 运行状态
    return process;
}

// 更新其他进程的等待时间
void updateProcessWaitTimes(int timeSlice, struct PCB* runningProcess) 
{
    WaitForSingleObject(queueMutex, INFINITE);
    for (int i = 0; i < NUM_QUEUES; i++) 
    {
        struct PCB* p = queues[i].front;
        while (p != NULL) {
            if (p != runningProcess) 
            {
                p->totalWaitTime += timeSlice;
            }
            p = p->next;
        }
    }
    ReleaseMutex(queueMutex);
}

// 打印所有队列状态
void printAllQueues() 
{
    WaitForSingleObject(outputMutex, INFINITE);
    for (int i = 0; i < NUM_QUEUES; i++) 
    {
        printf("Queue %d: ", i);
        struct PCB* p = queues[i].front;
        if (p == NULL) 
        {
            printf("\n");
            continue;
        }
        while (p != NULL) 
        {
            printf("%s ", p->pid);
            p = p->next;
        }
        printf("\n");
    }
    ReleaseMutex(outputMutex);
}

// 执行进程
void executor(int queueIndex, struct PCB* process) 
{
    int timeSlice = queues[queueIndex].timeQuantum;

    WaitForSingleObject(outputMutex, INFINITE);
    printf("Executor: Process %s in queue %d consumes %d ms\n",
        process->pid, queueIndex, timeSlice);
    ReleaseMutex(outputMutex);

    // 模拟进程执行
    Sleep(timeSlice);
    process->usedTime += timeSlice;

    // 更新其他进程的等待时间
    updateProcessWaitTimes(timeSlice, process);
}

// 进程生成器线程
DWORD WINAPI generator(LPVOID lpParam) 
{
    srand((unsigned int)time(NULL));

    for (int i = 0; i < MAX_PROCESSES; i++) 
    {
        struct PCB* newProcess = (struct PCB*)malloc(sizeof(struct PCB));
        sprintf_s(newProcess->pid, sizeof(newProcess->pid), "P%d", i);
        newProcess->state = 'w';
        newProcess->priority = 1;
        newProcess->neededTime = (rand() % 199) + 2; // [2,200] ms
        newProcess->usedTime = 0;
        newProcess->arrivalTime = GetTickCount() - startTime;
        newProcess->totalWaitTime = 0;
        newProcess->next = NULL;

        // 入队到队列0
        WaitForSingleObject(sema_a, INFINITE);
        WaitForSingleObject(queueMutex, INFINITE);
        enqueue(&queues[0], newProcess);
        ReleaseMutex(queueMutex);
        ReleaseSemaphore(sema_a, 1, NULL);

        // 打印生成信息
        WaitForSingleObject(outputMutex, INFINITE);
        printf("Generator: Process %s is generated, neededTime = %d, arrivalTime = %d\n",
            newProcess->pid, newProcess->neededTime, newProcess->arrivalTime);
        ReleaseMutex(outputMutex);

        // 通知调度器有新进程
        ReleaseSemaphore(sema_b, 1, NULL);

        // 随机延迟
        int waitTime = rand() % 100 + 1; // [1,100] ms
        WaitForSingleObject(outputMutex, INFINITE);
        printf("Generator: Sleep for %d ms before generating next new process...\n", waitTime);
        ReleaseMutex(outputMutex);

        Sleep(waitTime);
        processCount++;
    }

    WaitForSingleObject(outputMutex, INFINITE);
    printf("Generator: All %d processes created.\n", MAX_PROCESSES);
    ReleaseMutex(outputMutex);

    return 0;
}

// 调度器线程
DWORD WINAPI scheduler(LPVOID lpParam) 
{
    while (completedProcesses < MAX_PROCESSES) 
    {
        // 等待有进程可调度
        WaitForSingleObject(sema_b, INFINITE);

        struct PCB* process = NULL;
        int queueIndex = -1;

        // 查找最高优先级的非空队列
        WaitForSingleObject(queueMutex, INFINITE);
        for (int i = 0; i < NUM_QUEUES; i++) 
        {
            if (queues[i].front != NULL) 
            {
                process = dequeue(&queues[i]);
                queueIndex = i;
                break;
            }
        }

        // 如果没有找到进程，继续等待
        if (process == NULL) 
        {
            ReleaseMutex(queueMutex);
            continue;
        }
        ReleaseMutex(queueMutex);

        // 执行进程
        executor(queueIndex, process);

        // 检查进程是否完成
        if (process->usedTime >= process->neededTime) 
        {
            WaitForSingleObject(outputMutex, INFINITE);
            printf("Scheduler: Process %s finished, total wait time = %d\n",
                process->pid, process->totalWaitTime);
            ReleaseMutex(outputMutex);

            free(process);
            completedProcesses++;

            // 如果还有进程，继续调度
            WaitForSingleObject(queueMutex, INFINITE);
            if (hasProcesses()) 
            {
                ReleaseSemaphore(sema_b, 1, NULL);
            }
            ReleaseMutex(queueMutex);
        }
        else 
        {
            // 进程未完成，降级到下一队列
            int nextQueue = (queueIndex + 1 < NUM_QUEUES) ? queueIndex + 1 : queueIndex;
            process->priority = queues[nextQueue].priority;

            WaitForSingleObject(queueMutex, INFINITE);
            enqueue(&queues[nextQueue], process);
            ReleaseMutex(queueMutex);

            WaitForSingleObject(outputMutex, INFINITE);
            printf("Scheduler: Process %s is moved to queue %d, priority = %d\n",
                process->pid, nextQueue, process->priority);
            ReleaseMutex(outputMutex);

            printAllQueues();

            // 继续调度
            ReleaseSemaphore(sema_b, 1, NULL);
        }
    }

    WaitForSingleObject(outputMutex, INFINITE);
    printf("Scheduler: All processes completed.\n");
    ReleaseMutex(outputMutex);

    return 0;
}

int main() 
{
    startTime = GetTickCount();

    // 初始化队列
    initQueues();

    // 创建同步对象
    sema_a = CreateSemaphore(NULL, 1, 1, NULL);
    sema_b = CreateSemaphore(NULL, 0, SEMAPHORE_MAX, NULL);
    queueMutex = CreateMutex(NULL, FALSE, NULL);
    outputMutex = CreateMutex(NULL, FALSE, NULL);

    // 创建线程
    HANDLE hGenerator = CreateThread(NULL, 0, generator, NULL, 0, NULL);
    HANDLE hScheduler = CreateThread(NULL, 0, scheduler, NULL, 0, NULL);

    // 等待线程结束
    WaitForSingleObject(hGenerator, INFINITE);
    WaitForSingleObject(hScheduler, INFINITE);

    // 释放资源
    CloseHandle(hGenerator);
    CloseHandle(hScheduler);
    CloseHandle(sema_a);
    CloseHandle(sema_b);
    CloseHandle(queueMutex);
    CloseHandle(outputMutex);

    return 0;
}