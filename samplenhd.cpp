//---------------------------------------------------------
/*
(c)2014 Newhaven Display International, Inc. 

 	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/
//---------------------------------------------------------

/*******************************************************************************
* Function Name  : TFT_24_7789_Init
* Description    : Initializes LCD with built-in ST7789S controller.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TFT_24_7789_Init(void)
{
int n;
GPIO_ResetBits(GPIOC, CS1);
GPIO_SetBits(GPIOC, nRD);
GPIO_ResetBits(GPIOC, nWR);
GPIO_WriteBit(GPIOC, RES, Bit_RESET);
TFT_delay(100);
GPIO_WriteBit(GPIOC, RES, Bit_SET);
TFT_delay(100);
TFT_24_7789_Write_Command(0x0011);//exit SLEEP mode
TFT_delay(100);

TFT_24_7789_Write_Command(0x0036);TFT_24_7789_Write_Data(0x0080);//MADCTL: memory data access control
TFT_24_7789_Write_Command(0x003A);TFT_24_7789_Write_Data(0x0066);//COLMOD: Interface Pixel format *** I use 262K-colors in 18bit/pixel format when using 8-bit interface to allow 3-bytes per pixel
//TFT_24_7789_Write_Command(0x003A);TFT_24_7789_Write_Data(0x0055);//COLMOD: Interface Pixel format  *** I use 65K-colors in 16bit/pixel (5-6-5) format when using 16-bit interface to allow 1-byte per pixel
TFT_24_7789_Write_Command(0x00B2);TFT_24_7789_Write_Data(0x000C);TFT_24_7789_Write_Data(0x0C);TFT_24_7789_Write_Data(0x00);TFT_24_7789_Write_Data(0x33);TFT_24_7789_Write_Data(0x33);//PORCTRK: Porch setting
TFT_24_7789_Write_Command(0x00B7);TFT_24_7789_Write_Data(0x0035);//GCTRL: Gate Control
TFT_24_7789_Write_Command(0x00BB);TFT_24_7789_Write_Data(0x002B);//VCOMS: VCOM setting
TFT_24_7789_Write_Command(0x00C0);TFT_24_7789_Write_Data(0x002C);//LCMCTRL: LCM Control
TFT_24_7789_Write_Command(0x00C2);TFT_24_7789_Write_Data(0x0001);TFT_24_7789_Write_Data(0xFF);//VDVVRHEN: VDV and VRH Command Enable
TFT_24_7789_Write_Command(0x00C3);TFT_24_7789_Write_Data(0x0011);//VRHS: VRH Set
TFT_24_7789_Write_Command(0x00C4);TFT_24_7789_Write_Data(0x0020);//VDVS: VDV Set
TFT_24_7789_Write_Command(0x00C6);TFT_24_7789_Write_Data(0x000F);//FRCTRL2: Frame Rate control in normal mode
TFT_24_7789_Write_Command(0x00D0);TFT_24_7789_Write_Data(0x00A4);TFT_24_7789_Write_Data(0xA1);//PWCTRL1: Power Control 1
TFT_24_7789_Write_Command(0x00E0);TFT_24_7789_Write_Data(0x00D0);
								  TFT_24_7789_Write_Data(0x0000);
								  TFT_24_7789_Write_Data(0x0005);
								  TFT_24_7789_Write_Data(0x000E);
								  TFT_24_7789_Write_Data(0x0015);
								  TFT_24_7789_Write_Data(0x000D);
								  TFT_24_7789_Write_Data(0x0037);
								  TFT_24_7789_Write_Data(0x0043);
								  TFT_24_7789_Write_Data(0x0047);
								  TFT_24_7789_Write_Data(0x0009);
								  TFT_24_7789_Write_Data(0x0015);
								  TFT_24_7789_Write_Data(0x0012);
								  TFT_24_7789_Write_Data(0x0016);
								  TFT_24_7789_Write_Data(0x0019);//PVGAMCTRL: Positive Voltage Gamma control	
TFT_24_7789_Write_Command(0x00E1);TFT_24_7789_Write_Data(0x00D0);
								  TFT_24_7789_Write_Data(0x0000);
								  TFT_24_7789_Write_Data(0x0005);
								  TFT_24_7789_Write_Data(0x000D);
								  TFT_24_7789_Write_Data(0x000C);
								  TFT_24_7789_Write_Data(0x0006);
								  TFT_24_7789_Write_Data(0x002D);
								  TFT_24_7789_Write_Data(0x0044);
								  TFT_24_7789_Write_Data(0x0040);
								  TFT_24_7789_Write_Data(0x000E);
								  TFT_24_7789_Write_Data(0x001C);
								  TFT_24_7789_Write_Data(0x0018);
								  TFT_24_7789_Write_Data(0x0016);
								  TFT_24_7789_Write_Data(0x0019);//NVGAMCTRL: Negative Voltage Gamma control
TFT_24_7789_Write_Command(0x002A);TFT_24_7789_Write_Data(0x0000);TFT_24_7789_Write_Data(0x0000);TFT_24_7789_Write_Data(0x0000);TFT_24_7789_Write_Data(0x00EF);//X address set
TFT_24_7789_Write_Command(0x002B);TFT_24_7789_Write_Data(0x0000);TFT_24_7789_Write_Data(0x0000);TFT_24_7789_Write_Data(0x0001);TFT_24_7789_Write_Data(0x003F);//Y address set

TFT_delay(10);
}




/*******************************************************************************
* Function Name  : TFT_24_7789_Write_Command
* Description    : writes a 1 byte command to 2.4" TFT.
* Input          : command = one byte command (register address)
* Output         : None
* Return         : None
*******************************************************************************/
void TFT_24_7789_Write_Command(unsigned int command)
{
GPIO_ResetBits(GPIOC, CS1);
GPIO_ResetBits(GPIOC, RS);
GPIO_SetBits(GPIOC, nRD);
GPIO_ResetBits(GPIOC, nWR);
GPIO_Write(GPIOB, command);//when using 16-bit interface (DB17:10,DB8:1)//when using 8-bit interface (DB17:10)
TFT_delay(10);
GPIO_SetBits(GPIOC, nWR);
TFT_delay(1);
}

