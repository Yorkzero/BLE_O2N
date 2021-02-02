/*************************************************************
Copyright (c) 2020 Shawn Yan, All rights reserved
-------------------------------------------------------------
File Name: user_app.c

Desc     : user application

Author   : Shawn Yan

Date     : 2020-11-23

*************************************************************/
/*------------------------- Includes -----------------------*/
#include "user_app.h"

/*----------- Global Definitions and Declarations ----------*/


/*-------------------- Type Declarations -------------------*/

/*------------------ Variable Declarations -----------------*/
volatile myFlag myflag = {
                            1,  //BLE state flag 0:MESH, 1:NON-MESH
                            1,  //LOCK state flag 0:LOCKED, 1:UNLOCKED
                            1,  //Determine whether there is a duplication 0:repeat, 1:no repeat
                            0,  //initialization state flag 0:not initialized yet, 1:initialized ready
                            0,  //used to record the number of bound mac addresses, x=0, 1, 2, 3
                            0,  ////used to enter status of msg exc 0:not ready, 1:ready
                            0,  //Determine whether to send hop msg or ending control msg 0:hop, 1:end
                         }; 

/*------------------- Function Prototype -------------------*/


/*------------------- Function Implement -------------------*/

#if (1 == CODE_VERSION)//Version 1  
/*************************************************************
Function Name       : AT_Send
Function Description: send AT cmd
Param_in            : uint8_t *atcmd
Param_out           : 
Return Type         : u16 tag
Note                : 0: succeed/1: failed
                      based on WH-BLE103
Author              : Yan
Time                : 2020-11-27
*************************************************************/
uint8_t AT_Send(uint8_t *atcmd)
{
    uint16_t tag = 1;
    uint8_t t;
    USART1_RX_STA = 0;
    memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
    uint8_t retry = 10;//number of AT command sending attempts
    while (retry--)
    {
        if (0 == BLE_CTS_READ())
        {
            USART1_SendWord(atcmd);
            delay_ms_1(2);
        }
        for (t = 0; t < 60; t++)
        {
            if(USART1_RX_STA & 0x8000)
                break;
            delay_ms_1(1);
        }
        if ((USART1_RX_STA & 0x8000))//receive the data
        {
            tag = USART1_RX_STA & 0x7FFF;//get the length of data
            USART1_RX_STA = 0;//clear the state flag
            if ((('O' == USART1_RX_buf[tag-4]) &&
                ('K' == USART1_RX_buf[tag-3])) ||
                (('o' == USART1_RX_buf[tag-4]) &&
                ('k' == USART1_RX_buf[tag-3])))
            {
                tag = 0;//enter succeed
                break;
            }
        }
    }
    //clear the rx buffer
    USART1_RX_STA = 0;
    memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
    if(0 == retry)tag = 1;//enter failed
    return tag;
}
#endif
/*************************************************************
Function Name       : BLE_Send
Function Description: send BLE cmd
Param_in            : uint8_t *atcmd
Param_out           : 
Return Type         : u16 tag
Note                : 0: succeed/1: failed
Author              : Yan
Time                : 2020-11-27
*************************************************************/
uint8_t BLE_Send(uint8_t *atcmd)
{
    uint16_t tag = 1;
    uint8_t t;
    USART1_RX_STA = 0;
    memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
    uint8_t retry = 10;//number of AT command sending attempts
    while (retry--)
    {
        if (0 == BLE_CTS_READ())
        {
            USART1_SendWord(atcmd);
            delay_ms_1(2);
        }
        for (t = 0; t < 200; t++)//delay 1s
        {
            if(USART1_RX_STA & 0x8000)
                break;
            delay_ms_1(5);
        }
        if ((USART1_RX_STA & 0x8000))//receive the data
        {
            USART1_RX_STA = 0;//clear the state flag
            if ('y' == USART1_RX_buf[0])//Successful handshake
            {
                tag = 0;
                break;
            }
            else if ('n' == USART1_RX_buf[0])//falied handshake
            {
                tag = 1;
                break;
            }
        }
    }
    //clear the rx buffer
    USART1_RX_STA = 0;
    memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
    if(0 == retry)tag = 1;//enter failed
    return tag;
}
/*************************************************************
Function Name       : AT_Get_State
Function Description: use AT to get BLE state
Param_in            : char *sta
                      allow to query the following status:
                      NAME/MODE/MAC/CIVER/TPL/PASS/PASSEN/
                      UART/UARTIM/AUTOSLEEP/DEEPSLEEP/
                      HIBERNATE/HELLO/LINK/UUID etc.
Param_out           : USART1_STA_buf[]
                      remeber to clear the state buffer after used.
Return Type         : u16 flag
Note                : 0: succeed/1: failed
Author              : Yan
Time                : 2020-11-27
*************************************************************/
uint8_t AT_Get_State(char *sta)
{
    uint16_t tag = 1;
    uint16_t temp = 1;
    uint8_t t;
    uint8_t retry = 10;
    uint8_t stalen = 0;
    char *stastring = connect2("AT+", sta);
    while (*sta)    //get the length of sta
    {
        *sta ++;
        stalen += 1;
    }
    while (retry--)
    {
        USART1_SendWord((uint8_t*) (stastring));
        USART1_SendWord("?\r\n");
        delay_ms_1(10);
        for ( t = 0; t < 10; t++)//50ms outtime
        {
            if(USART1_RX_STA & 0x8000)
                break;
            delay_ms_1(5);
        }
        if ((USART1_RX_STA & 0x8000))//receive the data
        {
            temp = USART1_RX_STA & 0x7FFF;//get the length of data
            USART1_RX_STA = 0;//clear the state flag
            if (
                ('O' == USART1_RX_buf[temp-4]) &&
                ('K' == USART1_RX_buf[temp-3]) )
            {
                tag = 0;//enter succeed 
                break;  
            }
            
        }
    }
    if(!tag && (0 != retry))
    {
        uint16_t i = 0;  
        for (t = 4 + stalen; t < (temp - 4); t++)
        {   //storage the relevant data in the buffer, empty it when access.   
            USART1_STA_buf[i] = USART1_RX_buf[t];
            i++;
        }
    }
    //clear the rx buffer
    memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
    if(0 == retry)tag = 1;//enter failed
    return tag;
}
/*************************************************************
Function Name       : AT_Get_Cnt_List
Function Description: get the connected device list
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-26
*************************************************************/
uint8_t AT_Get_Cnt_List(void)
{
    uint16_t tag = 1;
    uint16_t temp = 1;
    uint8_t t;
    uint8_t retry = 10;
    uint8_t stalen = 8;
    USART1_RX_STA = 0;
    memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
    while (retry--)
    {
        USART1_SendWord("AT+CNT_LIST\r\n");
        delay_ms_1(10);
        for ( t = 0; t < 10; t++)//50ms outtime
        {
            if(USART1_RX_STA & 0x8000)
                break;
            delay_ms_1(5);
        }
        if ((USART1_RX_STA & 0x8000))//receive the data
        {
            temp = USART1_RX_STA & 0x7FFF;//get the length of data
            USART1_RX_STA = 0;//clear the state flag
            if (
                ('O' == USART1_RX_buf[temp-4]) &&
                ('K' == USART1_RX_buf[temp-3]) )
            {
                tag = 0;//enter succeed 
                break;  
            }
            
        }
    }
    if(!tag && (0 != retry))
    {
        uint16_t i = 0;  
        for (t = 5 + stalen; t < (temp - 4); t++)
        {   //storage the relevant data in the buffer, empty it when access.   
            USART1_STA_buf[i] = USART1_RX_buf[t];
            i++;
        }
    }
    //clear the rx buffer
    memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
    if(0 == retry)tag = 1;//enter failed
    return tag;
}
/*************************************************************
Function Name       : BLE_Name_Change
Function Description: change device name to notify that mesh finished
Param_in            : ENABLE or DISABLE
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-25
*************************************************************/
void BLE_Name_Change(FunctionalState Newstate)
{
    AT_Send("+++");
    assert_param(IS_FUNCTIONAL_STATE(Newstate));
    if (ENABLE == Newstate)
    {
        AT_Get_State("NAME");
        uint8_t *sta_ptr = USART1_STA_buf;
        USART1_STA_buf[0] = 'r';
        AT_Send((uint8_t *)(connect2("AT+NAME=", sta_ptr)));
    }
    if (DISABLE == Newstate)
    {
        AT_Get_State("NAME");
        uint8_t *sta_ptr = USART1_STA_buf;
        USART1_STA_buf[0] = 'R';
        AT_Send((uint8_t *)(connect2("AT+NAME=", sta_ptr)));
    }
    memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
    AT_Send("AT+EXIT\r\n");
}
/*************************************************************
Function Name       : BLE_status_it
Function Description: Use peripherals to express BLE status
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2020-11-30
*************************************************************/
void BLE_status_it(void)
{
/**
   Non-MESH mode
            NAME: Dx >>>Device x (x=1, 2, 3,...)
        LED MODE: Flashing LEDG in freq 1000ms
        KEY MODE: Enter MESH Mode through short press(<3s)
       LINK PROC: Flashing LEDR in freq 100ms(NU)
         LINK OK: The LEDR(G) flash alternately for 1.5s with beep play
        LINK ERR: The buzzer beeps for 2s and the LEDR is on
       MESH mode
            NAME: Nx >>>Node x (x=1, 2, 3,...)
        LED MODE: Flashing LEDR in freq 1000ms
        KEY MODE: Exit MESH Mode through long press(>3s)
    DISLINK PROC: Flashing LEDG in freq 100ms(NU)
      DISLINK OK: The LEDR(G) flash alternately for 1.5s with beep play
     DISLINK ERR: The buzzer beeps for 2s and the LEDG is on   
**/
    uint8_t i = 1;
    if (0 == AT_Send("+++a"))//enter AT mode
    {
        if (0 == AT_Get_State("MODE"))//get the mode
        {
            if ('e' == USART1_STA_buf[1])//MESH mode
            {
                memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));//clear the buffer
                myflag.BLE_STA_flag = 0;
            }
            else//Non-MESH mode
            {
                memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));//clear the buffer
                myflag.BLE_STA_flag = 1;
            }
            
        }
        while (AT_Send("AT+ENTM\r\n"))//exit AT mode
        {
            if((i++) >= 5)
            {                
                USART1_SendWord("Unable to exit AT mode...\r\n");
                break;
            }
        }
    }
    else
        USART1_SendWord("Unable to get the state...\r\n");
}
/*************************************************************
Function Name       : BLE_status_run
Function Description: show the BLE status in main
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2020-12-04
--------------------------------------------------------------
log:
data:  2021-1-19
note:  use this to display the mesh
*************************************************************/
void BLE_status_run(void)
{
    // LEDR_L();
    // LEDG_L();
    if (1 == myflag.LINK_STA_flag)//process link msg
    {
        LEDR_L();
        LEDG_L();
        return;
    }
    
    if (1 == myflag.BLE_STA_flag)//NON-MESH
    {
        LEDR_L();
        LEDG_R();
        delay_ms_1(500);
        LEDG_R();
        delay_ms_1(500);
    }
    else
    {
        LEDG_L();
        LEDR_R();
        delay_ms_1(500);
        LEDR_R();
        delay_ms_1(500);
    }
    
}
/*************************************************************
Function Name       : BLE_Init
Function Description: initialization of BLE
Param_in            : 
Param_out           : 
Return Type         : 
Note                : Pull down the RTS level for at least 50ms before use
Author              : Yan
Time                : 2021-01-15
*************************************************************/
void BLE_Init(void)
{
    // if (1 == myflag.INIT_STA_flag)//avoid init again
    //     return;
    GPIO_Init(BLE_CTS_PORT, BLE_CTS_PIN, GPIO_Mode_In_PU_No_IT);      //UART receive init
    CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);
    // CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, ENABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM3, ENABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM4,ENABLE);
    BLE_SEND_ENABLE();
    USART_Cmd(USART1, ENABLE);
    EXTI_ClearITPendingBit(EXTI_IT_Pin1);
    BLE_Name_Change(DISABLE);
    AT_Send("+++");//enter AT mode
    AT_Send("AT+ROLE=2\r\n");//set the role: slave and master
    AT_Send("AT+POWER=-28\r\n");//set the TX power as -28db
    AT_Send("AT+PACK=200,5\r\n");//set the pack range and frame as 200byte and 5ms outtime
    AT_Send("AT+CNT_INTERVAL=240\r\n");//set the connect interval as (240 * 1.25 = 300)ms
    AT_Send("AT+ADS=1,1,50\r\n");//set the advertise interval as 50ms
    AT_Send("AT+DEV_DEL=ALL\r\n");//delet all device
    AT_Send("AT+RESTART\r\n");//restart the device
    AT_Send("AT+EXIT\r\n");
    myflag.BLE_STA_flag = 1;
    myflag.LINK_STA_flag = 0;
    myflag.MAC_NUM_flag = 0;
    myflag.REPEAT_flag = 1;
    myflag.HOP_STA_flag = 0;
    myflag.INIT_STA_flag = 1;//init ok
    FSM_Transfer(&system_FSM, S_STA_INIT);
    beep_play(E_BEEP_MODE_INIT);
}
/*************************************************************
Function Name       : BLE_MESH
Function Description: add up to 3 slave device to list
Param_in            : 
Param_out           : 
Return Type         : uint8_t flag
Note                : 0: succeed/1: failed
Author              : Yan
Time                : 2021-01-15
*************************************************************/
uint8_t BLE_MESH(void)
{
    if (0 == myflag.BLE_STA_flag)//meshed
        return 0;
    uint8_t flag = 1;
    AT_Send("+++");//enter AT mode
    uint8_t num = scan_packet_process(100);
    if (0 == myflag.MAC_NUM_flag)//only with master
    {
        myflag.BLE_STA_flag = 0;
        flag = 0;
        AT_Send("AT+TTM_ROLE=0\r\n");
        AT_Send("AT+CNT_INTERVAL=480\r\n");//set the connect interval as (480 * 1.25 = 600)ms
        AT_Send("AT+ADS=0,1,500\r\n");//set the advertise interval as 0.5s
        AT_Get_State("MAC");
        AT_Send("AT+EXIT\r\n");
        uint8_t string_m[] = "m00:00 ";
        for (uint8_t i = 0; i < 5; i++)
        {
            string_m[i+1] = USART1_STA_buf[i+12];
        }
        memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
        USART1_SendWord(string_m);
        delay_ms_1(500);
        USART1_SendWord("done!");
        FSM_Transfer(&system_FSM, S_STA_MESH_OK);
        return flag;
    }
    else
    {
        BLE_FINISH_MESH(num);
    }
    myflag.BLE_STA_flag = 0;
    flag = 0;
    AT_Send("AT+CNT_INTERVAL=480\r\n");//set the connect interval as (480 * 1.25 = 600)ms
    AT_Send("AT+ADS=0,1,500\r\n");//set the advertise interval as 0.5s
    AT_Send("AT+EXIT\r\n");
    return flag;
}
/*************************************************************
Function Name       : BLE_FINISH_MESH
Function Description: finish mesh and send mesh enable msg to the slave device
Param_in            : uint8_t num
Param_out           : 
Return Type         : uint8_t flag
Note                : before used, run AT_Get_State() first
Author              : Yan
Time                : 2021-01-19
*************************************************************/
uint8_t BLE_FINISH_MESH(uint8_t num)
{
    uint8_t flag = 1;
    AT_Get_Cnt_List();
    uint8_t *sta_ptr = USART1_STA_buf;
    uint8_t cur_cnt = 0;
    uint8_t handle_list[2];
    uint8_t i = 0;
    while (*sta_ptr)
    {
        if (' ' == USART1_STA_buf[cur_cnt])
        {
            handle_list[i] = USART1_STA_buf[cur_cnt-1];
            i++;
        }
        cur_cnt++;
        sta_ptr++;
    }
    i = 0;
    uint8_t a[] = "1\r\n";
    memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
    AT_Send("AT+TTM_ROLE=1\r\n");
    while (num--)
    {
        a[0] = handle_list[i];
        AT_Send((uint8_t *)connect2("AT+TTM_HANDLE=", a));
        AT_Send("AT+EXIT\r\n");
        BLE_Send("en");
        AT_Send("+++");
        i++;
    }
    return flag;
}

/*************************************************************
Function Name       : ble_lock
Function Description: used to send lock or unlock cmd
Param_in            : ENABLE or DISABLE
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2020-12-18
*************************************************************/
void ble_lock(FunctionalState Newstate)
{
    assert_param(IS_FUNCTIONAL_STATE(Newstate));
    if((ENABLE == Newstate) && (1 == myflag.LOCK_STA_flag))//lock the door
    {
        MOTO_FW();
        delay_10ms_rtc(25);
        MOTO_WT();
        MOTO_BR();
        myflag.LOCK_STA_flag = 0;//change the flag
    }
    else if ((DISABLE == Newstate) && (0 == myflag.LOCK_STA_flag))//open the door
    {
        MOTO_BW();
        delay_10ms_rtc(25);
        MOTO_WT();
        MOTO_BR();
        myflag.LOCK_STA_flag = 1;//change the flag
    }
    
}
/*************************************************************
Function Name       : heart_beat_run
Function Description: used to send the heart beat packet
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2020-12-29
*************************************************************/
void heart_beat_run(void)
{
    
}
/*--------------------------- END --------------------------*/