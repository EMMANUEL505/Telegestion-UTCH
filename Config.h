#include <30f5015.h>

#device ADC=10
#fuses FRC_PLL4, NOWRT, NOPROTECT, NOWDT, NOBROWNOUT, NOMCLR
#use delay(clock=29480000) 
#use rs232(baud=115200,bits=8, xmit=PIN_F3,rcv=PIN_F2, parity=N, timeout=2000)
#byte OERR=0x020E
#USE FIXED_IO(B_OUTPUTS=PIN_B2,PIN_B3,PIN_B4)


#define Zebra_Mode      0
#define Automated_Mode  1
#define Manual_Mode     2

#define Lamp_On         1
#define Lamp_Off        0

#define day_             0
#define month_           1
#define year_            2
#define hour_            3
#define min_             4
#define sec_             5

#define RoboCable        0
#define RoboEnergia      1
#define LamparaFundida   2
