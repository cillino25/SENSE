/**
 * \file EEPROM.h
 * \brief EEPROM chip communication interface, header.
 *
 * \date 08/11/2011
 * \author Stefano Cillo <cillino.25@gmail.com>
 * \version v0.1
 * 
 * 
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#define MICROCHIP	1
#define ATMEL		2

#define EEPROM_BRAND	MICROCHIP

/// EEPROM size in KByte
#define EEPROM_SIZE_KB	512

#define EEPROM_EXTENDED_SIZE	1
	
/// EEPROM page size (see corresponding datasheet)
#define EEPROM_PAGESIZE 128


/// Slave address
#define SLA			0xa0		// EEPROM will be used with A1=A0=0 (GND)

/// Page number (only for EEPROM chips with memory size > 512 KByte)
#define PAGE_0		0x0

#if EEPROM_BRAND==MICROCHIP
  #define PAGE_1		8			// 0b00000100, page bit for 24AA1025
#elif EEPROM_BRAND==ATMEL
  #define PAGE_1		1			// 0b00000001, page bit for AT24C1024B
#endif

#define  W			0x0
#define  R			0x1



/***************** EEPROM MiddleWare *****************/
#define AT24_RR_ACK_TYPE		0		// ACK type (ack or Nack) for RANDOM READ type of operation
										// for AT24XX-like eeproms: at24cXX terminates RANDOM READ with a NACK!!
										// but for another chip could be different

#define AT24_BW_ACK_TYPE		1			// at24c uses ACK for BYTE WRITE operations




/************************************************************************************/
/************************************************************************************/

/*************************************************************
 Public Function: EEPROM_Open

 Purpose: Initialise the TWI interface for using the EEPROM.
		Set TWI bitrate: if it's too high, it will return the error.

 Input Parameter:
 	- uint16_t	TWI_Bitrate (Hz)

 Return Value: uint8_t
 	- FALSE:	Bitrate too high
 	- TRUE:		Bitrate OK

*************************************************************/
unsigned char EEPROM_open (void);



/****************************************************************
 Public Function: EEPROM_readByte

 Purpose: Read a byte from the EEPROM at the specified address.

 Input Parameter:
 	- uint16_t			Device address
 	- unsigned char		Type of ACK (ack or Nack)

 Return Value: uint8_t
	- Byte read at ADDRESS position.
	  
*****************************************************************/
unsigned char EEPROM_readByte( uint32_t address, unsigned char ACK );


unsigned char EEPROM_writeByte( uint32_t address, uint8_t data, unsigned char ACK );
unsigned char * EEPROM_readPage( unsigned int pageNumber );
unsigned char EEPROM_writePage( unsigned int pageNumber, unsigned char * data );
unsigned char EEPROM_sequentialRead( uint16_t address, uint16_t numOfBytes, unsigned char * dest, unsigned char ACK );
unsigned char EEPROM_sequentialWrite( uint16_t address, uint16_t numOfBytes, unsigned char * data );
unsigned char EEPROM_erase( void );

#endif // EEPROM_H_