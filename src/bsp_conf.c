/*************************************************************
Copyright (c) 2020 Shawn Yan, All rights reserved
-------------------------------------------------------------
File Name: bsp_conf.c

Desc     : 用于存放各类底层驱动文件

Author   : Shawn Yan

Date     : 2020-11-20

*************************************************************/

/*------------------------- Includes -----------------------*/
#include "bsp_conf.h"

/*----------- Global Definitions and Declarations ----------*/

/*-------------------- Type Declarations -------------------*/
const uint8_t beep_period_buf[E_BEEP_PERIOD_END] = {1, 2, 5, 6, 12, 63, 124, 250};
uint8_t USART1_RX_buf[USART1_RX_MAX_LEN];//USART1 receive buffer
uint8_t USART1_STA_buf[USART1_STA_MAX_LEN];//USART1 state buffer

/*------------------ Variable Declarations -----------------*/
volatile uint8_t key_flag = 0;       //key state flag
volatile uint32_t beep_play_time = 0; //record the beep play time
/**
 * @brief define the receive state of usart1 
 * @param bit [15] 0: waiting for receive, 1: received a set of data
 * @param bit [14:0] record the length of data
 */
volatile uint16_t USART1_RX_STA = 0;
 
/*------------------- Function Prototype -------------------*/

/*------------------- Function Implement -------------------*/
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
void bsp_gpio_init(void)
{   
    __disable_interrupt();
    //Initialization of all used pin, change the mode if needed.
    GPIO_Init(LEDG_PORT, LEDG_PIN, GPIO_Mode_Out_PP_Low_Slow);        //Green led init
    GPIO_Init(LEDR_PORT, LEDR_PIN, GPIO_Mode_Out_PP_Low_Slow);        //Red led init
    GPIO_Init(LOWV_PORT, LOWV_PIN, GPIO_Mode_In_FL_No_IT);            //Low pwr detection, opening interrupt when used.
    GPIO_Init(KEY_PORT, KEY_PIN, GPIO_Mode_In_PU_No_IT);              //key detection, opening interrupt when used.
    GPIO_Init(BLE_CTS_PORT, BLE_CTS_PIN, GPIO_Mode_In_PU_No_IT);        //ble CTS state/active low
    GPIO_Init(BLE_RTS_PORT, BLE_RTS_PIN, GPIO_Mode_Out_PP_High_Slow);    //ble RTS state/active low
    GPIO_Init(BEEP_PORT, BEEP_PIN, GPIO_Mode_Out_PP_Low_Slow);        //beep init
    GPIO_Init(UART_RX_PORT, UART_RX_PIN, GPIO_Mode_In_PU_No_IT);      //UART receive init
    GPIO_Init(UART_TX_PORT, UART_TX_PIN, GPIO_Mode_Out_PP_High_Fast); //UART transmitt init
    GPIO_Init(MOTOA_PORT, MOTOA_PIN, GPIO_Mode_Out_PP_Low_Slow);      //motor A init
    GPIO_Init(MOTOB_PORT, MOTOB_PIN, GPIO_Mode_Out_PP_Low_Slow);      //motor B init
    //Initialization of EXIT
    EXTI_SetPinSensitivity(KEY_EXTI_PIN, EXTI_Trigger_Falling); //key trigger falling
    EXTI_SetPinSensitivity(EXTI_Pin_2, EXTI_Trigger_Falling); //ble trigger falling
    //IT Priority
    ITC_SetSoftwarePriority(EXTI4_IRQn,ITC_PriorityLevel_1); //key 1
    ITC_SetSoftwarePriority(EXTI2_IRQn,ITC_PriorityLevel_1); //key 1
    //Set unused pin mode: IN_PU_NO_IT
    GPIO_Init(GPIOA, PA_UNUSED_PIN, GPIO_Mode_In_PU_No_IT);
    GPIO_Init(GPIOB, PB_UNUSED_PIN, GPIO_Mode_In_PU_No_IT);
    GPIO_Init(GPIOC, PC_UNUSED_PIN, GPIO_Mode_In_PU_No_IT);
    GPIO_Init(GPIOD, PD_UNUSED_PIN, GPIO_Mode_In_PU_No_IT);
}
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
void bsp_clk_init(void)
{
    CLK_HSICmd(ENABLE);                           //internal RC 16M enable
    CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI); //choose HSI as system clk
    CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);         // div 1

    //peripharal clk state
    //comment when used
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM1, DISABLE);
    //CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, DISABLE);
    //CLK_PeripheralClockConfig(CLK_Peripheral_TIM3, DISABLE);
    //CLK_PeripheralClockConfig(CLK_Peripheral_TIM4,DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_I2C1, DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_SPI1, DISABLE);
    //CLK_PeripheralClockConfig(CLK_Peripheral_USART1,DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_BEEP, DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_RTC, DISABLE);
}

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
void bsp_key_detec(void)
{
    LEDG_L();
    LEDR_L();
    bsp_tim2_init(12500);//100ms upload
    delay_ms_1(20);//avoid shaking
    if(!KEY_READ())
    {
        TIM2_Cmd(ENABLE);
        while(!KEY_READ());
        TIM2_Cmd(DISABLE);
        CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, DISABLE); //enable the clk
    }
    if (30 > key_flag)//short press
    {
#if(RELAY_DEV == DEVICE_ID)
        // USART1_RX_STA = 0;
        // memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
        // BLE_MESH();
        AT_Send("+++");
        AT_Send("AT+DEV_DEL=?\r\n");
        AT_Send("AT+EXIT\r\n");
        // BLE_Init();
#else
    
#endif

    }
    if (30 <= key_flag)//long press
    {
        BLE_Init();
    }
    key_flag = 0;
}


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
void bsp_tim2_init(uint16_t period)
{
    TIM2_DeInit();
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, ENABLE); //enable the clk
#if (SYS_CLK_FREQ_16M == SYS_CLK_FREQ)
    TIM2_TimeBaseInit(TIM2_Prescaler_128, TIM2_CounterMode_Up, period);
