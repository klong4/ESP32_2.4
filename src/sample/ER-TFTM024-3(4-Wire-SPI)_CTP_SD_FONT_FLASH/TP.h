//FT6236
/* touch panel interface define */
sbit SDA	   =    P3^0;
sbit SCL       =    P3^1;
sbit PEN       =    P3^2;	

sbit KEY = P3^3;

#define uchar      unsigned char
#define uint       unsigned int
#define ulong      unsigned long

#define WHITE          0xFFFF
#define BLACK          0x0000
#define GRAY           0xF7DE
#define BLUE           0x001F
#define BLUE2          0x051F
#define RED            0xF800
#define PURPLE         0xF81F
#define GREEN          0x07E0
#define CYAN           0x7FFF
#define YELLOW         0xFFE0
#define DGREEN         0x07E0


//Touch Status	 
#define Key_Down 0x01
#define Key_Up   0x00 

struct _ts_event
{
    uint    x1;
    uint    y1;
    uint    x2;
    uint    y2;
    uint    x3;
    uint    y3;
    uint    x4;
    uint    y4;
    uint    x5;
    uint    y5;
    uchar     touch_point;
	uchar     Key_Sta;	
};

 uchar gesture;

struct _ts_event ts_event; 

#define WRITE_ADD	0x70  
#define READ_ADD	0x71  

void TOUCH_Init(void);
void TOUCH_Start(void);
void TOUCH_Stop(void);
void   TOUCH_Wait_Ack(void);
void TOUCH_Ack(void);
void TOUCH_NAck(void);

void TOUCH_Send_Byte(uchar txd);
uchar TOUCH_Read_Byte(unsigned char ack);
void TOUCH_Wr_Reg(uchar RegIndex,uchar RegValue1);
void TOUCH_RdParFrPCTPFun(uchar *PCTP_Par,uchar ValFlag);
uchar   TOUCH_Read_Reg(uchar RegIndex);
void Draw_Big_Point(uint x,uint y,uint colour);
uchar ft6x0x_read_data(void);


//IIC start
void TOUCH_Start(void)
{ 
	SDA=1;  
	delayus(5);	  
	SCL=1;
	delayus(5);
	SDA=0;  
	delayus(5);
	SCL=0;
	delayus(5);
}	  


//IIC stop
void TOUCH_Stop(void)
{
	SDA=0;
	delayus(5);
	SCL=1;
	delayus(5);
	SDA=1;
	delayus(5);
	SCL=0;
	delayus(5);							   	
}


//Wait for an answer signal
void TOUCH_Wait_Ack(void)
{     bit ack=1;

	SDA=1;
	delayus(5);
	SCL=1;
	_nop_();
  	while(ack)
	{
	   ack=SDA;
	}
	SCL=0;
	delayus(5);
}




//Acknowledge
void TOUCH_Ack(void)
{	SCL=0;
	SDA=0;
	delayus(5);
	SCL=1;
	delayus(5);
	SCL=0;
	delayus(5);
}



//NO Acknowledge		    
void TOUCH_NAck(void)
{	SCL=0;
	SDA=1;
	delayus(5);
	SCL=1;
	delayus(5);
	SCL=0;
	delayus(5);
}	
	

//IIC send one byte		  
void TOUCH_Send_Byte(uchar Byte)
{	uchar t;  		

    for(t=0;t<8;t++)
    { 	SCL=0;            
	   	SDA=(bit)(Byte & 0x80) ;
	   	Byte <<=1;
	delayus(5);
	   	SCL=1;
	delayus(5);
	   	SCL=0;
	delayus(5);
    }	
 

} 




/******************************************************************************************
*Function name£ºDraw_Big_Point(u16 x,u16 y)
* Parameter£ºuint16_t x,uint16_t y xy
* Return Value£ºvoid
* Function£ºDraw touch pen nib point 3 * 3
*********************************************************************************************/		   
void Draw_Big_Point(uint x,uint y,uint colour)
{uint num;	    
	
	LCD_SetPos(x,x+1,y,y+1);
	    for(num=0;num<4;num++)
		{    
          	 Write_Data(colour>>8,colour);
		}
}




