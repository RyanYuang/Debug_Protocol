#include "PerformanceTest.h"

#if PERFORMACE == 1

void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // 启用 DWT
    DWT->CYCCNT = 0; // 清零计数器
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; // 启用周期计数器
}

// 获取微秒级时间戳
uint32_t DWT_GetMicrosecond(void)
{
    uint32_t cycles = DWT->CYCCNT;
    return cycles / (HAL_RCC_GetHCLKFreq() / 1000000); // 转换为微秒
}


void Show_Performance(void)
{
	if(Tick_Rec_Finish_Flag)
	{
		printf("Start Tick:%d\r\n Head Process Tick:%d\r\n Name Process Tick:%d\r\n Data Process Tick:%d\r\n Total Tick:%d\r\n",

								Start_Tick,
								/* 接收到头的时间 - 刚开始函数的时间 */
								Head_Tick - Start_Tick,
								/* 完成变量名处理的时间 - 完成头部接收的时间 */
								Name_Tick - Head_Tick,
								/* 完成数据处理的时间 - 完成名称处理接收的时间 */
								Data_Tick - Name_Tick,
								Total_Tick - Start_Tick);
		Tick_Rec_Finish_Flag = 0;
		Name_Tick = 0;
		Data_Tick = 0;
		Head_Tick = 0;
		Total_Tick = 0;
		Start_Tick = 0;

	}
}

#endif