#elif (SYS_CLK_FREQ_8M == SYS_CLK_FREQ)
    TIM2_TimeBaseInit(TIM2_Prescaler_64, TIM2_CounterMode_Up, period);
#elif (SYS_CLK_FREQ_4M == SYS_CLK_FREQ)
    TIM2_TimeBaseInit(TIM2_Prescaler_32, TIM2_CounterMode_Up, period);
#elif (SYS_CLK_FREQ_2M == SYS_CLK_FREQ)
    TIM2_TimeBaseInit(TIM2_Prescaler_16, TIM2_CounterMode_Up, period);
#elif (SYS_CLK_FREQ_1M == SYS_CLK_FREQ)
    TIM2_TimeBaseInit(TIM2_Prescaler_8, TIM2_CounterMode_Up, period);
#else
#error "Invalid system clk value..."
#endif
    TIM2_ARRPreloadConfig(ENABLE); //enable auto reload
    TIM2_ClearFlag(TIM2_FLAG_Update);
    TIM2_ITConfig(TIM2_IT_Update, ENABLE);
    ITC_SetSoftwarePriority(TIM2_UPD_OVF_TRG_BRK_IRQn, ITC_PriorityLevel_3);//priority 3(HIGHEST)
    TIM2_Cmd(DISABLE);
}
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
void TIM2_IRQHandler(void)
{
    if(TIM2_GetFlagStatus(TIM2_FLAG_Update) != RESET)
    {
        if (255 == key_flag)
            TIM2_Cmd(DISABLE);
        else
            key_flag++;
        TIM2_SetCounter(0);//recounter    
    }
    TIM2_ClearITPendingBit(TIM2_IT_Update);
}
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
void bsp_tim3_init(uint16_t period)
{
    TIM3_DeInit();
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM3, ENABLE); //enable the clk
#if (SYS_CLK_FREQ_16M == SYS_CLK_FREQ)
    TIM3_TimeBaseInit(TIM3_Prescaler_128, TIM3_CounterMode_Up, period);
