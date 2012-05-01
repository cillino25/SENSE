/**
 * \file SENSE.h
 * \brief Header file for SENSE project.
 *
 * \date 08/07/2011 11:32:02
 * \author Stefano Cillo <cillino.25@gmail.com>
 * \version v0.04
 *
 * This file will contain all necessary includes and definitions.
 * 
 */ 

/*
Doxygen documenting commands:
\brief		add a short description.
\class		is used to indicate that the comment block contains documentation for the class.
\struct		to document a C-struct.
\union		to document a union..
\enum		to document an enumeration type.
\fn			to document a function.
\var		to document a variable or typedef or enum value.
\def		to document a #define.
\typedef	to document a type definition.
\file		to document a file.
\namespace	to document a namespace.
\package	to document a Java package.
\interface	to document an IDL interface.
*/

#ifndef SENSE_H_
#define SENSE_H_



/** \def CPU Frequency := 16 MHz */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/** \def Twi bitrate := 400 KHz */
#ifndef TWI_BITRATE
#define TWI_BITRATE 400000UL
#endif

#ifndef __HAS_DELAY_CYCLES
  #define __HAS_DELAY_CYCLES 1
#endif


#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <string.h>
#include <avr/sfr_defs.h>

#include <math.h>
#ifndef _UTIL_DELAY_H_
  #include <util/delay.h>
#endif

//#include <util/24c_.c>
#include <util/atomic.h>

#include "SENSE_util/lcd.c"
#include "SENSE_util/EEPROM.c"
#include "SENSE_util/i2c.c"



/************************* MiddleWare ***************************/

#define BUTTON_PORT				PORTD
#define BUTTON_PINS				PIND

#define BUTTON_A				BIT2
#define BUTTON_B				BIT4
#define BUTTON_C				BIT3

#define BACKLIGHT_PORT			PORTD
#define BACKLIGHT_PORT_DDR		DDRD
#define BACKLIGHT_PIN			BIT6



/************************* Constants ****************************/

#define ACK		1
#define NACK	0

#define BIT0	1
#define BIT1	2
#define BIT2	4
#define BIT3	8
#define BIT4	16
#define BIT5	32
#define BIT6	64
#define BIT7	128


#define DEBOUNCE_TIME				7		// RTC counting tens of milliseconds: this is 100ms!
#define REPEATED_PRESSION_TIME		35		//
#define LONG_PRESSION_TIME			100		// 
#define BACKLIGHT_TIME				800		// and this 8s.


/************* EEPROM ************/
#define MICROCHIP_EEPROM_READ_ACK_TYPE		NACK
#define MICROCHIP_EEPROM_WRITE_ACK_TYPE		ACK

/********** Sensors **********/
#define VREF					5.0
#define TEMP_SENSOR_GAIN		0.01   //  V / °C   == 10 mV / °C

#define HIH_ZERO_OFFSET		0.826
#define HIH_SLOPE			31.483



/*	bPriLev  */
#define PRI_TIMER0	1
#define PRI_ADC		2
#define PRI_TIMER2	3
#define PRI_MAIN	9


/*  bBtn  */
#define NO_BTN		0
#define BTN_A		2
#define BTN_A_LONG	3
#define BTN_B		4
#define BTN_B_LONG	5
#define BTN_C		6
#define BTN_C_LONG	7
#define BTN_AB		8
#define BTN_AB_LONG	9


/*  bState  */
#define STATE_IDLE							0
#define STATE_MENU							1
#define STATE_EDIT_DATE						2
#define STATE_EDIT_DATE_CONFIRM				3
#define STATE_EDIT_TIME						4
#define STATE_EDIT_TIME_CONFIRM				5
#define STATE_EDIT_HUM_ON_TH				6
#define STATE_EDIT_HUM_ON_TH_CONFIRM		7
#define STATE_EDIT_HUM_AL_TH				8
#define STATE_EDIT_HUM_AL_TH_CONFIRM		9



/*  bSelectionMenu  */ //Occhio: questi valori devono essere congruenti con la posizione delle relative stringhe quando stampo il menu
#define SEL_HUM_TH_1			0
#define SEL_HUM_TH_2			1
#define SEL_DATE				2
#define SEL_TIME				3

