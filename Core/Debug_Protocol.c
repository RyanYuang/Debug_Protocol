/*********************************************************************************************************************
 * Debug-Protocol is a serial debugging protocol toolkit developed in C language
 *
 * The release of this open-source tool is intended to be useful, but no guarantees are made,
 * including no implied warranties of merchantability or fitness for a particular purpose.
 * Users are welcome to use and distribute this program, but any modifications must retain the author's copyright notice (i.e., this statement).
 *
 * File Name:         Debug_Protocol.c
 * Author:            Ryan Yuang
 * Version:           V0
 * Development Env:   Keil V5 IDE
 * Supported Platform: Any embedded platform supporting serial communication
 * Repository Link:   https://github.com/RyanYuang/Debug-Protocol
 *
 * Change Log:
 * Date            Author         Notes
 * 2025-04-26      Ryan Yuang     Initial version
 ********************************************************************************************************************/
#include "Debug_Protocol.h"

/* Global Variables */

/* Temporary storage for variables */
Val_t Val[50];

/* Total number of created variables */
int Val_Num;

/* Command array */
CMD_t CMD_Arry[50];

/* Total number of created commands */
int CMD_Num;

/* Temporary storage array for floating-point numbers */
float fData_Arry[10];

/* Protocol structure definitions */
Protocol_t USART1_Protocol, USART3_Protocol;

/* Buffer Size */
int Buffer_Size = 0;

/* Protocol state enumeration */
typedef enum {
    STATE_HEAD = 0,    // Header detection
    STATE_TYPE,        // Type detection
    STATE_MACHINE,     // Machine address detection
    STATE_NAME,        // Name reception
    STATE_DATA,        // Data reception
    STATE_IDLE         // Idle state
} ProtocolState;

/* Performance parameters */
uint32_t Start_Tick = 0;
uint32_t Head_Tick = 0;
uint32_t Name_Tick = 0;
uint32_t Data_Tick = 0;
uint32_t Total_Tick = 0;
uint8_t Tick_Rec_Finish_Flag = 0;

//-------------------------------------------------------------------------------------------------------------------
// Function:      Data reception processing
// Parameters:    *x      Pointer to the protocol object (structure)
//                Data    Input data
// Return:        void
// Example:       Rec_Proc(&USART1_Protocol, Rec_Data);
// Notes:         This function must be called within the serial receive interrupt
//-------------------------------------------------------------------------------------------------------------------
void Rec_Proc(Protocol_t* x, uint8_t* Data)
{
    uint32_t uwTick = 0;
    // Get the length of the input string
    size_t data_len = strlen((char*)Data);

    // Calculate remaining buffer space
    size_t buffer_size = sizeof(x->Buffer.Protocol_Buffer);
    size_t available_space = buffer_size - (x->Buffer.Stroage_Ptr - x->Buffer.Protocol_Buffer);

    size_t Cut_size = 0;

    // Ensure no buffer overflow
    if (data_len > available_space)
    {
        Cut_size = data_len - available_space;
        data_len = available_space; // Truncate to available space
    }

    // Quickly copy string to buffer
    memcpy(x->Buffer.Stroage_Ptr, Data, data_len);

    // Update storage pointer
    x->Buffer.Stroage_Ptr += data_len;

    // Boundary check to prevent pointer out-of-bounds
    if (x->Buffer.Stroage_Ptr > x->Buffer.Protocol_Buffer + buffer_size - 1 ||
        x->Buffer.Stroage_Ptr < x->Buffer.Protocol_Buffer)
    {
        x->Buffer.Stroage_Ptr = x->Buffer.Protocol_Buffer; // Reset pointer
    }

    /* Handle truncated data by appending it to the buffer */
    Data += data_len;
    if (Cut_size)
    {
        memcpy(x->Buffer.Stroage_Ptr, Data, Cut_size);
        x->Buffer.Stroage_Ptr += Cut_size;
    }
}

