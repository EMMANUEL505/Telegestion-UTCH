/******************Global definitions********************/
#define AT "AT+"
#define CPIN "CPIN?\r\n"
#define AIPDCONT "AIPDCONT=\"INTERNET.MOVISTAR.MX\",\"MOVISTAR\",\"MOVISTAR\"\r\n"
#define AIPO "AIPO=1,,\"www.serverdeus.somee.com\",80,0,,1\r\n"

/******************Variable and constant definitions*****/
const char hexcode[17]="0123456789abcdef";

char InputBuffer[350]={0};
int16 ptime=100;

/***********StringToHex***********************************
**   Description: This function converts a string in    **
**     their hex codification                           **
**                                                      **
*********************************************************/
void StringToHex(char* Str,unsigned int size,char* result)
{
	int index=0,aux=0,aux2=0;
	for(index=0;index<size;index++)
		{
			aux=*(Str+index);
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
    output_bit(PIN_D9,1);
	delay_ms(100);
	output_bit(PIN_D9,0);
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
			delay_ms(500);
	
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
					delay_ms(500);
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
					printf("AT+AIPO=1,,\"www.serverdeus.somee.com\",80,0,,1,2\r\n");  //Conect to server
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
	//***Close data connection************
	delay_ms(50);
	printf("+++\r\n");
	OERR=16;
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
	return status;
}

/***********GetSunriseTime********************************
**   Description: Connects to server to obtain the      **
**     current sunrise time of the specified device     **
**                                                      **
**  Notes: incoming format:                             **
**  hr11mn52$ (11:52 a.m)                               **
*********************************************************/
void GetSunriseTime(int Id, byte* DateTime)
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
	printf("GET /TIMEGPRS/GETSUNRISE/%d HTTP/1.1\r\nHost: www.serverdeus.somee.com\r\n\r\n",Id);

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
void GetSunsetTime(int Id, byte* DateTime)
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
	printf("GET /TIMEGPRS/GETSUNSET/%d HTTP/1.1\r\nHost: www.serverdeus.somee.com\r\n\r\n",Id);
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

/***********ReportData************************************
**   Description: Connects to server to report the      **
**     current consuptions of current and voltage       **
**                                                      **
*********************************************************/
void ReportData(int Id, int state,float curr,float volt)
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
	printf("GET /MONITORINGGPRS/CREATE?ID=%d&STATUS=%d&CURRENT=%6.3f&VOLTAGE=%6.3f HTTP/1.1\r\nHost: www.serverdeus.somee.com\r\n\r\n",Id,state,curr,volt);
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
	printf("GET /ALERTGGPRS/CREATE?ID=%d&TYPE=%d HTTP/1.1\r\nHost: www.serverdeus.somee.com\r\n\r\n",Id,type);
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

