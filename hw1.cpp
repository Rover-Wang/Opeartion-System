#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <io.h> //为了使控制台能打印UTF-16宽字符
#include <fcntl.h> //为了使控制台能打印UTF-16宽字符

//（1）
int main() 
{
    wchar_t exeName[256];
    _setmode(_fileno(stdout), _O_U16TEXT); //设置控制台为Unicode模式
    wprintf(L"请输入可执行程序的路径（例如： C:\\Windows\\System32\\notepad.exe）：\n");
    
    wscanf_s(L"%s", exeName, (unsigned int)sizeof(exeName) / sizeof(wchar_t));

    PROCESS_INFORMATION pi; //输出结果
    STARTUPINFO si;  //输入配置，给系统传参

	// 初始化结构体
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // 定义创建进程是否成功
    BOOL success = CreateProcessW(
        exeName,          // 可执行程序名称（宽字符）
        NULL,             // 命令行参数
        NULL,             // 进程安全属性
        NULL,             // 线程安全属性
        FALSE,            // 不继承句柄
        0,                // 创建标志
        NULL,             // 使用父进程环境变量
        NULL,             // 使用父进程当前目录
        &si,              // 启动信息
        &pi               // 进程信息
    );

    if (!success)
    {
        DWORD errorCode = GetLastError();
        if (errorCode == ERROR_FILE_NOT_FOUND)
        {
            wprintf(L"错误: 找不到指定的文件 - %s\n", exeName);
            wprintf(L"请检查路径是否正确，确保文件存在\n");
        }
        else if (errorCode == ERROR_PATH_NOT_FOUND)
        {
            wprintf(L"错误: 找不到指定的路径 - %s\n", exeName);
            wprintf(L"请检查路径是否正确\n");
        }
        else
        {
            wprintf(L"创建进程失败，错误代码: %d\n", errorCode);
        }
        return 1;
    }

    // 输出进程ID
    wprintf(L"成功创建进程，PID: %d\n", pi.dwProcessId);

    // 等待进程结束
    WaitForSingleObject(pi.hProcess, INFINITE);
    wprintf(L"进程已结束\n");

    // 关闭句柄
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}


//（2）

// 定义四个信号量的句柄
HANDLE sem1, sem2, sem3, sem4;

// 线程1
DWORD WINAPI Thread1(LPVOID lpParam) 
{
    // 等待第一个信号量
    WaitForSingleObject(sem1, INFINITE); //INFINITE：无限等待直到信号量到来
    printf("This");
    // 释放第二个信号量，允许线程2执行
    ReleaseSemaphore(sem2, 1, NULL);
    return 0;
}

// 线程2
DWORD WINAPI Thread2(LPVOID lpParam) 
{
    WaitForSingleObject(sem2, INFINITE);
    printf(" is");
    // 释放第三个信号量，允许线程3执行
    ReleaseSemaphore(sem3, 1, NULL);
    return 0;
}

// 线程3
DWORD WINAPI Thread3(LPVOID lpParam) 
{
    WaitForSingleObject(sem3, INFINITE);
    printf(" Jinan");
    // 释放第四个信号量，允许线程4执行
    ReleaseSemaphore(sem4, 1, NULL);
    return 0;
}

// 线程4
DWORD WINAPI Thread4(LPVOID lpParam) 
{
    WaitForSingleObject(sem4, INFINITE);
    printf(" University!");
    return 0;
}

int main() 
{
	//存储四个线程的句柄和ID
    HANDLE threads[4];
    DWORD threadIds[4];

    // 创建信号量
    // sem1初始值为1，允许第一个线程先执行
    sem1 = CreateSemaphore(NULL, 1, 1, NULL);
    // 其他信号量初始值为0，等待前一个线程释放
    sem2 = CreateSemaphore(NULL, 0, 1, NULL);
    sem3 = CreateSemaphore(NULL, 0, 1, NULL);
    sem4 = CreateSemaphore(NULL, 0, 1, NULL);

    // 创建四个线程
    threads[0] = CreateThread(NULL, 0, Thread1, NULL, 0, &threadIds[0]);
    threads[1] = CreateThread(NULL, 0, Thread2, NULL, 0, &threadIds[1]);
    threads[2] = CreateThread(NULL, 0, Thread3, NULL, 0, &threadIds[2]);
    threads[3] = CreateThread(NULL, 0, Thread4, NULL, 0, &threadIds[3]);

    // 等待所有线程执行完毕
    WaitForMultipleObjects(4, threads, TRUE, INFINITE);

    // 打印换行
    printf("\n");

    // 关闭线程和信号量句柄，释放资源
    for (int i = 0; i < 4; i++) 
    {
        CloseHandle(threads[i]);
    }
    CloseHandle(sem1);
    CloseHandle(sem2);
    CloseHandle(sem3);
    CloseHandle(sem4);

    return 0;
}



//（3）

HANDLE sem1, sem2, sem3, sem4;

DWORD WINAPI Thread1(LPVOID lpParam)
{
    WaitForSingleObject(sem1, INFINITE); //INFINITE：无限等待直到信号量到来
    printf("This");
    ReleaseSemaphore(sem2, 1, NULL);
    return 0;
}

// 线程2
DWORD WINAPI Thread2(LPVOID lpParam)
{
    WaitForSingleObject(sem2, INFINITE);
    printf(" is");
    ReleaseSemaphore(sem3, 1, NULL);
    return 0;
}

DWORD WINAPI Thread3(LPVOID lpParam)
{
    WaitForSingleObject(sem3, INFINITE);
    printf(" Jinan");
    ReleaseSemaphore(sem4, 1, NULL);
    return 0;
}

DWORD WINAPI Thread4(LPVOID lpParam)
{
    WaitForSingleObject(sem4, INFINITE);
    printf(" University!");
    return 0;
}

int main()
{
    int n = 0;
    printf("请输入语句重复次数：\n");
    scanf_s("%d", &n);

    HANDLE threads[4];
    DWORD threadIds[4];

    sem1 = CreateSemaphore(NULL, 1, 1, NULL);
    sem2 = CreateSemaphore(NULL, 0, 1, NULL);
    sem3 = CreateSemaphore(NULL, 0, 1, NULL);
    sem4 = CreateSemaphore(NULL, 0, 1, NULL);

    for (int i = 0; i < n; i++)
    {
        // 每次循环重新创建线程
        threads[0] = CreateThread(NULL, 0, Thread1, NULL, 0, &threadIds[0]);
        threads[1] = CreateThread(NULL, 0, Thread2, NULL, 0, &threadIds[1]);
        threads[2] = CreateThread(NULL, 0, Thread3, NULL, 0, &threadIds[2]);
        threads[3] = CreateThread(NULL, 0, Thread4, NULL, 0, &threadIds[3]);

        // 释放第一个信号量，启动线程 1
        ReleaseSemaphore(sem1, 1, NULL);

        // 等待所有线程完成一次输出
        WaitForMultipleObjects(4, threads, TRUE, INFINITE);
        printf("\n");

        // 关闭线程句柄
        for (int j = 0; j < 4; j++)
        {
            CloseHandle(threads[j]);
        }
    }
    CloseHandle(sem1);
    CloseHandle(sem2);
    CloseHandle(sem3);
    CloseHandle(sem4);
    return 0;
}