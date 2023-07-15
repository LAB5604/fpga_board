#include ".\STC32G.h"

#include "stdio.h"
#include "intrins.h"
/**********************************************************************************************
   Copyright (c) [2023] [Lab5604 studio]
   [E4M7K325 EC program] is licensed under Mulan PSL v2.
   You can use this software according to the terms and conditions of the Mulan PSL v2. 
   You may obtain a copy of Mulan PSL v2 at:
            http://license.coscl.org.cn/MulanPSL2 
   THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
   See the Mulan PSL v2 for more details. 
 __         ______      ____       ______      ____       __      __ __      
/\ \       /\  _  \    /\  _`\    /\  ___\    /'___\    /'__`\   /\ \\ \     
\ \ \      \ \ \L\ \   \ \ \L\ \  \ \ \__/   /\ \__/   /\ \/\ \  \ \ \\ \    
 \ \ \  __  \ \  __ \   \ \  _ <'  \ \___``\ \ \  _``\ \ \ \ \ \  \ \ \\ \_  
  \ \ \L\ \  \ \ \/\ \   \ \ \L\ \  \/\ \L\ \ \ \ \L\ \ \ \ \_\ \  \ \__ ,__\
   \ \____/   \ \_\ \_\   \ \____/   \ \____/  \ \____/  \ \____/   \/_/\_\_/
    \/___/     \/_/\/_/    \/___/     \/___/    \/___/    \/___/       \/_/  

   Name: simple ec program for e4m7k325 board
   Date: 2023
   Desc: 只有PSON在这个程序中起作用，其他ec按键均不起作用
***********************************************************************************************/

typedef 	unsigned char	u8;
typedef 	unsigned int	u16;
typedef 	unsigned long	u32;

#define MAIN_Fosc        24000000UL

#define Timer0_Reload   (MAIN_Fosc / 100)      //Timer 0 中断频率, 100次/秒 每10ms读取一次按键状态

#define STATUS_STD       0x00
#define STATUS_12VON     0x01
#define STATUS_COREON    0x02
#define STATUS_IOON      0x03
#define STATUS_RUN       0x04

u8 sys_status;              //系统状态
bit key_pson_edge_flag;       //按键边沿标志位
bit key_pson_sample;         //读取按键的采样值（仅在中断函数中更新）
bit key_sysrst_edge_flag;    //按键边沿标志位
bit key_sysrst_sample;       //读取按键的采样值（仅在中断函数中更新）

//-------------power PG&EN pair--------------------
#define EN_12V           P60
#define PG_12V           P61
#define EN_5V            P62
#define PG_5V            P63
#define EN_3V3           P70
#define PG_3V3           P71
#define EN_CORE          P02
#define PG_CORE          P03
//-----------user key and led-----------------------
#define M0_KEY           P64		//pull low when push
#define M1_KEY           P65
#define M2_KEY           P66
#define M3_KEY           P67
#define POWER_LED        P00
#define SYSFAULT_LED     P01
#define EC_LED0          P45

void    Timer0_init(void);
void    delay_ms(u8 ms);

