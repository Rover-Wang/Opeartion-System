#include <stdlib.h>     //包含随机数产生函数
#include <time.h>       //与时间有关的函数头文件
#include <windows.h>    //针对Windows操作系统
#include <stdio.h>

int bank_balance = 10;
CRITICAL_SECTION cs;
//HANDLE can_withdraw = CreateSemaphore(NULL, 0, LONG_MAX, NULL); // 信号量，初始值为0

typedef struct {
    char name[20];
    int num;
    int thread_id;
	int is_deposit; // 1表示存钱，0表示取钱
}Transaction;

//存钱线程函数
DWORD WINAPI Deposit(LPVOID lpParam) 
{
    Transaction* info = (Transaction*)lpParam;
    // 随机延迟，模拟操作耗时
    Sleep(rand() % 100);
    EnterCriticalSection(&cs);
    bank_balance += info->num;
    printf("thread_id:%d %s存了%d元，当前余额：%d元\n", info->thread_id, info->name,info->num,bank_balance);
    //ReleaseSemaphore(can_withdraw, 1, NULL);
    LeaveCriticalSection(&cs);
    free(info);
    return 0;
}

//信用卡取钱线程函数
DWORD WINAPI Withdraw(LPVOID lpParam)
{
    Transaction* info = (Transaction*)lpParam;
    int output =  info->num;
    // 随机延迟，模拟操作耗时
    Sleep(rand() % 100);
    EnterCriticalSection(&cs);
    bank_balance -= output;
    printf("thread_id:%d %s取了%d元，当前余额：%d元\n", info->thread_id, info->name, output, bank_balance);
    LeaveCriticalSection(&cs);
    free(info);
    return 0;
}
/*
DWORD WINAPI Withdraw(LPVOID lpParam) 
{
    Transaction* info = (Transaction*)lpParam;
    Sleep(rand() % 100);
    int can_proceed = 0;
    while (!can_proceed) 
    {
        EnterCriticalSection(&cs);
        if (bank_balance >= info->num) 
        {
            // 余额足够，执行取款
            bank_balance -= info->num;
            printf("thread_id:%d %s取出%d元，当前余额：%d元\n",
                info->thread_id, info->name, info->num, bank_balance);
            can_proceed = 1;
            LeaveCriticalSection(&cs);
        }
        else 
        {
            // 余额不足，等待信号量"
            printf("thread_id:%d %s想取%d元，当前余额：%d元,余额不足，等待中\n",
                info->thread_id, info->name, info->num, bank_balance);
            LeaveCriticalSection(&cs);
            WaitForSingleObject(can_withdraw, INFINITE);
        }
    }

    free(info);
    return 0;
}
*/
int main()
{
    printf("初始账户余额：%d 元\n", bank_balance);

    srand((unsigned int)time(NULL));
    InitializeCriticalSection(&cs);

    HANDLE threads[14];  // 14个线程
    int thread_idx = 0;
    Transaction all_transactions[14];

    struct {
        char name[20];
        int num;
    }depositors[] = {
        {"爸爸",10},{"妈妈",20},{"奶奶",30},{"爷爷",40},{"舅舅",50}
    };
    int dep_num = sizeof(depositors) / sizeof(depositors[0]);

    struct {
        char name[20];
        int num;
    }withdrawers[] = {
        {"Mary",50},{"Sally",100}
    };
    int with_num = sizeof(withdrawers) / sizeof(withdrawers[0]);

    // 存款交易
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 2; j++) {
            strcpy_s(all_transactions[thread_idx].name, sizeof(all_transactions[thread_idx].name), depositors[i].name);
            all_transactions[thread_idx].num = depositors[i].num;
            all_transactions[thread_idx].is_deposit = 1;
            thread_idx++;
        }
    }

    // 取款交易
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            strcpy_s(all_transactions[thread_idx].name, sizeof(all_transactions[thread_idx].name), withdrawers[i].name);
            all_transactions[thread_idx].num = withdrawers[i].num;
            all_transactions[thread_idx].is_deposit = 0;
            thread_idx++;
        }
    }

    // 随机打乱交易顺序
    for (int i = thread_idx - 1; i > 0; i--) 
    {
        int j = rand() % (i + 1);
        Transaction temp = all_transactions[i];
        all_transactions[i] = all_transactions[j];
        all_transactions[j] = temp;
    }

    // 按随机顺序创建线程
    for (int i = 0; i < 14; i++) 
    {
        Transaction* trans = (Transaction*)malloc(sizeof(Transaction));
        memcpy(trans, &all_transactions[i], sizeof(Transaction));
        trans->thread_id = i + 1;

        if (trans->is_deposit) {
            threads[i] = CreateThread(NULL, 0, Deposit, trans, 0, NULL);
        }
        else {
            threads[i] = CreateThread(NULL, 0, Withdraw, trans, 0, NULL);
        }

        if (threads[i] == NULL) {
            printf("创建线程失败！\n");
            free(trans);
        }
    }

    // 等待所有线程完成
    WaitForMultipleObjects(14, threads, TRUE, INFINITE);

    // 清理资源
    for (int i = 0; i < 14; i++) {
        if (threads[i] != NULL) {
            CloseHandle(threads[i]);
        }
    }
    DeleteCriticalSection(&cs);

    // 输出最终余额
    printf("\n最终账户余额：%d 元\n", bank_balance);

    return 0;
}

