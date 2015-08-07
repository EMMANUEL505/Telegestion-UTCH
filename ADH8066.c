///////////////////////////////////////////////////////////////////////////////
////                             ADH8066.C                                 ////
////                 Driver for ADH8066 GPRS modules                       ////
////                                                                       ////
////  StringToHex(Str,size,result)                                         ////
////                                                                       ////
////  lcd_putc(c)  Will display c on the next position of the LCD.         ////
////                     The following have special meaning:               ////
////                      \f  Clear display                                ////
////                      \n  Go to start of second line                   ////
////                      \b  Move back one position                       ////
////                                                                       ////
////  lcd_gotoxy(x,y) Set write position on LCD (upper left is 1,1)        ////
////                                                                       ////
////  lcd_getc(x,y)   Returns character at position x,y on LCD             ////
////                                                                       ////
////  CONFIGURATION                                                        ////
////  The LCD can be configured in one of two ways: a.) port access or     ////
////  b.) pin access.  Port access requires the entire 7 bit interface     ////
////  connected to one GPIO port, and the data bits (D4:D7 of the LCD)     ////
////  connected to sequential pins on the GPIO.  Pin access                ////
////  has no requirements, all 7 bits of the control interface can         ////
////  can be connected to any GPIO using several ports.                    ////
////                                                                       ////
////  To use port access, #define LCD_DATA_PORT to the SFR location of     ////
////  of the GPIO port that holds the interface, -AND- edit LCD_PIN_MAP    ////
////  of this file to configure the pin order.  If you are using a         ////
////  baseline PIC (PCB), then LCD_OUTPUT_MAP and LCD_INPUT_MAP also must  ////
////  be defined.                                                          ////
////                                                                       ////
////  Example of port access:                                              ////
////     #define LCD_DATA_PORT getenv("SFR:PORTD")                         ////
////                                                                       ////
////  To use pin access, the following pins must be defined:               ////
////     LCD_ENABLE_PIN                                                    ////
////     LCD_RS_PIN                                                        ////
////     LCD_RW_PIN                                                        ////
////     LCD_DATA4                                                         ////
////     LCD_DATA5                                                         ////
////     LCD_DATA6                                                         ////
////     LCD_DATA7                                                         ////
////                                                                       ////
////  Example of pin access:                                               ////
////     #define LCD_ENABLE_PIN  PIN_E0                                    ////
////     #define LCD_RS_PIN      PIN_E1                                    ////
////     #define LCD_RW_PIN      PIN_E2                                    ////
////     #define LCD_DATA4       PIN_D4                                    ////
////     #define LCD_DATA5       PIN_D5                                    ////
////     #define LCD_DATA6       PIN_D6                                    ////
////     #define LCD_DATA7       PIN_D7                                    ////
////                                                                       ////
///////////////////////////////////////////////////////////////////////////////
////        (C) Copyright 2014  Adolfo Emmanuel Sigala Villa           ////
//// This source code may only be used by licensed users of the CCS C  ////
//// compiler.  This source code may only be distributed to other      ////
//// licensed users of the CCS C compiler.  No other use, reproduction ////
//// or distribution is permitted without written permission.          ////
//// Derivative programs created using this software in object code    ////
//// form are not restricted in any way.                               ////
///////////////////////////////////////////////////////////////////////////

/******************Global definitions********************/
//#define AT "AT+"
//#define CPIN "CPIN?\r\n"
//#define AIPDCONT "AIPDCONT=\"INTERNET.MOVISTAR.MX\",\"MOVISTAR\",\"MOVISTAR\"\r\n"
//#define AIPO "AIPO=1,,\"www.serverdeus.somee.com\",80,0,,1,2\r\n"
//#define HOST "HTTP/1.1\r\nHost: www.serverdeus.somee.com\r\n\r\n"

#define Buzzer_delay	200
#define Buzzer_On		1
#define Buzzer_Off      0
extern float current,battery,temp,s1,s2,reference;
/******************Variable and constant definitions*****/
const char hexcode[17]="0123456789abcdef";

char InputBuffer[350]={0};
int16 ptime=100;