/*******************************************************************************
* Function Name  : TFT_24_7789_Write_Data
* Description    : writes 1 byte of data to 2.4" TFT.
* Input          : data1 = one byte of display data or command parameter
* Output         : None
* Return         : None
*******************************************************************************/
void TFT_24_7789_Write_Data(unsigned int data1)
{
GPIO_Write(GPIOB, data1);//when using 16-bit interface (DB17:10,DB8:1)//when using 8-bit interface (DB17:10)
GPIO_SetBits(GPIOC, RS);
GPIO_ResetBits(GPIOC, nWR);
TFT_delay(1);
GPIO_SetBits(GPIOC, nWR);
}







/*******************************************************************************
* Function Name  : TFT_24_7789_demo
* Description    : Loads bmp from SD card and writes to NHD-2.4-240320CF-CTXI#.
* Input          : None
* Output         : None
* Return         : 1-end of function reached
*******************************************************************************/
int TFT_24_7789_demo(void)
{
GPIO_SetBits(GPIOC, IM0);		//8-bit mode
//GPIO_ResetBits(GPIOC, IM0);		//16-bit mode
TFT_24_7789_Init();
TFT_24S_Write_Command(0x002C);				//Memory write

for (n=0;n<3;n++){
	memset(RGB16,0x0000,sizeof(RGB16));
	for (i=0;i<25600;i++)					//for each 24-bit pixel...
	{
		f_read(&File1, &blue, 1, &blen);	//read the blue 8-bits
		f_read(&File1, &green, 1, &blen);	//read the green 8-bits
		f_read(&File1, &red, 1, &blen);		//read the red 8-bits
		
/* un-comment below for 8-bit interface */
		GPIO_SetBits(GPIOC, RS);
		
		GPIO_Write(GPIOB, red);
		GPIO_ResetBits(GPIOC, nWR);
		GPIO_SetBits(GPIOC, nWR);
		
		GPIO_Write(GPIOB, green);
		GPIO_ResetBits(GPIOC, nWR);
		GPIO_SetBits(GPIOC, nWR);
		
		GPIO_Write(GPIOB, blue);
		GPIO_ResetBits(GPIOC, nWR);
		GPIO_SetBits(GPIOC, nWR);
		
/* END of 8-bit interface */

/* un-comment below for 16-bit interface */
//
//		red=red>>3;						   	//shift down to 5-bits
//		green=green>>2;						//shift down to 6-bits
//		blue=blue>>3;						//shift down to 5-bits
//		RGB16[i]= (RGB16[i] | red);			//put red 5-bits into int
//		RGB16[i]= (RGB16[i] << 6);			//move red bits over, make room for green
//		RGB16[i]= (RGB16[i] | green);		//put green 6-bits into int
//		RGB16[i]= (RGB16[i] << 5);			//move red and green bits over, make room for blue
//		RGB16[i]= (RGB16[i] | blue);		//put blue 5-bits into int
//		GPIO_Write(GPIOB, RGB16[i]);
//		GPIO_SetBits(GPIOC, RS);
//		GPIO_ResetBits(GPIOC, nWR);
//		GPIO_SetBits(GPIOC, nWR);
//									 
/* END of 16-bit interface */

	}
}
					
TFT_24_7789_Write_Command(0x0029);				//display ON

return 1;
}