#elif (SYS_CLK_FREQ_8M == SYS_CLK_FREQ)
    TIM3_TimeBaseInit(TIM3_Prescaler_64, TIM3_CounterMode_Up, period);
#elif (SYS_CLK_FREQ_4M == SYS_CLK_FREQ)
    TIM3_TimeBaseInit(TIM3_Prescaler_32, TIM3_CounterMode_Up, period);
#elif (SYS_CLK_FREQ_2M == SYS_CLK_FREQ)
    TIM3_TimeBaseInit(TIM3_Prescaler_16, TIM3_CounterMode_Up, period);
#elif (SYS_CLK_FREQ_1M == SYS_CLK_FREQ)
    TIM3_TimeBaseInit(TIM3_Prescaler_8, TIM3_CounterMode_Up, period);
#else
#error "Invalid system clk value..."
#endif
    TIM3_ARRPreloadConfig(ENABLE); //enable auto reload
    TIM3_ClearFlag(TIM3_FLAG_Update);
    TIM3_ITConfig(TIM3_IT_Update, ENABLE);
    ITC_SetSoftwarePriority(TIM3_UPD_OVF_TRG_BRK_IRQn, ITC_PriorityLevel_3);//priority 3(HIGHEST)
    TIM3_Cmd(DISABLE);
}
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
void TIM3_IRQHandler(void)
{
    if(TIM3_GetFlagStatus(TIM3_FLAG_Update) != RESET)
    {
        USART1_RX_STA |= (uint16_t) 1<<15;//receive finished
        TIM3_ClearITPendingBit(TIM3_IT_Update);
        TIM3_Cmd(DISABLE);
        return;
    }
}
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
void bsp_tim4_init(uint8_t period)
{
    TIM4_DeInit();
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE); //enable the clk
#if (SYS_CLK_FREQ_16M == SYS_CLK_FREQ)
    TIM4_TimeBaseInit(TIM4_Prescaler_128, period);
#elif (SYS_CLK_FREQ_8M == SYS_CLK_FREQ)
    TIM4_TimeBaseInit(TIM4_Prescaler_64, period);
#elif (SYS_CLK_FREQ_4M == SYS_CLK_FREQ)
    TIM4_TimeBaseInit(TIM4_Prescaler_32, period);
#elif (SYS_CLK_FREQ_2M == SYS_CLK_FREQ)
    TIM4_TimeBaseInit(TIM4_Prescaler_16, period);
#elif (SYS_CLK_FREQ_1M == SYS_CLK_FREQ)
    TIM4_TimeBaseInit(TIM4_Prescaler_8, period);
