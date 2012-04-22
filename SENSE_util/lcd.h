/**
 * \file lcd.h
 * \brief LCD lib header file.
 *
 * \date 08/09/2011
 * \author Avinash Gupta 
 * \author Stefano Cillo <cillino.25@gmail.com>
 * \version v0.1
 *
 * A library to easily access standard LCD modules (HD44780-like) with AVR series of
 * MCUs from Atmel.	
*/


#include "myutils.h"

#ifndef F_CPU
	#define F_CPU 16000000UL
#endif

#ifndef LCD_H_
#define LCD_H_
/*_________________________________________________________________________________________*/

/************************************************
	LCD CONNECTIONS
*************************************************/

#define LCD_DATA	B		//Port PB0-PB3 are connected to D4-D7

#define LCD_E		D 		//Enable OR strobe signal
#define LCD_E_POS	PD7		//Position of enable in above port

#define LCD_RS		B	
#define LCD_RS_POS 	PB5

#define LCD_RW		B
#define LCD_RW_POS 	PB4


//************************************************

#define LS_BLINK 0B00000001
#define LS_ULINE 0B00000010


#define DOT_THREE       0.3f
#define HALF            0.5f
#define ONE             1
#define THIRTY          30


/***************************************************
			F U N C T I O N S
****************************************************/



void InitLCD(uint8_t style);
void LCDWriteString(const char *msg);
void LCDWriteInt(int val,unsigned int field_length);
void LCDGotoXY(uint8_t x,uint8_t y);
void LCDWriteStringXY(uint8_t x, uint8_t y, const char *msg);

//Low level
void LCDByte(uint8_t,uint8_t);

#define LCDCmd(command) (LCDByte(command,0))
#define LCDData(data) (LCDByte(data,1))

void LCDBusyLoop();





/***************************************************
			F U N C T I O N S     E N D
****************************************************/


/***************************************************
	M A C R O S
***************************************************/
#define LCDClear() LCDCmd(0b00000001);
#define LCDHome() LCDCmd(0b00000010);


#define LCDWriteIntXY(x,y,val,fl) {\
 LCDGotoXY(x,y);\
 LCDWriteInt(val,fl);\
}




#endif






