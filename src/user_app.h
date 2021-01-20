/*************************************************************
Copyright (c) 2020 Shawn Yan, All rights reserved
-------------------------------------------------------------
File Name: user_app.h

Desc     : 应用层函数头文件

Author   : Shawn Yan

Date     : 2020-11-23

*************************************************************/
#ifndef _USER_APP_H_
#define _USER_APP_H_
/*------------------------- Includes -----------------------*/
#include "main.h"

/*----------- Global Definitions and Declarations ----------*/
//Use bit fields to define flag
typedef struct sysFlag
{
  uint8_t BLE_STA_flag:1;//BLE state flag 0:MESH, 1:NON-MESH
  uint8_t LOCK_STA_flag:1;//LOCK state flag 0:LOCKED, 1:UNLOCKED
  uint8_t SYS_STA_flag:1;//system state flag 0:halt, 1:run
  uint8_t INIT_STA_flag:1;//initialization state flag 0:not initialized yet, 1:initialized ready
  uint8_t e:1;//unused
  uint8_t f:1;//unused
  uint8_t g:1;//unused
  uint8_t h:1;//unused
}myFlag;
extern volatile myFlag myflag;
#if (RELAY_DEV == DEVICE_ID)
extern volatile uint8_t ctrl_string[];//used to control LED group
extern volatile uint8_t sta_string[];//used to record node status
#endif
/*-------------------- Type Declarations -------------------*/
/*------------------ Variable Declarations -----------------*/


/*------------------- Function Prototype -------------------*/
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
--------------------------------------------------------------
log:
data:  2021-1-19
note:  use this to display the mesh state
*************************************************************/
void BLE_status_run(void);
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
void BLE_Init(void);
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
uint8_t BLE_MESH(void);
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
uint8_t BLE_FINISH_MESH(uint8_t num);
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