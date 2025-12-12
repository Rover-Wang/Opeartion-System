#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 10 //进程页数10
#define M 3 //物理页数3
#define K 20 // 访问序列长度

//随机函数随机产生k=20个页面的访问序列
void generate_string(int ref_str[],int length, int num_pages)
{
	srand((unsigned)time(NULL));
	for (int i = 0; i < K; i++)
	{
		ref_str[i] = rand() % num_pages;//模进程页数
	}
	printf("页面访问序列：\n");
	for (int i = 0; i < length; i++)
	{
		printf("%d ", ref_str[i]);
	}
	printf("\n");
}

//LRU页面置换算法
void LRU(int ref_str[], int length)//传参访问序列与序列长度
{
	int frame[M]; //物理块
	int time[M];  //时间戳
	int i, j, k, pos, max, page_faults = 0;

	//初始化物理块和时间戳
	for (i = 0; i < M; i++)
	{
		frame[i] = -1;
		time[i] = 0;
	}

	for (i = 0; i < length; i++)
	{
		int page = ref_str[i];//导入遍历到的序列页面
		int found = 0;

		//检查页面是否在物理块中
		for (j = 0; j < M; j++)
		{
			if (frame[j] == page)
			{
				found = 1;
				time[j] = i; //更新时间戳
				break;
			}
		}

		//页面未命中，进行置换
		if (!found)
		{
			page_faults++; //未命中数++，用于计算缺页率
			//查找有无空闲页
			for (j = 0; j < M; j++)
			{
				if (frame[j] == -1)//有空闲页
				{
					frame[j] = page; //更新物理页面状态
					time[j] = i;
					found = 1;
					break;
				}
			}

			//如果没有空闲块，进行LRU置换
			if (!found)
			{
				max = time[0];
				pos = 0;
				for (j = 1; j < M; j++)
				{
					if (time[j] < max)//比较时间戳（按照页面数组码的大小），找出最久未使用的页面
					{
						max = time[j];
						pos = j;
					}
				}
				frame[pos] = page;
				time[pos] = i;//更新时间戳
			}
		}

		//打印当前物理块状态
		printf("访问页面 %d: ", page);
		for (j = 0; j < M; j++)
		{
			if (frame[j] != -1)
				printf("%d ", frame[j]);
			else
				printf("- ");
		}
		printf("\n");
	}

	printf("缺页次数: %d\n", page_faults);
	printf("缺页率: %.2f%%\n", (float)page_faults / length * 100);
}

int main()
{
	int ref_str[K]; 
	generate_string(ref_str, K, N); 
	LRU(ref_str, K); //LRU算法

	return 0;
}