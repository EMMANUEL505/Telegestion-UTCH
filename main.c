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
	setup_adc_ports(sAN0|sAN1|sAN2|sAN3|sAN4|sAN5|sAN6);//*******Setup CH0-CH6 as ADC
	setup_adc(ADC_CLOCK_INTERNAL);
	//******Configure PORTB***********
	SET_TRIS_B(0x007F);   //PB0-PB6 as input, PB7-PB15 as output
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
			delay_ms(LCD_Delay);
		break;
		default:
			lcd_putc("\fCMD BAD");
		break;

	}
	//ClosePort(1);
	//***While loop, main program starts here***
	while(TRUE)
	{//*****Start of while loop**************       
		for(loops=0;loops<2;loops++)
		{//*****Start of for loop**************
			//**********BUZZER********************
			output_bit(PIN_D9,alert_status);  //BUZZER depend of alert state
			//**********GPO***********************
			//******Get status/mode***********************
			lcd_putc("\fGetting data\nfrom server.+.");
			GetMode(Device_Id,&operation_mode,&lamp_statusw);	
			delay_ms(LCD_Delay);		
			switch(operation_mode)
			{
				case Timmer_Mode:
					GetOnTime(Device_Id,OnTime);
					GetOffTime(Device_Id,OffTime);
					GetPort(Device_Id,&port_on,&port_off,&port_in);
					if(DateTime[hour_]==OnTime[hour_]&&((DateTime[min_]>=(OnTime[min_]-1))&&(DateTime[min_]<=(OnTime[min_]+1)))) lamp_status=port_on;
					else if(DateTime[hour_]==OffTime[hour_]&&((DateTime[min_]>=(OffTime[min_]-1))&&(DateTime[min_]<=(OffTime[min_]+1)))) lamp_status=port_off;
					//else lamp_status=0;
					lamp1= (lamp_status&0x01); lamp2=(lamp_status&0x02)>>1;
				break;	
				case Automated_Mode:
					GetAutomated(Device_Id,&set_point,&histeresys);
					delay_ms(300);
					GetPort(Device_Id,&port_on,&port_off,&port_in);
					if(S1>set_point+histeresys) lamp_status=port_on;
					else if(S1<set_point-histeresys) lamp_status=port_off;
					else if((S1<set_point+0.9)&&(S1>set_point-0.9)) lamp_status=port_off;	
					lamp1= (lamp_status&0x01); lamp2=(lamp_status&0x02)>>1;				
				break;	
				case Manual_Mode:
					lamp_status=lamp_statusw;
					lamp1= (lamp_status&0x01); lamp2=(lamp_status&0x02)>>1;
				break;	
				default:
				break;
			}						
			output_bit(PIN_B7,lamp1);	output_bit(PIN_B8,lamp2);					
			//******RTC Get and show datetime*****
			lcd_putc("\f");
			lcd_putc("Datetime:\n");
			rtc_get_date(DateTime[day_],DateTime[month_],DateTime[year_],dow1);
			rtc_get_time(DateTime[hour_],DateTime[min_],DateTime[sec_]);
			printf(lcd_putc,"%d/%d/20%d %02d:%02d",DateTime[day_],DateTime[month_],DateTime[year_],DateTime[hour_],DateTime[min_]);
			delay_ms(LCD_Delay);

			//******Get and show status/mode*******
			lcd_putc("\f");
			printf(lcd_putc,"Mode: %d\nStatus: %d",operation_mode,lamp_status);
			delay_ms(LCD_Delay);

			//******Get and show ADC values***
			temp=ReadADC(TEMP_ch,100,10);
			current=ReadADC(CURRENT_ch,100,10);
			s1=ReadADC(SENSOR1_ch,100,10);
			s2=ReadADC(SENSOR2_ch,100,10);
			reference=ReadADC(REFERENCE_ch,100,10);
			battery=ReadADC(POWER_ch,100,10);

			reference=0.412/reference;
			temp=(temp*reference)*1000;
			current=(current*reference)*8;
			battery=(battery*reference)*3.11;	
			s1=s1*reference/.165; 	//165 Ohms resitor in serie with sensor, (v/165)*1000 (in mA)
			s2=s2*reference/.165;	//165 Ohms resitor in serie with sensor, (v/165)*1000 (in mA)

			lcd_putc("\f");
			printf(lcd_putc,"Battery= %6.3fV",battery);
			delay_ms(LCD_Delay);
			lcd_putc("\f");
			printf(lcd_putc,"Current: %6.3fA",current);//\nTemperature: %6.3fmV",current,temp);//1.1,1.1);//current,temp);
			delay_ms(LCD_Delay);
			lcd_putc("\f");
			printf(lcd_putc,"Reference= \n%6.3fmV",(reference*1000));
			delay_ms(LCD_Delay);
			lcd_putc("\f");
			printf(lcd_putc,"S1: %6.3fmA\nS2: %6.3fmA",s1,s2);
			delay_ms(LCD_Delay);

			if(battery<6.3) {alert_status=1; CreateAlert(Device_Id,2);}
			else alert_status=0;
		}//*****End of for loop**************
		ReportData(Device_Id,lamp_status);//1,1.1,1.1);
	}//*****End of while loop**************
}
