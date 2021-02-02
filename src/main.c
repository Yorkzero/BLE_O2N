/*************************************************************
Copyright (c) 2020 Shawn Yan, All rights reserved
-------------------------------------------------------------
File Name: main.c

Desc     : main process

Author   : Shawn Yan

Date     : 2021-1-13
*************************************************************/

/*------------------------- Includes -----------------------*/
#include "main.h"

/*----------- Global Definitions and Declarations ----------*/


/*-------------------- Type Declarations -------------------*/


/*------------------ Variable Declarations -----------------*/


/*------------------- Function Prototype -------------------*/


/*------------------- Function Implement -------------------*/

/*************************************************************
Function Name       : main
Function Description: 
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2020-11-20
*************************************************************/
int main(void)
{ 
    //system init
    bsp_clk_init();
    bsp_gpio_init();
    bsp_uart_init(); 
#if (SIM_UART_PRINTF_EN)
    GPIO_Init(SIM_TX_PORT, SIM_TX_PIN, GPIO_Mode_Out_PP_High_Fast);
#endif

#if (1 == CODE_VERSION)//Version 1     
    KEY_ENABLE();
    delay_ms_1(100);
    __enable_interrupt();
    //init buff
    memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
    USART1_RX_STA = 0;
    BLE_SEND_DISABLE();
    delay_ms_1(30);
    BLE_SEND_ENABLE();
    delay_ms_1(500);
    AT_Send("+++");
    AT_Send("AT+SLEEP=1,1\r\n");
    AT_Send("AT+EXIT\r\n");
    FSM_Init(&system_FSM, sys_table, S_STA_INIT);
    PWR_FastWakeUpCmd(DISABLE);
    PWR_UltraLowPowerCmd(ENABLE);
    // BLE_Init();
    while(1)
    {
        // AT_Send("+++");
        // AT_Send("AT+SLEEP=0,1\r\n");
        // AT_Send("AT+EXIT\r\n");
        // BLE_SEND_DISABLE();
        // Sleep_Handler();
        // BLE_status_run();
        // request_msg_process();
        FSM_Run();
    }
#endif
}

#ifdef USE_FULL_ASSERT
void assert_failed(u8* file,u32 line)
{
    while(1)
    {
    }
}
#endif
/*--------------------------- END --------------------------*/