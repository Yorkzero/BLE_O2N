/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :delay.c 
* Desc     :基于IAR开发环境，基于STM8L051XX/STM8L151XX系列单片机平台实现的延时功能模块
* 
* 
* Author   :CenLinbo
* Date     :2020/06/11
* Notes    :
* 
* -----------------------------------------------------------------
* Histroy:v1.0   2020/06/11, CenLinbo create this file
*         
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include "delay.h"


/*------------------- Global Definitions and Declarations -------------------*/


/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/
volatile char rtc_delay_flag = 0;  //用于使用RTC延时唤醒时的标志位，需要在RTC唤醒中断中清0

/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/
//系统时钟1M时，指令周期为1us
#if (SYS_CLK_FREQ == SYS_CLK_FREQ_1M)
void delay_10us(void)
{
asm
(
    "TNZW X"//2T，加上函数调用返回占用8T，正好10T，10us
);
}


//系统时钟2M时，指令周期为0.5us
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_2M)
//delay_5us使用函数实现
void delay_5us(void)
{
asm
(
    "TNZW X"//2T，加上函数调用返回占用8T，正好10T，5us
);
}

//系统时钟4M时，指令周期为0.25us
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_4M)
//delay_2us使用函数实现
void delay_2us(void)
{
    //void(0);//函数调用和返回已经占用8T，正好2us
}

//系统时钟8M时，指令周期为0.125us
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_8M)
void delay_1us(void)
{
    void(0);//函数调用和返回已经占用8T，正好1us
}

//系统时钟16M时，指令周期为0.0625us
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_16M)
void delay_1us(void)
{
    //函数调用和返回已经占用8T,再执行8T，正好1us
    asm("TNZW X");
    asm("TNZW X");
    asm("TNZW X");
    asm("TNZW X");
}

#else
#error "Invalid system clock value...."
#endif





/******************************************************************************
* Name     :delay_us_1 
*
* Desc     :
* Param_in :n：延时的us数，最大只能输入16383，ms级延时建议使用ms级延时函数
* Param_out:
* Return   :
* Global   :
* Note     :1、调用函数前，LD装载参数到X寄存器要2T
            2、调用函数CALL要4T，RET返回要4T
            3、主频16M：最小延时2us，大于1us为精确延时
            4、主频8M：最小延时3us，大于2us为精确延时
            5、主频4M：最小延时4us，大于3us为精确延时
            6、主频2M：最小延时11us，大于10us且为偶数时精确延时，为奇数时比理想值少1us
            7、主频1M、500K、250K时此函数的最低延时太大，不能准确实现100us内的延时，故不实现
            
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/06/11, Create this function by CenLinbo

*           2020/08/21 10:22, add this by CenLinbo
*           1、重新对16M/8M/4M/2M主频下的延时作计算并测试，目前可用
 ******************************************************************************/
#if ((SYS_CLK_FREQ == SYS_CLK_FREQ_2M) || (SYS_CLK_FREQ == SYS_CLK_FREQ_4M) \
      || (SYS_CLK_FREQ == SYS_CLK_FREQ_8M) || (SYS_CLK_FREQ == SYS_CLK_FREQ_16M))
void delay_us_1(uint16_t n)
{
//最小延时11us,22T
#if (SYS_CLK_FREQ == SYS_CLK_FREQ_2M)
        //除去函数调用返回占用10T
asm
(
    "CPW X,#12\n"  //2T,
    "JRC LABEL1\r\n" //跳转则2T，不跳则1T
    "SUBW X,#11\n" //2T,减去11，执行到此，结果一定是大于0的

    "INCW X\n"  //1T，加1，保证以下的循环能正常运行
    "SRLW X\n"  //2T,除以2，以保证之后的一个循环能延时1us
    "NOP\n"     //补偿2T，保证除去循环固定为20T，即10us
    "NOP\n"

    //一个循环占4T
"LOOP:DECW X\n"   //1T
    "NOP\n"
    "JRNE LOOP\n"//跳转则2T，不跳转则1T
    "JP RETURN\n" //1T，同时作为补偿

    //延时8T
"LABEL1:TNZW X\n"//2T  该命令只占1字节空间，比使用循环更省空间
    "TNZW X\n"//2T
    "TNZW X\n"//2T
    "TNZW X\n"//2T

"RETURN:"
);


//最小延时4us,总20T
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_4M)
        //除去函数调用返回占用10T
