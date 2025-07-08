/*
 * PerformanceTest.h
 *
 *  Created on: Jul 7, 2025
 *      Author: RyanY
 */

#ifndef PERFORMANCETEST_H_
#define PERFORMANCETEST_H_
/* Include */
#include "HeadFiles.h"


/* 开启性能检测 */
#define PERFORMACE 0	/* 开启设置为1，关闭设置为0，节省编译时间*/

#if PERFORMACE == 1

/* DWT函数 辅助时间获取 */
void DWT_Init(void);
uint32_t DWT_GetMicrosecond(void);
/* 时间统计函数*/
void Show_Performance(void);

#endif

#endif /* PERFORMANCETEST_H_ */
