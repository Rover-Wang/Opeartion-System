#include<stdio.h>
#include<stdlib.h>
#include <time.h>

#define N 5 //进程数
#define M 3 //资源种类数

//全局向量
int initAvailable[M];   //系统初始可用资源向量
int Available[M];          //系统可用资源向量
int Max[N][M];            //进程最大需求矩阵
int Allocation[N][M];     //进程已分配资源矩阵
int Need[N][M];          //进程需求矩阵

//函数声明
void GenerateResources();
void PrintResources();	
int BankerAlgorithm();

int main()
{
	srand((unsigned int)time(NULL)); //初始化随机数种子
	printf("实验参数：进程数n=%d，资源种类数m=%d\n", N, M);

	//随机生成资源数据
	GenerateResources();

	//打印资源矩阵
	printf("Initial Available:\n");
	PrintResources();

	//银行家算法检测安全序列
	int isSafe = BankerAlgorithm();
	//结果判断
	if (isSafe)
	{
		printf("存在安全序列\n");
	}
	else 
	{
		printf("\nDeadlock！\n");
	}

	return 0;
}

void GenerateResources() 
{
	int i, j;
	for (j = 0; j < M; j++)
	{
		initAvailable[j] = rand() % 10 + 1; 
		Available[j] = initAvailable[j];
	}

	for (i = 0; i < N; i++)
	{
		for (j = 0; j < M; j++)
		{
			Max[i][j] = rand() % initAvailable[j] + 1; //进程最大需求在1-Available之间
		}

	}

	int resourceExhausted[M] = { 0 }; //标记分配资源是否已耗尽
	for (i = 0; i < N; i++)
	{
		for (j = 0; j < M; j++)
		{
			if (resourceExhausted[j])
			{
				Allocation[i][j] = 0; //资源已耗尽，当前与后续所有进程分配为0
				continue;
			}
			if (rand() % 2 == 0)
			{
				Allocation[i][j] = 0;
			}
			else
			{
				int maxFeasible = (Available[j] < Max[i][j]) ? Available[j] : Max[i][j];
				if (maxFeasible >= 1)
				{
					Allocation[i][j] = rand() % maxFeasible + 1;
					Available[j] -= Allocation[i][j];
				}
				else
				{
					Allocation[i][j] = 0;
				}
			}

			if (Available[j] == 0)
			{
				resourceExhausted[j] = 1; //标记资源已耗尽
			}
		}

	}

	//更新Need矩阵	
	for (i = 0; i < N; i++)
	{
		for (j = 0; j < M; j++)
		{
			Need[i][j] = Max[i][j] - Allocation[i][j];
		}	
	}
}

void PrintResources()
{
	int i, j;
	
	for (j = 0; j < M; j++)
	{
		printf("%d ", initAvailable[j]);
	}
	printf("\n");

	printf("\n进程最大需求矩阵Max:\n");
	printf("     R1  R2  R3\n");
	for (i = 0; i < N; i++)
	{
		printf("P%d: ", i);
		for (j = 0; j < M; j++)
		{
			printf("%3d ", Max[i][j]);//%3d右对齐
		}
		printf("\n");
	}

	printf("\n进程已分配资源矩阵Allocation:\n");
	printf("     R1  R2  R3\n");
	for (i = 0; i < N; i++)
	{
		printf("P%d: ", i);
		for (j = 0; j < M; j++)
		{
			printf("%3d ", Allocation[i][j]);
		}
		printf("\n");
	}

	printf("\n进程可分配资源Available:\n");
	printf(" R1  R2  R3\n");
	for (j = 0; j < M; j++)
	{
		printf("%3d ", Available[j]);//%3d右对齐
	}
	printf("\n");

	printf("\nwork:\n");
	printf(" R1  R2  R3\n");
	for (j = 0; j < M; j++)
	{
		printf("%3d ", Available[j]);//%3d右对齐
	}
	printf("\n");

	printf("\n进程需求矩阵Need:\n");
	printf("     R1  R2  R3\n");
	for (i = 0; i < N; i++)
	{
		printf("P%d: ", i);
		for (j = 0; j < M; j++)
		{
			printf("%3d ", Need[i][j]);
		}
		printf("\n");
	}
}

int BankerAlgorithm()
{
	int Work[M]; //工作向量，表示系统当前可用资源
	int Finish[N] = { 0 }; //初始化Finish向量未完成
	int safeSequence[N];
	int count = 0;

	//初始化Work向量
	for (int j = 0; j < M; j++)
	{
		Work[j] = Available[j];
	}

	//循环查找安全序列
	while (count < N)
	{
		int found = 0; //初始化标记，还未找到进程
		for (int i = 0; i < N; i++)
		{
			if (Finish[i] == 1) continue; //进程i已完成，跳过
			else
			{
				int canAllocate = 1; //假设可以分配资源
				int j;
				for (j = 0; j < M; j++)
				{
					if (Need[i][j] > Work[j])
					{
						canAllocate = 0;
						break; //资源不足，跳出循环
					}
				}
				if (canAllocate) //所有资源需求都满足
				{
					//释放资源
					for (int k = 0; k < M; k++)
					{
						Work[k] += Allocation[i][k];
					}
					safeSequence[count++] = i; //记录安全序列
					Finish[i] = 1; //标记进程i已完成
					// 打印当前进程完成后的Work变化
					printf("P%d 完成后，Work: ", i);
					for (int k = 0; k < M; k++)
					{
						printf("%d ", Work[k]);
					}
					printf("\n");
					found = 1; // 找到一个进程
					break; // 跳出进程循环，重新开始查找
				

				}
			}
		}
		// 若本次循环未找到任何可完成的进程，且还有进程未完成→死锁
		if (!found)
		{
			return 0;
		}
	}
	//打印安全序列
	printf("\n系统安全序列为: ");
	for (int i = 0; i < N; i++)
	{
		printf("P%d ", safeSequence[i]);
	}
	printf("\n");
	return 1; //存在安全序列
}