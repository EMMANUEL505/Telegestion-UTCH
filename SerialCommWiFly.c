#define CMD_SIZE 3
#define SET_CMD 1
#define SET_STR "SET"

#define READ_CMD 0
#define READ_STR "RAD"

#define DAC 2
#define DAC_STR "DAC"

#define GPO 3
#define GPO_STR "GPO"

#define TIM 4
#define TIM_STR "TIM"

#define LCD 5
#define LCD_STR "LCD"

#define BUZ 6
#define BUZ_STR "BUZ"

#define SER 7
#define SER_STR "SER"


#define ADC 8
#define ADC_STR "ADC"

#define GPI 9
#define GPI_STR "GPI"

#define TEM 10
#define TEM_STR "TEM"

float HEX_StringToNumber(char *HexString,int Size);
int Module_Type(char *ModStr);
int CMD_Type(char *CMDStr);

int Module_Type(char *ModStr)
{
   int ModType=-1;
   char DAC_S[4]={DAC_STR};
   char GPO_S[4]={GPO_STR};
   char TIM_S[4]={TIM_STR};
   char LCD_S[4]={LCD_STR};
   char BUZ_S[4]={BUZ_STR};
   char SER_S[4]={SER_STR};
   char TEM_S[4]={TEM_STR};

   char ADC_S[4]={ADC_STR};
   char GPI_S[4]={GPI_STR};

   if(*ModStr==DAC_S[0]&& *(ModStr+1)==DAC_S[1]&& *(ModStr+2)==DAC_S[2]) ModType=DAC;
   if(*ModStr==GPO_S[0]&& *(ModStr+1)==GPO_S[1]&& *(ModStr+2)==GPO_S[2]) ModType=GPO;
   if(*ModStr==TIM_S[0]&& *(ModStr+1)==TIM_S[1]&& *(ModStr+2)==TIM_S[2]) ModType=TIM;
   if(*ModStr==LCD_S[0]&& *(ModStr+1)==LCD_S[1]&& *(ModStr+2)==LCD_S[2]) ModType=LCD;
   if(*ModStr==BUZ_S[0]&& *(ModStr+1)==BUZ_S[1]&& *(ModStr+2)==BUZ_S[2]) ModType=BUZ;
   if(*ModStr==SER_S[0]&& *(ModStr+1)==SER_S[1]&& *(ModStr+2)==SER_S[2]) ModType=SER;
   if(*ModStr==TEM_S[0]&& *(ModStr+1)==TEM_S[1]&& *(ModStr+2)==TEM_S[2]) ModType=TEM;

   if(*ModStr==ADC_S[0]&& *(ModStr+1)==ADC_S[1]&& *(ModStr+2)==ADC_S[2]) ModType=ADC; 
   if(*ModStr==GPI_S[0]&& *(ModStr+1)==GPI_S[1]&& *(ModStr+2)==GPI_S[2]) ModType=GPI;

   return ModType;
}

int CMD_Type(char *CMDStr)
{
   int CmdType=-1;
   char CMD_S[4]={SET_STR};
   char CMD_R[4]={READ_STR};
   if(*CMDStr==CMD_S[0]&& *(CMDStr+1)==CMD_S[1]&& *(CMDStr+2)==CMD_S[2]) CmdType=SET_CMD;
   if(*CMDStr==CMD_R[0]&& *(CMDStr+1)==CMD_R[1]&& *(CMDStr+2)==CMD_R[2]) CmdType=READ_CMD;
   return CmdType;
}
float HEX_StringToNumber(char *HexString,int Size)
{
   long counter=0,pow=16;
   char numeric[10];
   float ReturnNumber=0;
   
   for(counter=0;counter<Size;counter++)
      {
         numeric[counter]=*HexString;
         if(numeric[counter]<58) numeric[counter]-=48;
         if(numeric[counter]>58) numeric[counter]-=55;
         HexString+=1;
      }
   ReturnNumber+=(float)(numeric[Size-1]);  
   if(Size>1)
   {   
      for(counter=2;counter<(Size+1);counter++)
         {
            ReturnNumber+=(float)((float)(numeric[Size-counter])*pow);
            pow*=16;
         }        
   }  
   return ReturnNumber;
}