#define NUMBER_OF_OPTIONS		7



/* LCD cursor position possible values */
#define CLOCK_CURSOR_POSITION	11
#define TEMP_CURSOR_POSITION	0
#define HUM_CURSOR_POSITION		11
#define ZONE_CURSOR_POSITION	19


/***********************************************************************/


#define BACKLIGHT_ON() BACKLIGHT_PORT |= BACKLIGHT_PIN;
#define BACKLIGHT_OFF() BACKLIGHT_PORT &= ~BACKLIGHT_PIN;

#define TIMER2_CS	(1<<CS22)|(1<<CS21)|(1<<CS20)		// Timer2 clock selection bits: FCPU/1024

#define START_BACKLIGHT()\
	BACKLIGHT_PORT |= BACKLIGHT_PIN;\
	TCCR2B |= TIMER2_CS;

//#define RESET_BACKLIGHT()\
	
#define STOP_BACKLIGHT()\
	BACKLIGHT_PORT &= ~BACKLIGHT_PIN;\
	TCCR2B &= ~(TIMER2_CS);


/****************************** LCD Macros ******************************/

#define LCD_SET_BLINKING_CURSOR		LCDCmd(0x0f);
#define LCD_SET_UNDERLINE_CURSOR	LCDCmd(0x0e);
#define LCD_MAKE_CURSOR_INVISIBLE	LCDCmd(0x0c);

#define LCD_CURSOR_LEFT_N(n)	for( i=0; i<n; i++ ){ LCDCmd(0x10); }
#define LCD_CURSOR_RIGHT_N(n)	for( i=0; i<n; i++ ){ LCDCmd(0x14); }

//changes cursor position keeping it in the same row
#define LCD_SET_CURSOR_POSITION(n)\
		LCDHome(); LCD_CURSOR_RIGHT_N(n)

#define LCD_RESET()\
		LCDClear(); LCDCmd(0x02); LCDCmd(0x0C);


/*********************************** ADC Macros ***************************************/

#define ADC_PRESCALER_VALUE			(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)

#define ADC_HUMIDITY_CHANNEL		0
#define ADC_TEMPERATURE_CHANNEL		1


// ADC1, single ended
#define ADC_SET_TEMPERATURE_CHANNEL()\
	ADMUX=1<<MUX0;

// ADC0, single ended
#define ADC_SET_HUMIDITY_CHANNEL()\
	ADMUX=0;

#define START_ADC()\
	ADCSRA |= (1<<ADEN)|(1<<ADSC);
	


/*************************************************************************************/
/*********************************** Typedef *****************************************/
/*************************************************************************************/
	
	

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t longword;


typedef struct{
	word wMilli;
	byte bSec;
	byte bMin;
	byte bHour;
	byte bDay;
	byte bMonth;
	byte bYear;
} time_date;

typedef struct{
	byte bMin;
	byte bHour;
} time;

typedef struct{
	word wA;
	word wB;
	word wC;
	word wAB;
} count;

typedef struct{
	byte bDay;
	byte bMonth;
	byte bYear;
} date;

#define NUMBER_OF_LOGS_PER_DAY			48

typedef struct{
	date	dDate;
	float	fValues[NUMBER_OF_LOGS_PER_DAY];
} daily_log_type;

typedef struct{
	daily_log_type dsHumidity;
	daily_log_type dsTemperature;
} daily_log;




/*************************************************************************************/
/*********************************** Headers *****************************************/
/*************************************************************************************/


void _init_AVR(void);
void init_ADC(void);
void init_LCD(uint8_t bPowerUp);
void init_TIMER0_B(void);
void init_TIMER2_B(void);
void startADC();
float getTemperature(void);
float getHumidity(float temperature);
void refreshQuote(void);
void vConfirmState(void);
int isLeapYear(byte year);
int checkDay(volatile time_date *time, volatile byte* days);
void toggleTimeColon(void);
int _round(double x);

void vSaveToEEPROM()

char *itoa(int value, char * str, int base);
int sprintf(char * str, const char * format, ...);



#endif /* _SENSE_H_ */