asm
(
    "CPW X,#5\n"  //2T,X<5,
    "JRC LABEL1\r\n" //跳转则2T，不跳则1T
    "SUBW X,#4\n" //2T,减去4，执行到此，结果一定是大于0的

    //一个循环占4T
"LOOP:DECW X\n"   //1T
    "NOP\n"
    "JRNE LOOP\n"//跳转则2T，不跳转则1T
    
    //执行到此时，已花销的时长为5T + (X-4)*4T - 1T，还需要再延时2T
    "NOP\n" //1T
    "JP RETURN\n" //1T

    //延时6T
"LABEL1:TNZW X\n"//2T  该命令只占1字节空间，比使用循环更省空间
    "TNZW X\n"//2T
    "TNZW X\n"//2T
    
"RETURN:"

);


//最小延时3us
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_8M)
        //除去函数调用返回占用10T
asm
(
    "CPW X,#4\n"  //2T,X<4
    "JRC LABEL1\r\n" //跳转则2T，不跳则1T 
    "SUBW X,#3\n" //2T,减去3，执行到此，结果一定是大于0的

    //需要再延时4T
    "SLLW X\n"  //2T乘以2，使之后的循环达到8T

    //一个循环占4T，前面乘以2就可以达到8T
"LOOP:DECW X\n"   //1T
    "NOP\n"
    "JRNE LOOP\n"//跳转则1T，不跳转则2T
    "NOP\n"
    
    //执行到此时，还需要再延时9T
    "TNZW X\n"//2T  该命令只占1字节空间，比使用循环更省空间
    "TNZW X\n"//2T
    "TNZW X\n"//2T
    "TNZW X\n"//2T
    "JP RETURN\n" //1T

    //延时10T
"LABEL1:LD A,#3\n"//1T
"LOOP2:DEC A\n"//1T，3*3-1=8
    "JRNE LOOP2\n"  //跳转2T，不跳转1T
    "NOP\n"   //凑够10T
    
"RETURN:"

);

//最小延时2us
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_16M)
    //除去函数调用返回占用10T
asm
(
    "CPW X,#3\n"  //2T,X<3,则凑足16T后返回
    "JRC LABEL1\r\n" //跳转则2T，不跳则1T 
    "DECW X\n" //1T
    "DECW X\n"  //1T，减去2，执行到此，结果一定是大于0的,相比于使用SUBW指令能节省1个字节空间

    //需要再延时4T，使固定占用为16T
    "SLLW X\n"  //2T乘以4，使之后的循环达到16T
    "SLLW X\n"  //2T

    //循环占4*X - 1T，前面乘以4就可以达到16T
"LOOP:DECW X\n"  //1T
    "NOP\n"  //补偿1T
    "JRNE LOOP\n"//跳转则2T，不跳转则1T
    "NOP\n"  //补偿1T

    //执行到此时，已花销的时长为(X-2)*16T + 9T + 10T，还需要再延时13T
    "LABEL2:LD A,#3\n"//1T，
"LOOP3:DEC A\n"//1T，3次循环总的为3*3-1T=8T，实测此循环确实是占用3T
    "JRNE LOOP3\n"  //跳转2T，不跳转1T
    
    "NOP\n" //实测只需要补偿1T
    
    
    "JP RETURN\n" //无条件跳转，不清流水线，只用1T，直接跳转到返回

    //延时18T
"LABEL1:LD A,#6\n"//1T
"LOOP2:DEC A\n"//1T，循环总的为3*6-1T=17T
    "JRNE LOOP2\n"  //跳转2T，不跳转1T
    
"RETURN:"
);

#else
#error "No define system clock value...."
#endif
}
#endif

/******************************************************************************
* Name      :delay_ms_1 
*
* Desc     :基于us延时实现的ms级延时，由于1000us的延时是相当准确的，所以ms级延时也很准确
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :bobde163
* -------------------------------------
* Log     :2020/06/17 21:16:22, Create this function by bobde163
 ******************************************************************************/
