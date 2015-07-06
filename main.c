/******************Configuration file********************/
#include "Config.h"

/******************Global variables***********************/
int operation_mode=0,lamp_status=1,lamp_statusw=1,alert_status=0,alert_type;
int lamp1=0,lamp2=0;
int16 i=0,Device_Id=14,loops=0;

float current=0,voltage=0,battery=0;
//float nominal_c[3][2]={{0,5},{0,5},{0,5}};
//float nominal_v[3][2];{{0,5},{0,5},{0,5}};

byte DateTime[6]={0,2,13,8,30,0}; //day=1,month=2,year=13,hour=8,minute=30,second=0 (1/feb/2013 08:30:00)
byte RiseTime[6]={25,2,13,19,27,0}; //day=1,month=2,year=13,hour=8,minute=30,second=0 (1/feb/2013 08:30:00)
byte SetTime[6]={25,2,13,19,29,0}; //day=1,month=2,year=13,hour=8,minute=30,second=0 (1/feb/2013 08:30:00)
byte dow1=0;

/******************Include files*************************/
#include "DS1302_2.c"
#include "SerialCommWiFly.c"
#include "mcp4921_DSPIC.c"
#include "lcd_2.c"
#include "ADH8066.c"

/******************Main function, program****************/
void main(void)
{	

	//***Component initializers****
	lcd_init();        //LCD INITIALIZE
	//init_dac();      //MPC4921 DAC INITIALIZE
	rtc_init();		   //DS1302 RTC INITIALIZE

	//***Device initialize*********
	lcd_putc("\fTelegestion UTCH");
	delay_ms(100);
	setup_adc_ports(sAN0|sAN1|sAN2);//*******Setup ch0 and ch1 as ADC inputs in Px and Py******
	setup_adc(ADC_CLOCK_INTERNAL);

	//******Configure PORTB***********
	SET_TRIS_B(0x0007);   //PB0, PB1 as input, PB2-PB15 as output (0b0000000000000011)

	//******Clear RS232 error**********
	OERR=16;

	//******Configure GPRS device******
	ConfigureGPRS();

	//******Set datetime*****************
	switch(ConfigureDatetime(Device_Id, DateTime))
	{
		case 0:
			lcd_putc("\fCMD BAD");
		break;
		case 1:
			rtc_set_datetime(DateTime[day_],DateTime[month_],DateTime[year_],dow1,DateTime[hour_],DateTime[min_]);
			lcd_putc("\f");
			lcd_putc("Datetime:\n");
			printf(lcd_putc,"%d/%d/20%d %02d:%02d",DateTime[day_],DateTime[month_],DateTime[year_],DateTime[hour_],DateTime[min_]);
			delay_ms(1000);
		break;
		default:
			lcd_putc("\fCMD BAD");
		break;

	}
	ClosePort(1);

	//***While loop, main program starts here***
	while(TRUE)
	{//*****Start of while loop**************       
		for(loops=0;loops<=25;loops++)
			{//*****Start of for loop**************
				 //**********BUZZER********************
					output_bit(PIN_D9,alert_status);  //BUZZER depend of alert state

				    //**********GPO***********************

					//******Get status/mode***********************
					lcd_putc("\fGetting data\nfrom server.+.");
					GetMode(Device_Id,&operation_mode,&lamp_statusw);

					if(DateTime[day_]!=RiseTime[day_]) 
						{
							GetSunriseTime(Device_Id,RiseTime);	
							RiseTime[day_]=DateTime[day_];
						}
					if(DateTime[day_]!=SetTime[day_]) 
						{
							GetSunsetTime(Device_Id,SetTime);
							SetTime[day_]=DateTime[day_];
						}					

					switch(operation_mode)
						{
							case Manual_Mode:
								lamp_status=lamp_statusw;
								lamp1= lamp_status; lamp2=lamp_status;
							break;

							case Automated_Mode:
								if(DateTime[day_]==RiseTime[day_]&&DateTime[hour_]==RiseTime[hour_]&&DateTime[min_]>=RiseTime[min_]) lamp_status=0;
								if(DateTime[day_]==SetTime[day_]&&DateTime[hour_]==SetTime[hour_]&&DateTime[min_]>=SetTime[min_]) lamp_status=1;
								lamp1= lamp_status; lamp2=lamp_status;
							break;

							case Zebra_Mode:
								if(DateTime[day_]==RiseTime[day_]&&DateTime[hour_]==RiseTime[hour_]&&DateTime[min_]>=RiseTime[min_]) lamp_status=0;
								if(DateTime[day_]==SetTime[day_]&&DateTime[hour_]==SetTime[hour_]&&DateTime[min_]>=SetTime[min_]) lamp_status=1;
								if((DateTime[day_]%2)==0)
									{ 
										lamp1= 1*lamp_status; lamp2=0*lamp_status;
									}
								if((DateTime[day_]%2)==1)
									{ 
										lamp1= 0*lamp_status; lamp2=1*lamp_status;
									}
							break;
			
							default:
							break;
						}
						
					output_bit(PIN_B3,lamp1);
					output_bit(PIN_B4,lamp2);
					
        			//**********DAC***********************
        			
					//write_
					//dac(dac_reg);

					//******RTC Get and show datetime*****
					lcd_putc("\f");
					lcd_putc("Datetime:\n");
					rtc_get_date(DateTime[day_],DateTime[month_],DateTime[year_],dow1);
					rtc_get_time(DateTime[hour_],DateTime[min_],DateTime[sec_]);

					printf(lcd_putc,"%d/%d/20%d %02d:%02d",DateTime[day_],DateTime[month_],DateTime[year_],DateTime[hour_],DateTime[min_]);
					delay_ms(2000);

					//******Get and show status/mode*******
					lcd_putc("\f");
					printf(lcd_putc,"Mode: %d\nStatus: %d",operation_mode,lamp_status);
					delay_ms(2000);

					//******Get and show current/voltage***
					set_adc_channel(0);  //temp sensor
					current=read_ADC();
					set_adc_channel(1);
					voltage=read_ADC();
					set_adc_channel(2);
					battery=read_ADC();

					lcd_putc("\f");
					printf(lcd_putc,"Current: %6.3f\nVoltage: %6.3f",current,voltage);
					//ReportData(Device_Id,lamp_status,current,voltage);
					delay_ms(2000);
					lcd_putc("\f");
					printf(lcd_putc,"Battery= %6.3f",battery);
					delay_ms(2000);

			}//*****End of for loop**************
			ReportData(Device_Id,lamp_status,current,voltage);
	}//*****End of while loop**************
}
