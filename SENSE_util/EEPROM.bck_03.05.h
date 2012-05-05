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

#define EEPROM_SIZE_B			131072UL
#define EEPROM_EXTENDED_SIZE	1
	
#define EEPROM_PAGESIZE 128		///< EEPROM page size (see corresponding datasheet)

#define EEPROM_PAGE_NUMBER	EEPROM_SIZE_B / EEPROM_PAGESIZE


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
uint8_t EEPROM_open (void);



/****************************************************************
 Public Function: EEPROM_readByte

 Purpose: Read a byte from the EEPROM at the specified address.

 Input Parameter:
 	- uint16_t			Device address
 	- uint8_t		Type of ACK (ack or Nack)

 Return Value: uint8_t
	- Byte read at ADDRESS position.
	  
*****************************************************************/
uint8_t EEPROM_readByte( uint32_t address );
uint8_t EEPROM_writeByte( uint32_t address, uint8_t data);
uint8_t EEPROM_writeData( uint32_t address, uint8_t * bpData, uint8_t length);
uint8_t EEPROM_readData( uint32_t address, uint8_t * bpData, uint8_t lenght);
uint8_t * EEPROM_readPage( uint16_t pageNumber );
uint8_t EEPROM_writePage( uint32_t pageNumber, uint8_t * data);
uint32_t EEPROM_sequentialRead( uint32_t address, uint32_t numOfBytes, uint8_t * dest);
uint8_t EEPROM_sequentialWrite( uint32_t address, uint32_t numOfBytes, uint8_t * data);
uint8_t EEPROM_erase( uint32_t dimension );

#endif // EEPROM_H_