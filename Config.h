#include <30f5015.h>

#device ADC=10
#fuses FRC_PLL4, NOWRT, NOPROTECT, NOWDT, NOBROWNOUT, NOMCLR
#use delay(clock=29480000) 
#use rs232(baud=115200,bits=8, xmit=PIN_F3,rcv=PIN_F2, parity=N)//, timeout=2000)
#byte OERR=0x020E
#USE FIXED_IO(B_OUTPUTS=PIN_B7,PIN_B8)

#define Timmer_Mode     1	//new mode  operation
#define Automated_Mode  3
#define Manual_Mode     2

#define Lamp_On         1
#define Lamp_Off        0

#define day_             0
#define month_           1
#define year_            2
#define hour_            3
#define min_             4
#define sec_             5

#define RoboCable			1
#define RoboEnergia			2
#define LamparaFundida		3

#define LCD_Delay			1000

#define TEMP_ch				0
#define CURRENT_ch			1
#define AN2_ch				2
#define SENSOR1_ch			3
#define SENSOR2_ch			4
#define POWER_ch			5
#define REFERENCE_ch		6

/******************Include files*************************/
#include "DS1302_2.c"		//Driver for Real Time Clock 
//#include "mcp4921_DSPIC.c"
#include "lcd_2.c"			//Driver for common LCD modules   
#include "ADH8066.c"

/******************Global variables***********************/
int operation_mode=0,lamp_status=0,lamp_statusw=0,alert_status=0;//,alert_type;
int port_on=0,port_off=0,port_in=0; //New variables used in new operation mode
int lamp1=0,lamp2=0;
int set_point=0,histeresys=0;
int16 Device_Id=21,loops=0;

float current=0,battery=0,temp=0,s1=0,s2=0,reference=0;

byte DateTime[6]={0,2,13,8,30,0}; //day=1,month=2,year=13,hour=8,minute=30,second=0 (1/feb/2013 08:30:00)
//byte RiseTime[6]={25,2,13,19,27,0}; //day=1,month=2,year=13,hour=8,minute=30,second=0 (1/feb/2013 08:30:00)
//byte SetTime[6]={25,2,13,19,29,0}; //day=1,month=2,year=13,hour=8,minute=30,second=0 (1/feb/2013 08:30:00)
byte dow1=0;

/*struct ADC_Channels
{
	float value;
	char  name[10];
	char  unit[4];	

}ADC_Channels[7];*/

//new datetime variables for new operation mode
byte OnTime[6]={25,2,13,19,27,0}; //day=1,month=2,year=13,hour=8,minute=30,second=0 (1/feb/2013 08:30:00)
byte OffTime[6]={25,2,13,19,29,0}; //day=1,month=2,year=13,hour=8,minute=30,second=0 (1/feb/2013 08:30:00)