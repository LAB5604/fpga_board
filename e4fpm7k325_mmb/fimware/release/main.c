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
   Desc: ֻ��PSON����������������ã�����ec��������������
***********************************************************************************************/

typedef 	unsigned char	u8;
typedef 	unsigned int	u16;
typedef 	unsigned long	u32;

#define MAIN_Fosc        24000000UL

#define Timer0_Reload   (MAIN_Fosc / 100)      //Timer 0 �ж�Ƶ��, 100��/�� ÿ10ms��ȡһ�ΰ���״̬

#define STATUS_STD       0x00
#define STATUS_12VON     0x01
#define STATUS_COREON    0x02
#define STATUS_IOON      0x03
#define STATUS_RUN       0x04

u8 sys_status;              //ϵͳ״̬
bit key_pson_edge_flag;       //�������ر�־λ
bit key_pson_sample;         //��ȡ�����Ĳ���ֵ�������жϺ����и��£�
bit key_sysrst_edge_flag;    //�������ر�־λ
bit key_sysrst_sample;       //��ȡ�����Ĳ���ֵ�������жϺ����и��£�

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
    WTST = 0;  //���ó���ָ����ʱ��������ֵΪ0�ɽ�CPUִ��ָ����ٶ�����Ϊ���
    EAXFR = 1; //��չ�Ĵ���(XFR)����ʹ��
    CKCON = 0; //��߷���XRAM�ٶ�
    //    p07   p06   p05  p04  p03  p02  p01  p00 
    //   ����   ����   ���� ���� ��©  ��� ��� ���
    //M0  0      0     0    0    1    1    1    1
    //M1  1      1     1    1    1    0    0    0
    //PU         0     0    0    0    0    0    0
    P0M0=0x0f; P0M1=0xf8; P0PU=0x00;
    //   p17   p16   p15  p14   p13  p12  p11  p10
    //   ����  ����  ��©  ��©  ����  NA   NA   NA
    //M0  0    0     1     1     0    0    0    0
    //M1  1    1     1     1     1    1    1    1
    //PU             1     1
    P1M0=0x30; P1M1=0xff; P1PU=0x30;
    //   p27   p26   p25   p24   p23   p22   p21   p20
    //   ����  ����   ����  ����  ����   ����  ����  ����
    //M0  0     0     0     0     0     0     0     0
    //M1  1     1     1     1     1     1     1     1
    P2M0=0x00; P2M1=0xff;
    //   p37   p36   p35  p34   p33  p32  p31  p30
    //   ����   NA    NA  ���  ���  ����  NA   NA
    //M0  0     0     0    1     1    0    0    0
    //M1  1     1     1    0     0    1    1    1
    P3M0=0x18; P3M1=0xe7;
    //               p45
    //               ���
    //M0   0    0     1     0
    //M1   0    0     0     0
    P4M0 |= 0x20; P4M1 &= ~0x20; 
    //                    P5������
    //   p67  p66  p65  p64  p63  p62  p61  p60
    //   ���� ����  ���� ���� ���� ��©  ���� ��©
    //M0  0    0    0    0    0    1   0    1
    //M1  1    1    1    1    1    1   1    1
    //PU                      1    1   1    1
    P6M0=0x05; P6M1=0xff; P6PU=0x0F;
    //                                 P71  P70
    //                                 ���� ��©
    //M0                                0    1
    //M1                                1    1
    //PU                                     1
    P7M0=0x01; P7M1=0xff; P7PU=0x01;
    Timer0_init();      //��ʼ����ʱ��Timer0�����ڶ�ȡ����
		sys_status=STATUS_STD;
		SYSFAULT_LED=0;
    EA = 1;             //���жϿ�
    while(1){
        if(sys_status==STATUS_STD){
            EN_CORE=0;      //���ĵ�Դ�ر�
            EN_3V3=0;      //�װ�3.3��Դ�ر�
            EN_5V=0;      //�װ�5V��Դ�ر�
            EN_12V=0;      //�װ�12V�رգ�ȫ���µ�
            POWER_LED=0;      //syspower��Ϩ
            if(key_pson_edge_flag){    //pson���£������ϵ�����
                sys_status=STATUS_12VON;
                key_pson_edge_flag = 0; //���pson flag
                SYSFAULT_LED=0;              //���sys faultָʾ��
            }
        }else if(sys_status==STATUS_12VON){
            EN_12V=1;      //����12V��Դ��
            if(PG_12V){
                sys_status=STATUS_COREON;   //12V��Դ�����󿪺��ĵ�Դ
            }
        }else if(sys_status==STATUS_COREON){
            EN_CORE=1;      //����1V��Դ��
            if(PG_CORE){
                sys_status=STATUS_IOON; //���ĵ�ԴPG������IO�ϵ�׶�
            }
        }else if(sys_status==STATUS_IOON){
            EN_5V=1;      //�װ�5V��Դ��
            EN_3V3=1;      //�װ�3.3V��Դ��
            delay_ms(200);    //�ӳ�200ms�ȴ���Դ����
            sys_status=STATUS_RUN;
        }else if(sys_status==STATUS_RUN){
            if(!(PG_12V & PG_5V & PG_3V3 & PG_CORE)){
                sys_status=STATUS_STD;      //����PGΪ0��ʾ��Դ�����⣬�������ģʽ��ͬʱsys faultָʾ������
                POWER_LED=0;
                SYSFAULT_LED=1;
            }else{
                POWER_LED=1;
                SYSFAULT_LED=0;
            }
            if(key_pson_edge_flag){         //�ٴΰ���pson�������µ�
                sys_status=STATUS_STD;
                key_pson_edge_flag=0;
            }
        }
    }
}
//========================================================================
// ����: void  delay_ms(unsigned char ms)
// ����: ��ʱ������
// ����: ms,Ҫ��ʱ��ms��, ����ֻ֧��1~255ms. �Զ���Ӧ��ʱ��.
// ����: none.
// �汾: VER1.0
// ����: 2013-4-1
// ��ע: 
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
// ����: void Timer0_init(void)
// ����: timer0��ʼ������.
// ����: none.
// ����: none.
// �汾: V1.0, 2015-1-12
//========================================================================
void Timer0_init(void)
{
        TR0 = 0;    //ֹͣ����

    #if (Timer0_Reload < 64)    // ����û�����ֵ�����ʣ� ��������ʱ��
        #error "Timer0���õ��жϹ���!"

    #elif ((Timer0_Reload/12) < 65536UL)    // ����û�����ֵ�����ʣ� ��������ʱ��
        ET0 = 1;    //�����ж�
    //  PT0 = 1;    //�����ȼ��ж�
        TMOD &= ~0x03;
        TMOD |= 0;  //����ģʽ, 0: 16λ�Զ���װ, 1: 16λ��ʱ/����, 2: 8λ�Զ���װ, 3: 16λ�Զ���װ, ���������ж�
    //  T0_CT = 1;  //����
        T0_CT = 0;  //��ʱ
    //  T0CLKO = 1; //���ʱ��
        T0CLKO = 0; //�����ʱ��

        #if (Timer0_Reload < 65536UL)
            T0x12 = 1;  //1T mode
            TH0 = (u8)((65536UL - Timer0_Reload) / 256);
            TL0 = (u8)((65536UL - Timer0_Reload) % 256);
        #else
            T0x12 = 0;  //12T mode
            TH0 = (u8)((65536UL - Timer0_Reload/12) / 256);
            TL0 = (u8)((65536UL - Timer0_Reload/12) % 256);
        #endif

        TR0 = 1;    //��ʼ����

    #else
        #error "Timer0���õ��жϹ���!"
    #endif
}
//========================================================================
// ����: void timer0_int (void) interrupt TIMER0_VECTOR
// ����:  timer0�жϺ���.
// ����: none.
// ����: none.
// �汾: V1.0, 2015-1-12
//========================================================================
void timer0_int (void) interrupt 1
{
   if((P27==1) & (key_pson_sample==0)){
        key_pson_edge_flag = 1;         //P27���£���⵽������
   }
   key_pson_sample = P27;               //���²���ֵ
   if((P37==1)&(key_sysrst_sample==0)){
        key_sysrst_edge_flag = 1;
   }
   key_sysrst_sample = P37;
}