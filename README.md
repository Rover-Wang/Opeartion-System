# 操作系统原理实验手册

## 实验1 线程、信号量、线程同步

### 实验准备

“操作系统原理实验”这门课程以C/C++编程语言为基础，通过编程来实现“操作系统原理”这门课中学习到的各种机制、方法、策略的算法实现和模拟。

#### 1. 实验平台准备

- 使用C/C++编程语言

安装Visual Studio C++，或者是Eclipse+C语言插件

#### 2. 相关的随机数、线程、信号量、临界区函数

- 首先，C程序头的文件中需要包含进操作系统的库函数

  #include “stdlib.h”     //包含随机数产生函数

  #include “time.h”       //与时间有关的函数头文件

  #include “windows.h”    //针对Windows操作系统

  #include “pthread.h”    //针对linux操作系统的多线程头文件

  #include <sys/sem.h>      //针对linux操作系统的信号量头文件

  #include <string>         //字符串处理头文件

- 使用到的相关函数
  ##### 1. 随机数种子产生函数

` `srand(unsigned int seed);      //随机数种子函数；只运行1次  

` `例如，srand((unsigned)time(NULL)); //用当前时间做随机数种子，

`                                           `//使得每次运行rand()时产      //生的随机数序列都不相同

1. 随机数产生函数

` `rand();      //每调用一次,产生一个[0,RAND\_MAX]之间的整数

例如，rand() % 500       //产生[0,500]之间的一个随机整数；

1. 睡眠等待函数

Sleep(int millisecond); //睡眠等待一定时间，会造成OS重新调度

//其它的线程运行；

` `例如，Sleep(10);   //当前线程睡眠10毫秒后重新执行

1. 创建进程

CreateProcess(

LPSECURITY\_ATTRIBUTES // 是否继承进程句柄

LPSECURITY\_ATTRIBUTES //是否继承线程句柄

BOOL bInheritHandles //是否继承句柄

DWORD dwCreationFlags //有没有创建标志

LPVOID lpEnvironment // 是否使用父进程环境变量

LPCTSTR lpCurrentDirectory //使用父进程目录作为当前目录，

可以自己设置目录

LPSTARTUPINFO lpStartupInfo //STARTUPINFOW结构体详细信

息（启动状态相关信息）

LPPROCESS\_INFORMATION //PROCESS\_INFORMATION结构体进程信

息

);

1. 启动线程

CreateThread(ThreadAttribures, stack\_size, ThreadFunctionAddress, Parameters, CreationFlags, ThreadID);

例如，HANDLE t1 = CreateThread(NULL,0,Func,NULL,0,&ThreadID);

1. 定义信号量Semaphore

`       `例如，HANDLE sema;

1. 创建信号量：

CreateSemaphore(Attributes,InitialCount, MaxCount, SemaphoreID);

例如，sema = CreateSemaphore(NULL, 0, 1, NULL);

1. 申请访问信号量

WaitForSingleObject(HANDLE, millisecond);  //等同于wait(...)

调用该函数后，如果信号量为0，则线程阻塞；否则信号量当前值减1，然后继续执行下一行语句；

例如，WaitForSingleObject(sema,INFINITE);

1. 释放信号量

ReleaseSemaphore(HANDLE, releaseCount, \*PreviousCount); //等同于Signal(...)

调用该函数后，信号量当前值加上releastCount，然后继续执行下一行语句；

例如，ReleaseSemaphore(sema,1,NULL);    //信号量加1

1. 定义临界区

例如，CRITICAL\_SECTION cs;

**注意：**在Linux中没有对应于Windows中的临界区Critical Section函数； 因此，一般在Linux中用POSIX标准的信号量mutex来实现Windows中critical section的同样功能；

1. 临界区

例如，InitializeCriticalSection(&cs);

1. 进入和退出临界区

例如，

EnterCriticalSection(&cs);   //进入临界区

临界区代码；

LeaveCriticalSection(&cs);   //退出临界区

1. 关闭信号量/线程/临界区

例如，

CloseHandle(sema);

CloseHandle(t1);

DeleteCriticalSection(&cs);

1. 有用的参考网址

<http://www.cplusplus.com/>    //**强烈推荐；**英文版C/C++参考文献

<https://msdn.microsoft.com/en-us/>       //微软官方参考资料

1. 实验内容：线程、信号量、线程同步
2. 编写程序，在程序中根据用户输入的可执行程序名称，创建一个进程来运行该可执行程序，并输出该进程的PID。

3. 假设有四个线程，第一个线程输出 “This”，第二个线程输出 “is”, 第三个线程输出 “Jinan”, 第四个线程输出 “University！”。编制程序，在主程序main函数中四个线程依次启动，设计信号量(Semaphore)同步机制，当主程序运行时，输出的结果是字符串“This is Jinan University!”

4. 本实验题基于实验题目1。在主函数中依次启动四个线程，修改主程序，使得给定任意整数n，结果输出n个同样的字符串“This is Jinan University!”

