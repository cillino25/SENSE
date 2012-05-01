/**
 * \file EEPROM.c
 * \brief EEPROM chip communication interface, main file.
 *
 * \date 08/11/2011
 * \author Stefano Cillo <cillino.25@gmail.com>
 * \version v0.1
 * 
 * 
 */

#include "EEPROM.h"
#ifndef I2C_H_
  #include "i2c.h"
#endif


uint8_t EEPROM_open(void){
	
	TWSR = 0;
	TWBR = ((F_CPU / TWI_BITRATE) - 16) / 2;
	if (TWBR < 11)
		return 0;
	return 1;
}


unsigned char EEPROM_readByte(uint32_t address, unsigned char ACK){
	
	unsigned char errorStatus, i, data, page, highAddress, lowAddress, slaveAddress;
	page = (address>>16);
	highAddress=(address>>8);
	lowAddress=(address);
	
	slaveAddress=SLA;
	if(page!=0){	// addressing a byte inside first page
		slaveAddress += PAGE_1;
	}
		
	if((i2c_start_address(slaveAddress+W))!=0){
		i2c_stop();
		return 11; // returns 125 if encounters an error
	}
	errorStatus |= i2c_sendData_ACK(highAddress);
	errorStatus |= i2c_sendData_ACK(lowAddress);
	
	if(errorStatus==1){
		i2c_stop();
		return 12;
	}
	//errorStatus |= i2c_repeatStart();
	//errorStatus |= i2c_sendAddress_ACK(slaveAddress+R);
	
	i2c_start_address(slaveAddress+R);
	
	if(errorStatus==1){
		i2c_stop();
		return 13;
	}
	
	if(ACK)
		data = i2c_receiveData_ACK();
	else
		data = i2c_receiveData_NACK();
	
	if(data == ERROR_CODE){
		i2c_stop();
	   	return 14;
	}
	i2c_stop();
	return data;
}



uint8_t EEPROM_writeByte(uint32_t address, uint8_t data, uint8_t ACK){
	
	uint8_t errorStatus, page, highAddress, lowAddress, slaveAddress;
	page = address >> 16;
	highAddress=address>>8;
	lowAddress=address;
	char str[10];
	
	slaveAddress = SLA;
	#ifdef EEPROM_EXTENDED_SIZE
	  if(page!=0){				// addressing a byte inside first page
		  slaveAddress += PAGE_1;
	  }
	#endif
	
	if((i2c_start_address(slaveAddress+W))!=0){
		i2c_stop();
		return ERROR_CODE-1; // returns 125 if encounters an error
	}
	errorStatus |= i2c_sendData_ACK(highAddress);
	errorStatus |= i2c_sendData_ACK(lowAddress);
	
	if(errorStatus == 1){
		i2c_stop();
		return(ERROR_CODE-1);
	}
	if(ACK)
		errorStatus = i2c_sendData_ACK(data);
	else
		errorStatus = i2c_sendData_NACK(data);
	
	if(errorStatus == 1){
		i2c_stop();
	   	return(1);
	}
	
	i2c_stop();
	_delay_ms(5);
	if(page!=0) return 125;
	return(0);
}


uint8_t EEPROM_writeData( uint32_t address, uint8_t * bpData, uint8_t length, uint8_t ACK){
	uint8_t i;
	
	for(i=0; i<length; i++)
		EEPROM_writeByte(address+i, *bpData++, ACK);
	
	return 0;
}



uint8_t EEPROM_readData( uint32_t address, uint8_t * bpData, uint8_t length, uint8_t ACK ){	// occhio: avrgcc salva i float in little endian
	uint8_t i;
	
	for(i=0; i<length; i++){
		bpData[i] = EEPROM_readByte(address+i, ACK);
	}
	
	return length;
}


uint8_t * EEPROM_readPage( unsigned int pageNumber ){
	
	uint8_t *values;
	unsigned int pageAddress, numOfRead;
	
	pageAddress = pageNumber * EEPROM_PAGESIZE;	// 128 = page size
	
	numOfRead = EEPROM_sequentialRead(pageAddress, EEPROM_PAGESIZE, values, AT24_RR_ACK_TYPE);
	if(values==NULL){ return NULL; }
	else return values;
 } 
  

uint8_t EEPROM_writePage( unsigned int pageNumber, uint8_t * data ){
	
	uint8_t highAddress, lowAddress, errorStatus, i;
	unsigned int pageBaseAddress;
	
	pageBaseAddress = pageNumber * EEPROM_PAGESIZE;
	
	if((i2c_start_address(SLA+W))!=0){
		i2c_stop();
		return ERROR_CODE-1; // returns 125 if encounters an error
	}
	
	errorStatus |= i2c_sendData_ACK(highAddress);
	errorStatus |= i2c_sendData_ACK(lowAddress);
	
	
	
	for(i=0;i<EEPROM_PAGESIZE;i++){
		
		
		
		if(errorStatus){
			i2c_stop();
			return 1;
		}
	}
	
	
	return(0);  
} 


uint8_t EEPROM_sequentialRead(uint16_t address, uint16_t numOfBytes, uint8_t * dest, uint8_t ACK){
	
	uint8_t errorStatus, i;
	uint8_t highAddress=(address>>8), lowAddress=(address);
	
	errorStatus = i2c_start();
	_delay_ms(3);
	errorStatus |= i2c_sendAddress_ACK(SLA+W);
	errorStatus |= i2c_sendData_ACK(highAddress);
	errorStatus |= i2c_sendData_ACK(lowAddress);
	errorStatus |= i2c_repeatStart();
	errorStatus |= i2c_sendAddress_ACK(SLA+R);
	
	if(errorStatus){
		i2c_stop();
		return errorStatus;
	}
	
	for(i=0;i<numOfBytes;i++){
		if(ACK)
			*dest = i2c_receiveData_ACK();
		else
			*dest = i2c_receiveData_NACK();
		
		if(*(++dest) == ERROR_CODE){
			i2c_stop();
	   		return(ERROR_CODE);
		}
	}	
	i2c_stop();
	
	return numOfBytes;
}


uint8_t EEPROM_sequentialWrite(uint16_t address, uint16_t numOfBytes, uint8_t * data){
	uint8_t errorStatus, i;
	
	for(i=0;i<numOfBytes;i++){
		errorStatus=EEPROM_writeByte(address,data[i], AT24_BW_ACK_TYPE);
		if(errorStatus){
			i2c_stop();
			return 1;
		}
	}
	return 0;
}


uint8_t EEPROM_erase(void){
  
	uint8_t errorStatus;
	unsigned int i;
	
	errorStatus = i2c_start_address(SLA+W);
	errorStatus |= i2c_sendData_ACK(0x00);
	errorStatus |= i2c_sendData_ACK(0x00);
	if(errorStatus == 1){
		i2c_stop();
		return(1);
	} 
	
	for(i=0;i<0x8000;i++){
		errorStatus = i2c_sendData_ACK(0xff);
		if(errorStatus == 1){
			i2c_stop();
			return(1);
		}
	}
	
	i2c_stop();
	
	return(0);
}
