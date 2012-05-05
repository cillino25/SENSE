/**
 * \file SENSE.c
 * \brief Main file for SENSE project.
 *
 * \date 08/07/2011 11:32:02
 * \author Stefano Cillo <cillino.25@gmail.com>
 * \version v0.04
 * 
 * The purpose of this code is to get ambient parameters like
 * temperature and humidity in a room; it will also record data into
 * an external EEPROM chip and make it available to the user thanks to
 * an FTDI serial-to-USB interface.
 *
 */

/*
Hungarian notation:
	b - byte (8 bit)
	w - word (16 bit)
	l - longword (32 bit)
	f - float (32 bit)
	d - double (64 bit)
*/

#include "SENSE.h"


volatile time_date tTime;				///< The actual RTC time.
volatile time_date tTimeEditing;		///< The value which the RTC will be set to.

volatile count cButtonIntegrator;		///< Keeps track of the time a button is being pressed.

volatile byte bTimeChanged;				///< Reports time is changed and quotes have to be refreshed
volatile byte bDateChanged;				///< Reports date has changed.
volatile byte bTempChanged;				///< Reports temperature has changed.
volatile byte bHumChanged;				///< Reports humidity has changed.
volatile byte bPrintQuotes;				///< Reports that quotes has to be printed.


volatile word wADC_garbage;				///< Garbage counter for ADC samples.
volatile word wADC_counts;				///< Counter for ADC samples.
volatile byte bChannel;					///< ADC channel selector value.

volatile word wBacklightCounter;		///< Time counter for backlight.


/**
 *	\brief Temperature samples array.
 *	
 *	Array kept in memory in order to store the acquired temperature samples;
 *	every NUMBER_OF_CONVERSIONS the average of these values will be calculated,
 *	and the variable dTemperature will be assigned.
 *	\sa NUMBER_OF_CONVERSION
 *	\sa dTemperature
*/
volatile float fTemperature;

/**
 * \brief Humidity samples array.
 *
 *  Array kept in memory in order to store the acquired humidity samples;
 *	every NUMBER_OF_CONVERSIONS the average of these values will be calculated,
 *	and the variable fHumidity will be assigned.
 *	\sa NUMBER_OF_CONVERSION
 *	\sa dHumidity
 */
volatile float fHumidity;				///< Humidity measured.

//volatile daily_log dlDataLog;			// Struct containing humidity and temperature logs.

volatile word wLoggedDays;				///< Number of days until last EEPROM export.
volatile byte bTodayLogs;				///< Number of logs taken today.
volatile longword lLastIndex;			///< Address of the next byte to be written into EEPROM (first FREE byte).

volatile byte bFirstConversion=1;
volatile byte bHumOverflow;				///< Needed for displaying correctly the humidity value onto the LCD.


volatile byte bHumOnThreshold;
volatile byte bHumAlarmThreshold;
byte bHumOnThresholdEditing;
byte bHumAlarmThresholdEditing;


volatile byte bBtnAPressed;
volatile byte bBtnBPressed;
volatile byte bBtnCPressed;
volatile byte bInhibite;
volatile byte bPort;
							
volatile byte bSelectionMenu;
volatile byte bSelectionMenuChanged;
volatile byte bSelection;
volatile byte bSelectionChanged;
volatile byte bTimeCommaState;
volatile byte bTimeColonToToggle;
volatile byte bBacklightActive;

volatile byte sreg;
volatile byte bPriLev;
volatile byte bState=STATE_IDLE;
volatile byte bStateOld=STATE_IDLE;
volatile byte bBtn;

char str[17]="";
char options[NUMBER_OF_OPTIONS+1][16]={"1.Soglia 1-DEUM","2.Soglia 2-ALL ", "3.Data         ",
					"4.Ora          ", "5.hello       ", "6.world        ", "7.ciao         ", "              "};