/***********StringToHex***********************************
**   Description: This function converts a string in    **
**     their hex codification                           **
**                                                      **
*********************************************************/
void StringToHex(char* str,unsigned int size,char* result)
{
	int index=0,aux=0,aux2=0;
	for(index=0;index<size;index++)
		{
			aux=*(str+index);
			aux2=aux/16;
			*(result+(index*2))=hexcode[aux2];
			if(aux>15)
				{
					aux=aux-(aux2*16);
				}
			*(result+((index*2)+1))=hexcode[aux];	
		}
}

/***********FillArray*************************************
**   Description: Function used to fill an array with   **
**     value of (val) variable                          **
**                                                      **
*********************************************************/
void FillArray(char* arrayval,int size,char val)
{
	int index=0;
	for(index=0;index<size;index++)
	{
		*(arrayval+index)=val;
	}
}

/***********ReadBuffer************************************
**   Description: Reads incoming characters until arrive**
**     a carriage return '\r' and save the buffer in    **
**       *buffer array                                  **
*********************************************************/
void ReadBuffer(char* buffer)
{
	int16 index=0;
	OERR=16;
	//******Get incoming buffer data******
	while(getc()!='\n');
    do
    {
       *(buffer+index)=getc();	
		index++;
    }while(index<350 && *(buffer+index-1)!='\r');

	//***Buzzer sound when receive********
    //output_bit(PIN_D9,Buzzer_On);
	delay_ms(Buzzer_delay);
	//output_bit(PIN_D9,Buzzer_Off);
	OERR=16;

}

/***********PrintBuffer***********************************
**   Description: Prints in LCD the specified buffer    **
**     of characters with a permanency of ms            **
**                                                      **
*********************************************************/
void PrintBuffer(char* buffer,int16 permanency)
{
	lcd_putc("\f");
    printf(lcd_putc,"%s",buffer);
	delay_ms(permanency);
}

/***********ValidateCommand*******************************
**   Description: Validate that input string does not   **
**     contains an ERROR                                **
**                                                      **
*********************************************************/
int ValidateCommand(char* cmdstr,int size)
{
	int index=0,result=1;
	for(index=0;index<size;index++)
	{
		if(*(cmdstr+index)=='E' && *(cmdstr+(index+1))=='R') 
		{
			result=0;
		}
	}
    return result;
}

/***********GetDecVal*************************************
**   Description: Function used to find a decimal value **
**     into string                                      **
**                                                      **
*********************************************************/
int GetDecVal(char* arrayval,int16 size,char stch1,char stch2)
{
	int16 ind=0;
    int16  value=-1;
	while(ind<size && *(arrayval+ind)!='$')// && value==-1)
	{
		if(*(arrayval+ind)==stch1 && *(arrayval+(ind+1))==stch2)
			{
				ind+=2;
				value=0;
				while(*(arrayval+ind)>47 && *(arrayval+ind)<58)
					{
						value*=10;
						value+=(*(arrayval+ind)-48);
						ind++;
					}
			}
		ind++;
	}
	return value;
}

