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
#include "Debug_Protocol.h"

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

/* 浮点数临时存储数组 */
float fData_Arry[10];

/* 协议结构体定义 */
Protocol_t USART1_Protocol,USART3_Protocol;

/* Buffer Size */
int Buffer_Size = 0;
// 定义协议状态枚举
typedef enum {
    STATE_HEAD = 0,           // 头部检测
    STATE_TYPE,           // 类型检测
    STATE_MACHINE,        // 机器地址检测
    STATE_NAME,           // 名称接收
    STATE_DATA,            // 数据接收
	STATE_IDLE       // 空闲状态
} ProtocolState;

/* 处理方法枚举 */
typedef enum
{
	BLOCK = 0,
	UNBLOCK
}Process_Way;

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     数据接收处理
// 参数说明     *x                  协议对象（结构体）指针
// 参数说明     Data                填入的数据
// 返回参数     void
// 使用示例     Rec_Proc(USART1_Protocol, Rec_Data);
// 备注信息     本函数需要在串口接收中断中调用
//-------------------------------------------------------------------------------------------------------------------
void Rec_Proc(Protocol_t* x, uint8_t* Data)
{
    // 获取字符串长度
    size_t data_len = strlen((char*)Data);

    // 计算缓冲区剩余空间
    size_t buffer_size = sizeof(x->Buffer.Protocol_Buffer);
    size_t available_space = buffer_size - (x->Buffer.Stroage_Ptr - x->Buffer.Protocol_Buffer);

    // 确保不会溢出缓冲区
    if (data_len > available_space) {
        data_len = available_space; // 截断到可用空间大小
    }

    // 快速复制字符串到缓冲区
    memcpy(x->Buffer.Stroage_Ptr, Data, data_len);

    // 更新存储指针
    x->Buffer.Stroage_Ptr += data_len;

    // 边界检查，防止指针越界
    if (x->Buffer.Stroage_Ptr > x->Buffer.Protocol_Buffer + buffer_size - 1 ||
        x->Buffer.Stroage_Ptr < x->Buffer.Protocol_Buffer) {
        x->Buffer.Stroage_Ptr = x->Buffer.Protocol_Buffer; // 重置指针
    }
}

