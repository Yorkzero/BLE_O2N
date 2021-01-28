/*************************************************************
Copyright (c) 2020 Shawn Yan, All rights reserved
-------------------------------------------------------------
File Name: bsp_conf.h

Desc     : 底层驱动关联头文件

Author   : Shawn Yan

Date     : 2020-11-20

*************************************************************/
#ifndef _BSP_CONF_H_
#define _BSP_CONF_H_
/*------------------------- Includes -----------------------*/
#include "main.h"

/*----------- Global Definitions and Declarations ----------*/
//USART1相关宏定义
#define USART1_RX_MAX_LEN      200  //最大接收缓存字节
#define USART1_STA_MAX_LEN     100  //最大状态量缓存字节
extern volatile uint8_t key_flag;   //key state flag
extern volatile uint32_t beep_play_time; //record the beep play time
extern volatile uint16_t USART1_RX_STA;  //record the receive data
extern uint8_t USART1_RX_buf[USART1_RX_MAX_LEN];//USART1 receive buffer
extern uint8_t USART1_STA_buf[USART1_STA_MAX_LEN];//USART1 state buffer
/*-------------------- Type Declarations -------------------*/


/*------------------ Variable Declarations -----------------*/



/*------------------- Function Prototype -------------------*/
/*************************************************************
Function Name       : bsp_gpio_init
Function Description: gpio initialization
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2020-11-20
*************************************************************/
void bsp_gpio_init(void);
/*************************************************************
Function Name       : bsp_clk_init
Function Description: system clk initializatiom
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2020-11-20
*************************************************************/
void bsp_clk_init(void);
/*************************************************************
Function Name       : bsp_key_detec
Function Description: used to detec press time
Param_in            : 
Param_out           : 
Return Type         : 
Note                : short press: <3s
                      long press: >3s
Author              : Yan
Time                : 2020-11-30
*************************************************************/
void bsp_key_detec(void);
/*************************************************************
Function Name       : bsp_tim2_init
Function Description: initialization of TIM2
Param_in            : u16 period
Param_out           : 
Return Type         : 
Note                : used when key state detecting
Author              : Yan
Time                : 2020-11-30
*************************************************************/
void bsp_tim2_init(uint16_t period);
/*************************************************************
Function Name       : TIM2_IRQHandler
Function Description: TIM2 IT function
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2020-11-30
*************************************************************/
void TIM2_IRQHandler(void);
/*************************************************************
Function Name       : bsp_tim3_init
Function Description: initialization of TIM3
Param_in            : u16 period
Param_out           : 
Return Type         : 
Note                : used when usart1 receives data
Author              : Yan
Time                : 2020-11-27
*************************************************************/
void bsp_tim3_init(uint16_t period);
/*************************************************************
Function Name       : TIM3_IRQHandler
Function Description: TIM3 IT function
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2020-11-27
*************************************************************/
void TIM3_IRQHandler(void);
/*************************************************************
Function Name       : bsp_tim4_init
Function Description: initialization of TIM4
Param_in            : u8 period
Param_out           : 
Return Type         : 
Note                : used in beep play
Author              : Yan
Time                : 2020-11-26
*************************************************************/
void bsp_tim4_init(uint8_t period);
/*************************************************************
Function Name       : bsp_beep_it
Function Description: beep ctrl function used in TIM4 IRQHandler
Param_in            : 
Param_out           : 
Return Type         : 
Note                : use this function to play different kinds of music
Author              : Yan
Time                : 2020-11-26
*************************************************************/
void bsp_beep_it(void);
/*************************************************************
Function Name       : bsp_beep_freq
Function Description: set the frequency of beep
Param_in            : u8 freq
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2020-11-27
*************************************************************/
void bsp_beep_freq(uint8_t freq);
/*************************************************************
Function Name       : bsp_beep_play_ms
Function Description: used to control the play time
Param_in            : u8 freq, u16 time
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2020-11-26
*************************************************************/
void bsp_beep_play_ms(uint8_t freq, uint16_t time);
/*************************************************************
Function Name       : beep_play
Function Description: play different kinds of music
Param_in            : style
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2020-11-26
*************************************************************/
void beep_play(uint8_t style);
/*************************************************************
Function Name       : bsp_uart_init
Function Description: uart initialization
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2020-11-20
*************************************************************/
void bsp_uart_init(void);
/*************************************************************
Function Name       : USART1_IRQHandler
Function Description: usart1 IT function
Param_in            : 
Param_out           : 
Return Type         : 
Note                : The interval between each data can not exceed 10ms
Author              : Yan
Time                : 2020-11-27
*************************************************************/
void USART1_IRQHandler(void);
/*************************************************************
Function Name       : mode_IRQHandler
Function Description: change exti mode to usart mode
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2020-12-26
*************************************************************/
void mode_IRQHandler(void);
/*************************************************************
Function Name       : bsp_rtc_IRQHandler
Function Description: rtc IT function
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-28
*************************************************************/
void bsp_rtc_IRQHandler(void);
/*************************************************************
Function Name       : USART1_SendWord
Function Description: Transmits 8 bit *data through the USART1 peripharal
Param_in            : *Data
Param_out           : 
Return Type         : 
Note                : eg. USART1_SendWord("AT+NAME/r/n")
Author              : Yan
Time                : 2020-11-23
*************************************************************/
void USART1_SendWord(uint8_t *Data);
/*************************************************************
Function Name       : data_packet_process
Function Description: used to process and send long data(>20byte)
Param_in            : uint8_t *longdata
Param_out           : 
Return Type         : 
Note                : based on WH-BLE103
Author              : Yan
Time                : 2020-12-10
*************************************************************/
void data_packet_process(uint8_t *longdata);
/*************************************************************
Function Name       : node_info_query
Function Description: inquire the information of node
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2020-12-10
*************************************************************/
void node_info_query(void);
/*************************************************************
Function Name       : scan_packet_process
Function Description: used to process scan packet
Param_in            : uint16_t scan_cnt(1~50ms per scan)
Param_out           : 
Return Type         : uint8_t flag
Note                : the num of the slave device(x=0, 1, 2, 3)
Author              : Yan
Time                : 2021-01-15
*************************************************************/
uint8_t scan_packet_process(uint16_t scan_cnt);
/*************************************************************
Function Name       : request_msg_process
Function Description: used to deal with the request message after connected
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-26
*************************************************************/
void request_msg_process(void);
//模拟串口打印
#if (SIM_UART_PRINTF_EN)
extern void sim_uart_printf(uint8_t data);

extern void sim_printf_string(uint8_t *str);
extern void sim_printf_hex(uint8_t data);

#else
#define sim_uart_printf(N)

#define sim_printf_string(N)
#define sim_printf_hex(N)
#endif
/*------------------- Function Implement -------------------*/

#endif
/*--------------------------- END --------------------------*/