/***********ConfigureGPRS*********************************
**   Description: Function used to configure the GPRS   **
**     module to conect it to internet                  **
**                                                      **
*********************************************************/
void ConfigureGPRS()
{
	int status=0,count=0;
	while(status==0 && count<5)
		{
			FillArray(InputBuffer,350,0);  //Clear input_buffer
			printf("AT+CPIN?\r\n");         //Verify chip
			ReadBuffer(InputBuffer);
			PrintBuffer(InputBuffer,100);
			status=ValidateCommand(InputBuffer,30);
			if(status==1) lcd_putc("\fCMD OK");
			else 
				{	
					lcd_putc("\fCMD BAD");
					status=0;
				}
			delay_ms(100);
			if(status==1)
				{
					FillArray(InputBuffer,350,0);  //Clear input_buffer
					printf("AT+AIPDCONT=\"internet.movistar.mx\",\"movistar\",\"movistar\"\r\n");  //Configure internet
					ReadBuffer(InputBuffer);
					PrintBuffer(InputBuffer,1000);
					status=ValidateCommand(InputBuffer,30);
					if(status==1) lcd_putc("\fCMD OK");
					else 
						{
							lcd_putc("\fCMD BAD");
							status=0;
						}
					delay_ms(600);
				}		
			if(status==1)
				{
					FillArray(InputBuffer,350,0);  //Clear input_buffer
					printf("AT+AIPA=1\r\n");        //Conect to internet
					ReadBuffer(InputBuffer);
					PrintBuffer(InputBuffer,1000);
					status=ValidateCommand(InputBuffer,30);
					if(status==1) lcd_putc("\fCMD OK");
					else 
						{
							lcd_putc("\fCMD BAD");
							status=0;
						}
					delay_ms(500);
				}
			if(status==1)
				{
					FillArray(InputBuffer,350,0);  //Clear input_buffer
					printf("AT+AIPO=1,,\"www.serverdeus.somee.com\",80,0,,1,2\r\n");  //Conect to server"AIPO=1,,\"www.serverdeus.somee.com\",80,0,,1,2\r\n"
					ReadBuffer(InputBuffer);																	 
					PrintBuffer(InputBuffer,1000);
					status=ValidateCommand(InputBuffer,30);
					if(status==1) lcd_putc("\fCMD OK");
					else 
						{
							lcd_putc("\fCMD BAD");
							status=0;
						}
					delay_ms(500);
				}
			count++;
		}
	//return status;
}
/***********OpenPort**************************************
**   Description: Open a TCP port in domain name and    **
**     specified in dname and rport variables           **
**                                                      **
*********************************************************/
int OpenPort(int socket, int lport,char dname[100], int rport)
{
	int state=0;
	printf("AT+AIPO=%d,%d,\"%s\",%d,0,,1,2\r\n",socket,lport,dname,rport);  //Conect to server
	//******Get incoming buffer data******
	ReadBuffer(InputBuffer);
	//***Validate ok command**************
	state=ValidateCommand(InputBuffer,30);
	if(state==1) lcd_putc("\fCMD OK");
	else
		{
			 lcd_putc("\fCMD BAD");
			 state=0;
		}
	delay_ms(ptime);
	return state;
}

/***********ClosePort*************************************
**   Description: Close a TCP port in domain name and   **
**     specified in dname and rport variables           **
**                                                      **
*********************************************************/
void ClosePort(int socket)
{
	FillArray(InputBuffer,350,0);  //Clear input_buffer
	printf("AT+AIPC=%d\r\n",socket);  //Disconnect
	ReadBuffer(InputBuffer);
	//if(ValidateCommand(InputBuffer,30)==1) lcd_putc("\fCMD OK");
	//else lcd_putc("\fCMD BAD");
	//delay_ms(ptime);
}

/***********ConfigureDatetime*****************************
**   Description: Connects to server to obtain the      **
**     current datetime of the specified device         **
**                                                      **
**  Notes: incoming format:                             **
**  dy24mt3yr2014hr11mn52$ (24/marzo/2014 11:52 a.m)    **
*********************************************************/
int ConfigureDatetime(int Id, byte* DateTime)
{
	int status=0;
	int16 index=0;
	FillArray(InputBuffer,350,0);  //Clear input_buffer
	printf("GET /TIMEGPRS/NOWTIME/%d HTTP/1.1\r\nHost: www.serverdeus.somee.com\r\n\r\n",Id);
    do
    {
        InputBuffer[index]=getc();	
		index++;
    }while(index<350 && InputBuffer[index-1]!='$');

	if(ValidateCommand(InputBuffer,index)==1) 
		{
			lcd_putc("\fCMD OK");
			delay_ms(ptime);

			*(DateTime)=(int)GetDecVal(InputBuffer,index,'d','y');
			*(DateTime+1)=(int)GetDecVal(InputBuffer,index,'m','t');
			*(DateTime+2)=(int)(GetDecVal(InputBuffer,index,'y','r')-2000);
			*(DateTime+3)=(int)GetDecVal(InputBuffer,index,'h','r');
			*(DateTime+4)=(int)GetDecVal(InputBuffer,index,'m','n');

			status=1;
		}

	else 
		{
			status=0;
		}
	//***Close data connection************
	delay_ms(50);
	printf("+");
	printf("+");
	printf("+");
	OERR=16;
	ReadBuffer(InputBuffer);
	//PrintBuffer(InputBuffer,ptime);
	delay_ms(50);
	ClosePort(1);
	return status;
}

