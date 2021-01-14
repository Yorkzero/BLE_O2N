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

#if (3 == CODE_VERSION)//Version 3     
    KEY_ENABLE();
    // LINK_ENABLE();
    delay_ms_1(100);
    __enable_interrupt();
    delay_ms_1(500);
    //init
    memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
    USART1_RX_STA = 0;    
    while(1)
    {
        halt();
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