//-------------------------------------------------------------------------------------------------------------------
// Function:      Reset protocol state
// Parameters:    *x      Pointer to the protocol object (structure)
// Return:        void
// Notes:         Resets the protocol state and pointers to initial values
//-------------------------------------------------------------------------------------------------------------------
void Protocol_Reset(Protocol_t* x)
{
    x->Status = STATE_HEAD;                             // Enter header processing mode
    x->Val_Name_Ptr = x->Val_Name;                      // Reset variable name pointer
    x->Val_Data_Ptr = x->Val_Data;                      // Reset variable data pointer
    x->Command_Type = 0;                                // Reset command type parameter
    x->Machine_Addr = 0;                                // Reset machine address parameter
    //*x->Buffer.Main_Ptr = 0;                          // Clear received content
    //x->Buffer.Main_Ptr++;
}

//-------------------------------------------------------------------------------------------------------------------
// Function:      Protocol buffer main pointer boundary check and reset
// Parameters:    *x      Pointer to the protocol object (structure), containing buffer-related members (e.g., main pointer, buffer start address)
// Return:        void
// Notes:         Checks if the main pointer (Main_Ptr) exceeds the valid buffer range; resets the pointer and outputs debug info if an anomaly occurs
//-------------------------------------------------------------------------------------------------------------------
void Ptr_Dect(Protocol_t* x)
{
    /* Check if the current pointer position is out of bounds */
    if (x->Buffer.Main_Ptr > (x->Buffer.Protocol_Buffer + BufferSize - 1) ||  // Check if pointer exceeds buffer end
        x->Buffer.Main_Ptr < x->Buffer.Protocol_Buffer)                       // Check if pointer is before buffer start
    {
        x->Buffer.Main_Ptr = x->Buffer.Protocol_Buffer;                       // Reset pointer to buffer start
#if DEBUG_MODE == 1
        printf("\n=========== Over Range Reset ==========\n");                // Output debug info for pointer reset
#endif
    }
}
//-------------------------------------------------------------------------------------------------------------------
// Function:      Protocol processing core
// Parameters:    *x      Pointer to the protocol object (structure), containing buffer, state, and data pointers
//                Mode    Data processing mode (SLOW_TYPE for slow mode, FAST_TYPE for fast mode)
// Return:        void
// Example:       Protocol(&USART1_Protocol, SLOW_TYPE);
// Notes:         This function is the core of the protocol processing, handling incoming serial data in either slow or fast mode.
//                It processes data based on the current state of the protocol (header, type, machine address, name, data, or idle).
//                The function must be called in a continuous while loop in the main program, not in an interrupt, to avoid blocking.
//                Slow mode processes structured data (commands or variables) with detailed parsing, while fast mode handles simpler,
//                comma-separated data streams. The function ensures buffer safety by calling Ptr_Dect to prevent pointer overflows.
//-------------------------------------------------------------------------------------------------------------------
void Protocol(Protocol_t* x, uint8_t Mode)
{
    // Check and reset the main buffer pointer if it exceeds the valid range to prevent buffer overflows
    Ptr_Dect(x);

    // Exit early if the current buffer content is null (0x00) to avoid unnecessary processing
    //if(*x->Buffer.Main_Ptr == 0) return; // Commented out to allow continuous processing

    // Process based on the specified mode (SLOW_TYPE or FAST_TYPE)
    switch (Mode)
    {
        // Slow mode: Detailed parsing for structured data (commands or variables)
        case SLOW_TYPE:
            // Handle different protocol states using a state machine
            switch (x->Status)
            {
                // Header reception mode: Detects the start of a protocol frame
                case STATE_HEAD:
                    // Check for the protocol header character '@'
                    if (*x->Buffer.Main_Ptr == '@')
                    {
#if PERFORMACE == 1
                        // Record the start time for performance measurement (microseconds)
                        Start_Tick = DWT_GetMicrosecond();
#endif
                        // Transition to type processing mode
                        x->Status = STATE_TYPE;
                        // Reset pointers and parameters for new frame processing
                        x->Val_Name_Ptr = x->Val_Name;    // Reset variable/command name pointer
                        x->Val_Data_Ptr = x->Val_Data;    // Reset data pointer
                        x->Command_Type = 0;              // Clear command type
                        x->Machine_Addr = 0;              // Clear machine address
                        *x->Buffer.Main_Ptr = 0;          // Clear current buffer content
                        x->Buffer.Main_Ptr++;             // Advance to next buffer position
                        Ptr_Dect(x);                      // Ensure pointer stays within bounds
                        x->Data_Type = INT;               // Default data type to integer

#if DEBUG_MODE == 1
                        // Debug output: Indicate header detection
                        printf("Get Head\r\n");
#endif

                        // Store the command type from the next byte
                        x->Command_Type = *x->Buffer.Main_Ptr;
                        *x->Buffer.Main_Ptr = 0;          // Clear current buffer content
                        x->Buffer.Main_Ptr++;             // Advance to next buffer position
                        Ptr_Dect(x);                      // Ensure pointer stays within bounds
                        x->Status = STATE_MACHINE;        // Transition to machine address processing

#if DEBUG_MODE == 1
                        // Debug output: Indicate command type
                        if (x->Command_Type == CMD_TYPE)
                        {
                            printf("IS CMD\r\n");         // Command type detected
                        }
                        if (x->Command_Type == VAR_TYPE)
                        {
                            printf("IS VAR\r\n");         // Variable type detected
                        }
#endif

                        // Store the machine address from the next byte
                        x->Machine_Addr = *x->Buffer.Main_Ptr;
                        *x->Buffer.Main_Ptr = 0;          // Clear current buffer content
                        x->Buffer.Main_Ptr++;             // Advance to next buffer position
                        Ptr_Dect(x);                      // Ensure pointer stays within bounds
                        x->Status = STATE_NAME;           // Transition to name reception mode

#if PERFORMACE == 1
                        // Record time after header processing for performance measurement
                        Head_Tick = DWT_GetMicrosecond();
#endif

                        // Verify if the machine address matches the current device
                        if (x->Machine_Addr == MACHINE_ADDR)
                        {
#if DEBUG_MODE == 1
                            // Debug output: Acknowledge valid machine address
                            printf("ACK\r\n");
#endif
                        }
                        else
                        {
#if DEBUG_MODE == 1
                            // Debug output: Indicate mismatch in machine address
                            printf("NO ACK For %d\r\n", x->Machine_Addr);
#endif
                            // Reset protocol if the address does not match
                            Protocol_Reset(x);
                        }
                    }
                    break;

                // Name reception mode: Collects variable or command name
                case STATE_NAME:
                    // Blocking mode: Process name until a colon is encountered
                    if (x->Block == BLOCK)
                    {
                        // Continue reading until a colon ':' is found
                        while (*x->Buffer.Main_Ptr != ':')
                        {
                            // Copy character to name buffer
                            *(x->Val_Name_Ptr) = *(x->Buffer.Main_Ptr);
                            x->Val_Name_Ptr++;            // Advance name pointer
                            *x->Buffer.Main_Ptr = 0;      // Clear current buffer content
                            x->Buffer.Main_Ptr++;         // Advance to next buffer position
                            Ptr_Dect(x);                  // Ensure pointer stays within bounds

                            // Exit loop if colon is detected
                            if (*x->Buffer.Main_Ptr == ':')
                            {
                                x->Status = STATE_IDLE;   // Transition to idle mode
                                break;
                            }
                        }
                    }
                    // Non-blocking mode: Process one character at a time
                    else
                    {
                        // Check if the current character is not a colon
                        if (*x->Buffer.Main_Ptr != ':')
                        {
                            // Copy character to name buffer
                            *(x->Val_Name_Ptr) = *(x->Buffer.Main_Ptr);
                            x->Val_Name_Ptr++;            // Advance name pointer
                            *x->Buffer.Main_Ptr = 0;      // Clear current buffer content
                            x->Buffer.Main_Ptr++;         // Advance to next buffer position
                            Ptr_Dect(x);                  // Ensure pointer stays within bounds
                        }
                    }

                    // End name reception: Check for command termination
                    if (*x->Buffer.Main_Ptr == BUFFER_END_SIGN && x->Command_Type == CMD_TYPE)
                    {
                        x->Status = STATE_IDLE;           // Transition to idle mode
                        *x->Buffer.Main_Ptr = 0;          // Clear current buffer content
                        x->Buffer.Main_Ptr++;             // Advance to next buffer position
                        Ptr_Dect(x);                      // Ensure pointer stays within bounds

#if PERFORMACE == 1
                        // Record time after name reception for performance measurement
                        Name_Tick = DWT_GetMicrosecond();
#endif
                    }

                    // End condition for variable data: Colon detected
                    if (*x->Buffer.Main_Ptr == ':' && x->Command_Type == VAR_TYPE)
                    {
                        x->Status = STATE_DATA;           // Transition to data reception mode
                        *x->Buffer.Main_Ptr = 0;          // Clear current buffer content
                        x->Buffer.Main_Ptr++;             // Advance to next buffer position
                        Ptr_Dect(x);                      // Ensure pointer stays within bounds

#if PERFORMACE == 1
                        // Record time after name reception for performance measurement
                        Name_Tick = DWT_GetMicrosecond();
#endif
                    }
                    break;

                // Data reception mode: Collects variable data
                case STATE_DATA:
                    // Check for end of data (buffer end sign)
                    if (*x->Buffer.Main_Ptr == BUFFER_END_SIGN)
                    {
                        x->Status = STATE_IDLE;           // Transition to idle mode
                        *x->Buffer.Main_Ptr = 0;          // Clear current buffer content
                        x->Buffer.Main_Ptr++;             // Advance to next buffer position
                        Ptr_Dect(x);                      // Ensure pointer stays within bounds

#if PERFORMACE == 1
                        // Record time after data reception for performance measurement
                        Data_Tick = DWT_GetMicrosecond();
#endif
                    }
                    else
                    {
                        // Blocking mode: Process data until end sign is encountered
                        if (x->Block == BLOCK)
                        {
                            while (*x->Buffer.Main_Ptr != BUFFER_END_SIGN)
                            {
                                // Detect decimal point to set data type to float
                                if (*(x->Buffer.Main_Ptr) == '.')
                                {
                                    x->Data_Type = FLOAT;
                                }
                                // Copy character to data buffer
                                *(x->Val_Data_Ptr) = *(x->Buffer.Main_Ptr);
                                x->Val_Data_Ptr++;            // Advance data pointer
                                *x->Buffer.Main_Ptr = 0;      // Clear current buffer content
                                x->Buffer.Main_Ptr++;         // Advance to next buffer position
                                Ptr_Dect(x);                  // Ensure pointer stays within bounds

                                // Exit loop if end sign is detected
                                if (*x->Buffer.Main_Ptr == BUFFER_END_SIGN)
                                {
                                    x->Status = STATE_IDLE;   // Transition to idle mode
#if PERFORMACE == 1
                                    // Record time after data reception
                                    Data_Tick = DWT_GetMicrosecond();
#endif
                                    break;
                                }
                            }
                        }
                        // Non-blocking mode: Process one character at a time
                        else
                        {
                            // Copy character to data buffer
                            *(x->Val_Data_Ptr) = *(x->Buffer.Main_Ptr);
                            x->Val_Data_Ptr++;            // Advance data pointer
                            *x->Buffer.Main_Ptr = 0;      // Clear current buffer content
                            x->Buffer.Main_Ptr++;         // Advance to next buffer position
                            Ptr_Dect(x);                  // Ensure pointer stays within bounds
                        }
                    }
                    break;

                // Idle mode: Process completed frame (variable or command)
                case STATE_IDLE:
                    // Handle variable data
                    if (x->Command_Type == VAR_TYPE)
                    {
                        // Reset protocol state for the next frame
                        Protocol_Reset(x);

                        // Get lengths of name and data strings
                        int Val_Name_Len = strlen((char*)x->Val_Name);
                        int Val_Data_Len = strlen((char*)x->Val_Data);

                        // Match variable name and update its value
                        for (int i = 0; i < Val_Num; i++)
                        {
                            if (!strcmp((char*)x->Val_Name, (char*)Val[i].Name))
                            {
                                // Convert and store data based on type
                                if (x->Data_Type == FLOAT)
                                    *Val[i].Data.f_Data = strtof((char*)x->Val_Data, NULL);
                                else
                                    *Val[i].Data.I_Data = strtol((char*)x->Val_Data, NULL, 10);
                            }
                        }

                        // Clear name and data buffers
                        for (int i = 0; i < Val_Name_Len; i++)
                            x->Val_Name[i] = 0;
                        for (int i = 0; i < Val_Data_Len; i++)
                            x->Val_Data[i] = 0;

#if PERFORMACE == 1
                        // Record total processing time and set completion flag
                        Total_Tick = DWT_GetMicrosecond();
                        Tick_Rec_Finish_Flag = 1;
#endif
                        return; // Exit after processing
                    }

                    // Handle command execution
                    if (x->Command_Type == CMD_TYPE)
                    {
                        // Reset protocol state for the next frame
                        Protocol_Reset(x);

                        // Get length of command name
                        int Val_Name_Len = strlen((char*)x->Val_Name);

                        // Match command name and execute callback
                        for (int i = 0; i < CMD_Num; i++)
                        {
                            if (!strcmp((char*)x->Val_Name, (char*)CMD_Arry[i].Name))
                            {
                                // Clear name buffer
                                for (int j = 0; j < Val_Name_Len; j++)
                                    x->Val_Name[j] = 0;
                                // Execute the associated callback function
                                CMD_Arry[i].func();
                            }
                        }

                        // Clear name buffer (redundant, as it’s already cleared above)
                        for (int i = 0; i < Val_Name_Len; i++)
                            x->Val_Name[i] = 0;

                        return; // Exit after processing
                    }

#if PERFORMACE == 1
                    // Record total processing time if no specific action taken
                    Total_Tick = DWT_GetMicrosecond();
#endif
                    Tick_Rec_Finish_Flag = 1; // Set completion flag
                    break;
            }
            break;

        // Fast mode: Simplified parsing for comma-separated data streams
        case FAST_TYPE:
            switch (x->Status)
            {
                // Header detection state
                case STATE_HEAD:
                    // Check for protocol header '@'
                    if (*x->Buffer.Main_Ptr == '@')
                    {
                        x->Status = STATE_DATA;           // Transition to data reception mode
                        *x->Buffer.Main_Ptr = 0;          // Clear current buffer content
                        x->Buffer.Main_Ptr++;             // Advance to next buffer position
                        return;                           // Exit to process next byte
                    }
                    break;

                // Data reception mode: Collect comma-separated data
                case STATE_DATA:
                    // Process non-delimiter characters (not ',' or '\r')
                    if (*x->Buffer.Main_Ptr != ',' && *x->Buffer.Main_Ptr != '\r')
                    {
                        // Store character in data buffer
                        x->Data_Buffer.Data_Arry[x->Data_Buffer.Data_Arry_Index][x->Data_Buffer.Data_Index] = *x->Buffer.Main_Ptr;
                        x->Data_Buffer.Data_Index++;      // Increment index for current data field
                        *x->Buffer.Main_Ptr = 0;          // Clear current buffer content
                        x->Buffer.Main_Ptr++;             // Advance to next buffer position
                    }

                    // Handle data field separator (comma)
                    if (*x->Buffer.Main_Ptr == ',')
                    {
                        *x->Buffer.Main_Ptr = 0;          // Clear current buffer content
                        x->Buffer.Main_Ptr++;             // Advance to next buffer position
                        x->Data_Buffer.Data_Index = 0;    // Reset index for next data field
                        x->Data_Buffer.Data_Arry_Index++; // Move to next data array slot
                    }

                    // Handle end of frame (carriage return)
                    if (*x->Buffer.Main_Ptr == '\r')
                    {
                        x->Status = STATE_HEAD;           // Transition back to header detection
                        *x->Buffer.Main_Ptr = 0;          // Clear current buffer content
                        x->Data_Buffer.Data_Arry_Index = 0; // Reset data array index
                        x->Data_Buffer.Data_Index = 0;    // Reset data field index
                        x->Buffer.Main_Ptr++;             // Advance to next buffer position
                    }
                    break;
            }
            break;
    }
}