/***********GetSunriseTime********************************
**   Description: Connects to server to obtain the      **
**     current sunrise time of the specified device     **
**                                                      **
**  Notes: incoming format:                             **
**  hr11mn52$ (11:52 a.m)                               **
*********************************************************/
void GetOnTime(int Id, byte* DateTime)
{
	int16 index=0;
	FillArray(InputBuffer,350,0);  //Clear input_buffer
	printf("AT+AIPA=1\r\n");        //Conect to internet
	ReadBuffer(InputBuffer);
	delay_ms(ptime);

	FillArray(InputBuffer,350,0);  //Clear input_buffer
	printf("AT+AIPO=1,,\"www.serverdeus.somee.com\",80,0,,1,2\r\n");  //Conect to server
	ReadBuffer(InputBuffer);
	delay_ms(ptime);

	FillArray(InputBuffer,350,0);  //Clear input_buffer
	printf("GET /TIMEGPRS/ONTIME/%d HTTP/1.1\r\nHost: www.serverdeus.somee.com\r\n\r\n",Id);

    do
    {
        InputBuffer[index]=getc();	
		index++;
    }while(index<350 && InputBuffer[index-1]!='$');

	if(ValidateCommand(InputBuffer,index)==1) 
		{
			*(DateTime+3)=(int)GetDecVal(InputBuffer,index,'h','r');
			*(DateTime+4)=(int)GetDecVal(InputBuffer,index,'m','n');
		}

	//***Close data connection************
	delay_ms(50);
	printf("+");
	printf("+");
	printf("+");
	OERR=16;
	ReadBuffer(InputBuffer);
	ClosePort(1);
}

/***********GetSunsetTime********************************
**   Description: Connects to server to obtain the      **
**     current sunset time of the specified device      **
**                                                      **
**  Notes: incoming format:                             **
**  hr11mn52$ (11:52 a.m)                               **
*********************************************************/
void GetOffTime(int Id, byte* DateTime)
{
	int16 index=0;
	FillArray(InputBuffer,350,0);  //Clear input_buffer
	printf("AT+AIPA=1\r\n");        //Conect to internet
	ReadBuffer(InputBuffer);
	delay_ms(ptime);

	FillArray(InputBuffer,350,0);  //Clear input_buffer
	printf("AT+AIPO=1,,\"www.serverdeus.somee.com\",80,0,,1,2\r\n");  //Conect to server
	ReadBuffer(InputBuffer);
	delay_ms(ptime);

	FillArray(InputBuffer,350,0);  //Clear input_buffer
	printf("GET /TIMEGPRS/OFFTIME/%d HTTP/1.1\r\nHost: www.serverdeus.somee.com\r\n\r\n",Id);
    do
    {
        InputBuffer[index]=getc();	
		index++;
    }while(index<350 && InputBuffer[index-1]!='$');

	if(ValidateCommand(InputBuffer,index)==1) 
		{
			*(DateTime+3)=(int)GetDecVal(InputBuffer,index,'h','r');
			*(DateTime+4)=(int)GetDecVal(InputBuffer,index,'m','n');
		}

	//***Close data connection************
	delay_ms(50);
	printf("+");
	printf("+");
	printf("+");
	OERR=16;
	ReadBuffer(InputBuffer);
	delay_ms(50);
	ClosePort(1);
}

