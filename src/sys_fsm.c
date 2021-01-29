/*************************************************************
Copyright (c) 2021 Shawn Yan, All rights reserved
-------------------------------------------------------------
File Name: sys_fsm.c

Desc     : system finite state machine 

Author   : Shawn Yan

Date     : 2021-01-28

*************************************************************/
/*------------------------- Includes -----------------------*/
#include "sys_fsm.h"

/*----------- Global Definitions and Declarations ----------*/
uint8_t addr_string_t[] = "00000000000000000000";//used to store the addr of target, format: xx:xx:xx:xx:xx:xx\r\n(1/0)
uint8_t addr_string_m[] = "00000000000000000";//used to store the master addr, format: xx:xx:xx:xx:xx:xx
FSM_table_t sys_table[]={
/* 0*/    /*cur_state     event           eventfunction   next_state*/
/* 1*/    {S_STA_INIT,    S_EVE_NOMESH,   Wait_for_mesh,  S_STA_MESH},
/* 2*/    {S_STA_HALT,    S_EVE_ITWU,     Halt_to_wait,   S_STA_WFM},
/* 3*/    {S_STA_WFM,     S_EVE_RS1,      Motor_Run,      S_STA_MOV},
/* 4*/    {S_STA_WFM,     S_EVE_RS2,      Search_List,    S_STA_SEARCH},
/* 5*/    {S_STA_SEARCH,  S_EVE_TS1,      Link_One,       S_STA_SPEC},
/* 6*/    {S_STA_SEARCH,  S_EVE_TS2,      Link_All,       S_STA_HOP},
/* 7*/    {S_STA_MOV,     S_EVE_SLEEP,    Sleep_Handler,  S_STA_HALT},
/* 8*/    {S_STA_SPEC,    S_EVE_SLEEP,    Sleep_Handler,  S_STA_HALT},
/* 9*/    {S_STA_HOP,     S_EVE_SLEEP,    Sleep_Handler,  S_STA_HALT},
/*10*/    {S_STA_INIT,    S_EVE_SLEEP,    Sleep_Handler,  S_STA_HALT},
/*11*/    {S_STA_MESH,    S_EVE_SLEEP,    Sleep_Handler,  S_STA_HALT},
};
FSM_t system_FSM;//init system FSM
/*-------------------- Type Declarations -------------------*/


/*------------------ Variable Declarations -----------------*/