//-------------------------------------------------------------------------------------------------------------------
// Function:      Protocol object initialization
// Parameters:    *x      Pointer to the protocol object (structure)
//                Mode    Data processing mode (SLOW_TYPE: Slow mode, FAST_TYPE: Fast mode)
// Return:        void
// Example:       Protocol_Init(&USART1_Protocol, SLOW_TYPE);
// Notes:         This function must be called during initialization to set up the protocol object
//-------------------------------------------------------------------------------------------------------------------
void Protocol_Init(Protocol_t* x, uint8_t Mode)
{
    x->Buffer.Protocol_Buffer[99] = BUFFER_END_SIGN;     // Set end sign
    x->Buffer.Main_Ptr = x->Buffer.Protocol_Buffer;
    x->Buffer.Stroage_Ptr = x->Buffer.Protocol_Buffer;
}

//-------------------------------------------------------------------------------------------------------------------
// Function:      Create a variable
// Parameters:    *Name   Variable name
//                *Var    Pointer to the variable
// Return:        void
// Example:       Val_Create("Speed", &Speed);
// Notes:         This function must be called during initialization to create a variable
//-------------------------------------------------------------------------------------------------------------------
void Val_Create(char* Name, void* Var)
{
    strcpy((char*)Val[Val_Num].Name, (char*)Name);
    Val[Val_Num].Data.Data_Addr = Var;
    Val_Num++;
}