/***********GetMode***************************************
**   Description: Connects to server to obtain the      **
**     current datetime of the specified device         **
**                                                      **
*********************************************************/
void GetMode(int Id, int* mod,int state)
{
	int status=0;
	int16 index=0;

	FillArray(InputBuffer,350,0);  //Clear input_buffer
	printf("AT+AIPA=1\r\n");        //Conect to internet
	ReadBuffer(InputBuffer);
	//PrintBuffer(InputBuffer,ptime);
	//if(ValidateCommand(InputBuffer,30)==1) lcd_putc("\fCMD OK");
	//else lcd_putc("\fCMD BAD");
	delay_ms(ptime);

	FillArray(InputBuffer,350,0);  //Clear input_buffer
	printf("AT+AIPO=1,,\"www.serverdeus.somee.com\",80,0,,1,2\r\n");  //Conect to server
	ReadBuffer(InputBuffer);
	//PrintBuffer(InputBuffer,ptime);
	//if(ValidateCommand(InputBuffer,30)==1) lcd_putc("\fCMD OK");
	//else lcd_putc("\fCMD BAD");
	delay_ms(ptime);

	//**************GET REQUEST TO OBTAIN MODE****************
	FillArray(InputBuffer,350,0);  //Clear input_buffer
	printf("GET /DEVICEGPRS/GETMODE/%d HTTP/1.1\r\nHost: www.serverdeus.somee.com\r\n\r\n",Id);
	while(getc()!='\n');
	do
	{
		InputBuffer[index]=getc();
		index++;
	}while(index<350 && InputBuffer[index-1]!='$');

	OERR=16;
	//PrintBuffer(InputBuffer,ptime);

	if(ValidateCommand(InputBuffer,index)==1) 
		{
			*mod=(int)GetDecVal(InputBuffer,index,'m','d');
			status=1;
		}

	//**************GET REQUEST TO OBTAIN STATUS**************
	FillArray(InputBuffer,350,0);  //Clear input_buffer
	index=0;
	printf("GET /DEVICEGPRS/GETSTATUS/%d HTTP/1.1\r\nHost: www.serverdeus.somee.com\r\n\r\n",Id);
	while(getc()!='\n');
	do
	{
		InputBuffer[index]=getc();
		index++;
	}while(index<350 && InputBuffer[index-1]!='$');

	if(ValidateCommand(InputBuffer,index)==1) 
		{
			*state=(int)GetDecVal(InputBuffer,index,'s','t');
			status=1;
		}

	//***Close data connection************
	delay_ms(50);
	printf("+");
	printf("+");
	printf("+");
	OERR=16;
	ReadBuffer(InputBuffer);
	//PrintBuffer(InputBuffer,ptime);
	delay_ms(50);
	ClosePort(1);
}

