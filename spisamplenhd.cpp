//---------------------------------------------------------
/*
(c)2019 Parham Keshavarzi - Newhaven Display International, Inc. 

 	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/
//---------------------------------------------------------

/*******************************************************************************/
command(unsigned int d)
{
unsigned char i;
unsigned char mask = 0x80;

digitalWrite(DC, LOW);  //Setting Data/command pin to command
digitalWrite(CS_PIN, LOW); //Enabling controller

//For loop to pass data one bit at a time
for(i=0;i<8;i++)
{
digitalWrite(SCLK_PIN, LOW); //Setting clock low

//Determine if 1 or zero
if((d & mask) >> 7 == 1)
 {
 digitalWrite(MOSI_PIN, HIGH); //Sending bit as 1
 }
else
 {
 digitalWrite(MOSI_PIN, LOW); //Sending bit as 0
 }
digitalWrite(SCLK_PIN, HIGH); //Data clocked in on rising edge 
d = d << 1; //shift byte value over by one bit
}
digitalWrite(CS_PIN, HIGH); //disabling controller
}
/*******************************************************************************/
data(unsigned int d)
{
unsigned char i;
unsigned char mask = 0x80;

digitalWrite(DC, HIGH);  //Setting Data/command pin to data
digitalWrite(CS_PIN, LOW); //Enabling controller

//For loop to pass data one bit at a time in each byte
for(i=0;i<8;i++)
{
digitalWrite(SCLK_PIN, LOW); //Setting clock low

//Determine if 1 or 0
if((d & mask) >> 7 == 1)
 {
 digitalWrite(MOSI_PIN, HIGH); //Sending bit as 1
 }
else
 {
 digitalWrite(MOSI_PIN, LOW); //Sending bit as 0
 }
digitalWrite(SCLK_PIN, HIGH); //Data clocked in on rising edge 
d = d << 1; //shift byte value over by one bit
}
digitalWrite(CS_PIN, HIGH); //disabling controller
}

/*******************************************************************************/
void setup() {
  pinMode(SCLK_PIN, OUTPUT);
  pinMode(MOSI_PIN, OUTPUT);
  pinMode(DC, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  pinMode(RST, OUTPUT);
  digitalWrite(RST, HIGH);
  
  //Factory initialization
  command(0x28);//Turn off display
  command(0x11);//Exit sleep mode
  command(0x36);
  data(0x88);   //MADCTL: memory data access control Old: 0x88
  command(0x3A);
  data(0x66);   //COLMOD: Interface Pixel format (18-bits per pixel @ 262K colors)
  command(0xB2);
  data(0x0C);
  data(0x0C);
  data(0x00);
  data(0x33);
  data(0x33);   //PORCTRK: Porch setting
  command(0xB7);
  data(0x35);   //GCTRL: Gate Control
  command(0xBB);
  data(0x2B);   //VCOMS: VCOM setting
  command(0xC0);
  data(0x2C);   //LCMCTRL: LCM Control
  command(0xC2);
  data(0x01);
  data(0xFF);   //VDVVRHEN: VDV and VRH Command Enable
  command(0xC3);
  data(0x11);   //VRHS: VRH set
  command(0xC4);
  data(0x20); //VDVS: VDV Set
  command(0xC6);
  data(0x0F); //FRCTRL2: Frame Rate control in normal mode
  command(0xD0);
  data(0xA4);
  data(0xA1);   //PWCTRL1: Power Control 1
  command(0xE0);
  data(0xD0);
  data(0x00);
  data(0x05);
  data(0x0E);
  data(0x15);
  data(0x0D);
  data(0x37);
  data(0x43);
  data(0x47);
  data(0x09);
  data(0x15);
  data(0x12);
  data(0x16);
  data(0x19); //PVGAMCTRL: Positive Voltage Gamma control
  command(0xE1);
  data(0xD0);
  data(0x00);
  data(0x05);
  data(0x0D);
  data(0x0C);
  data(0x06);
  data(0x2D);
  data(0x44);
  data(0x40);
  data(0x0E);
  data(0x1C);
  data(0x18);
  data(0x16);
  data(0x19); //NVGAMCTRL: Negative Voltage Gamma control
  command(0x2A);
  data(0x00);
  data(0x00);
  data(0x00);
  data(0xEF); //X address set
  command(0x2B);
  data(0x00);
  data(0x00);
  data(0x01);
  data(0x3F); //Y address set
  command(0x29);
  delay(10);
/*******************************************************************************/