//-------------------------------------------------------------------------------------------------------------------
// Function:      Register a command
// Parameters:    *Name            Command name
//                *CallBack_Function  Pointer to the callback function
// Return:        void
// Example:       CMD_Create("Reply", &OK);
// Notes:         This function must be called during initialization to register a command
//-------------------------------------------------------------------------------------------------------------------
void CMD_Create(char *Name, void (*CallBack_Function)())
{
    strcpy((char*)CMD_Arry[CMD_Num].Name, (char*)Name);
    CMD_Arry[CMD_Num].func = CallBack_Function;
    CMD_Num++;
}

//-------------------------------------------------------------------------------------------------------------------
// Function:      Get the offset of the current write pointer in the protocol buffer
// Parameters:    *x      Pointer to the protocol object (structure), containing buffer members (e.g., storage pointer, buffer start address)
// Return:        uint32_t  Offset of the write pointer relative to the buffer start address (in bytes)
// Example:       uint32_t write_offset = Get_Write_Ptr(&USART1_Protocol);  // Get the current write offset for the USART1 protocol object
// Notes:         This helper function calculates the position of the write pointer (Stroage_Ptr) relative to the start of the protocol buffer.
//                It is used to monitor the current writing position in the buffer, aiding in debugging, buffer management, or flow control.
//                The returned offset indicates how many bytes the write pointer has advanced from the buffer's starting address, which is 
//                useful for tracking the amount of data stored in the buffer during protocol operations.
//-------------------------------------------------------------------------------------------------------------------
uint32_t Get_Write_Ptr(Protocol_t *x)
{
    return x->Buffer.Stroage_Ptr - x->Buffer.Protocol_Buffer;
}

//-------------------------------------------------------------------------------------------------------------------
// Function:      Get the offset of the current read pointer in the protocol buffer
// Parameters:    *x      Pointer to the protocol object (structure), containing buffer members (e.g., main pointer, buffer start address)
// Return:        uint32_t  Offset of the read pointer relative to the buffer start address (in bytes)
// Example:       uint32_t read_offset = Get_Read_Ptr(&USART1_Protocol);  // Get the current read offset for the USART1 protocol object
// Notes:         This helper function calculates the position of the read pointer (Main_Ptr) relative to the start of the protocol buffer.
//                It is used to monitor the current reading position in the buffer, which is useful for debugging, buffer management, 
//                or tracking the progress of data processing in the protocol. The returned offset indicates how many bytes the read 
//                pointer has advanced from the buffer's starting address.
//-------------------------------------------------------------------------------------------------------------------
uint32_t Get_Read_Ptr(Protocol_t *x)
{
    return x->Buffer.Main_Ptr - x->Buffer.Protocol_Buffer;
}
