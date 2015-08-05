#include <30f5015.h>

#device ADC=10
#fuses FRC_PLL4, NOWRT, NOPROTECT, NOWDT, NOBROWNOUT, NOMCLR
#use delay(clock=29480000) 
#use rs232(baud=115200,bits=8, xmit=PIN_F3,rcv=PIN_F2, parity=N)//, timeout=2000)
#byte OERR=0x020E
#USE FIXED_IO(B_OUTPUTS=PIN_B7,PIN_B8)

#define Timmer_Mode     0	//new mode  operation
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

/******************Include files*************************/
#include "DS1302_2.c"
#include "mcp4921_DSPIC.c"
#include "lcd_2.c"
#include "ADH8066.c"

/******************Global variables***********************/
int operation_mode=0,lamp_status=1,lamp_statusw=1,alert_status=0;//,alert_type;
//int port_status=0,port_statusw=0; //New variables used in new operation mode
int lamp1=0,lamp2=0;
int16 Device_Id=9,loops=0,i_=0;

float current=0,battery=0,temp=0,s1=0,s2=0,reference=0;

byte DateTime[6]={0,2,13,8,30,0}; //day=1,month=2,year=13,hour=8,minute=30,second=0 (1/feb/2013 08:30:00)
byte RiseTime[6]={25,2,13,19,27,0}; //day=1,month=2,year=13,hour=8,minute=30,second=0 (1/feb/2013 08:30:00)
byte SetTime[6]={25,2,13,19,29,0}; //day=1,month=2,year=13,hour=8,minute=30,second=0 (1/feb/2013 08:30:00)
byte dow1=0;

//new datetime variables for new operation mode
//byte OnTime[6]={25,2,13,19,27,0}; //day=1,month=2,year=13,hour=8,minute=30,second=0 (1/feb/2013 08:30:00)
//byte OffTime[6]={25,2,13,19,29,0}; //day=1,month=2,year=13,hour=8,minute=30,second=0 (1/feb/2013 08:30:00)