void Protocol_Reset(Protocol_t* x)
{
	x->Status = STATE_HEAD; 		/* 进入类型处理模式 */
	x->Val_Name_Ptr = x->Val_Name;	/* 重置变量名获取指针 */
	x->Val_Data_Ptr = x->Val_Data;	/* 重置变量获取指针 */
	x->Data_Type = 0;				/* 重置数据类型参数 */
	x->Machine_Addr = 0;			/* 重置机器地址参数 */
	*x->Buffer.Main_Ptr = 0;		/* 接收的内容设置为空 */
	x->Buffer.Main_Ptr = x->Buffer.Protocol_Buffer;
	x->Buffer.Stroage_Ptr = x->Buffer.Protocol_Buffer;
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
	/* 检测当前指针位置是否异常 */
	if(
			x->Buffer.Main_Ptr > x->Buffer.Protocol_Buffer + sizeof(x->Buffer.Protocol_Buffer)-1  	/* 检测越界 */
			|| x->Buffer.Main_Ptr < x->Buffer.Protocol_Buffer)										/* 检测错误地址 */
	{
		x->Buffer.Main_Ptr = x->Buffer.Protocol_Buffer;												/* 重置指针位置为协议缓冲区的头部 */
	}
	if(*x->Buffer.Main_Ptr == 0)return;																/* 遇到0x00直接退出，防止损耗CPU性能 */
	switch (Mode)
	{
		/* 慢模式的参数调节 */
		case SLOW_TYPE:
			switch (x->Status)
			{
			/* 头部接收模式 */
			case STATE_HEAD:
				if(*x->Buffer.Main_Ptr == '@')
				{
					x->Status = STATE_TYPE; 		/* 进入类型处理模式 */
					x->Val_Name_Ptr = x->Val_Name;	/* 重置变量名获取指针 */
					x->Val_Data_Ptr = x->Val_Data;	/* 重置变量获取指针 */
					x->Data_Type = 0;				/* 重置数据类型参数 */
					x->Machine_Addr = 0;			/* 重置机器地址参数 */
					*x->Buffer.Main_Ptr = 0;		/* 接收的内容设置为空 */
					x->Buffer.Main_Ptr++;			/* 推进接收缓冲区 */

					/* ======== For DeBug ======== */
					/*printf("Get Head\r\n");*/
					/* ======== For DeBug ======== */

					x->Data_Type = *x->Buffer.Main_Ptr;			/* 存储数据类型 */
					*x->Buffer.Main_Ptr = 0;					/* 清空当前数据 */
					x->Buffer.Main_Ptr++;						/* 推进数据指针 */
					x->Status = STATE_MACHINE;					/* 更新标志位  */
					/* ======== For DeBug ======== */
					/* ======== 判断数据类型 ======== */
					if(x->Data_Type == CMD_TYPE)
					{
						/* ======== For DeBug ======== */
						printf("IS CMD\r\n");
						/* ======== For DeBug ======== */
					}
					if(x->Data_Type == VAR_TYPE)
					{
						/* ======== For DeBug ======== */
						printf("IS VAR\r\n");
						/* ======== For DeBug ======== */
					}
					/* ======== For DeBug ======== */

					/* 接收机器地址 */
					x->Machine_Addr = *x->Buffer.Main_Ptr;	/* 存储机器地址 */
					*x->Buffer.Main_Ptr = 0;				/* 清空当前数据 */
					x->Buffer.Main_Ptr++;					/* 推进数据指针 */
					x->Status = STATE_NAME;					/* 更新标志位  */

					/* 判断是否控制当前设备 */
					if(x->Machine_Addr == MACHINE_ADDR)
					{
						printf("ACK\r\n");
					}
					else
					{
						/* 重置整个协议 */
						printf("NO ACK\r\n");
						Protocol_Reset(x);
					}
				}
			break;
			case STATE_NAME:
				/* 接收变量名/指令名	*/
				if(x->Block == BLOCK)
				{
					while(*x->Buffer.Main_Ptr != ':')
					{
						*(x->Val_Name_Ptr) = *(x->Buffer.Main_Ptr);
						x->Val_Name_Ptr++;
						/* 推进指针 */
						*x->Buffer.Main_Ptr = 0;
						*x->Buffer.Main_Ptr++;

					}
				}
				else
				{
					if(*x->Buffer.Main_Ptr != ':')
					{
						*(x->Val_Name_Ptr) = *(x->Buffer.Main_Ptr);
						x->Val_Name_Ptr++;
						/* 推进指针 */
						*x->Buffer.Main_Ptr = 0;
						*x->Buffer.Main_Ptr++;

					}
				}

				/* 结束条件 */
				if(*x->Buffer.Main_Ptr == BUFFER_END_SIGN && x->Data_Type == CMD_TYPE)
				{
					x->Status = STATE_IDLE; 	/* 进入数据处理模式 */
					/* 推进指针 */
					*x->Buffer.Main_Ptr = 0;
					x->Buffer.Main_Ptr++;
				}
				/* 结束条件*/
				if(*x->Buffer.Main_Ptr == ':' && x->Data_Type == VAR_TYPE)//接收到名字帧尾，退出名字接收模式
				{
					x->Status = STATE_DATA;		/* 进入数据接收模式 */
					/* 推进指针 */
					*x->Buffer.Main_Ptr = 0;
					x->Buffer.Main_Ptr++;
				}
			break;
			case STATE_DATA:
				if(*x->Buffer.Main_Ptr == BUFFER_END_SIGN)
				{
					x->Status = STATE_IDLE; 	/* 进入数据处理模式 */
					/* 推进指针 */
					*x->Buffer.Main_Ptr = 0;	/* 清空当前数据*/
					x->Buffer.Main_Ptr++;
				}
				else
				{
					if(x->Block == BLOCK)
					{
						while(*x->Buffer.Main_Ptr != BUFFER_END_SIGN)
						{

							*(x->Val_Data_Ptr) = *(x->Buffer.Main_Ptr);
							x->Val_Data_Ptr++;
							*x->Buffer.Main_Ptr = 0;
							x->Buffer.Main_Ptr++;
							if(*x->Buffer.Main_Ptr == BUFFER_END_SIGN)
							{
								break;
							}
						}
					}
					else
					{
						*(x->Val_Data_Ptr) = *(x->Buffer.Main_Ptr);
						x->Val_Data_Ptr++;
						*x->Buffer.Main_Ptr = 0;
						x->Buffer.Main_Ptr++;
					}


				}
				break;
			case STATE_IDLE:
				//接收到结束码并且数据为参数
				if(x->Data_Type == VAR_TYPE)
				{
					Protocol_Reset(x);
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
				if(x->Data_Type == CMD_TYPE)
				{
						Protocol_Reset(x);
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
			break;
			}


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

