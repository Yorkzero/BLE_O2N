/*************************************************************
Copyright (c) 2021 Shawn Yan, All rights reserved
-------------------------------------------------------------
File Name: sys_fsm.h

Desc     : system finite state machine driver

Author   : Shawn Yan

Date     : 2021-01-28

*************************************************************/
#ifndef _SYS_FSM_H_
#define _SYS_FSM_H_
/*------------------------- Includes -----------------------*/
#include "main.h"

/*----------- Global Definitions and Declarations ----------*/

/*-------------------- Type Declarations -------------------*/
typedef uint8_t State;
typedef uint8_t Event;
typedef struct FSM_table
{
    State cur_state;
    Event event;
    void (*eventFunction)();
    State next_state;
}FSM_table_t;
typedef struct FSM
{
    State cur_state;
    FSM_table_t *pFSM_table_t;
    uint8_t size;
}FSM_t;
extern FSM_table_t sys_table[];
extern FSM_t system_FSM;
/*------------------ Variable Declarations -----------------*/
enum sys_state{
    S_STA_INIT = 0, //init state
    S_STA_MESH,     //start to mesh
    S_STA_MESH_OK,  //mesh OK
    S_STA_HALT,     //halt
    S_STA_WFM,      //wait for msg
    S_STA_MOV,      //motor move
    S_STA_DWM,      //deal with the master msg
    S_STA_SPEC,     //link to the specified device
    S_STA_HOP,      //hop message to bound devices  
    S_STA_ONOFFLINE,//BLE notify the status changed of linked device    
};
enum sys_event{
    S_EVE_ITWU = 0, //wake uped by IT
    S_EVE_NOMESH,   //not meshed yet
    S_EVE_WFI,      //wait for mac message
    S_EVE_RS1,      //receive sitiuation 1: itself is a designated device
    S_EVE_RS2,      //receive sitiuation 2: link call
    S_EVE_TS1,      //transmitts sitiuation 1: transmitts msg to the target
    S_EVE_TS2,      //transmitts sitiuation 2: hop msg to next device
    S_EVE_SLEEP,    //end this action and go to sleep
};
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
void FSM_Register(FSM_t *pFSM, FSM_table_t *pSYS_table);
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
void FSM_Transfer(FSM_t *pFSM, State state);
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
void FSM_EventHandler(FSM_t *pFSM, Event event);
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
void FSM_Init(FSM_t *pFSM, FSM_table_t *pSYS_table, State initState);
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
void FSM_Run(void);
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
void Halt_to_wait(void);
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
void Init_sta_detec(void);
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
void Wait_for_mesh(void);
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
void Sleep_Handler(void);
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
void Received_msg_process(void);
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
void Mesh_wfm(void);
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
void Mesh_success(uint8_t *string_m, uint8_t length, uint8_t pos);
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
void Motor_Run(void);
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
void Transmitts_msg_process(void);
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
void Link_msg_process(void);
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
void Link_End(void);
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
void Link_Hop(void);
/*------------------- Function Implement -------------------*/

#endif
/*--------------------------- END --------------------------*/