byte baDays[12]={31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	
volatile longword i=0;



/******************************************************************************/
/*********************************  Main  *************************************/
/******************************************************************************/

int main(void){
	bPriLev=PRI_MAIN;
	
	_init_AVR();
	
	BACKLIGHT_ON();
	
	while(1) { /* Infinite Loop */
			
		switch( bState ){

/*----------------------------------------------------------__IDLE__------------------------------------*/
			case STATE_IDLE:
				switch( bBtn ){
					case NO_BTN:
						if( bTimeColonToToggle ){ toggleTimeColon(); bTimeColonToToggle=0; }
						
						refreshQuote();
						bPrintQuotes=1;
						break;
						
					case BTN_A:
					case BTN_B:
					case BTN_C:
					case BTN_A_LONG:
					case BTN_B_LONG:
						STOP_BACKLIGHT();
						START_BACKLIGHT();
						bBtn=NO_BTN;
						break;
						
					case BTN_C_LONG:
						bState = STATE_MENU;
						STOP_BACKLIGHT();
						BACKLIGHT_ON();
						bBacklightActive=1;
						bBtn=NO_BTN;
						break;
						
					default:
						break;
				}
				break;
				
/*----------------------------------------------------------__MENU__----------------------------------*/				
			case STATE_MENU:
				switch( bBtn ){
					case NO_BTN:
						if( bSelectionMenuChanged || bPrintQuotes ){
							bPrintQuotes=0;
							bSelectionMenuChanged=0;
							LCDWriteStringXY(0,0,"-");
							LCDWriteStringXY(1,0, options[bSelectionMenu]);
							LCDWriteStringXY(0,1," ");
							LCDWriteStringXY(1,1, options[bSelectionMenu+1]);
						}
						break;
					
					case BTN_A:
						bSelectionMenu++;
						bSelectionMenu %= NUMBER_OF_OPTIONS;
						bSelectionMenuChanged=1;
						bBtn=NO_BTN;
						break;
					
					case BTN_B:
						if( bSelectionMenu>0 ) bSelectionMenu--;
						else bSelectionMenu=(NUMBER_OF_OPTIONS-1);
						bSelectionMenu %= NUMBER_OF_OPTIONS;
						bSelectionMenuChanged=1;
						bBtn=NO_BTN;
						break;
					
					case BTN_C:
						switch( bSelectionMenu ){
							case SEL_DATE:
								bState = STATE_EDIT_DATE;
								bBtn = NO_BTN;
								break;
								
							case SEL_TIME:
								bState = STATE_EDIT_TIME;
								bBtn = NO_BTN;
								break;
								
							case SEL_HUM_TH_1:
								bState = STATE_EDIT_HUM_ON_TH;
								bBtn = NO_BTN;
								break;
								
							case SEL_HUM_TH_2:
								bState = STATE_EDIT_HUM_AL_TH;
								bBtn = NO_BTN;
								break;
								
							default:
								break;
						}
						bPrintQuotes=1;
						break;
					
					case BTN_C_LONG:
						bState = STATE_IDLE;
						//init_LCD(0);		// Initialize the LCD, but we have already powered it up.
						LCD_RESET();
						
						bSelectionMenu=0;
						bDateChanged=1;		// Appena rientro in idle stampo le quote
						bTimeChanged=1;
						bTempChanged=1;
						bHumChanged=1;
						
						bBacklightActive=0;
						BACKLIGHT_OFF();
						START_BACKLIGHT();
						bBtn=NO_BTN;
						break;
					
					default:
						break;
				}				
				break;
			
/*--------------------------------------------------------------__DATE__-------------------------------*/			
			case STATE_EDIT_DATE:
				switch( bBtn ){
					case NO_BTN:
						if( bPrintQuotes ){
							bPrintQuotes=0;
							LCDClear();
							tTimeEditing = tTime;
							sprintf(str, "%02d/%02d/%02d", tTimeEditing.bDay, tTimeEditing.bMonth, tTimeEditing.bYear);
							LCDWriteStringXY(0,0, "Editing date:");
							LCDWriteStringXY(3,1, str);
							LCD_SET_UNDERLINE_CURSOR;
							LCD_CURSOR_LEFT_N(7);
						}
						break;
						
					case BTN_A:
						switch( bSelection ){
							case 0:		// day
								if(++tTimeEditing.bDay > baDays[tTimeEditing.bMonth-1]) tTimeEditing.bDay=1;
								if(tTimeEditing.bMonth == 2 && !isLeapYear(tTimeEditing.bYear) && tTimeEditing.bDay == 29) tTimeEditing.bDay=1;
								break;
							case 1:		// month
								if(++tTimeEditing.bMonth > 12) tTimeEditing.bMonth=1;
								checkDay(&tTimeEditing, baDays);
								break;
							case 2:		// year
								tTimeEditing.bYear++;
								checkDay(&tTimeEditing, baDays);
								break;
							default: break;
						}
						sprintf(str, "%02d/%02d/%02d", tTimeEditing.bDay, tTimeEditing.bMonth, tTimeEditing.bYear);
						LCDWriteStringXY(3,1,str);
						LCD_CURSOR_LEFT_N(7-3*bSelection);
						bBtn = NO_BTN;
						break;
						
					case BTN_B:
						switch( bSelection ){
							case 0:		// day
								if(--tTimeEditing.bDay < 1){
									tTimeEditing.bDay=baDays[tTimeEditing.bMonth-1];
									if((tTimeEditing.bMonth==2) && (!isLeapYear(tTimeEditing.bYear))) tTimeEditing.bDay=28;
								}								
								break;
							case 1:		// month
								if(--tTimeEditing.bMonth < 1) tTimeEditing.bMonth=12;
								checkDay(&tTimeEditing, baDays);
								break;
							case 2:		// year
								if(--tTimeEditing.bYear < 1) tTimeEditing.bYear=99;
								checkDay(&tTimeEditing, baDays);
								break;
							default: break;
						}
						sprintf(str, "%02d/%02d/%02d", tTimeEditing.bDay, tTimeEditing.bMonth, tTimeEditing.bYear);
						LCDWriteStringXY(3,1,str);
						LCD_CURSOR_LEFT_N(7-3*bSelection);
						bBtn = NO_BTN;
						break;
						
					case BTN_C:
						if( bSelection<2 ){
							LCD_CURSOR_RIGHT_N(3);
							bSelection++;
						}else{
							bSelection=0;
							LCD_CURSOR_LEFT_N(6);
						}
						if(checkDay(&tTimeEditing, baDays)){
							sprintf(str, "%02d/%02d/%02d", tTimeEditing.bDay, tTimeEditing.bMonth, tTimeEditing.bYear);
							LCDWriteStringXY(3,1,str);
							LCD_CURSOR_LEFT_N(7-3*bSelection);
						}
						bBtn = NO_BTN;
						break;
						
					case BTN_C_LONG:
						bState=STATE_EDIT_DATE_CONFIRM;
						bBtn=NO_BTN;
						bPrintQuotes=1;
						break;
						
					default: break;
				}
				break;

/*----------------------------------------------------------__DATE_confirm__--------------------------------*/
			case STATE_EDIT_DATE_CONFIRM:
				vConfirmState();
				break;

/*--------------------------------------------------------------__TIME__-------------------------------*/
			case STATE_EDIT_TIME:
					switch( bBtn ){
						case NO_BTN:
							if( bPrintQuotes ){
								bPrintQuotes=0;
								LCDClear();
								tTimeEditing = tTime;
								sprintf(str, "%02d:%02d:%02d", tTimeEditing.bHour, tTimeEditing.bMin, tTimeEditing.bSec);
								LCDWriteStringXY(0,0, "Editing time:");
								LCDWriteStringXY(3,1, str);
								LCD_SET_UNDERLINE_CURSOR;
								LCD_CURSOR_LEFT_N(7);
							}
							break;
							
						case BTN_A:
							bBtn = NO_BTN;
							switch ( bSelection ){
								case 0: // Hours
									tTimeEditing.bHour++;
									tTimeEditing.bHour %= 24;
									break;
								case 1: // Minutes
									tTimeEditing.bMin++;
									tTimeEditing.bMin %= 60;
									break;
								case 2: // Seconds
									tTimeEditing.bSec++;
									tTimeEditing.bSec %= 60;
									break;
								default: break;
							}
							
							sprintf(str, "%02d:%02d:%02d", tTimeEditing.bHour, tTimeEditing.bMin, tTimeEditing.bSec);
							LCDWriteStringXY(3,1,str);
							LCD_CURSOR_LEFT_N(7-3*bSelection);
							break;
							
						case BTN_B:
							bBtn = NO_BTN;
							switch ( bSelection ){
								case 0:
									if(--tTimeEditing.bHour>24) tTimeEditing.bHour=23; // Sarebbe "<0" ma queste variabili non vengono interpretate come con segno
									break;
								case 1:
									if(--tTimeEditing.bMin>60) tTimeEditing.bMin=59;
									break;
								case 2:
									if(--tTimeEditing.bSec>60) tTimeEditing.bSec=59;
									break;
								default: break;
							}
							sprintf(str, "%02d:%02d:%02d", tTimeEditing.bHour, tTimeEditing.bMin, tTimeEditing.bSec);
							LCDWriteStringXY(3,1,str);
							LCD_CURSOR_LEFT_N(7-3*bSelection);
							break;
							
						case BTN_C:
							if( bSelection<2 ){
								LCD_CURSOR_RIGHT_N(3);
								bSelection++;
							}else{
								bSelection=0;
								LCD_CURSOR_LEFT_N(6);
							}
							bBtn = NO_BTN;
							break;
							
						case BTN_C_LONG:
							bState=STATE_EDIT_TIME_CONFIRM;
							bBtn=NO_BTN;
							bPrintQuotes=1;
							break;
							
						default:
							break;
					}
				
				break;

/*--------------------------------------------------------------__TIME_confirm__------------------------------*/
			case STATE_EDIT_TIME_CONFIRM:
				vConfirmState();
				break;
				
/*-------------------------------------------------------------------__HUMIDITY_ON__---------------------------*/
			case STATE_EDIT_HUM_ON_TH:
				
				switch( bBtn ){
					case NO_BTN:
						if( bPrintQuotes ){
							bPrintQuotes=0;
							LCDClear();
							bHumOnThresholdEditing = bHumOnThreshold;
							sprintf(str, "p:%02dRH,  c:%02d RH", bHumOnThreshold, bHumOnThresholdEditing);
							LCDWriteStringXY(0,0, "Edit Hum-On TH:");
							LCDWriteStringXY(0,1, str);
							//LCDWriteString("%");
							LCD_SET_UNDERLINE_CURSOR;
							LCD_CURSOR_LEFT_N(4);
						}
						break;
						
					case BTN_A:
						if( ++bHumOnThresholdEditing > 99 ) bHumOnThresholdEditing = 0;
						sprintf(str, "%02d", bHumOnThresholdEditing);
						LCDWriteStringXY(11,1, str);
						LCD_CURSOR_LEFT_N(1);
						bBtn=0;
						break;
						
					case BTN_B:
						if( --bHumOnThresholdEditing > 100 ) bHumOnThresholdEditing=99;
						sprintf(str, "%02d", bHumOnThresholdEditing);
						LCDWriteStringXY(11,1, str);
						LCD_CURSOR_LEFT_N(1);
						bBtn=0;
						break;
						
					case BTN_C_LONG:
						bState = STATE_EDIT_HUM_ON_TH_CONFIRM;
						sprintf(str, "");
						bPrintQuotes=1;
						bBtn=0;
						break;
					default: break;
				}
				
				break;
				

/*--------------------------------------------------------------__HUMIDITY_ON_confirm__-------------------------*/
			case STATE_EDIT_HUM_ON_TH_CONFIRM:
				vConfirmState();
				break;

/*-------------------------------------------------------------------__HUMIDITY_AL__---------------------------*/
			case STATE_EDIT_HUM_AL_TH:
				
				switch( bBtn ){
					case NO_BTN:
						if( bPrintQuotes ){
							bPrintQuotes=0;
							LCDClear();
							bHumAlarmThresholdEditing = bHumAlarmThreshold;
							sprintf(str, "p:%02dRH,  c:%02d RH", bHumAlarmThreshold, bHumAlarmThresholdEditing);
							LCDWriteStringXY(0,0, "Edit Hum-Al TH:");
							LCDWriteStringXY(0,1, str);
							LCD_SET_UNDERLINE_CURSOR;
							LCD_CURSOR_LEFT_N(4);
						}
						break;
						
					case BTN_A:
						if( ++bHumAlarmThresholdEditing > 99 ) bHumAlarmThresholdEditing = 0;
						sprintf(str, "%02d", bHumAlarmThresholdEditing);
						LCDWriteStringXY(11,1, str);
						LCD_CURSOR_LEFT_N(1);
						bBtn=0;
						break;
						
					case BTN_B:
						if( --bHumAlarmThresholdEditing > 100 ) bHumAlarmThresholdEditing=99;
						sprintf(str, "%02d", bHumAlarmThresholdEditing);
						LCDWriteStringXY(11,1, str);
						LCD_CURSOR_LEFT_N(1);
						bBtn=0;
						break;
						
					case BTN_C_LONG:
						bState = STATE_EDIT_HUM_AL_TH_CONFIRM;
						
						bPrintQuotes=1;
						bBtn=0;
						break;
					default: break;
				}
				
				break;
				

/*--------------------------------------------------------------__HUMIDITY_AL_confirm__-------------------------*/
			case STATE_EDIT_HUM_AL_TH_CONFIRM:
				vConfirmState();
				break;
				
/*---------------------------------------------------------------__LOGGING_DATA__-------------------------------*/
			case STATE_LOG_DATA:
				if(bTodayLogs == 0){
					EEPROM_writeByte(lLastIndex++, tTime.bDay);
					EEPROM_writeByte(lLastIndex++, tTime.bMonth);
					EEPROM_writeByte(lLastIndex++, tTime.bYear);
				}
				EEPROM_writeData(lLastIndex, (byte*)&fHumidity, SIZE_OF_LOG);
				lLastIndex += SIZE_OF_LOG;
				EEPROM_writeData(lLastIndex, (byte*)&fTemperature, SIZE_OF_LOG);
				lLastIndex += SIZE_OF_LOG;
				
				if(++bTodayLogs >= NUMBER_OF_LOGS_PER_DAY){
					bTodayLogs=0;
					wLoggedDays++;
					
					// Ho raggiunto il numero massimo di log giornalieri: aggiorno i valori della data e LoggedDays nella EEPROM.
					EEPROM_writeByte(EEPROM_DAY_ADD, tTime.bDay);
					EEPROM_writeByte(EEPROM_MONTH_ADD, tTime.bMonth);
					EEPROM_writeByte(EEPROM_YEAR_ADD, tTime.bYear);
					
					EEPROM_writeData(EEPROM_LOGGED_DAYS_ADD, (byte*)&wLoggedDays, sizeof(word));
				}
				
				// Aggiorno todayLogs, lastIndex e ora ad ogni campionamento.
				EEPROM_writeByte(EEPROM_TODAY_LOGS_ADD, bTodayLogs);
				EEPROM_writeData(EEPROM_LAST_INDEX_ADD, (byte*)&lLastIndex, sizeof(long));
				
				EEPROM_writeByte(EEPROM_MIN_ADD, tTime.bMin);
				EEPROM_writeByte(EEPROM_HOUR_ADD, tTime.bHour);
				
				//LCDClear();
				//sprintf(str, "%d", bState);
				//LCDWriteStringXY(0,0,str);
				
				bState = bStateOld;
				
				//sprintf(str, "%d", bState);
				//LCDWriteStringXY(0,1,str);
				//
				//_delay_ms(200);
				
				break;
				
/*------------------------------------------------------------------------------------------------------------*/
			default:
				break;
		}
	}	
}




/************************************************************************************/
/*******************************   Interrupts   *************************************/
/************************************************************************************/


/****************************  RealTimeClock Interrupt ******************************/
ISR(TIMER0_COMPB_vect){
	if(bPriLev<PRI_TIMER0)	return;
	byte bOldPriLev = bPriLev;
	bPriLev=PRI_TIMER0;
	
	
/*	*************** FILTERS **************	*/
	bPort = BUTTON_PINS;
	bBtnAPressed = bPort & BIT2;		// UP
	bBtnBPressed = bPort & BIT4;		// DOWN
	bBtnCPressed = bPort & BIT3;		// MENU
	// bBtnXPressed=0  -->button X pressed! connected to ground
	
	if(!bBtnCPressed&&(!bInhibite)){ //btnC is pressed
		cButtonIntegrator.wC++;
		if(cButtonIntegrator.wC>LONG_PRESSION_TIME){ bInhibite=1; }  // inhibites the btnC counter whenever entered in the menu
	}
	else{
		if(bBtnCPressed!=0){	//btnC is not pressed <-- è qui la furbata! disinibisco il pulsante C solo nel momento in 
			bInhibite=0;		// cui questo viene rilasciato; se entro nel menu e il pulsante è ancora premuto,
		}						// bInhibite resta a 1 e quindi wC a 0; questo finchè mantengo premuto C.
								// Quando lo rilascio lo disinibisco, riattivandolo.
		
		if(cButtonIntegrator.wC<DEBOUNCE_TIME){
			cButtonIntegrator.wC=0;
		}else if((cButtonIntegrator.wC>DEBOUNCE_TIME)&&(cButtonIntegrator.wC<LONG_PRESSION_TIME)){
			cButtonIntegrator.wC=0;
			bBtn = BTN_C;
		}else if(cButtonIntegrator.wC>LONG_PRESSION_TIME){
			cButtonIntegrator.wC=0;
			bBtn = BTN_C_LONG;
		}
	}
	
	
	if(!bBtnAPressed){ 
		if(++cButtonIntegrator.wA>REPEATED_PRESSION_TIME){
			bBtn = BTN_A;
			cButtonIntegrator.wA=0;
		}
	}else{
		if(cButtonIntegrator.wA<DEBOUNCE_TIME){ cButtonIntegrator.wA=0; }
		else{
			bBtn = BTN_A;
			cButtonIntegrator.wA=0;
		}
	}
	
	
	if(!bBtnBPressed){
		if(++cButtonIntegrator.wB>REPEATED_PRESSION_TIME){
			bBtn = BTN_B;
			cButtonIntegrator.wB=0;
		}
	}else{
		if(cButtonIntegrator.wB<DEBOUNCE_TIME){ cButtonIntegrator.wB=0; }
		else{
			bBtn = BTN_B;
			cButtonIntegrator.wB=0;
		}
	}
	
	
	if((!bBtnAPressed)&&(!bBtnBPressed)){ cButtonIntegrator.wAB++; }
	else{
		if(cButtonIntegrator.wAB<DEBOUNCE_TIME){ cButtonIntegrator.wAB=0; }
		else{
			bBtn = BTN_AB;
			cButtonIntegrator.wAB=0;
		}
	}
	
	
/* ******************************* RTC ******************************** */	
	
	if(tTime.wMilli<99) tTime.wMilli++;
	else{
		tTime.wMilli=0;
		if( tTime.bSec<59 ){
			tTime.bSec++;
			bTimeColonToToggle=1;	// time colon is flashing at 1 Hz
			//START_ADC();			//testing: adc int every 1 sec
		}else{
			tTime.bSec=0;
			if( tTime.bMin<59 ){
				tTime.bMin++;
			}else{
				tTime.bMin=0;
				if( tTime.bHour<23 ) tTime.bHour++;
				else {
					tTime.bHour=0;
					if(tTime.bDay<(baDays[tTime.bMonth-1])){
						tTime.bDay++;
						if(tTime.bDay==29 && tTime.bMonth==2 && (!isLeapYear(tTime.bYear))){
							tTime.bDay=1;
							tTime.bMonth=3;
						}
					}else{
						tTime.bDay=1;
						if(tTime.bMonth<12) tTime.bMonth++;
						else{
							tTime.bMonth=1;
							tTime.bYear++;  // non c'è bisogno di impostare la data e il giorno, viene eseguito
											// automaticamente dall'insù, cioè dai millisecondi.
						} // month
					} // day
					bDateChanged=1;
				}	// hour				
			}  // minute
			bTimeChanged=1;		// refresh quote every min for the minutes changing
			
			if(isTimeToSample(&tTime)){		// if it is time to log data into EEPROM
				START_ADC();					// start ADC
			}
			
		}	// second
	} // millisecond
	
	bPriLev = bOldPriLev;
}


/*************************** Timer2 Interrupt / Backlight *****************************/
ISR(TIMER2_COMPB_vect){			// Timer2 : 8 bit
	if(bPriLev<PRI_TIMER2) return;
	byte bOldPriLev = bPriLev;
	
	if(bBacklightActive) return;
	
	if(wBacklightCounter<BACKLIGHT_TIME){ wBacklightCounter++; return; }
	wBacklightCounter=0;
	STOP_BACKLIGHT();
	
	bPriLev = bOldPriLev;
}


/****************************  ADC Interrupt ******************************/
ISR(ADC_vect){
	if(bPriLev<PRI_ADC){
		ADCSRA |= 1<<ADIF;
		return;
	}
	
	sreg = MCUSR;	// float operations: avoid interrupts to make operations reliable
	cli();
	
	byte bOldPriLev = bPriLev;
	
	
	if(bFirstConversion){
		bFirstConversion=0;
		ADCSRA |= 1<<ADSC;		// imposto l'adc perchè faccia la seconda campionatura
		return;					// mentre la prima viene scartata
	}
	
	
	float fTemperatureOld;
	float fHumidityOld;
	
	switch(bChannel){
		case ADC_TEMPERATURE_CHANNEL:
			fTemperatureOld = fTemperature;
			fTemperature = getTemperature();
			if(fTemperatureOld != fTemperature) bTempChanged=1;
			
			ADCSRA |= 1<<ADSC;
			
			ADC_SET_HUMIDITY_CHANNEL();
			bChannel = ADC_HUMIDITY_CHANNEL;
			bFirstConversion=1;
			break;
			
		case ADC_HUMIDITY_CHANNEL:
			fHumidityOld = fHumidity;
			fHumidity = getHumidity(fTemperature);
			if(fHumidityOld != fHumidity) bHumChanged=1;
			
			ADC_SET_TEMPERATURE_CHANNEL();
			bChannel = ADC_TEMPERATURE_CHANNEL;
			bFirstConversion=1;
			
			bStateOld = bState;			// cambio lo stato qui e non all'interno di ISR(TIMER_0) perchè è dopo questa
			bState = STATE_LOG_DATA;	// routine che sono pronti i campioni.
			
			break;
		
		default: break;
		
	}
	
	sei();
	MCUSR=sreg;
	
	bPriLev = bOldPriLev;
}


/*******************************************************************/
/*******************************************************************/

void init_EEPROM(void){
	EEPROM_open();
	//EEPROM_erase(1024);
	#ifdef TESTING
		EEPROM_writeByte(EEPROM_DAY_ADD, 06);
		EEPROM_writeByte(EEPROM_MONTH_ADD, 05);
		EEPROM_writeByte(EEPROM_YEAR_ADD, 12);
		EEPROM_writeByte(EEPROM_MIN_ADD, 0);
		EEPROM_writeByte(EEPROM_HOUR_ADD, 0);
		
		
		EEPROM_writeByte(EEPROM_TODAY_LOGS_ADD, 0x0);
		
		word daysNum = 2;
		EEPROM_writeData(EEPROM_LOGGED_DAYS_ADD, (uint8_t*)&daysNum, sizeof(word));
		
		long add = 0x33;
		EEPROM_writeData(EEPROM_LAST_INDEX_ADD, (uint8_t*)&add, sizeof(long));
	#endif
	
	
	
	
}

void init_ADC(void){
	ADCSRA = ADC_PRESCALER_VALUE;					// ADC Prescaler = Fck/128
	ADCSRA |= (1<<ADIE);							// enabling ADC Interrupt
	ADC_SET_TEMPERATURE_CHANNEL();					// let's start with temperature
	bChannel = ADC_TEMPERATURE_CHANNEL;
}

void init_LCD(uint8_t bPowerUp){
	if(bPowerUp) InitLCD(0);
	printIdleLCD();
}

/*
	OCCHIO: Timer0_A e Timer0_B condividono lo stesso prescaler, il cui valore va assegnato sul registro TCCR0B
*/
void init_TIMER0_B(void){
	TCCR0B |= (1<<CS02)|(1<<CS00);			// clock: F_CPU / 1024
	TCCR0A |= (1<<WGM01);					// Clear Timer on Compare
	TIMSK0 |= (1<<OCIE0B);					// Output compare match interrupt enable
	OCR0A = 156;							// Interrupt every 10ms
}


void init_TIMER2_B(void){
	TCCR2A |= (1<<WGM21);
	TIMSK2 |= (1<<OCIE2B);
	OCR2A = 156;
}


void _init_AVR(void){
	bDateChanged = 1;
	bPrintQuotes = 1;
	
	/******************* Pin Configuration ***********************/
	BACKLIGHT_PORT_DDR |= BACKLIGHT_PIN;				// PORTD.6 = output (backlight)
	
	BUTTON_PORT = BUTTON_A+BUTTON_B+BUTTON_C;		// pins 2,3,4 of BUTTON_PORT (portD) are pulled high
	
	
	init_EEPROM();
	init_CTRL_Data_fromEEPROM();
	init_ADC();
	init_LCD(1);			// Initialize the LCD while powering it up.
	init_TIMER0_B();
	init_TIMER2_B();
	sei();						// SEt Interrupts: let's start!
}

void init_CTRL_Data_fromEEPROM(void){
	
	time_date tEE;
	
	tEE.bDay = EEPROM_readByte(EEPROM_DAY_ADD);
	tEE.bMonth = EEPROM_readByte(EEPROM_MONTH_ADD);
	tEE.bYear = EEPROM_readByte(EEPROM_YEAR_ADD);
	tEE.bMin = EEPROM_readByte(EEPROM_MIN_ADD);
	tEE.bHour = EEPROM_readByte(EEPROM_HOUR_ADD);
	tEE.bSec = 0;
	tEE.wMilli = 0;
	
	if(isValidTimeDate(&tEE)){
		tTime = tEE;
	}else{
		tTime.bDay = 4;
		tTime.bMonth = 5;
		tTime.bYear = 12;
		tTime.wMilli=0;
		tTime.bSec=0;
		tTime.bMin=0;
		tTime.bHour=0;
		
		EEPROM_writeByte(EEPROM_DAY_ADD, tTime.bDay);
		EEPROM_writeByte(EEPROM_MONTH_ADD, tTime.bMonth);
		EEPROM_writeByte(EEPROM_YEAR_ADD, tTime.bYear);
		EEPROM_writeByte(EEPROM_MIN_ADD, tTime.bMin);
		EEPROM_writeByte(EEPROM_HOUR_ADD, tTime.bHour);
	}
	
	byte todayLogsTemp;
	word daysLoggedTemp;
	long lastIndexTemp;
	
	todayLogsTemp = EEPROM_readByte(EEPROM_TODAY_LOGS_ADD);
	EEPROM_readData(EEPROM_LOGGED_DAYS_ADD, (byte*)&daysLoggedTemp, sizeof(word));
	EEPROM_readData(EEPROM_LAST_INDEX_ADD, (byte*)&lastIndexTemp, sizeof(long));
	
	if(( todayLogsTemp < 0 )||( todayLogsTemp > NUMBER_OF_LOGS_PER_DAY )){
		bTodayLogs = 0;
		EEPROM_writeByte(EEPROM_TODAY_LOGS_ADD, bTodayLogs);
	}else{
		bTodayLogs = todayLogsTemp;
	}
	
	if(( daysLoggedTemp < 0 )){
		wLoggedDays = 0;
		EEPROM_writeData(EEPROM_LOGGED_DAYS_ADD, (byte*)&wLoggedDays, sizeof(word));
	}else{
		wLoggedDays = daysLoggedTemp;
	}
	
	if(( lastIndexTemp < EEPROM_LAST_INDEX_ADD+4 )||( lastIndexTemp >= EEPROM_SIZE_B )){
		#ifdef TESTING
			lLastIndex = EEPROM_LAST_INDEX_ADD+5;
		#else
			lLastIndex = EEPROM_LAST_INDEX_ADD+4;
		#endif
		
		EEPROM_writeData(EEPROM_LAST_INDEX_ADD, (byte*)&lLastIndex, sizeof(long));
	}else{
		lLastIndex = lastIndexTemp;
	}
	
	return;
}

float getTemperature(){
	float temp;
	float fVadc1;
	
	fVadc1 = ADC * VREF/1024;
	temp = fVadc1 / TEMP_SENSOR_GAIN;
	
	return temp;
}

float getHumidity(float temperature){
	float fVadc0;
	float fRH;
	float fRH_comp;
	
	fVadc0 = ADC * VREF/1024;
	fRH = (fVadc0/VREF - 0.16) / 0.0062;					// Formulas given by HIH-4030 datasheet	
	fRH_comp = fRH/(1.0546-0.00216*temperature);			//
	
	return fRH_comp;
}

void refreshQuote(){
	if(bDateChanged){
		bDateChanged=0;
		sprintf(str, "%02d/%02d/%02d,", tTime.bDay, tTime.bMonth, tTime.bYear);
		LCDWriteStringXY(0,0,str);
	}
	if(bTimeChanged){
		bTimeChanged=0;
		sprintf(str, "%02d", tTime.bHour);
		LCDWriteStringXY(CLOCK_CURSOR_POSITION, 0, str);
		sprintf(str, "%02d", tTime.bMin);
		LCDWriteStringXY(CLOCK_CURSOR_POSITION+3, 0, str);
	}
	if(bTempChanged){
		bTempChanged=0;
		sprintf(str, "%04.1f", fTemperature);		// float printed with 4 digits (dot included), 1 of which is decimal, zero padded
		LCDWriteStringXY(TEMP_CURSOR_POSITION,1, str);
		LCDByte(0b11011111, 1);
		LCDWriteStringXY(TEMP_CURSOR_POSITION+5, 1, "C,");
		
	}
	if(bHumChanged){
		bHumChanged=0;
		float hundred = 99.95;
		LCDWriteStringXY(HUM_CURSOR_POSITION-3, 1, "RH=");
		if(fHumidity<hundred){
			sprintf(str, "%04.1f", fHumidity);
		}else{
			sprintf(str, " %3.0f", fHumidity);
		}
		LCDWriteStringXY(HUM_CURSOR_POSITION, 1, str);
		LCDWriteString("%");
	}
}

void dataLog(time_date *time, void * humidity, void * temperature){
	START_ADC();
	
}


void vConfirmState(void){
	
	switch(bBtn){
		case NO_BTN:
			if(bPrintQuotes){ 
				LCDWriteStringXY(0,0, "Confermi? Si/No");
				LCD_CURSOR_LEFT_N(5);
				bPrintQuotes=0;
				bSelection=0;
			}
			if(bSelectionChanged){
				if(bSelection==0){		// Utilizzo bSelection per mantenere la scelta Si/No
					LCD_SET_CURSOR_POSITION(10);
				}else{
					LCD_SET_CURSOR_POSITION(13);
				}				
				bSelectionChanged=0;
			}
							
			break;
						
		case BTN_C:
			bSelection = (bSelection == 0)?1:0;
			bSelectionChanged=1;
			bBtn=NO_BTN;
			break;
						
		case BTN_C_LONG:
			if(bSelection){
				bState = STATE_MENU;
				bSelection=0;
			}
			else{				// Confermo la modifica
				switch(bState){
					case STATE_EDIT_DATE_CONFIRM:
						tTime.bDay = tTimeEditing.bDay;
						tTime.bMonth = tTimeEditing.bMonth;
						tTime.bYear = tTimeEditing.bYear;
						break;
						
					case STATE_EDIT_TIME_CONFIRM:
						tTime.bSec = tTimeEditing.bSec;
						tTime.bMin = tTimeEditing.bMin;
						tTime.bHour = tTimeEditing.bHour;
						break;
						
					case STATE_EDIT_HUM_ON_TH_CONFIRM:
						bHumOnThreshold = bHumOnThresholdEditing;
						break;
					
					case STATE_EDIT_HUM_AL_TH_CONFIRM:
						bHumAlarmThreshold = bHumAlarmThresholdEditing;
						break;
					
					default: break;
				}				
				bState = STATE_MENU;
				LCD_RESET();
			}
			bBtn = NO_BTN;
			bPrintQuotes=1;
			break;
		default: break;
	}
}

uint8_t isValidTimeDate(volatile time_date * time){
	
	if(( time->bDay > 31 )||( time->bDay == 0 )) return 0;
	if((time->bMonth > 12)||(time->bMonth == 0)) return 0;
	if(time->bYear > 99) return 0;
	if(time->bMin > 59) return 0;
	if(time->bHour > 23) return 0;
	
	return 1;
}

uint8_t isTimeToSample(volatile time_date *time){
	if(((time->bHour*60 + time->bMin) % MINS_UNTIL_LOG)==0) return 1;
	return 0;
}

/*uint8_t updateEEPROM_TimeDate(volatile time_date * time){
	byte error;
	error = EEPROM_writeByte(EEPROM_DAY_ADD, time->bDay);
	error |= EEPROM_writeByte(EEPROM_MONTH_ADD, time->bMonth);
	error |= EEPROM_writeByte(EEPROM_YEAR_ADD, time->bYear);
	error |= EEPROM_writeByte(EEPROM_MIN_ADD, time->bMin);
	error |= EEPROM_writeByte(EEPROM_HOUR_ADD, time->bHour);
	
	if(error) return 1;
	return 0;
}*/

uint8_t isLeapYear(byte year){
	if(year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) return 1;
	return 0;
}

uint8_t checkDay(volatile time_date *time, volatile byte *days){	
	/* se bDay supera il valore massimo del corrispondente bMonth,	 *
	 * esso viene portato al massimo valore consentito.				 */
	
	if(time->bDay >= days[time->bMonth-1]){
		if((time->bMonth == 2)&&(!isLeapYear(time->bYear)))
			time->bDay = days[time->bMonth-1]-1;	// Caso Febbraio, anno non bisestile: bDays=28!
		else
			time->bDay = days[time->bMonth-1];
		
		return 1;
	}
	return 0;
}

void toggleTimeColon( void ){
	if(bTimeCommaState){
		LCDWriteStringXY(CLOCK_CURSOR_POSITION+2, 0, ":");
		bTimeCommaState=0;
	}else{
		LCDWriteStringXY(CLOCK_CURSOR_POSITION+2, 0, " ");
		bTimeCommaState=1;
	}
}

void printIdleLCD( void ){
	LCDClear();
	
	bDateChanged = 1;
	bTimeChanged = 1;
	bHumChanged = 1;
	bTempChanged = 1;
	
	refreshQuote();
	
	//sprintf(str, "%02d/%02d/%02d", tTime.bDay, tTime.bMonth, tTime.bYear);
	//LCDWriteStringXY(0,0,str);
	//
	//LCDWriteStringXY(CLOCK_CURSOR_POSITION,0,"00:00");
	//LCDWriteStringXY(TEMP_CURSOR_POSITION,1,"00.0");
	//LCDByte(0b11011111, 1);		// Scrive il carattere °: dalla tabella 4 del datasheet HD44780.pdf vediamo che il carattere ? 11011111;
								//// lo mandiamo come byte (LCDByte()) sapendo che dobbiamo mettere RS a 1 (dato!)
	//LCDWriteStringXY(TEMP_CURSOR_POSITION+5, 1, "C,");
	//LCDWriteStringXY(HUM_CURSOR_POSITION-3, 1, "RH=88.8%");
	
}


int _round(double x){
	if((x-((int)x))>0.5) return ((int)x)+1;
	else return (int)x;
}



// PAx --> Offset del pin x all'interno del registro PINA
// DDxn = 1 --> Pxn = output pin, DDxn = 0 --> Pxn = input pin
// 
//

