/******************Configuration file********************/
#include "Config.h"
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
	setup_adc_ports(sAN0|sAN1|sAN2|sAN3|sAN4|sAN5|sAN6);//*******Setup ch0, ch1 and ch2 as ADC inputs in Px, Py and Pz******
	setup_adc(ADC_CLOCK_INTERNAL);
	//******Configure PORTB***********
	SET_TRIS_B(0x007F);   //PB0, PB1, PB2 as input, PB3-PB15 as output (0b0000000000000111)
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
		for(loops=0;loops<4;loops++)
			{//*****Start of for loop**************
				 //**********BUZZER********************
					output_bit(PIN_D9,alert_status);  //BUZZER depend of alert state

				    //**********GPO***********************

					//******Get status/mode***********************
					lcd_putc("\fGetting data\nfrom server.+.");
					GetMode(Device_Id,&operation_mode,&lamp_statusw);	
					delay_ms(1500);		

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
							default:
							break;
						}						
					output_bit(PIN_B7,lamp1);
					output_bit(PIN_B8,lamp2);					
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
					temp=read_ADC();
					delay_us(100);
					set_adc_channel(1);
					current=read_ADC();
					delay_us(100);
					set_adc_channel(5);
					battery=read_ADC();
					delay_us(100);
					set_adc_channel(3);
					s1=read_ADC();
					delay_us(100);
					set_adc_channel(4);
					s2=read_ADC();
					delay_us(100);
					
					for(i_=0;i_<10;i_++)
					{
						set_adc_channel(6);
						reference=reference+read_ADC();
						delay_us(50);
					}
					reference=4.12/reference;
					temp=(temp*reference);
					battery=(battery*reference)*3.11;
					s1=s1*reference/.165;
					s2=s2*reference/.165;

					lcd_putc("\f");
					printf(lcd_putc,"Battery= %6.3fv",battery);
					delay_ms(2000);
					lcd_putc("\f");
					printf(lcd_putc,"Current: %6.3fA\nTemperature: %6.3f",current,temp);//1.1,1.1);//current,temp);
					//ReportData(Device_Id,lamp_status,current,voltage);
					delay_ms(2000);
					lcd_putc("\f");
					printf(lcd_putc,"Reference= %6.3f",reference);
					delay_ms(2000);
					lcd_putc("\f");
					printf(lcd_putc,"S1: %6.3fmA\nS2: %6.3fmA",s1,s2);
					delay_ms(10000);
			}//*****End of for loop**************
			ReportData(Device_Id,lamp_status,(current+100),(temp+100));//1,1.1,1.1);
	}//*****End of while loop**************
}
