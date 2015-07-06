#include <30f5015.h>
#device ADC=10
#fuses FRC_PLL4, NOWRT, NOPROTECT, NOWDT, NOBROWNOUT, NOMCLR
#use delay(clock=29480000) 
#use rs232(baud=9600,bits=8, xmit=PIN_F3,rcv=PIN_F2, parity=N)
#byte OERR=0x020E
#USE FIXED_IO(B_OUTPUTS=PIN_B2,PIN_B3,PIN_B4)
#include "SerialCommWiFly.c"
#include "mcp4921_DSPIC.c"
#include "lcd_2.c"
#include "DS1302_2.C"

char input_buffer[45];
int i=0,aux=0,cmd,module,N=0;
byte day=0,mth=2,year=13,dow=0,hour=8,min=30,sec=0;
long int adc_reg=0,tem_reg=0;
float dec;
int buzer=0,servo=0,port=0;
long int dac_reg=0;

#int_rda
void input_uart()
{
    disable_interrupts (INT_RDA);
    while(getc()!='.');
	i=0;
    do
    {
       input_buffer[i]=getch();	
	   i++;	
    }while((input_buffer[i-1]!='>') && (i<45));

    cmd=CMD_Type(&input_buffer[0]);
    if(cmd==SET_CMD || cmd==READ_CMD)
      {
		module=Module_Type(&input_buffer[4]);
		switch(module)
		{
			case DAC://OK
		     dac_reg=HEX_StringToNumber(&input_buffer[8],3);	
        	 printf("\n\r%4ld\n\r",dac_reg);
			break;	

			case GPO://OK
        	 port=HEX_StringToNumber(&input_buffer[8],1);
        	 printf("\n\rACK %3s CMD\n\r",GPO_STR);
			break;	

			case TIM:
        	 dec=HEX_StringToNumber(&input_buffer[8],3);
        	 printf("\n\rACK %3s CMD\n\r",TIM_STR);
			break;	

			case LCD://OK
        	 printf("\n\rACK %3s CMD\n\r",LCD_STR);
			 lcd_putc("\f");
			 for(aux=0;aux<(i-9);aux++)
				{
					lcd_putc(input_buffer[8+aux]);
					if(aux==15) lcd_putc("\n");
				}
			break;	

			case BUZ://OK
        	 buzer=HEX_StringToNumber(&input_buffer[8],1);
        	 printf("\n\rACK %3s CMD\n\r",BUZ_STR);
			break;	

			case SER:
        	 servo=HEX_StringToNumber(&input_buffer[8],3);
        	 printf("\n\rACK %3s CMD\n\r",SER_STR);
			break;

			case ADC://OK
			 set_adc_channel(1);    	
        	 //printf("\n\rACK %3s CMD\n\r",ADC_STR);	
			 //delay_ms(100);
			 adc_reg=read_ADC(); 
			 printf("%4ld",adc_reg);
			break;

			case TEM://OK 
			 set_adc_channel(0);    	
        	 //printf("\n\rACK %3s CMD\n\r",TEM_STR);
			 tem_reg=read_ADC(); 
			 printf("%4ld",tem_reg);
			break;		

			case GPI:
        	 dec=HEX_StringToNumber(&input_buffer[8],3);
        	 printf("\n\rACK %3s CMD\n\r",GPI_STR);
			break;

			default:
			 printf("\n\rBAD argument %3s \n\r",&input_buffer[4]);
			break;	
		}
      }
      
    else
      {
         printf("\n\rUnknown command  \n\r");
      }
}

void main(void)
{	
	lcd_init();
	lcd_putc("\f LOADING.");
	for(N=0;N<6;N++)
		{
			delay_ms(1000);
			lcd_putc(".");
		}
	lcd_putc("\fWiFly dsPIC");
	init_dac();
	rtc_init();
	setup_adc_ports(sAN0|sAN1);
	setup_adc(ADC_CLOCK_INTERNAL);
	OERR=16;
    rtc_set_datetime(day,mth,year,dow,hour,min);
	SET_TRIS_B(0x0003);
	while(TRUE)
	{       
////////////BUZZER/////////////////
		output_bit(PIN_D9,buzer);
////////////GPO////////////////////
		//output_bit(PIN_B1,port&0x01);
		output_bit(PIN_B2,(port>>1)&0x01);
		output_bit(PIN_B3,(port>>2)&0x01);
		output_bit(PIN_B4,(port>>3)&0x01);
////////////DAC////////////////////
        write_dac(dac_reg);
////////////////////////////////////
       // rtc_get_date(day,mth,year,dow);                                                                        
       // rtc_get_time(hour,min,sec);
       //	printf(lcd_putc,"\f%d/%d/%d\n%d:%d:%d",day,mth,year,hour,min,sec);
	   //	delay_ms(100);
        enable_interrupts(INT_RDA);
	}	
}
