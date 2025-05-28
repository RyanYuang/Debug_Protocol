/*********************************************************************************************************************
* Debug-Protocol 即（调试协议）是一个基于C语言开发的串口调试协议工具包
*
* 本开源工具的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 欢迎各位使用并传播本程序 但修改内容时必须保留作者的版权声明（即本声明）
*
* 文件名称          Debug_Protocol.c
* 作者名字          Ryan Yuang
* 版本信息          V0
* 开发环境          Keil V5 IDE
* 适用平台          任何支持串口通信的嵌入式平台
* 仓库链接			https://github.com/RyanYuang/Debug-Protocol
*
* 修改记录
* 日期              作者                备注
* 2025-4-26       Ryan Yuang            first version
********************************************************************************************************************/
#include "Common_Communicate_Protocol.h"

/* 全局变量 */

/* 变量临时存储 */
Val_t Val[50];

/* 已创建的可变变量的总数 */
int Val_Num;


/* 指令数组 */
CMD_t CMD_Arry[50];

/* 已创建的指令数量  */
int CMD_Num;

/* 数据临时存放数组 */
uint8_t Data_Arry[50][10],
				(*Data_Main_Ptr)[10],
				*Data_Ptr;

/* 数据下标 */
uint8_t Data_Arry_Index,Data_Index;

/* 浮点数*/
float fData_Arry[10];