/***********GetAutomated***************************************
**   Description: Connects to server to obtain the      **
**     current datetime of the specified device         **
**                                                      **
*********************************************************/
void GetAutomated(int Id, int* setpoint,int* histeresys)
{
	int status=0;
	int16 index=0;

	FillArray(InputBuffer,350,0);  //Clear input_buffer
	printf("AT+AIPA=1\r\n");        //Conect to internet
	ReadBuffer(InputBuffer);
	//PrintBuffer(InputBuffer,ptime);
	//if(ValidateCommand(InputBuffer,30)==1) lcd_putc("\fCMD OK");
	//else lcd_putc("\fCMD BAD");
	delay_ms(ptime);

	FillArray(InputBuffer,350,0);  //Clear input_buffer
	printf("AT+AIPO=1,,\"www.serverdeus.somee.com\",80,0,,1,2\r\n");  //Conect to server
	ReadBuffer(InputBuffer);
	//PrintBuffer(InputBuffer,ptime);
	//if(ValidateCommand(InputBuffer,30)==1) lcd_putc("\fCMD OK");
	//else lcd_putc("\fCMD BAD");
	delay_ms(ptime);

	//**************GET REQUEST TO OBTAIN MODE****************
	FillArray(InputBuffer,350,0);  //Clear input_buffer
	printf("GET /DEVICEGPRS/GETSETPOINT/%d HTTP/1.1\r\nHost: www.serverdeus.somee.com\r\n\r\n",Id);
	while(getc()!='\n');
	do
	{
		InputBuffer[index]=getc();
		index++;
	}while(index<350 && InputBuffer[index-1]!='$');

	OERR=16;
	//PrintBuffer(InputBuffer,ptime);

	if(ValidateCommand(InputBuffer,index)==1) 
		{
			*setpoint=(int)GetDecVal(InputBuffer,index,'s','t');
			status=1;
		}

	//**************GET REQUEST TO OBTAIN STATUS**************
	FillArray(InputBuffer,350,0);  //Clear input_buffer
	index=0;
	printf("GET /DEVICEGPRS/GETHIST/%d HTTP/1.1\r\nHost: www.serverdeus.somee.com\r\n\r\n",Id);
	while(getc()!='\n');
	do
	{
		InputBuffer[index]=getc();
		index++;
	}while(index<350 && InputBuffer[index-1]!='$');

	if(ValidateCommand(InputBuffer,index)==1) 
		{
			*histeresys=(int)GetDecVal(InputBuffer,index,'s','t');
			status=1;
		}

	//***Close data connection************
	delay_ms(50);
	printf("+");
	printf("+");
	printf("+");
	OERR=16;
	ReadBuffer(InputBuffer);
	//PrintBuffer(InputBuffer,ptime);
	delay_ms(50);
	ClosePort(1);
}
/***********GetPort***************************************
**   Description: Connects to server to obtain the      **
**     current datetime of the specified device         **
**                                                      **
*********************************************************/
void GetPort(int Id, int* porton,int* portoff,int* portin)
{
	int status=0;
	int16 index=0;

	FillArray(InputBuffer,350,0);  //Clear input_buffer
	printf("AT+AIPA=1\r\n");        //Conect to internet
	ReadBuffer(InputBuffer);
	//PrintBuffer(InputBuffer,ptime);
	//if(ValidateCommand(InputBuffer,30)==1) lcd_putc("\fCMD OK");
	//else lcd_putc("\fCMD BAD");
	delay_ms(ptime);

	FillArray(InputBuffer,350,0);  //Clear input_buffer
	printf("AT+AIPO=1,,\"www.serverdeus.somee.com\",80,0,,1,2\r\n");  //Conect to server
	ReadBuffer(InputBuffer);
	//PrintBuffer(InputBuffer,ptime);
	//if(ValidateCommand(InputBuffer,30)==1) lcd_putc("\fCMD OK");
	//else lcd_putc("\fCMD BAD");
	delay_ms(ptime);

	//**************GET REQUEST TO OBTAIN MODE****************
	FillArray(InputBuffer,350,0);  //Clear input_buffer
	printf("GET /DEVICEGPRS/GETPORTON/%d HTTP/1.1\r\nHost: www.serverdeus.somee.com\r\n\r\n",Id);
	while(getc()!='\n');
	do
	{
		InputBuffer[index]=getc();
		index++;
	}while(index<350 && InputBuffer[index-1]!='$');

	OERR=16;
	//PrintBuffer(InputBuffer,ptime);

	if(ValidateCommand(InputBuffer,index)==1) 
		{
			*porton=(int)GetDecVal(InputBuffer,index,'s','t');
			status=1;
		}

	//**************GET REQUEST TO OBTAIN STATUS**************
	FillArray(InputBuffer,350,0);  //Clear input_buffer
	index=0;
	printf("GET /DEVICEGPRS/GETPORTOFF/%d HTTP/1.1\r\nHost: www.serverdeus.somee.com\r\n\r\n",Id);
	while(getc()!='\n');
	do
	{
		InputBuffer[index]=getc();
		index++;
	}while(index<350 && InputBuffer[index-1]!='$');

	if(ValidateCommand(InputBuffer,index)==1) 
		{
			*portoff=(int)GetDecVal(InputBuffer,index,'s','t');
			status=1;
		}
	//**************GET REQUEST TO OBTAIN STATUS**************
	FillArray(InputBuffer,350,0);  //Clear input_buffer
	index=0;
	printf("GET /DEVICEGPRS/GETPORTIN/%d HTTP/1.1\r\nHost: www.serverdeus.somee.com\r\n\r\n",Id);
	while(getc()!='\n');
	do
	{
		InputBuffer[index]=getc();
		index++;
	}while(index<350 && InputBuffer[index-1]!='$');

	if(ValidateCommand(InputBuffer,index)==1) 
		{
			*portin=(int)GetDecVal(InputBuffer,index,'s','t');
			status=1;
		}
	//***Close data connection************
	delay_ms(50);
	printf("+");
	printf("+");
	printf("+");
	OERR=16;
	ReadBuffer(InputBuffer);
	//PrintBuffer(InputBuffer,ptime);
	delay_ms(50);
	ClosePort(1);
}
/***********ReportData************************************
**   Description: Connects to server to report the      **
**     current consuptions of current and voltage       **
**                                                      **
*********************************************************/