//Read one byte£¬ack=0£¬Send Acknowledge£¬ack=1£¬NO Acknowledge   
uchar TOUCH_Read_Byte(uchar ack)
{	uchar t,receive=0;

	SCL=0;
	SDA=1;
	for(t = 0; t < 8; t++)
	{	_nop_();
	 	SCL = 1;
			delayus(5);
	 	receive<<=1;
	 	if(SDA == 1)
	 	receive=receive|0x01;
	delayus(5);
	 	SCL=0;
		delayus(5);
	}

					 
   	if (ack)  TOUCH_NAck();//NO Acknowledge 
   	else       TOUCH_Ack(); //Send Acknowledge   
    
	 return receive;
}





void TOUCH_Wr_Reg(uchar RegIndex,uchar RegValue1)
{
	TOUCH_Start();
	TOUCH_Send_Byte(WRITE_ADD);
	TOUCH_Wait_Ack();	
	TOUCH_Send_Byte(RegIndex);
	TOUCH_Wait_Ack();

	
	TOUCH_Send_Byte(RegValue1);
	TOUCH_Wait_Ack();

	TOUCH_Stop();
	delayus(10);
}


void TOUCH_RdParFrPCTPFun(uchar *PCTP_Par,uchar ValFlag)
{	uchar k;

	TOUCH_Start();
	TOUCH_Send_Byte(READ_ADD);
	TOUCH_Wait_Ack();	
	for(k=0;k<ValFlag;k++)
	{
		if(k==(ValFlag-1))  *(PCTP_Par+k)=TOUCH_Read_Byte(1);
		else                *(PCTP_Par+k)=TOUCH_Read_Byte(0);
	}		
	TOUCH_Stop();
}


uchar TOUCH_Read_Reg(uchar RegIndex)
{	uchar receive=0;

	TOUCH_Start();
	TOUCH_Send_Byte(WRITE_ADD);
	TOUCH_Wait_Ack();
	TOUCH_Send_Byte(RegIndex);
	TOUCH_Wait_Ack();
	TOUCH_Stop();
	
	TOUCH_Start();
	TOUCH_Send_Byte(READ_ADD);
	TOUCH_Wait_Ack();	
	receive=TOUCH_Read_Byte(1);
	TOUCH_Stop();
 	 
	return receive;
}


void ft6x0x_i2c_txdata(uchar *txdata, uchar length)
{	uchar ret =0;	uint num;

  	TOUCH_Start();
  	TOUCH_Send_Byte(WRITE_ADD);      
  	TOUCH_Wait_Ack();
  	for(num=0;num<length;num++)
  	{
		TOUCH_Send_Byte(txdata[num]); 
    	TOUCH_Wait_Ack();      
  	}                   
  	TOUCH_Stop();
 	delayus(5);


}

uchar ft6x0x_i2c_rxdata(uchar *rxdata, uchar length)
{	uchar num;	uchar *rxdatatmp =  rxdata;

  	TOUCH_Start();
  	TOUCH_Send_Byte(READ_ADD);
  	TOUCH_Wait_Ack();            
	for(num=0;num<length;num++)
  	{
		if(num==(length-1))  
		rxdatatmp[num]=TOUCH_Read_Byte(0);   
        else 
    	rxdatatmp[num]=TOUCH_Read_Byte(1);   
  	}

  	TOUCH_Stop();
	
  	return rxdatatmp;
}