#else
#error "Invalid system clk value..."
#endif
    TIM4_ARRPreloadConfig(ENABLE); //enable auto reload
    TIM4_ClearFlag(TIM4_FLAG_Update);
    TIM4_ITConfig(TIM4_IT_Update, ENABLE);
    ITC_SetSoftwarePriority(TIM4_UPD_OVF_TRG_IRQn, ITC_PriorityLevel_3);//priority 3
    TIM4_Cmd(ENABLE);
}
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
void bsp_beep_it(void)
{
    if (beep_play_time > 0)
    {
        BEEP_R();
        beep_play_time -= 1;
    }
    else
    {
        BEEP_L();
        TIM4_Cmd(DISABLE);
        TIM4_DeInit();
        CLK_PeripheralClockConfig(CLK_Peripheral_TIM4,DISABLE);
    }
    TIM4_ClearITPendingBit(TIM4_IT_Update);
}
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
void bsp_beep_freq(uint8_t freq)
{
    if (E_BEEP_PERIOD_10US == freq)
        beep_play_time = 100;
    else if (E_BEEP_PERIOD_20US == freq)
        beep_play_time = 50;
    else if (E_BEEP_PERIOD_40US == freq)
        beep_play_time = 25;
    else if (E_BEEP_PERIOD_50US == freq)
        beep_play_time = 20;
    else if (E_BEEP_PERIOD_100US == freq)
        beep_play_time = 10;
    else if (E_BEEP_PERIOD_500US == freq)
        beep_play_time = 2;
    else if (E_BEEP_PERIOD_1MS == freq)
        beep_play_time = 1;    
    else if (E_BEEP_PERIOD_2MS == freq)
        // beep_play_time = 0.5;//error value, avoid to use it
        return;
    else
        return;//invalid value
}
/*************************************************************
Function Name       : bsp_beep_play_ms
Function Description: used to control the play time
Param_in            : u8 freq, u16 time
Param_out           : 
Return Type         : 
Note                : range from 2 to 65535ms
Author              : Yan
Time                : 2020-11-26
*************************************************************/
void bsp_beep_play_ms(uint8_t freq, uint16_t time)
{
    bsp_beep_freq(freq);
    beep_play_time *= time;
    bsp_tim4_init(beep_period_buf[freq]);
}
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
void beep_play(uint8_t style)
{
    LEDG_L();
    LEDR_L();
    switch (style)
    {
    case E_BEEP_MODE_INIT:
        bsp_beep_play_ms(E_BEEP_PERIOD_500US, 300);
        delay_ms_1(300);
        bsp_beep_play_ms(E_BEEP_PERIOD_500US, 300);
        goto EXIT;
        break;
    case E_BEEP_MODE_ERR:
        bsp_beep_play_ms(E_BEEP_PERIOD_100US, 2000);
        LEDR_H();
        delay_ms_1(2000);
        LEDR_L();
        break;
    case E_BEEP_MODE_SUCCESS:
        LEDR_L();
        LEDG_H();
        delay_ms_1(100);
        bsp_beep_play_ms(E_BEEP_PERIOD_100US, 250);
        LEDR_H();
        LEDG_L();
        delay_ms_1(100);
        bsp_beep_play_ms(E_BEEP_PERIOD_100US, 250);
        LEDR_L();
        LEDG_H();
        delay_ms_1(100);
        bsp_beep_play_ms(E_BEEP_PERIOD_100US, 250);
        LEDR_H();
        LEDG_L();
        delay_ms_1(100);
        bsp_beep_play_ms(E_BEEP_PERIOD_100US, 250);
        LEDR_L();
        LEDG_L();
        goto EXIT;
        break;
    case E_BEEP_MODE_RX:
        
        break;
    case E_BEEP_MODE_WAIT:

        break;
    default:
        break;
    }
EXIT:
    return;
}
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
void bsp_uart_init(void)
{
    
    USART_DeInit(USART1);
    CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);
    /**
    * @param USART1 ,
    * @param BR: 115200, 
    * @param WL: 8bit, 
    * @param SB: 1 bit, 
    * @param PR: no parity, 
    * @param MODE: RX/TX mode 
    */
    USART_Init(USART1, (uint32_t)115200, 
                        USART_WordLength_8b, 
                        USART_StopBits_1, 
                        USART_Parity_No, 
                        (USART_Mode_Tx | USART_Mode_Rx));
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    ITC_SetSoftwarePriority(USART1_RX_IRQn, ITC_PriorityLevel_2);//priority 2
    USART_Cmd(USART1, ENABLE);
    USART1_RX_STA = 0;
    // bsp_tim3_init(1250);//Enter interrupt after 10ms
    bsp_tim3_init(500);//Enter interrupt after 4ms
}
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
void USART1_IRQHandler(void)
{
    
    uint8_t receive_data;
    if(USART_GetITStatus(USART1,USART_IT_RXNE) != RESET)        //Check whether the specified UART1 interrupt occurs. 
    {
        receive_data = USART_ReceiveData8(USART1);
        if((USART1_RX_STA & (uint16_t)(1<<15)) == 0)                      
        {
            if(USART1_RX_STA < USART1_RX_MAX_LEN)               //Determine whether the byte overflows
            {
                TIM3_SetCounter(0);//recount
                if(0 == USART1_RX_STA)
                    TIM3_Cmd(ENABLE);
                USART1_RX_buf[USART1_RX_STA++] = receive_data;
            }
            else
            {
                USART1_RX_STA |= (uint16_t) 1<<15;//enforce finish receive 
                TIM3_ClearITPendingBit(TIM3_IT_Update);
                TIM3_Cmd(DISABLE);
            }    
        }

        USART_ClearITPendingBit(USART1,USART_IT_RXNE);            //Clear UART1 pending flag
    }
}
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
void mode_IRQHandler(void)
{
    delay_ms_1(100);
    GPIO_Init(UART_RX_PORT, UART_RX_PIN, GPIO_Mode_In_PU_No_IT);      //UART receive init
    CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, ENABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM3, ENABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM4,ENABLE);
    
    
    USART_Cmd(USART1, ENABLE);
    EXTI_ClearITPendingBit(EXTI_IT_Pin2);
    myflag.SYS_STA_flag = 1;
}
/*************************************************************
Function Name       : USART1_SendWord
Function Description: Transmits 8 bit *data through the USART1 peripharal
Param_in            : *Data
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2020-11-23
*************************************************************/
void USART1_SendWord(uint8_t *Data)
{
    while (*Data)
    {
        USART_SendData8(USART1, *Data++);
        while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));
    }
}

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
void data_packet_process(uint8_t *longdata)
{
    uint16_t i = strlen(longdata);
    uint8_t *Data;
    Data = longdata;
    if (i <= PACKET_MAX_LEN)//if user sends the short data(<20byte)
    {
        USART1_SendWord(longdata);
        return;
    }
    while (i > PACKET_MAX_LEN)//process the packet of long data
    {
        uint8_t temp = 0;//record the num of bytes
        for (temp = 0; temp < PACKET_MAX_LEN; temp++)
        {
            USART_SendData8(USART1, *Data++);
            while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));
        }
        i -= PACKET_MAX_LEN;
        delay_ms_1(25);
        
    }
    while(*Data)//send the remained data
    {
        USART_SendData8(USART1, *Data++);
        while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));
    }
}
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
void node_info_query(void)
{
    uint8_t *temp_string;//intermediate variables
    // uint8_t *node_info_string = "N2: 0\r\nN3: 0\r\nN4: 0\r\nN5: 0\r\nN6: 0\r\nN7: 0\r\nN8: 0\r\nN9: 0\r\n";//record the status of node
    uint8_t k = 4, j = 0;
    temp_string = (uint8_t *)ctrl_string;
    temp_string += 3;
    uint8_t i =strlen(temp_string);
    i *= 7;
    uint8_t *node_info_string = (uint8_t *)malloc(i+1);//record the status of node
    memset(node_info_string, 0, sizeof(node_info_string));
    for (i = 0; i < (strlen(temp_string) * 7); i+=7)
    {
        node_info_string[i] = 'N';
        node_info_string[i+1] = '2' + j;
        node_info_string[i+2] = ':';
        node_info_string[i+3] = ' ';
        node_info_string[i+4] = '0';
        node_info_string[i+5] = '\r';
        node_info_string[i+6] = '\n';
        j++;
    }
    
    node_info_string+=4;
    *node_info_string = *temp_string;
    temp_string++;   
    while (*temp_string)
    {
        node_info_string+=7;
        *node_info_string = *temp_string;
        k+=7;
        temp_string++;
    }
    node_info_string -= k;
    data_packet_process(node_info_string);
    free(node_info_string);//release the rom
    node_info_string = NULL;
    
}
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
uint8_t scan_packet_process(uint16_t scan_cnt)
{
    uint8_t flag = myflag.MAC_NUM_flag;
    uint8_t t;
    uint16_t retry = scan_cnt;
    uint8_t *rx_buf_ptr = USART1_RX_buf;//debug monitoring pointer
    uint8_t *sta_buf_ptr =USART1_STA_buf;//debug monitoring pointer
    uint8_t *mac_addr = (uint8_t *)malloc(19);
    mac_addr[18] = '\n';
    mac_addr[17] = '\r';
    USART1_RX_STA = 0;
    memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
    memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
    AT_Send("AT+S_NAME=0\r\n");//stop scan
    AT_Send("AT+S_NAME=1\r\n");//start scan
    while (retry--)
    {
        if (3 == flag)
            break;
        for (t = 0; t < 50; t++)//50 ms redundancy
        {
            if(USART1_RX_STA & 0x8000)
                break;
            delay_ms_1(1);
        }
        if ((USART1_RX_STA & 0x8000))//receive the data
        {
            t = USART1_RX_STA & 0x7FFF;//get the length of data
            USART1_RX_STA = 0;
            if (200 == t)//buffer overflow
            {
                memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
                memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
                continue;
            }
            if ('M' != USART1_RX_buf[0])//message filtter
            {
                memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
                memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
                continue;
            }
            uint8_t cur_cnt = 0;//record current count
            uint8_t last_cnt = 0;//record last count
            while (t > cur_cnt)
            {
                if ('\n' == USART1_RX_buf[cur_cnt])
                {
                    
                    if (('_' == USART1_RX_buf[cur_cnt-6]) &&
                        ('R' == USART1_RX_buf[cur_cnt-12]))
                    {
                        last_cnt += 4;
                        for (uint8_t i = 0; i < 17; i++)
                        {
                            mac_addr[i] = USART1_RX_buf[last_cnt];
                            last_cnt++;
                        }
                        AT_Send("AT+S_NAME=0\r\n");
                        AT_Send((uint8_t *)(connect2("AT+CONNECT=,", mac_addr)));
                        for (uint8_t i = 0; i < 100; i++)//delay 1s
                        {
                            delay_ms_1(10);
                            if(USART1_RX_STA & 0x8000)
                                break;
                        }
                        if ((USART1_RX_STA & 0x8000))
                        {
                            t = USART1_RX_STA & 0x7FFF;//get the length of data
                            USART1_RX_STA = 0;
                            if ('C' == USART1_RX_buf[t-6])//successfully connected
                            {
                                AT_Get_Cnt_List();
                                uint8_t handler = strStr(sta_buf_ptr, mac_addr) - 3;
                                uint8_t a[] = "1\r\n";
                                a[0] = USART1_STA_buf[handler];
                                memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
                                AT_Send("AT+TTM_ROLE=1\r\n");
                                AT_Send((uint8_t *)connect2("AT+TTM_HANDLE=", a));
                                AT_Send("AT+EXIT\r\n");//exit AT mode
                                if (0 == (BLE_Send("pairing request")))
                                {
                                    flag += 1;
                                    AT_Send("+++");
                                    AT_Get_State("MAC");
                                    AT_Send("AT+EXIT\r\n");
                                    BLE_Send((uint8_t *)connect2("1 ", sta_buf_ptr));
                                    memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
                                    AT_Send("+++");
                                    
                                    AT_Send("AT+S_NAME=1\r\n");
                                    break;
                                }
                                else
                                {
                                    AT_Send("+++");
                                    AT_Send((uint8_t *)connect2("AT+DISCONNECT=2,", a));
                                    AT_Send("AT+S_NAME=1\r\n");
                                    break;
                                }
                            }
                        }
                        
                    }
                    if (t == (cur_cnt + 1))//the end
                    {
                        break;
                    }
                    last_cnt = cur_cnt + 1;
                }
                cur_cnt++;
            }
        }
        USART1_RX_STA = 0;
        memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
    }
    AT_Send("AT+S_NAME=0\r\n");
    //clear the rx buffer
    USART1_RX_STA = 0;
    memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
    memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
    
    myflag.MAC_NUM_flag = flag;
    return flag;
    
}
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
void request_msg_process(void)
{
    
    if ((USART1_RX_STA & 0x8000) && !myflag.LINK_STA_flag)//not linked yet
    {
        uint8_t t = USART1_RX_STA & 0x7fff;//get packet length
        USART1_RX_STA = 0;
        if (('S' == USART1_RX_buf[0]) && ('C' == USART1_RX_buf[2]))//be connected
        {
            myflag.LINK_STA_flag = 1;
            
        }
        memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
    }
    if ((USART1_RX_STA & 0x8000) && myflag.LINK_STA_flag)//get msg
    {
        uint8_t t = USART1_RX_STA & 0x7fff;//get packet length
        USART1_RX_STA = 0;
        if (('S' == USART1_RX_buf[0]) && ('D' == USART1_RX_buf[2]))//be disconnected
        {
            myflag.LINK_STA_flag = 0;//stop linked flag
            uint8_t *sta_buf_ptr = USART1_STA_buf;
            AT_Send("+++");
            AT_Send((uint8_t *)connect2("AT+CONNECT=,", sta_buf_ptr));
            for (uint8_t i = 0; i < 100; i++)//delay 1s
            {
                delay_ms_1(10);
                if(USART1_RX_STA & 0x8000)
                    break;
            }
            if ((USART1_RX_STA & 0x8000))
            {
                t = USART1_RX_STA & 0x7FFF;//get the length of data
                USART1_RX_STA = 0;
                if ('C' == USART1_RX_buf[t-6])//successfully connected
                {
                    myflag.MAC_NUM_flag += 1;
                    AT_Send("AT+DISCONNECT\r\n");
                    AT_Send("AT+EXIT\r\n");
                    memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
                    BLE_MESH();
                }
            }
        }    
        if (('p' == USART1_RX_buf[0]) && ('a' == USART1_RX_buf[1]))//be connected
        {
            BLE_Name_Change(ENABLE);
            AT_Send("+++");
            AT_Send("AT+TTM_ROLE=0\r\n");
            AT_Send("AT+EXIT\r\n");
            USART1_SendWord("y");
        }
        if (('1' == USART1_RX_buf[0]) && (' ' == USART1_RX_buf[1]))//msg format：1 xx:xx:xx:xx:xx:xx\r\n
        {
            for (uint8_t i = 0; i < 19; i++)
            {
                USART1_STA_buf[i] = USART1_RX_buf[i+2];
            }
            USART1_SendWord("y");
        }
    
        memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
    }
}
/**
 * 模拟串口打印输出调试信息
 * 波特率9600，即电平改变间隔1/9600=104us
 * 起始位为0，8位数据，无奇偶校验位，一位停止位
 * 使用PB3打印输出，修改时应修改main.h的宏定义
 */
#if (SIM_UART_PRINTF_EN)
void sim_uart_printf(uint8_t data)
{
    uint8_t i;
    uint8_t temp_value = 0;
    SIM_RESET();//发送起始位
    delay_us_1(100);//校正值
    for (i = 0; i < 8; i++)
    {
        temp_value = (data & 0x01);
        if(temp_value)
        {
            SIM_SET();
        }
        if(!temp_value)
        {
            SIM_RESET();
        }
        delay_us_1(100);
        data >>= 1;
    }
    SIM_SET();//发送停止位
    delay_us_1(100);
}
//模拟发送字符串
void sim_printf_string(uint8_t *str)
{
    while (*str != 0)
    {
        sim_uart_printf(*str++);
    }
}
//模拟发送数据
const uint8_t hex_tab[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
void sim_printf_hex(uint8_t data)
{
    sim_uart_printf('0');
    sim_uart_printf('x');
    sim_uart_printf(hex_tab[data >> 4]);//高4位
    sim_uart_printf(hex_tab[data & 0x0F]);//低4位
    sim_uart_printf(' ');

}
#endif
/*--------------------------- END --------------------------*/