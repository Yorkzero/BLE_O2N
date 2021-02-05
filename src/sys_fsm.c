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
//When adding or deleting this table, the size of the state machine must be modified synchronously
FSM_table_t sys_table[]={
/**/    /*cur_state      event           event_function      next_state*/
/**/    {S_STA_INIT,     S_EVE_NOMESH,   Wait_for_mesh,      S_STA_MESH},
/**/    {S_STA_HALT,     S_EVE_ITWU,     Halt_to_wait,       S_STA_WFM},
/**/    {S_STA_WFM,      S_EVE_RS1,      Motor_Run,          S_STA_MOV},
/**/    {S_STA_WFM,      S_EVE_RS2,      Link_msg_process,   S_STA_ONOFFLINE},
/**/    {S_STA_DWM,      S_EVE_TS1,      Link_End,           S_STA_SPEC},
/**/    {S_STA_DWM,      S_EVE_TS2,      Link_Hop,           S_STA_HOP},
/**/    {S_STA_MOV,      S_EVE_SLEEP,    Sleep_Handler,      S_STA_HALT},
/**/    {S_STA_WFM,      S_EVE_SLEEP,    Sleep_Handler,      S_STA_HALT},
/**/    {S_STA_SPEC,     S_EVE_SLEEP,    Sleep_Handler,      S_STA_HALT},
/**/    {S_STA_HOP,      S_EVE_SLEEP,    Sleep_Handler,      S_STA_HALT},
/**/    {S_STA_ONOFFLINE,S_EVE_SLEEP,    Sleep_Handler,      S_STA_HALT},
/**/    {S_STA_INIT,     S_EVE_SLEEP,    Sleep_Handler,      S_STA_HALT},
/**/    {S_STA_MESH,     S_EVE_WFI,      Mesh_wfm,           S_STA_MESH_OK},
/**/    {S_STA_MESH_OK,  S_EVE_SLEEP,    Sleep_Handler,      S_STA_HALT},
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
    pFSM->size = 14;
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
        Init_sta_detec();//initial state detect
        break;
    case S_STA_MESH:
        FSM_EventHandler(&system_FSM, S_EVE_WFI);
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
    case S_STA_DWM://deal with the master msg
        Transmitts_msg_process();
        break;
    case S_STA_SPEC://jump msg to the specified device
        FSM_EventHandler(&system_FSM, S_EVE_SLEEP);
        break;
    case S_STA_HOP://jump msg to other devices
        FSM_EventHandler(&system_FSM, S_EVE_SLEEP);
        break;
    case S_STA_MESH_OK://mesh OK
        FSM_EventHandler(&system_FSM, S_EVE_SLEEP);
        break;
    case S_STA_ONOFFLINE://link msg
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
    //do some adc sampling
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
        memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
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
            
            // FSM_Transfer(&system_FSM, S_STA_WFM);
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
    AT_Send("+++");
    AT_Send("AT+AUTO_CNT=1\r\n");
    AT_Send("AT+TTM_ROLE=0\r\n");
    AT_Send("AT+CNT_INTERVAL=480\r\n");//set the connect interval as (480 * 1.25 = 600)ms
    AT_Send("AT+STATUS=1\r\n");
    // AT_Send("AT+ADS=1,1,500\r\n");//set the advertise interval as 1000ms
    // AT_Send("AT+POWER=-28\r\n");//set the TX power as -3db
    AT_Send("AT+SLEEP=0,1\r\n");
    AT_Send("AT+EXIT\r\n");
    // USART_Cmd(USART1, DISABLE);
    GPIO_Init(BLE_CTS_PORT, BLE_CTS_PIN, GPIO_Mode_In_PU_IT);
    CLK_PeripheralClockConfig(CLK_Peripheral_USART1, DISABLE);
    // CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM3, DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM4,DISABLE);
    BLE_SEND_DISABLE();
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
    for (uint8_t i = 0; i < 200; i++)
    {
        if (USART1_RX_STA & 0x8000)
            break;
        delay_ms_1(5);
    }    
    if (USART1_RX_STA & 0x8000)
    {
        uint8_t correct_temp = 0;
        uint8_t t = USART1_RX_STA & 0x7fff;
        if (('u' == USART1_RX_buf[0]) && (' ' == USART1_RX_buf[1]))
        {
            key_flag = 0;
            while (t--)
            {
                USART1_STA_buf[key_flag] = USART1_RX_buf[key_flag];
                key_flag++;
            }
            key_flag = 0;
            AT_Send("+++");
            AT_Send("AT+TTM_ROLE=0\r\n");
            AT_Send("AT+EXIT\r\n");
            USART1_SendWord("y");
            delay_ms_1(20);
            correct_temp = 1;
            FSM_EventHandler(&system_FSM, S_EVE_RS1);
        }
        if ((('1' == USART1_RX_buf[0]) || ('0' == USART1_RX_buf[0]))
             && (' ' == USART1_RX_buf[1]))
        {
            key_flag = 0;
            while (t--)
            {
                USART1_STA_buf[key_flag] = USART1_RX_buf[key_flag];
                key_flag++;
            }
            AT_Send("+++");
            AT_Send("AT+TTM_ROLE=0\r\n");
            AT_Send("AT+EXIT\r\n");
            USART1_SendWord("y");
            delay_ms_1(20);
            correct_temp = 1;
            FSM_Transfer(&system_FSM, S_STA_DWM);
        }
        //offline situation
        if ('S' == USART1_RX_buf[0])//master offline or online
        {
            correct_temp = 1;
            USART1_RX_STA = 0;
            memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
            key_flag = 60;
        }
        if (('T' == USART1_RX_buf[t-5]) && (13 < t))//slave offline or online
        {
            correct_temp = 1;
            FSM_EventHandler(&system_FSM, S_EVE_RS2);
        }
        if ('D' == USART1_RX_buf[t-1])
        {
            correct_temp = 1;
            uint8_t *l_string = (uint8_t *)malloc(t);
            uint8_t i = 0;
            while (t--)//format xx:xx:xx:xx:xx:xx (DIS)CONNECTED
            {
                l_string[i] = USART1_RX_buf[i];
                i++;
            }
            AT_Send("+++");
            AT_Send("AT+TTM_ROLE=0\r\n");
            AT_Send("AT+EXIT\r\n");
            USART1_SendWord(l_string);
            delay_ms_1(20);
            memset(l_string, 0, sizeof(l_string));
            free(l_string);
            key_flag = 60;//sleep
        }
        if (!correct_temp)//incorrect msg
        {
            USART1_SendWord("n");
            delay_ms_1(20);
            USART1_RX_STA = 0;
            memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
            key_flag = 60;//sleep
        }
    }
    if (50 < key_flag)//outtime
    {
        key_flag = 0;
        FSM_EventHandler(&system_FSM, S_EVE_SLEEP);
    }
}
/*************************************************************
Function Name       : Mesh_wfm
Function Description: deal with mesh msg
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-31
*************************************************************/
void Mesh_wfm(void)
{
    if (!myflag.MAC_NUM_flag)//no slave
        return;
    AT_Send("+++");
    AT_Get_State("MAC");
    AT_Send("AT+TTM_ROLE=0\r\n");
    AT_Send("AT+EXIT\r\n");
    uint8_t string_m[] = "00:00 ";
    for (uint8_t i = 0; i < 5; i++)
    {
        string_m[i] = USART1_STA_buf[i+12];
    }
    memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
    uint8_t device_cnt = myflag.MAC_NUM_flag;//number of slaves
    while (device_cnt)
    {
        wfi();
        for (uint8_t i = 0; i < 200; i++)
        {
            if (USART1_RX_STA & 0x8000)
                break;
            delay_ms_1(1);
        }
        if (USART1_RX_STA & 0x8000)
        {
            uint8_t t = USART1_RX_STA & 0x7fff;
            uint8_t cur_cnt = 0;
            uint8_t last_cnt = 0;
            while (t > cur_cnt)
            {
                if ('m' == USART1_RX_buf[cur_cnt])//slave msg
                {
                    if ('d' == USART1_RX_buf[cur_cnt + 1])//slave msg done
                    {
                        device_cnt--;
                        if (device_cnt)
                            USART1_RX_buf[cur_cnt + 1] = ' ';
                    }
                    if (cur_cnt != last_cnt)
                    {
                        Mesh_success(string_m, (cur_cnt - last_cnt), last_cnt);
                        last_cnt = cur_cnt;
                    }
                }
                cur_cnt++;
            }
            Mesh_success(string_m, (t - last_cnt), last_cnt);//send the last msg
            memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
            USART1_RX_STA = 0;
        }
    } 
    delay_ms_1(20);
}
/*************************************************************
Function Name       : Mesh_success
Function Description: Send networking success command to the host
Param_in            : uint8_t *string_m  uint8_t length  uint8_t pos
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-30
*************************************************************/
void Mesh_success(uint8_t *string_m, uint8_t length, uint8_t pos)
{
    uint8_t *m_string = (uint8_t *)malloc(length);
    uint8_t i = 0;
    while (length--)
    {
        m_string[i] = USART1_RX_buf[i + pos];
        i++;
    }
    USART1_SendWord((uint8_t*)connect2(m_string, string_m));
    memset(m_string, 0, sizeof(m_string));
    free(m_string);
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
    if ('1' == USART1_STA_buf[2])//open door
    {
        ble_lock(DISABLE);
    }
    if ('0' == USART1_STA_buf[2])//close door
    {
        ble_lock(ENABLE);
    }
    USART1_RX_STA = 0;
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
    if (14 == key_flag)
        myflag.HOP_STA_flag = 1;//end
    else
        myflag.HOP_STA_flag = 0;//hop

    if (myflag.HOP_STA_flag)
        FSM_EventHandler(&system_FSM, S_EVE_TS1);
    else
        FSM_EventHandler(&system_FSM, S_EVE_TS2);
}
/*************************************************************
Function Name       : Link_msg_process
Function Description: deal with abnormal link situation
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-02-05
*************************************************************/
void Link_msg_process(void)
{
    uint8_t t = USART1_RX_STA & 0x7fff;
    uint8_t *l_string = (uint8_t *)malloc(t-2);
    uint8_t i = 0;
    t -= 2;
    while (t--)//format xx:xx:xx:xx:xx:xx (DIS)CONNECTED
    {
        l_string[i] = USART1_RX_buf[i];
        i++;
    }
    AT_Send("+++");
    AT_Send("AT+TTM_ROLE=0\r\n");
    AT_Send("AT+EXIT\r\n");
    USART1_SendWord(l_string);
    delay_ms_1(20);
    memset(l_string, 0, sizeof(l_string));
    free(l_string);
}
/*************************************************************
Function Name       : Link_End
Function Description: link to the ended one
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-28
*************************************************************/
void Link_End(void)
{
    uint8_t t = key_flag;
    key_flag = 0;
    uint8_t *m_string = (uint8_t *)malloc(t-6);
    uint8_t i = 0;
    t -= 6;
    while (t--)
    {
        m_string[i] = USART1_STA_buf[i];
        i++;
    }
    memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
    AT_Send("+++");
    AT_Get_Cnt_List();
    AT_Send("AT+TTM_ROLE=1\r\n");
    AT_Send("AT+EXIT\r\n");
    uint8_t t_string[5];
    for (uint8_t i = 0; i < 5; i++)
    {
        t_string[i] = m_string[i+2];
    }
    if (1 == myflag.MAC_NUM_flag)
        t = 22;
    if (2 == myflag.MAC_NUM_flag)
        t = 44;
    uint8_t handler;
    for(uint8_t n=0;n<t;n++){
        uint8_t j=0;
        uint8_t k=n;
        while(USART1_STA_buf[k]==t_string[j] && j<5){
            k++;
            j++;
            if(USART1_STA_buf[k]!=t_string[j] && j<5){
                break;
            }    
        }
        if(j==5){
                handler = n - 15;
            }
    }
    uint8_t a[] = "1\r\n";
    a[0] = USART1_STA_buf[handler];
    memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
    AT_Send("+++");
    AT_Send((uint8_t *)connect2("AT+TTM_HANDLE=", a));
    AT_Send("AT+EXIT\r\n");//exit AT mode
    if ('1' == m_string[0])//open door
    {
        if (0 == BLE_Send("u 1"))
        {
            AT_Send("+++");
            AT_Send("AT+TTM_ROLE=0\r\n");
            AT_Send("AT+EXIT\r\n");
            BLE_Send("T");
        }
        else
        {
            AT_Send("+++");
            AT_Send("AT+TTM_ROLE=0\r\n");
            AT_Send("AT+EXIT\r\n");
            BLE_Send("F");
        }
    }    
    else
    {
        if (0 == BLE_Send("u 0"))
        {
            AT_Send("+++");
            AT_Send("AT+TTM_ROLE=0\r\n");
            AT_Send("AT+EXIT\r\n");
            BLE_Send("T");
        }
        else
        {
            AT_Send("+++");
            AT_Send("AT+TTM_ROLE=0\r\n");
            AT_Send("AT+EXIT\r\n");
            BLE_Send("F");
        }
    }
    USART1_RX_STA = 0;
    memset(USART1_RX_buf, 0, sizeof(USART1_RX_buf));
    memset(m_string, 0, sizeof(m_string));
    free(m_string);
}
/*************************************************************
Function Name       : Link_Hop
Function Description: hop msg to next device
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2021-01-28
*************************************************************/
void Link_Hop(void)
{
    uint8_t t = key_flag;
    key_flag = 0;
    uint8_t *m_string = (uint8_t *)malloc(t-6);
    uint8_t i = 0;
    t -= 6;
    while (t--)
    {
        m_string[i] = USART1_STA_buf[i];
        i++;
    }
    memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
    AT_Send("+++");
    AT_Get_Cnt_List();
    AT_Send("AT+TTM_ROLE=1\r\n");
    AT_Send("AT+EXIT\r\n");
    if (1 == myflag.MAC_NUM_flag)
        t = 22;
    if (2 == myflag.MAC_NUM_flag)
        t = 44;
    uint8_t handler;
    for(uint8_t n=0;n<t;n++){
        uint8_t j=0;
        uint8_t k=n;
        while(USART1_STA_buf[k]==m_string[i - 6 + j] && j<5){
            k++;
            j++;
            if(USART1_STA_buf[k]!=m_string[i - 6 + j] && j<5){
                break;
            }    
        }
        if(j==5){
                handler = n - 15;
            }
    }
    uint8_t a[] = "1\r\n";
    a[0] = USART1_STA_buf[handler];
    memset(USART1_STA_buf, 0, sizeof(USART1_STA_buf));
    AT_Send("+++");
    AT_Send((uint8_t *)connect2("AT+TTM_HANDLE=", a));
    AT_Send("AT+EXIT\r\n");//exit AT mode
    if (0 == BLE_Send(m_string))//hop msg
    {
        bsp_tim2_init(12500);//set 100ms outtime
        TIM2_Cmd(ENABLE);
        while (70 >= key_flag)//set 7s outtime
        {
            wfi();
            if (USART1_RX_STA)
            {
                key_flag = 0;
                TIM2_Cmd(DISABLE);
                CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, DISABLE); //disable the clk
                break;
            }
        }
        if (70 < key_flag)//outtime
        {
            TIM2_Cmd(DISABLE);
            CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, DISABLE); //disable the clk
            key_flag = 0;
        }
        for (t = 0; t < 200; t++)//delay 200ms
        {
            if(USART1_RX_STA & 0x8000)
                break;
            delay_ms_1(1);
        }
        if ((USART1_RX_STA & 0x8000))//receive the data
        {
            USART1_SendWord("y");
            delay_ms_1(20);
            if ('T' == USART1_RX_buf[0])
            {
                AT_Send("+++");
                AT_Send("AT+TTM_ROLE=0\r\n");
                AT_Send("AT+EXIT\r\n");
                BLE_Send("T");
            }
            else
            {
                AT_Send("+++");
                AT_Send("AT+TTM_ROLE=0\r\n");
                AT_Send("AT+EXIT\r\n");
                BLE_Send("F");
            }
        }
    }
    else
    {
        AT_Send("+++");
        AT_Send("AT+TTM_ROLE=0\r\n");
        AT_Send("AT+EXIT\r\n");
        BLE_Send("F");
    }
    memset(m_string, 0, sizeof(m_string));
    free(m_string);
}
/*------------------- Function Implement -------------------*/


/*--------------------------- END --------------------------*/