#ifndef _COMMON_COMMUNICATION_PROTOCOL_H__
#define _COMMON_COMMUNICATION_PROTOCOL_H__
#include "HeadFiles.h"

/* 必要的头文件 */
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
/*
 *
命令格式：@ 0x01 0x20 [Content] \r\n
变量格式：@ 0x02 0x20 [VAR_Name]:[Data]\r\n
*/
#define DEBUG_MODE 0

/* -------------------------------------------------------------------------- */
/*                                    Marco                                   */
/* -------------------------------------------------------------------------- */
#define MACHINE_ADDR 0x20
#define CMD_TYPE 0x01
#define VAR_TYPE 0x02
#define FAST_TYPE 0x01
#define SLOW_TYPE 0x02
#define BUFFER_END_SIGN 0x0D
#define VAR_CREATE(Name,x) Val_Create(Name,&x)

//变量联合体
typedef union
{
	void *Data_Addr;
    float* f_Data;
    int* I_Data;
}Data_union;

//变量结构体
typedef struct Val
{
    uint8_t Name[50];//变量名
    Data_union Data;
}Val_t;

//协议缓冲区
typedef struct Protocol_Buffer
{
    uint8_t Protocol_Buffer[100];
    uint8_t *Main_Ptr;
    uint8_t *Stroage_Ptr;

}Protocol_Buffer_t;

/* 数据类型枚举 */
typedef enum
{
	INT = 0,
	FLOAT
}Data_Type;
//协议结构体
typedef struct Protocol
{
    uint16_t DR;				/*数据寄存器*/
    Protocol_Buffer_t Buffer;	/* 缓冲区结构体 */
    uint8_t Status;				/* 协议状态 */
    uint8_t Command_Type;		/* 协议接收的数据类型 指令or参数 */
    uint8_t Machine_Addr;		/* 机械地址 */
    uint8_t Val_Name[50];		/* 变量名缓冲区 */
    uint8_t *Val_Name_Ptr;		/*变量名指针 */
    uint8_t Val_Data[50];		/*变量数据缓冲区 */
    uint8_t *Val_Data_Ptr;		/*数据变量指针 */
    uint8_t Block;				/* 协议处理方式选择 BLOCK/UNBLOCK*/
    uint8_t Data_Type;			/* 接收到的数据类型 */
}Protocol_t;

typedef struct 
{
	uint8_t Name[50];
	void (*func)();
}CMD_t;

/* 处理方法枚举 */
typedef enum
{
	BLOCK = 0,
	UNBLOCK
}Process_Way;


/* -------------------------------------------------------------------------- */
/*                                   Extern                                   */
/* -------------------------------------------------------------------------- */
//变量数组
extern Val_t Val[50];
//可变变量的总数
extern int Val_Num;

//Protocol Demo
extern Protocol_t Protocol_Demo;

extern Protocol_t USART1_Protocol,USART3_Protocol;
extern CMD_t CMD_Arry[50];


extern uint8_t Data_Arry[50][10],
				(*Data_Main_Ptr)[10],
				*Data_Ptr;
				
extern float fData_Arry[10];
extern uint8_t Data_Index,Data_Arry_Index;

extern uint32_t Head_Tick;
extern uint32_t Name_Tick;
extern uint32_t Data_Tick;
extern uint32_t Total_Tick;
extern uint8_t Tick_Rec_Finish_Flag;
extern uint32_t Start_Tick;
/* -------------------------------------------------------------------------- */
/*                               Var_Name_Marco                               */
/* -------------------------------------------------------------------------- */



/* -------------------------------------------------------------------------- */
/*                            Function Declaration                            */
/* -------------------------------------------------------------------------- */
void Protocol_Init(Protocol_t* x,uint8_t Mode);
void Val_Create(char* Name,void*Var);
void Protocol(Protocol_t* x,uint8_t Mode);
void Rec_Proc(Protocol_t* x, uint8_t* Data);
void CMD_Create(char *Name,void (*CallBack_Function)());
void Fast_Type_Proc(Protocol_t* x,uint8_t DR);
float Fast_Type_Data_Converse(Protocol_t* x,uint8_t Index);


#endif