uchar ft6x0x_read_data(void)
{	uchar buf[13] = {0}; uchar ret = 0;


	TOUCH_Start();
	TOUCH_Send_Byte(WRITE_ADD);
	TOUCH_Wait_Ack();
	TOUCH_Send_Byte(0x00);
	TOUCH_Wait_Ack();
	TOUCH_Stop();

		TOUCH_RdParFrPCTPFun(buf, 13);


	ts_event.touch_point = buf[2] & 0xf;



					switch (ts_event.touch_point) 
					{

						    case 2:
						          	ts_event.x2 = (uint)(buf[9] & 0x0F)<<8 | (uint)buf[10];
						           	ts_event.y2 = (uint)(buf[11] & 0x0F)<<8 | (uint)buf[12];
							
						
						    case 1:
						           	ts_event.x1 = (uint)(buf[3] & 0x0F)<<8 | (uint)buf[4];
						           	ts_event.y1 = (uint)(buf[5] & 0x0F)<<8 | (uint)buf[6];
				
						    break;
						    default:
						    return 0;
					}
					
				
//	gesture=buf[1];
    
	return ret;
}


void inttostr(uint value,uchar *str)
{
	str[0]=value/1000+48;
	str[1]=value%1000/100+48;
	str[2]=value%100/10+48;
	str[3]=value%100%10+48;

}





//show one Character
void showzifu(unsigned int x,unsigned int y,unsigned char value,unsigned int dcolor,unsigned int bgcolor)	
{  
	unsigned char i,j;
	unsigned char *temp=zifu;    
    LCD_SetPos(x,x+7,y,y+11);      
	temp+=(value-32)*12;
	for(j=0;j<12;j++)
	{
		for(i=0;i<8;i++)
		{ 		     
		 	if((*temp&(1<<(7-i)))!=0)
			{
				Write_Data(dcolor>>8,dcolor);
			} 
			else
			{
				Write_Data(bgcolor>>8,bgcolor);
			}   
		}
		temp++;
	 }
}

//show one Character
void showzifustr(unsigned int x,unsigned int y,unsigned char *str,unsigned int dcolor,unsigned int bgcolor)	  
{  
	unsigned int x1,y1;
	x1=x;
	y1=y;
	while(*str!='\0')
	{	
		showzifu(x1,y1,*str,dcolor,bgcolor);
		x1+=7;
		str++;
	}	
}

////////////////////////////////////
void  counter0(void) interrupt 0
{
 	if(PEN==0)										//Detect the occurrence of an interrupt
 	{
		ts_event.Key_Sta=Key_Down;                              

 	}
}




void TPTEST(void)
{  
	uchar ss[2]=0;	
	 IT0=1;        //Falling edge trigger  
	 EX0=1;
	 EA=1;



	LCD_FULL(0x0000);					
    showzifustr(70,100,"PLEASE TOUCH ME!",RED,WHITE);	




	while(KEY)
	{
			if(ts_event.Key_Sta==Key_Down)        //The touch screen is pressed
			{
				EX0=0;//Close interrupt
			
			do
				{	
					ft6x0x_read_data();
					ts_event.Key_Sta=Key_Up;
		
					inttostr(ts_event.x1,ss);
					showzifustr(40,70,"X1:",BLUE,WHITE);
					showzifustr(70,70,ss,RED,WHITE);	
					inttostr(ts_event.y1,ss);
					showzifustr(40,85,"Y1:",BLUE,WHITE);
					showzifustr(70,85,ss,RED,WHITE);	


					Draw_Big_Point(ts_event.x1,ts_event.y1,RED);
					Draw_Big_Point(ts_event.x2,ts_event.y2,GREEN);
						
		/*	
					gesture=TOUCH_Read_Reg(0x01);
					inttostr(gesture,ss);
					showzifustr(10,125,"gesture=",BLUE,WHITE);
					showzifustr(90,125,ss,RED,WHITE);	


					gesture=TOUCH_Read_Reg(0x02);
					inttostr(gesture,ss);
					showzifustr(10,135,"point no.=",BLUE,WHITE);
					showzifustr(90,135,ss,RED,WHITE);	

		   */
                     
				}while(PEN==0);
				EX0=1;
			}


    }



}