/* 协议结构体定义 */
Protocol_t USART1_Protocol,USART3_Protocol;

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     数据接收处理
// 参数说明     *x                  协议对象（结构体）指针
// 参数说明     Data                填入的数据
// 返回参数     void
// 使用示例     Rec_Proc(USART1_Protocol, Rec_Data);
// 备注信息     本函数需要在串口接收中断中调用
//-------------------------------------------------------------------------------------------------------------------
void Rec_Proc(Protocol_t* x,uint8_t Data)
{
    /*1、接受数据寄存器的数据*/
    *(x->Buffer.Stroage_Ptr++) = Data;
		if(
		x->Buffer.Stroage_Ptr > x->Buffer.Protocol_Buffer + sizeof(x->Buffer.Protocol_Buffer)-2 || 
		x->Buffer.Stroage_Ptr < x->Buffer.Protocol_Buffer
		)
		{
			x->Buffer.Stroage_Ptr = x->Buffer.Protocol_Buffer;
		}
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     协议处理正文
// 参数说明     *x                  协议对象（结构体）指针
// 参数说明     Mode                数据处理模式（Slow Type【慢速模式】，Fast Type【快速模式】)
// 返回参数     void
// 使用示例     Protocol(USART1_Protocol, SLOW_TYPE);
// 备注信息     本函数需要While循环中调用，不要在中断中调用！
//-------------------------------------------------------------------------------------------------------------------
void Protocol(Protocol_t* x,uint8_t Mode)
{
	if(x->Buffer.Main_Ptr > x->Buffer.Protocol_Buffer + sizeof(x->Buffer.Protocol_Buffer)-2 || x->Buffer.Main_Ptr < x->Buffer.Protocol_Buffer)
	{
		x->Buffer.Main_Ptr = x->Buffer.Protocol_Buffer;
	}
	if(*x->Buffer.Main_Ptr == 0)return;//遇到0x00直接退出，防止损耗CPU性能
	switch (Mode)
	{
		case SLOW_TYPE:

			
			
			if(*x->Buffer.Main_Ptr == '@' && x->Status == 0)
			{
				x->Status = 1;
				//将指针指向变量头，准备处理
				x->Val_Name_Ptr = x->Val_Name;
				x->Val_Data_Ptr = x->Val_Data;
				x->Data_Type = 0;
				x->Machine_Addr = 0;
				*x->Buffer.Main_Ptr = 0;
				x->Buffer.Main_Ptr++;
				
				/* For DeBug */
				/*printf("Get Head\r\n");*/
				return;
			}
			if(*x->Buffer.Main_Ptr == CMD_TYPE && x->Status == 1)//确认是一个指令==暂时没有任何作用==
			{
				x->Status = 2;
				x->Data_Type = *x->Buffer.Main_Ptr;
				*x->Buffer.Main_Ptr = 0;
				x->Buffer.Main_Ptr++;
				
				/* For DeBug */
				//printf("IS CMD\r\n");
				return;
			}
			if(*x->Buffer.Main_Ptr == VAR_TYPE && x->Status == 1)//确认是一个变量==暂时没有任何作用==
			{
					x->Status = 2;
					x->Data_Type = *x->Buffer.Main_Ptr;
					*x->Buffer.Main_Ptr = 0;
					x->Buffer.Main_Ptr++;
					/* For DeBug */
					//printf("IS VAR\r\n");
					return;
			}
			if(*x->Buffer.Main_Ptr == MACHINE_ADDR && x->Status == 2)//方便对于不同的设备做不同的反应==暂时没有任何作用==
			{
				x->Status = 3;
				x->Machine_Addr = *x->Buffer.Main_Ptr;
				*x->Buffer.Main_Ptr = 0;
				x->Buffer.Main_Ptr++;
				/* For DeBug */
				//printf("IS MACHINE\r\n");
				return;
			}
			if(*x->Buffer.Main_Ptr == ':' && x->Status == 3 && x->Data_Type == VAR_TYPE)//接收到名字帧尾，退出名字接收模式
			{
				x->Status = 4;//进入数据接收模式
				*x->Buffer.Main_Ptr = 0;
				x->Buffer.Main_Ptr++;
				return;
			}
			if(*x->Buffer.Main_Ptr != ':' && x->Status == 3 && x->Data_Type == VAR_TYPE)//接收变量名
			{
				*(x->Val_Name_Ptr) = *(x->Buffer.Main_Ptr);
				x->Val_Name_Ptr++;
			}
			if(*x->Buffer.Main_Ptr != BUFFER_END_SIGN && x->Status == 3 && x->Data_Type == CMD_TYPE)//接收到名字帧尾，退出名字接收模式
			{
				*(x->Val_Name_Ptr) = *(x->Buffer.Main_Ptr);
				x->Val_Name_Ptr++;
				/* For DeBug */
				//printf("IS CMD NAME\r\n");
			}
			if(*x->Buffer.Main_Ptr != BUFFER_END_SIGN && x->Status == 4 && x->Data_Type == VAR_TYPE)
			{
				*(x->Val_Data_Ptr) = *(x->Buffer.Main_Ptr);
				x->Val_Data_Ptr++;
				//将临时存储的变量进行存储

			}

			
			//接收到结束码并且数据为参数
			if(*x->Buffer.Main_Ptr == BUFFER_END_SIGN && x->Status == 4 && x->Data_Type == VAR_TYPE)
			{
					x->Buffer.Stroage_Ptr = x->Buffer.Protocol_Buffer;
					*x->Buffer.Main_Ptr = 0;
					x->Buffer.Main_Ptr = x->Buffer.Protocol_Buffer;
					x->Status = 0;
					int Val_Name_Len = strlen((char*)x->Val_Name);
					int Val_Data_Len = strlen((char*)x->Val_Data);
					for(int i = 0; i < Val_Num;i++)
					{
							if(!strcmp((char*)x->Val_Name,(char*)Val[i].Name))
							{
							
									*Val[i].Data.f_Data = strtof((char*)x->Val_Data,NULL);


							 }
					}
							for(int i = 0 ; i < Val_Name_Len;i++)x->Val_Name[i] = 0;
							for(int i = 0 ; i < Val_Data_Len;i++)x->Val_Data[i] = 0;
				return;
			}
			//接收到结束码并且数据为指令
			if(*x->Buffer.Main_Ptr == BUFFER_END_SIGN && x->Status == 3 && x->Data_Type == CMD_TYPE)
			{
					x->Buffer.Stroage_Ptr = x->Buffer.Protocol_Buffer;				
					*x->Buffer.Main_Ptr = 0;
					x->Buffer.Main_Ptr = x->Buffer.Protocol_Buffer;
					x->Status = 0;
					int Val_Name_Len = strlen((char*)x->Val_Name);
					for(int i = 0; i < CMD_Num;i++)
					{
							if(!strcmp((char*)x->Val_Name,(char*)CMD_Arry[i].Name))
							{
									for(int i = 0 ; i < Val_Name_Len;i++)x->Val_Name[i] = 0;
									CMD_Arry[i].func();
							}
					}
					for(int i = 0 ; i < Val_Name_Len;i++)x->Val_Name[i] = 0;
					return;
			}
			*x->Buffer.Main_Ptr = 0;
			x->Buffer.Main_Ptr++;

		break;
		
		case FAST_TYPE:
			if(*x->Buffer.Main_Ptr == '@' && x->Status == 0)
			{
				Data_Arry_Index = 0;
				Data_Index = 0;
				x->Status = 1;
				//缓冲区 清空
				*x->Buffer.Main_Ptr = 0;
				//指针后移
				x->Buffer.Main_Ptr++;
				//Data_Main_Ptr = Data_Arry;
				//Data_Ptr = Data_Arry[Data_Arry_Index];
			}
			if(x->Status == 1 && *x->Buffer.Main_Ptr != ',')
			{
				Data_Arry[Data_Arry_Index][Data_Index] = *x->Buffer.Main_Ptr;
				Data_Index++;
			}
			if(x->Status == 1 && *x->Buffer.Main_Ptr == ',')
			{
				x->Status = 2;
				//缓冲区清空
				*x->Buffer.Main_Ptr = 0;
				//指针后移
				x->Buffer.Main_Ptr++;
				//数据存储缓冲区后移
				Data_Index = 0;
				Data_Arry_Index++;
				//还有数据
				if(*x->Buffer.Main_Ptr != '\r')
				{
					x->Status = 1;
				}
			}
			if(x->Status == 2 && *x->Buffer.Main_Ptr == '\r')
			{
				x->Status = 0;
				*x->Buffer.Main_Ptr = 0;
			//	x->Buffer.Main_Ptr = x->Buffer.Protocol_Buffer;
			

				Data_Arry_Index = 0;
				Data_Index = 0;

			}
			//缓冲区清空
			*x->Buffer.Main_Ptr = 0;
			//指针后移
			x->Buffer.Main_Ptr++;
			break;
	}
		
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     协议对象初始化
// 参数说明     *x                  协议对象（结构体）指针
// 参数说明     Mode                数据处理模式（Slow Type【慢速模式】，Fast Type【快速模式】)
// 返回参数     void
// 使用示例     Protocol_Init(USART1_Protocol, SLOW_TYPE);
// 备注信息     本函数需要初始化的过程中调用来对对象进行初始化
//-------------------------------------------------------------------------------------------------------------------
void Protocol_Init(Protocol_t* x,uint8_t Mode)
{
    x->Buffer.Protocol_Buffer[99] = BUFFER_END_SIGN;//End Sign
    x->Buffer.Main_Ptr = x->Buffer.Protocol_Buffer;
    x->Buffer.Stroage_Ptr = x->Buffer.Protocol_Buffer;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     可变变量的创建
// 参数说明     *Name                  变量名
// 参数说明     *Var                   变量本体的指针
// 返回参数     void
// 使用示例     Val_Create(“Speed”, &Speed);
// 备注信息     本函数需要初始化的过程中调用来
//-------------------------------------------------------------------------------------------------------------------
void Val_Create(char* Name,void*Var)
{
    strcpy((char*)Val[Val_Num].Name,(char*)Name);
    Val[Val_Num].Data.Data_Addr = Var;
    Val_Num++;
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     链接指令
// 参数说明     *Name                  变量名
// 参数说明     *CallBack_Function     回调函数指针
// 返回参数     void
// 使用示例     CMD_Create(“Reply”, &OK);
// 备注信息     本函数需要初始化的过程中调用来
//-------------------------------------------------------------------------------------------------------------------
void CMD_Create(char *Name,void (*CallBack_Function)())
{
	strcpy((char*)CMD_Arry[CMD_Num].Name,(char*)Name);
	CMD_Arry[CMD_Num].func = CallBack_Function;
	CMD_Num++;
}

