#include "User_Code.h"

uint8_t rx_data[1] = {0};
volatile uint8_t rx_flag = 0;
int Parament_A = 0;
float Parament_B= 0.0f;

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE {
	uint8_t data = (uint8_t)ch;
	while (CDC_Transmit_FS(&data, 1) == USBD_BUSY) {
		HAL_Delay(1); // 等待 USB 空闲
	}
	return ch;
}




void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        rx_flag = 1; // 设置接收标志
        HAL_UART_Receive_IT(&huart1, (uint8_t *)rx_data, sizeof(rx_data)); // 重新启动接收中断
        Rec_Proc(&USART1_Protocol,rx_data);
    }
}



void User_Main(void)
{
	Protocol_Init(&USART1_Protocol,SLOW_TYPE);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
    HAL_UART_Receive_IT(&huart1, rx_data, 1);

    /* Debugging parameter initialization */
    Val_Create("Para_A",&Parament_A);
    Val_Create("Para_B",&Parament_B);
    printf("System Online!\r\n");
    /*  */
    USART1_Protocol.Block = UNBLOCK;

	/*---------- Performance ----------*/
#if PERFORMACE == 1
    DWT_Init();
#endif
    while (1)
    {
    	Protocol(&USART1_Protocol,SLOW_TYPE);
//    	Parament_A = strtol((char*)USART1_Protocol.Data_Buffer.Data_Arry[0],NULL,10);
//    	Parament_B = strtof((char*)USART1_Protocol.Data_Buffer.Data_Arry[1],NULL);
    	//printf("%s,%s,%ld\n",USART1_Protocol.Data_Buffer.Data_Arry[0],USART1_Protocol.Data_Buffer.Data_Arry[1],Get_Write_Ptr(&USART1_Protocol));
    	printf("%d,%f,%d,%ld,%ld,%c,%ld\r\n",Parament_A,Parament_B,USART1_Protocol.Status,Get_Write_Ptr(&USART1_Protocol),Get_Read_Ptr(&USART1_Protocol),*USART1_Protocol.Buffer.Main_Ptr,Get_Read_Remain(&USART1_Protocol));

    	/*---------- Performance ----------*/
#if PERFORMACE == 1
    	Show_Performance();
#endif
    	/*---------- Performance ----------*/
    }

}