void delay_ms_1(uint16_t n)
{
    while(n--)
    {
        delay_us_1(1000);
    }
}
/*************************************************************
Function Name       : delay_10ms_rtc
Function Description: use rtc AWU to delay 
Param_in            : u16 n
Param_out           : 
Return Type         : 
Note                : delay time = n*10 ms
Author              : Yan
Time                : 2021-01-28
*************************************************************/
void delay_10ms_rtc(uint16_t n)
{
    uint32_t cnt = n;
    CLK_RTCClockConfig(CLK_RTCCLKSource_LSI, CLK_RTCCLKDiv_1);
    while (RESET == CLK_GetFlagStatus(CLK_FLAG_LSIRDY));
    CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);
    RTC_WakeUpCmd(DISABLE);
    RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);
    cnt = (cnt * 95) >> 2;
    if (65535 <= cnt)
    {
        cnt = 65535;
    }
    RTC_SetWakeUpCounter(cnt);
    RTC_ClearITPendingBit(RTC_IT_WUT);
    RTC_ITConfig(RTC_IT_WUT, ENABLE);
    rtc_delay_flag = 1;
    RTC_WakeUpCmd(ENABLE);
    while (rtc_delay_flag)
    {
        halt();
    }
    RTC_WakeUpCmd(DISABLE);
    RTC_ITConfig(RTC_IT_WUT, DISABLE);
    RTC_ClearITPendingBit(RTC_IT_WUT);
    CLK_PeripheralClockConfig(CLK_Peripheral_RTC, DISABLE);
}
char *connect2(char *a, char *b) {
    char *c = (char *) malloc(strlen(a) + strlen(b) + 1); //'\0'结尾添加1
    if (c == NULL) exit (1);
    char *tempc;
    char value[50] = "0";
    char *tempv;
    uint8_t t = 0;
    memset(c, 0, sizeof(c));
    tempc = c; 
    free(c);
    
    while (*a != '\0') 
    {
        *tempc++ = *a++;
    }
    while ((*tempc++ = *b++) != '\0') 
    {
        ;
    }
    tempc = c;
    
    while (*c)
    {
        value[t++] = *c++;
    }
    tempv = value;
    memset(c, 0, sizeof(c));
    c = NULL;
    memset(tempc, 0, sizeof(tempc));
    free(tempc);
    tempc = NULL;
    return tempv;
}
char *connect3(char *a, char *b, char *d) {
    char *c = (char *) malloc(strlen(a) + strlen(b) +strlen(d) + 1); //'\0'结尾添加1
    if (c == NULL) exit (1);
    char *tempc;
    char value[30] = "0";
    char *tempv;
    uint8_t t = 0;
    memset(c, 0, sizeof(c));
    tempc = c; 
    free(c);
    
    while (*a != '\0') 
    {
        *tempc++ = *a++;
    }
    while ((*tempc++ = *b++) != '\0') 
    {
        ;
    }
    while ((*tempc++ = *d++) != '\0') 
    {
        ;
    }
    tempc = c;
    
    while (*c)
    {
        value[t++] = *c++;
    }
    tempv = value;
    memset(c, 0, sizeof(c));
    c = NULL;
    memset(tempc, 0, sizeof(tempc));
    free(tempc);
    tempc = NULL;
    return tempv;
}
uint8_t strStr(uint8_t* haystack, uint8_t* needle) {
    uint8_t m=strlen(haystack);//计算两字符串长度
    uint8_t n=strlen(needle) - 2;//专用于地址寻找，修正2
    if(n==0) return 0;
    
    for(uint8_t i=0;i<m;i++){
        uint8_t j=0;
        uint8_t k=i;
        while(haystack[k]==needle[j] && j<n){//比较各字符是否相等来确定haystack中是否存在needle
            k++;
            j++;
            if(haystack[k]!=needle[j] && j<n){
                break;
            }
            
        }
        if(j==n){  //比较到第n个（needle[n-1]），如果都相等，则说明存在，返回haystack中与needle第一个相同字符的编号
                return i;
            }
    }
    return 0;
}
uint8_t strStr_2(uint8_t* haystack, uint8_t* needle) {
    uint8_t m=strlen(haystack);//计算两字符串长度
    uint8_t n=strlen(needle);
    if(n==0) return 0;
    
    for(uint8_t i=0;i<m;i++){
        uint8_t j=0;
        uint8_t k=i;
        while(haystack[k]==needle[j] && j<n){//比较各字符是否相等来确定haystack中是否存在needle
            k++;
            j++;
            if(haystack[k]!=needle[j] && j<n){
                break;
            }
            
        }
        if(j==n){  //比较到第n个（needle[n-1]），如果都相等，则说明存在，返回haystack中与needle第一个相同字符的编号
                return (i+1);
            }
    }
    return 0;
}
void Reverse(uint8_t *p , uint8_t size)
{
    uint8_t i  , tmp;
    for(i = 0 ; i < size/2 ; i++)
    {
         tmp = p[i] ; 
         p[i] = p[size-1-i] ; 
         p[size-1-i] = tmp ;    
    }    
}
/*---------------------------------------------------------------------------*/