void main(void){
    WTST = 0;  //设置程序指令延时参数，赋值为0可将CPU执行指令的速度设置为最快
    EAXFR = 1; //扩展寄存器(XFR)访问使能
    CKCON = 0; //提高访问XRAM速度
    //    p07   p06   p05  p04  p03  p02  p01  p00 
    //   输入   输入   输入 输入 开漏  输出 输出 输出
    //M0  0      0     0    0    1    1    1    1
    //M1  1      1     1    1    1    0    0    0
    //PU         0     0    0    0    0    0    0
    P0M0=0x0f; P0M1=0xf8; P0PU=0x00;
    //   p17   p16   p15  p14   p13  p12  p11  p10
    //   输入  输入  开漏  开漏  输入  NA   NA   NA
    //M0  0    0     1     1     0    0    0    0
    //M1  1    1     1     1     1    1    1    1
    //PU             1     1
    P1M0=0x30; P1M1=0xff; P1PU=0x30;
    //   p27   p26   p25   p24   p23   p22   p21   p20
    //   输入  输入   输入  输入  输入   输入  输入  输入
    //M0  0     0     0     0     0     0     0     0
    //M1  1     1     1     1     1     1     1     1
    P2M0=0x00; P2M1=0xff;
    //   p37   p36   p35  p34   p33  p32  p31  p30
    //   输入   NA    NA  输出  输出  输入  NA   NA
    //M0  0     0     0    1     1    0    0    0
    //M1  1     1     1    0     0    1    1    1
    P3M0=0x18; P3M1=0xe7;
    //               p45
    //               输出
    //M0   0    0     1     0
    //M1   0    0     0     0
    P4M0 |= 0x20; P4M1 &= ~0x20; 
    //                    P5无配置
    //   p67  p66  p65  p64  p63  p62  p61  p60
    //   输入 输入  输入 输入 输入 开漏  输入 开漏
    //M0  0    0    0    0    0    1   0    1
    //M1  1    1    1    1    1    1   1    1
    //PU                      1    1   1    1
    P6M0=0x05; P6M1=0xff; P6PU=0x0F;
    //                                 P71  P70
    //                                 输入 开漏
    //M0                                0    1
    //M1                                1    1
    //PU                                     1
    P7M0=0x01; P7M1=0xff; P7PU=0x01;
    Timer0_init();      //初始化定时器Timer0，用于读取按键
		sys_status=STATUS_STD;
		SYSFAULT_LED=0;
    EA = 1;             //总中断开
    while(1){
        if(sys_status==STATUS_STD){
            EN_CORE=0;      //核心电源关闭
            EN_3V3=0;      //底板3.3电源关闭
            EN_5V=0;      //底板5V电源关闭
            EN_12V=0;      //底板12V关闭，全板下电
            POWER_LED=0;      //syspower灯熄
            if(key_pson_edge_flag){    //pson按下，进入上电流程
                sys_status=STATUS_12VON;
                key_pson_edge_flag = 0; //清除pson flag
                SYSFAULT_LED=0;              //清除sys fault指示灯
            }
        }else if(sys_status==STATUS_12VON){
            EN_12V=1;      //整版12V电源开
            if(PG_12V){
                sys_status=STATUS_COREON;   //12V电源正常后开核心电源
            }
        }else if(sys_status==STATUS_COREON){
            EN_CORE=1;      //核心1V电源开
            if(PG_CORE){
                sys_status=STATUS_IOON; //核心电源PG，进入IO上电阶段
            }
        }else if(sys_status==STATUS_IOON){
            EN_5V=1;      //底板5V电源开
            EN_3V3=1;      //底板3.3V电源开
            delay_ms(200);    //延迟200ms等待电源启动
            sys_status=STATUS_RUN;
        }else if(sys_status==STATUS_RUN){
            if(!(PG_12V & PG_5V & PG_3V3 & PG_CORE)){
                sys_status=STATUS_STD;      //任意PG为0表示电源有问题，进入待机模式，同时sys fault指示灯拉高
                POWER_LED=0;
                SYSFAULT_LED=1;
            }else{
                POWER_LED=1;
                SYSFAULT_LED=0;
            }
            if(key_pson_edge_flag){         //再次按下pson，整板下电
                sys_status=STATUS_STD;
                key_pson_edge_flag=0;
            }
        }
    }
}
//========================================================================
// 函数: void  delay_ms(unsigned char ms)
// 描述: 延时函数。
// 参数: ms,要延时的ms数, 这里只支持1~255ms. 自动适应主时钟.
// 返回: none.
// 版本: VER1.0
// 日期: 2013-4-1
// 备注: 
//========================================================================
void  delay_ms(u8 ms)
{
     u16 i;
     do{
          i = MAIN_Fosc / 6000;
          while(--i);
     }while(--ms);
}
//========================================================================
// 函数: void Timer0_init(void)
// 描述: timer0初始化函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2015-1-12
//========================================================================
void Timer0_init(void)
{
        TR0 = 0;    //停止计数

    #if (Timer0_Reload < 64)    // 如果用户设置值不合适， 则不启动定时器
        #error "Timer0设置的中断过快!"

    #elif ((Timer0_Reload/12) < 65536UL)    // 如果用户设置值不合适， 则不启动定时器
        ET0 = 1;    //允许中断
    //  PT0 = 1;    //高优先级中断
        TMOD &= ~0x03;
        TMOD |= 0;  //工作模式, 0: 16位自动重装, 1: 16位定时/计数, 2: 8位自动重装, 3: 16位自动重装, 不可屏蔽中断
    //  T0_CT = 1;  //计数
        T0_CT = 0;  //定时
    //  T0CLKO = 1; //输出时钟
        T0CLKO = 0; //不输出时钟

        #if (Timer0_Reload < 65536UL)
            T0x12 = 1;  //1T mode
            TH0 = (u8)((65536UL - Timer0_Reload) / 256);
            TL0 = (u8)((65536UL - Timer0_Reload) % 256);
        #else
            T0x12 = 0;  //12T mode
            TH0 = (u8)((65536UL - Timer0_Reload/12) / 256);
            TL0 = (u8)((65536UL - Timer0_Reload/12) % 256);
        #endif

        TR0 = 1;    //开始运行

    #else
        #error "Timer0设置的中断过慢!"
    #endif
}
//========================================================================
// 函数: void timer0_int (void) interrupt TIMER0_VECTOR
// 描述:  timer0中断函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2015-1-12
//========================================================================
void timer0_int (void) interrupt 1
{
   if((P27==1) & (key_pson_sample==0)){
        key_pson_edge_flag = 1;         //P27按下，检测到上升沿
   }
   key_pson_sample = P27;               //更新采样值
   if((P37==1)&(key_sysrst_sample==0)){
        key_sysrst_edge_flag = 1;
   }
   key_sysrst_sample = P37;
}