void ReportData(int Id, int status)
{
	int16 index=0;
	FillArray(InputBuffer,350,0);  //Clear input_buffer
	printf("AT+AIPA=1\r\n");        //Conect to internet
	ReadBuffer(InputBuffer);
	delay_ms(ptime);

	FillArray(InputBuffer,350,0);  //Clear input_buffer
	printf("AT+AIPO=1,,\"www.serverdeus.somee.com\",80,0,,1,2\r\n");  //Conect to server
	ReadBuffer(InputBuffer);
	delay_ms(ptime);

	//**************GET REQUEST TO OBTAIN MODE****************
	FillArray(InputBuffer,350,0);  //Clear input_buffer /monitoringgprs/create?id=14&status=0&current=1&voltage=2
	printf("GET /MONITORINGGPRS/CREATE2?ID=%d&STATUS=%d",Id,status);
	printf("&CURRENT=%f",current);
	printf("&VOLTAGE=%.2f",battery);
	printf("&VAR1=%f",s1);
	printf("&VAR2=%f HTTP/1.1\r\nHost: www.serverdeus.somee.com\r\n\r\n",s2);
		//printf("GET /MONITORINGGPRS/CREATE?ID=9&STATUS=1&CURRENT=6&VOLTAGE=6 HTTP/1.1\r\nHost: www.serverdeus.somee.com\r\n\r\n");
	while(getc()!='\n');
	do
	{
		InputBuffer[index]=getc();
		index++;
	}while(index<350 && InputBuffer[index-1]!='$');

	//***Close data connection************
	delay_ms(50);
	printf("+");
	printf("+");
	printf("+");
	OERR=16;
	ReadBuffer(InputBuffer);
	delay_ms(50);
	ClosePort(1);
	delay_ms(5000);
}

/***********CreateAlert***********************************
**   Description: Connects to server to report the      **
**     current alert detected by the device             **
**                                                      **
*********************************************************/
void CreateAlert(int Id, int type)
{
	int16 index=0;

	FillArray(InputBuffer,350,0);  //Clear input_buffer
	printf("AT+AIPA=1\r\n");        //Conect to internet
	ReadBuffer(InputBuffer);
	PrintBuffer(InputBuffer,ptime);
	if(ValidateCommand(InputBuffer,30)==1) lcd_putc("\fCMD OK");
	else lcd_putc("\fCMD BAD");
	delay_ms(ptime);

	FillArray(InputBuffer,350,0);  //Clear input_buffer
	printf("AT+AIPO=1,,\"www.serverdeus.somee.com\",80,0,,1,2\r\n");  //Conect to server
	ReadBuffer(InputBuffer);
	PrintBuffer(InputBuffer,ptime);
	if(ValidateCommand(InputBuffer,30)==1) lcd_putc("\fCMD OK");
	else lcd_putc("\fCMD BAD");
	delay_ms(ptime);

	//**************GET REQUEST TO OBTAIN MODE****************
	FillArray(InputBuffer,350,0);  //Clear input_buffer /monitoringgprs/create?id=14&status=0&current=1&voltage=2
	printf("GET /ALERTGPRS/CREATE?ID=%d&TYPE=%d HTTP/1.1\r\nHost: www.serverdeus.somee.com\r\n\r\n",Id,type);
	while(getc()!='\n');
	do
	{
		InputBuffer[index]=getc();
		index++;
	}while(index<350 && InputBuffer[index-1]!='$');

	//***Close data connection************
	delay_ms(50);
	printf("+++\r\n");
	OERR=16;
	//PrintBuffer(InputBuffer,ptime);
	PrintBuffer(InputBuffer,1000);

	ClosePort(1);
}
/***********Read ADC**************************************
**   Description: Connects to server to obtain the      **
**     current datetime of the specified device         **
**                                                      **
*********************************************************/
int16 ReadADC(int channel,int delay, int16 samples)
{
	int sample=0;
	int16 measure=0;
	set_adc_channel(channel);
	for(sample=0;sample<samples;sample++)	
	{
		measure=measure+read_ADC();
		delay_us(delay);
	}
	measure=measure/samples;
return measure;	
}