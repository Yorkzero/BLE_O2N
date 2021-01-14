/*************************************************************
Copyright (c) 2020 Shawn Yan, All rights reserved
-------------------------------------------------------------
File Name: user_app.h

Desc     : Ӧ�ò㺯��ͷ�ļ�

Author   : Shawn Yan

Date     : 2020-11-23

*************************************************************/
#ifndef _USER_APP_H_
#define _USER_APP_H_
/*------------------------- Includes -----------------------*/
#include "main.h"

/*----------- Global Definitions and Declarations ----------*/
extern volatile uint8_t BLE_STA_flag;//BLE state flag 0:MESH, 1:NON-MESH
extern volatile uint8_t LOCK_STA_flag;//LOCK state flag 0:LOCKED, 1:UNLOCKED
extern volatile uint8_t SYS_STA_flag;//system state flag 0:halt, 1:run
#if (RELAY_DEV == DEVICE_ID)
extern volatile uint8_t ctrl_string[];//used to control LED group
extern volatile uint8_t sta_string[];//used to record node status
#endif
/*-------------------- Type Declarations -------------------*/
#define DEBUG_STATUS 0 //record the status of function

/*------------------ Variable Declarations -----------------*/


/*------------------- Function Prototype -------------------*/
/*************************************************************
Function Name       : AT_Test_Demo
Function Description: AT instuction test
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2020-11-23
*************************************************************/
void AT_Test_Demo(void);
/*************************************************************
Function Name       : AT_Send
Function Description: send AT cmd
Param_in            : uint8_t *atcmd
Param_out           : 
Return Type         : u16 tag
Note                : 0: succeed/1: failed
Author              : Yan
Time                : 2020-11-27
*************************************************************/
uint8_t AT_Send(uint8_t *atcmd);
/*************************************************************
Function Name       : AT_Get_State
Function Description: use AT to get BLE state
Param_in            : char *sta
                      allow to query the following status:
                      NAME/MODE/MAC/CIVER/TPL/PASS/PASSEN/
                      UART/UARTIM/AUTOSLEEP/DEEPSLEEP/
                      HIBERNATE/HELLO/LINK/UUID etc.
Param_out           : USART1_STA_buf[]
Return Type         : u16 flag
Note                : 0: succeed/1: failed
Author              : Yan
Time                : 2020-11-27
*************************************************************/
uint8_t AT_Get_State(char *sta);
#if DEBUG_STATUS
/*************************************************************
Function Name       : BLE_AT_Init
Function Description: Bluetooth initialization
Param_in            : char *name, char *mode
                      M: master/S: slave/F: mesh/B: broadcast or iBeacon
Param_out           : 
Return Type         : u16 tag 
Note                : 0: succeed/1: failed
Author              : Yan
Time                : 2020-11-27
------------------------------------------
log                 : 2020-11-28
                      debug ing, can not be used!
*************************************************************/
uint8_t BLE_AT_Init(char *name, char *mode);
#endif
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
void BLE_status_it(void);
/*************************************************************
Function Name       : BLE_status_run
Function Description: show the BLE status in main
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2020-12-04
*************************************************************/
void BLE_status_run(void);
/*************************************************************
Function Name       : user_app_run
Function Description: use key/phone to control led
Param_in            : 
Param_out           : 
Return Type         : 
Note                : 
Author              : Yan
Time                : 2020-11-25
*************************************************************/
void user_app_run(void);
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
void ble_lock(FunctionalState Newstate);
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
void heart_beat_run(void);
/*------------------- Function Implement -------------------*/

#endif
/*--------------------------- END --------------------------*/