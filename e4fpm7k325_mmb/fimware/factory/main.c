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

   Name: simple ec program for e4m7k325 board(in factory use)
   Date: 2023
   Desc: EC M0~M3�����ֶ������ϵ���ĸ�״̬���ϵ����STDģʽ
***********************************************************************************************/
#include ".\STC32G.h"

#include "stdio.h"
#include "intrins.h"

typedef 	unsigned char	u8;
typedef 	unsigned int	u16;
typedef 	unsigned long	u32;

#define MAIN_Fosc        24000000UL

#define STATUS_STD       0x00   //board state standby: only 3.3VSTD active 
#define STATUS_12VON     0x01   //12v on state: 12V active, 
#define STATUS_COREON    0x02   //core power on
#define STATUS_IOON      0x03   //IO power on(3.3V 5V)

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

u8 sys_status;      //board state

void delay_ms(u8 ms);

void main(void){
    WTST = 0;  
    EAXFR = 1;
    CKCON = 0;
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
    sys_status = STATUS_STD;
    POWER_LED=0;
    SYSFAULT_LED=0;
    while(1){
        if(sys_status==STATUS_12VON){
            EC_LED0=1;  //�������Ͻ���12V����ʱ��EC LED����ʾ������¼�̼�
        }
        else {
            EC_LED0=0;
        }
        if(!M0_KEY){
            sys_status=STATUS_STD;  //��EC M0���º󣬽���STD״̬
        }
        if(sys_status==STATUS_STD){
            EN_CORE=0;          //���ĵ�Դ��
            PG_CORE=0;          //����IO��Դ��      ���İ�IO��ԴEN�ɺ��ĵ�ԴPG��������PG�����Թر�IO��Դ
            EN_3V3=0;           //3.3V OFF
            EN_5V=0;            //5V OFF
            EN_12V=0;           //12V OFF
            POWER_LED=0;        //syspower LED OFF
            if(!M1_KEY){        //���EC M1���£�����12V ON״̬
                sys_status=STATUS_12VON;
                SYSFAULT_LED=0;      //sysfault LED reset
            }
        }else if(sys_status==STATUS_12VON){
            EN_12V=1;      //12V ON
            if(!M2_KEY){
                sys_status=STATUS_COREON;   //���EC M2���£���������ϵ�׶�
            }
        }else if(sys_status==STATUS_COREON){
            EN_CORE=1;      //Vcore ON
            PG_CORE=1;      //SoM IO power ON
            if(!M3_KEY){
                sys_status=STATUS_IOON;
            }
        }else if(sys_status==STATUS_IOON){
            EN_5V=1;      //5V ON
            EN_3V3=1;      //3.3V ON
            POWER_LED=1;
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