/*------------------- Function Prototype -------------------*/
/*************************************************************
Function Name       : FSM_Register
Function Description: regist FSM
Param_in            : FSM_t *pFSM, FSM_table_t *pSYS_table
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-28
*************************************************************/
void FSM_Register(FSM_t *pFSM, FSM_table_t *pSYS_table)
{
    pFSM->pFSM_table_t = pSYS_table;
}
/*************************************************************
Function Name       : FSM_Transfer
Function Description: FSM state transfer
Param_in            : FSM_t *pFSM, State state
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-28
*************************************************************/
void FSM_Transfer(FSM_t *pFSM, State state)
{
    pFSM->cur_state = state;
}
/*************************************************************
Function Name       : FSM_EventHandler
Function Description: deal with different events
Param_in            : FSM_t *pFSM, Event event
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-28
*************************************************************/
void FSM_EventHandler(FSM_t *pFSM, Event event)
{
    FSM_table_t *psystem_table = pFSM->pFSM_table_t;
    void (*eventFuction)() = NULL;
    State CurState = pFSM->cur_state;
    State NextState;
    uint8_t flag = 0;
    uint8_t loop_cnt = pFSM->size;
    for (uint8_t i = 0; i < loop_cnt; i++)
    {
        if ((event == psystem_table[i].event) && (CurState == psystem_table[i].cur_state))
        {
            flag = 1;
            eventFuction = psystem_table[i].eventFunction;
            NextState = psystem_table[i].next_state;
            break;
        }
    }
    if (flag)
    {
        FSM_Transfer(pFSM, NextState);
        (*eventFuction)();
    }
    
}
/*************************************************************
Function Name       : FSM_Init
Function Description: initialization of system FSM
Param_in            : FSM_t *pFSM, FSM_table_t *pSYS_table, State initState
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-28
*************************************************************/
void FSM_Init(FSM_t *pFSM, FSM_table_t *pSYS_table, State initState)
{
    FSM_Register(pFSM, pSYS_table);
    FSM_Transfer(pFSM, initState);
    pFSM->size = 11;
}
/*************************************************************
Function Name       : FSM_Run
Function Description: call on main loop
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-28
*************************************************************/
void FSM_Run(void)
{
    switch (system_FSM.cur_state)
    {
    case S_STA_INIT:
    case S_STA_MESH:
        Init_sta_detec();//initial state detect
        break;
    case S_STA_HALT://wake up
        FSM_EventHandler(&system_FSM, S_EVE_ITWU);
        break;
    case S_STA_WFM://ready to receive message
        Received_msg_process();
        break;
    case S_STA_MOV://control the motor
        FSM_EventHandler(&system_FSM, S_EVE_SLEEP);
        break;
    case S_STA_SEARCH://check the cnt_list
        Transmitts_msg_process();
        break;
    case S_STA_SPEC://jump msg to the specified device
        FSM_EventHandler(&system_FSM, S_EVE_SLEEP);
        break;
    case S_STA_HOP://jump msg to other devices
        FSM_EventHandler(&system_FSM, S_EVE_SLEEP);
        break;
    default:
        break;
    }
}
/*************************************************************
Function Name       : Halt_to_wait
Function Description: wake up and ready to receive relevant message
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-28
*************************************************************/
void Halt_to_wait(void)
{
    USART1_RX_STA = 0;
    memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
    AT_Send("+++");
    AT_Send("AT+TTM_ROLE=0\r\n");
    AT_Send("AT+EXIT\r\n");
    BLE_Send("ready for msg");
}
/*************************************************************
Function Name       : Init_sta_detec
Function Description: Check whether Bluetooth has been meshed
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-28
*************************************************************/
void Init_sta_detec(void)
{
    AT_Send("+++");
    AT_Get_State("NAME");
    if ('r' == USART1_STA_buf[0])//meshed
    {
        myflag.BLE_STA_flag = 0;
        memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
        AT_Get_State("DEV_DEL=");
        uint8_t *buf_ptr = USART1_STA_buf;
        while (*buf_ptr)
        {
            
            if ('\n' == *buf_ptr)
            {
                myflag.MAC_NUM_flag++;
            }
            buf_ptr++;
        }
        AT_Send("AT+EXIT\r\n");
        FSM_EventHandler(&system_FSM, S_EVE_SLEEP);//halt
    }
    if ('R' == USART1_STA_buf[0])//non-meshed
    {
        memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
        AT_Send("AT+EXIT\r\n");
        FSM_EventHandler(&system_FSM, S_EVE_NOMESH);//start mesh
        beep_play(E_BEEP_MODE_SUCCESS);
        AT_Send("+++");
        AT_Get_State("NAME");
        if ('r' == USART1_STA_buf[0])//meshed
        {
            myflag.BLE_STA_flag = 0;
            memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
            AT_Send("AT+EXIT\r\n");
            FSM_EventHandler(&system_FSM, S_EVE_SLEEP);//halt
        }
    }
}
/*************************************************************
Function Name       : Wait_for_mesh
Function Description: wait for mesh
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-28
*************************************************************/
void Wait_for_mesh(void)
{
    while (myflag.BLE_STA_flag)
    {
        BLE_status_run();
        request_msg_process();    
    }
}
/*************************************************************
Function Name       : Sleep_Handler 
Function Description: 
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-28
*************************************************************/
void Sleep_Handler(void)
{
    USART_Cmd(USART1, DISABLE);
    GPIO_Init(UART_RX_PORT, UART_RX_PIN, GPIO_Mode_In_PU_IT);
    CLK_PeripheralClockConfig(CLK_Peripheral_USART1, DISABLE);
    // CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM3, DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM4,DISABLE);
    halt();
}
/*************************************************************
Function Name       : Received_msg_process
Function Description: process the received message
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-28
*************************************************************/
void Received_msg_process(void)
{
    wfi();
    for (uint8_t i = 0; i < 200; i++)
    {
        if (USART1_RX_STA & 0x8000)
            break;
        delay_ms_1(20);
    }
    if (USART1_RX_STA & 0x8000)
    {
        if (('u' == USART1_RX_buf[0]) && (' ' == USART1_RX_buf[1]))
        {
            FSM_EventHandler(&system_FSM, S_EVE_RS1);
        }
        if (('m' == USART1_RX_buf[0]) && (' ' == USART1_RX_buf[1]))
        {
            FSM_EventHandler(&system_FSM, S_EVE_RS2);
        }
    }
    
    
    USART1_RX_STA = 0;
    memset(USART1_RX_buf,0,sizeof(USART1_RX_buf));
    
}
/*************************************************************
Function Name       : Motor_Run
Function Description: control the motor of the device
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-28
*************************************************************/
void Motor_Run(void)
{
    USART1_SendWord("y");
    delay_ms_1(10);
    if ('1' == USART1_RX_buf[2])//open door
    {
        ble_lock(DISABLE);
    }
    if ('0' == USART1_RX_buf[2])//close door
    {
        ble_lock(ENABLE);
    }
    AT_Send("+++");
    AT_Send("AT+DISCONNECT\r\n");
    AT_Send("AT+EXIT\r\n");
    USART1_RX_STA = 0;
    memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
}
/*************************************************************
Function Name       : Search_List
Function Description: search the specified device
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-28
*************************************************************/
void Search_List(void)
{
    for (uint8_t i = 0; i < 17; i++)//storage the master addr
    {
        addr_string_m[i] = USART1_RX_buf[i+2];
    }
    for (uint8_t i = 0; i < 20; i++)//storage the addr of target
    {
        addr_string_t[i] = USART1_RX_buf[i+22];
    }
    USART1_SendWord("y");
    delay_ms_1(20);
    AT_Send("+++");
    AT_Send("AT+DISCONNECT\r\n");
    delay_ms_1(100);
    USART1_RX_STA = 0;
    memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
    AT_Get_State("DEV_DEL=");
    AT_Send("AT+EXIT\r\n");
    uint8_t pos;
    //detect flag
    if (0 == (pos = strStr_2(USART1_STA_buf, addr_string_t)))//not bound
    {
        myflag.SPEC_flag = 0;//n
        if (0 == (pos = strStr_2(USART1_STA_buf, addr_string_m)))//not repeat
        {
            myflag.REPEAT_flag = 1;//n
        }
        else
        {
            myflag.REPEAT_flag = 0;//y
        }
    }
    else
    {
        myflag.SPEC_flag = 1;//y
    }
    memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
}
/*************************************************************
Function Name       : Transmitts_msg_process
Function Description: determine to send which kind of msg
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-29
*************************************************************/
void Transmitts_msg_process(void)
{
    if (myflag.SPEC_flag)
        FSM_EventHandler(&system_FSM, S_EVE_TS1);
    else
        FSM_EventHandler(&system_FSM, S_EVE_TS2);
}
/*************************************************************
Function Name       : Link_One
Function Description: link to the specified one
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-28
*************************************************************/
void Link_One(void)
{
    uint8_t addr_string[] = "00000000000000000\r\n";
    for (uint8_t i = 0; i < 17; i++)
    {
        addr_string[i] = addr_string_t[i];
    }
    AT_Send("+++");
    AT_Send((uint8_t *)connect2("AT+CONNECT=,", addr_string));
    for (uint8_t i = 0; i < 100; i++)//delay 1s
    {
        delay_ms_1(10);
        if(USART1_RX_STA & 0x8000)
            break;
    }
    if ((USART1_RX_STA & 0x8000))
    {
        uint8_t t = USART1_RX_STA & 0x7FFF;//get the length of data
        USART1_RX_STA = 0;
        if ('C' == USART1_RX_buf[t-6])//successfully connected
        {
            AT_Send("AT+TTM_ROLE=1\r\n");
            AT_Send("AT+EXIT\r\n");
            if ('1' == addr_string_t[19])//open door
                BLE_Send("u 1");
            else
                BLE_Send("u 0");
            for (uint8_t i = 0; i < 100; i++)//delay 1s
            {
                delay_ms_1(10);
                if(USART1_RX_STA & 0x8000)
                    break;
            }
            USART1_RX_STA = 0;
            memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
        }
    }
}
/*************************************************************
Function Name       : Link_All
Function Description: link to all bound devices, , except for the repeated one
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-28
*************************************************************/
void Link_All(void)
{
    if (myflag.REPEAT_flag)//no repeat
    {
        uint8_t i = myflag.MAC_NUM_flag;
        AT_Send("+++");
        AT_Send("AT+AUTO_CNT=1\r\n");
        AT_Send("AT+SLEEP=,0\r\n");
        AT_Send("AT+SLEEP=,1\r\n");
        while (i--)
        {
            for (uint8_t i = 0; i < 100; i++)//delay 1s
            {
                if(USART1_RX_STA & 0x8000)
                {
                    USART1_RX_STA = 0;
                    memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
                    break;
                }                    
                delay_ms_1(10);                
            }
        }
        AT_Send("AT+TTM_ROLE=1\r\n");
        i = myflag.MAC_NUM_flag;
        AT_Get_State("MAC");
        for (uint8_t k = 0; k < 17; k++)//refresh master addr
        {
            addr_string_m[k] = USART1_STA_buf[k];
        }
        memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
        uint8_t trans_msg[] = "m 00000000000000000 t 00000000000000000\r\n0";
        for (uint8_t k = 0; k < 17; k++)
        {
            trans_msg[k+2] = addr_string_m[k];
            trans_msg[k+22] = addr_string_t[k];
        }
        trans_msg[41] = addr_string_t[19];
        uint8_t handle[] = "0\r\n";
        while (i--)
        {
            handle[0] += 1;
            AT_Send((uint8_t *)connect2("AT+TTM_HANDLE=",handle));
            AT_Send("AT+EXIT\r\n");
            BLE_Send(trans_msg);
            AT_Send("+++");
        }
        AT_Send("AT+DISCONNECT\r\n");
        AT_Send("AT+EXIT\r\n");
    }
    else//repeat
    {
        uint8_t i = myflag.MAC_NUM_flag - 1;
        AT_Send("+++");       
        uint8_t addr_string[] = "00000000000000000\r\n";
        for (uint8_t i = 0; i < 17; i++)
        {   
            addr_string[i] = addr_string_m[i];
        }
        AT_Send("AT+AUTO_CNT=1\r\n");
        AT_Send((uint8_t *)connect2("AT+AUTO_CNT=0,", addr_string));
        AT_Send("AT+SLEEP=,0\r\n");
        AT_Send("AT+SLEEP=,1\r\n");
        while (i--)
        {
            for (uint8_t i = 0; i < 100; i++)//delay 1s
            {
                if(USART1_RX_STA & 0x8000)
                {
                    USART1_RX_STA = 0;
                    memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
                    break;
                }                    
                delay_ms_1(10);                
            }
        }
        AT_Send("AT+TTM_ROLE=1\r\n");
        i = myflag.MAC_NUM_flag - 1;
        AT_Get_State("MAC");
        for (uint8_t k = 0; k < 17; k++)//refresh master addr
        {
            addr_string_m[k] = USART1_STA_buf[k];
        }
        memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
        uint8_t trans_msg[] = "m 00000000000000000 t 00000000000000000\r\n0";
        for (uint8_t k = 0; k < 17; k++)
        {
            trans_msg[k+2] = addr_string_m[k];
            trans_msg[k+22] = addr_string_t[k];
        }
        trans_msg[41] = addr_string_t[19];
        uint8_t handle[] = "0\r\n";
        while (i--)
        {
            handle[0] += 1;
            AT_Send((uint8_t *)connect2("AT+TTM_HANDLE=",handle));
            AT_Send("AT+EXIT\r\n");
            BLE_Send(trans_msg);
            AT_Send("+++");
        }
        AT_Send("AT+DISCONNECT\r\n");
        AT_Send("AT+EXIT\r\n");
    }
}
/*------------------- Function Implement -------------------*/


/*--------------------------- END --------------------------*/