#ifndef _COMMON_COMMUNICATION_PROTOCOL_H__
#define _COMMON_COMMUNICATION_PROTOCOL_H__

#include "HeadFiles.h"

/* Necessary header files for string, standard input/output, and memory allocation */
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

/*
 * Slow Type
 * Command format: @ 0x01 [Machine Code] [Content] \r
 * Variable format: @ 0x02 [Machine Code] [VAR_Name]:[Data]\r
 * 
 * Fast Type
 * Data format: @ [Data_1] , [Data_2] , [Data_3] ... \r
 */
#define DEBUG_MODE 0  // Debug mode flag (0 = disabled, 1 = enabled)

/* -------------------------------------------------------------------------- */
/*                                    Macros                                  */
/* -------------------------------------------------------------------------- */
#define MACHINE_ADDR 0x20      // Machine address for protocol communication
#define CMD_TYPE 0x01          // Command type identifier
#define VAR_TYPE 0x02          // Variable type identifier
#define FAST_TYPE 0x01         // Fast processing type
#define SLOW_TYPE 0x02         // Slow processing type
#define BUFFER_END_SIGN 0x0D   // Buffer end sign (carriage return)
#define VAR_CREATE(Name, x) Val_Create(Name, &x)  // Macro to create a variable
#define BufferSize 100         // Size of the protocol buffer

/* -------------------------------------------------------------------------- */
/*                                  Data Types                                */
/* -------------------------------------------------------------------------- */

/* Union to store different types of data pointers (void, float, or int) */
typedef union
{
    void *Data_Addr;      // Generic pointer to data
    float *f_Data;        // Pointer to float data
    int *I_Data;          // Pointer to integer data
} Data_union;

/* Structure to hold variable information */
typedef struct Val
{
    uint8_t Name[50];     // Array to store variable name (up to 50 characters)
    Data_union Data;      // Union to store the variable's data pointer
} Val_t;

/* Structure for the protocol buffer */
typedef struct Protocol_Buffer
{
    uint8_t Protocol_Buffer[BufferSize]; // Buffer to store protocol data
    uint8_t *Main_Ptr;                  // Pointer to the main buffer position
    uint8_t *Stroage_Ptr;               // Pointer to the storage position in buffer
} Protocol_Buffer_t;

/* Enum to define supported data types */
typedef enum
{
    INT = 0,   // Integer data type
    FLOAT      // Float data type
} Data_Type;

/* Structure for data buffer used in fast mode */
typedef struct
{
    uint8_t Data_Arry_Index; // Index for the data array
    uint8_t Data_Index;      // Index for individual data within an array
    uint8_t Data_Arry[50][10]; // 2D array to store data (50 arrays of 10 bytes each)
} DataBuffer;

/* Main protocol structure */
typedef struct Protocol
{
    uint16_t DR;                // Data register
    Protocol_Buffer_t Buffer;   // Protocol buffer structure
    uint8_t Status;             // Current status of the protocol
    uint8_t Command_Type;       // Type of received data (command or variable)
    uint8_t Machine_Addr;       // Machine address for protocol
    uint8_t Val_Name[50];       // Buffer for variable name
    uint8_t *Val_Name_Ptr;      // Pointer to variable name
    uint8_t Val_Data[50];       // Buffer for variable data
    uint8_t *Val_Data_Ptr;      // Pointer to variable data
    uint8_t Block;              // Processing mode (BLOCK or UNBLOCK)
    uint8_t Data_Type;          // Type of received data (INT or FLOAT)
    DataBuffer Data_Buffer;     // Data buffer for fast mode
} Protocol_t;

/* Structure to store command information */
typedef struct 
{
    uint8_t Name[50];  // Command name (up to 50 characters)
    void (*func)();    // Pointer to the callback function for the command
} CMD_t;

/* Enum to define processing methods */
typedef enum
{
    BLOCK = 0,   // Blocking mode
    UNBLOCK      // Non-blocking mode
} Process_Way;

/* -------------------------------------------------------------------------- */
/*                                  Extern Variables                          */
/* -------------------------------------------------------------------------- */
extern Val_t Val[50];          // Array to store up to 50 variables
extern int Val_Num;            // Total number of variables created

extern Protocol_t Protocol_Demo; // Demo protocol instance
extern Protocol_t USART1_Protocol, USART3_Protocol; // Protocol instances for USART1 and USART3
extern CMD_t CMD_Arry[50];     // Array to store up to 50 commands

extern uint8_t Data_Arry[50][10]; // 2D array for data storage
extern uint8_t (*Data_Main_Ptr)[10]; // Pointer to main data array
extern uint8_t *Data_Ptr;      // Pointer to specific data
extern float fData_Arry[10];   // Array to store float data
extern uint8_t Data_Index;     // Index for data within an array
extern uint8_t Data_Arry_Index; // Index for the data array

extern uint32_t Head_Tick;     // Tick counter for protocol header
extern uint32_t Name_Tick;     // Tick counter for variable name
extern uint32_t Data_Tick;     // Tick counter for data
extern uint32_t Total_Tick;    // Total tick counter
extern uint8_t Tick_Rec_Finish_Flag; // Flag indicating tick reception completion
extern uint32_t Start_Tick;    // Start tick for timing

/* -------------------------------------------------------------------------- */
/*                            Function Declarations                           */
/* -------------------------------------------------------------------------- */
/* Initialize the protocol with specified mode */
void Protocol_Init(Protocol_t *x, uint8_t Mode);

/* Create a variable with given name and data pointer */
void Val_Create(char *Name, void *Var);

/* Process protocol data with specified mode */
void Protocol(Protocol_t *x, uint8_t Mode);

/* Process received data for the protocol */
void Rec_Proc(Protocol_t *x, uint8_t *Data);

/* Create a command with name and associated callback function */
void CMD_Create(char *Name, void (*CallBack_Function)());

/* Process fast type data for the protocol */
void Fast_Type_Proc(Protocol_t *x, uint8_t DR);

/* Convert fast type data to float */
float Fast_Type_Data_Converse(Protocol_t *x, uint8_t Index);

/* Get the write pointer position in the protocol buffer */
uint32_t Get_Write_Ptr(Protocol_t *x);

/* Get the read pointer position in the protocol buffer */
uint32_t Get_Read_Ptr(Protocol_t *x);

/* Get the remaining read space in the protocol buffer */
uint32_t Get_Read_Remain(Protocol_t *x);

#endif