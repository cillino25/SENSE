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

/******************** WARNING!!! ********************

	after byte writing we have to wait a time before we can read a consistent data. D4 is error!!

******************************************************/

uint8_t EEPROM_readByte(uint32_t address){
	
	uint8_t errorStatus, i, data, page, highAddress, lowAddress, slaveAddress;
	page = (address>>16);
	highAddress=(address>>8);
	lowAddress=(address);
	
	slaveAddress=SLA;
	if(page!=0){	// addressing a byte inside first page
		slaveAddress += PAGE_1;
	}
	if((i2c_start_address(slaveAddress+W))!=0){
		i2c_stop();
		return ERROR_CODE-1; // returns 125 if encounters an error
	}
	
	
	errorStatus |= i2c_sendData_ACK(highAddress);
	errorStatus |= i2c_sendData_ACK(lowAddress);
	
	errorStatus |= i2c_repeatStart();
	errorStatus |= i2c_sendAddress_ACK(slaveAddress+R);
	
	
	if(errorStatus==1){
		i2c_stop();
		return ERROR_CODE+1;
	}
	
	
	//if(ACK)
	//	data = i2c_receiveData_ACK();
	//else
		data = i2c_receiveData_NACK();
	
	if(data == ERROR_CODE){
		i2c_stop();
	   	return ERROR_CODE-1;
	}
	i2c_stop();
	return data;
}


uint8_t EEPROM_writeByte(uint32_t address, uint8_t src){
	
	uint8_t errorStatus, page, highAddress, lowAddress, slaveAddress;
	page = address >> 16;
	highAddress=address>>8;
	lowAddress=address;
	char str[10];
	//return page;
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
	
	errorStatus = i2c_sendData_ACK(src);
	
	if(errorStatus == 1){
		i2c_stop();
	   	return(1);
	}
	
	i2c_stop();
	_delay_ms(10);
	
	return(0);
}


uint8_t EEPROM_writeData( uint32_t address, uint8_t * bpData, uint8_t length ){
	uint8_t i;
	
	for(i=0; i<length; i++)
		EEPROM_writeByte(address+i, *bpData++);
	
	return 0;
}



uint8_t EEPROM_readData( uint32_t address, uint8_t * bpDest, uint8_t length ){	// occhio: avrgcc salva i float in little endian
	uint8_t i;
	
	for(i=0; i<length; i++){
		bpDest[i] = EEPROM_readByte(address+i);
	}
	
	return length;
}


uint8_t EEPROM_readPage( uint32_t pageNumber, uint8_t * dest ){
	uint8_t page, highAddress, lowAddress, slaveAddress, errorStatus, i;
	uint32_t actualAddress;
	
	actualAddress = pageNumber * EEPROM_PAGESIZE;
	page = (actualAddress>>16);
	highAddress = (actualAddress>>8);
	lowAddress = actualAddress;
	
	slaveAddress = SLA;
	
	#ifdef EEPROM_EXTENDED_SIZE
	  if(page!=0){				// addressing a byte inside first page
		  slaveAddress += PAGE_1;
	  }
	#endif
	
	if((i2c_start_address(slaveAddress+W))!=0){
		i2c_stop();
		return ERROR_CODE;
	}
	
	errorStatus |= i2c_sendData_ACK(highAddress);
	errorStatus |= i2c_sendData_ACK(lowAddress);
	errorStatus |= i2c_repeatStart();
	errorStatus |= i2c_sendAddress_ACK(slaveAddress+R);
	
	if(errorStatus) return ERROR_CODE;
	
	for(i=0;i<EEPROM_PAGESIZE-1;i++){
		*dest++ = i2c_receiveData_ACK();
	}
	*dest++ = i2c_receiveData_NACK();
	
	i2c_stop();
	return 0;
 } 
  

uint8_t EEPROM_writePage( uint32_t pageNumber, uint8_t * src){
	
	uint8_t page, highAddress, lowAddress, slaveAddress, errorStatus, i;
	uint32_t pageBaseAddress;
	
	pageBaseAddress = pageNumber * EEPROM_PAGESIZE;
	page = (pageBaseAddress>>16);
	highAddress = (pageBaseAddress>>8);
	lowAddress = pageBaseAddress;
	
	slaveAddress = SLA;
	
	#ifdef EEPROM_EXTENDED_SIZE
	  if(page!=0){				// addressing a byte inside first page
		  slaveAddress += PAGE_1;
	  }
	#endif
	
	if((i2c_start_address(slaveAddress+W))!=0){
		i2c_stop();
		return ERROR_CODE;
	}
	
	errorStatus |= i2c_sendData_ACK(highAddress);
	errorStatus |= i2c_sendData_ACK(lowAddress);
	
	if(errorStatus){
		i2c_stop();
		return ERROR_CODE;
	}	
	for(i=0;i<EEPROM_PAGESIZE;i++){
		
		errorStatus |= i2c_sendData_ACK(*src++);
		
		if(errorStatus){
			i2c_stop();
			return ERROR_CODE;
		}
	}
	
	i2c_stop();
	return(0);  
} 


uint8_t EEPROM_sequentialRead(uint32_t address, uint32_t numOfBytes, uint8_t * dest ){
	uint8_t page, highAddress, lowAddress, slaveAddress, errorStatus, i;
	uint32_t actualAddress;
	
	page = (address>>16);
	highAddress = (address>>8);
	lowAddress = address;
	
	slaveAddress = SLA;
	
	#ifdef EEPROM_EXTENDED_SIZE
	  if(page!=0){				// addressing a byte inside first page
		  slaveAddress += PAGE_1;
	  }
	#endif
	
	if((i2c_start_address(slaveAddress+W))!=0){
		i2c_stop();
		return ERROR_CODE;
	}
	
	errorStatus |= i2c_sendData_ACK(highAddress);
	errorStatus |= i2c_sendData_ACK(lowAddress);
	errorStatus |= i2c_repeatStart();
	errorStatus |= i2c_sendAddress_ACK(slaveAddress+R);
	//return slaveAddress;
	if(errorStatus) return ERROR_CODE;
	
	for(i=0;i<numOfBytes;i++){
	
		if( i==(numOfBytes-1) ){	// last byte to be read
			*dest++ = i2c_receiveData_NACK();		// last byte read has to terminate with a NACK
			i2c_stop();
			if(errorStatus) return ERROR_CODE;
			return 0;
			
		}else if(((address+i+1) == 0x10000) || ((address+i+1) == 0x20000)){
		// here we still have bytes to read but we have also to reinitialize the eeprom to the correct address
			
			*dest++ = i2c_receiveData_NACK();		// last byte read has to terminate with a NACK
			i2c_stop();
			_delay_ms(10);
			slaveAddress = SLA;
			actualAddress = (address + i +1) % 0x20000;
			page = (actualAddress>>16);
			highAddress = (actualAddress>>8);
			lowAddress = actualAddress;
			#ifdef EEPROM_EXTENDED_SIZE
				if(page!=0){				// addressing a byte inside first page
					slaveAddress += PAGE_1;
				}
			#endif
			
			if((i2c_start_address(slaveAddress+W))!=0){
				i2c_stop();
				return ERROR_CODE;
			}
			
			errorStatus |= i2c_sendData_ACK(highAddress);
			errorStatus |= i2c_sendData_ACK(lowAddress);
			errorStatus |= i2c_repeatStart();
			errorStatus |= i2c_sendAddress_ACK(slaveAddress+R);
			
			if(errorStatus) return ERROR_CODE;
		}else{
			*dest++ = i2c_receiveData_ACK();
		}
		_delay_ms(10);
	}
	
	i2c_stop();
	return 0;
}


uint8_t EEPROM_sequentialWrite(uint32_t address, uint32_t numOfBytes, uint8_t * src ){
		
	uint8_t page, highAddress, lowAddress, slaveAddress, errorStatus, i;
	uint32_t actualAddress;
	
	page = (address>>16);
	highAddress = (address>>8);
	lowAddress = address;
	
	slaveAddress = SLA;
	
	#ifdef EEPROM_EXTENDED_SIZE
	  if(page!=0){				// addressing a byte inside first page
		  slaveAddress += PAGE_1;
	  }
	#endif
	
	if((i2c_start_address(slaveAddress+W))!=0){
		i2c_stop();
		return ERROR_CODE;
	}
	
	errorStatus |= i2c_sendData_ACK(highAddress);
	errorStatus |= i2c_sendData_ACK(lowAddress);
	
	if(errorStatus) return ERROR_CODE;
	
	for(i=0;i<numOfBytes;){
		
		errorStatus |= i2c_sendData_ACK(*src++);
		
		i++;
		if((((address+i) == 0x10000) || (address+i) == 0x20000)&&(i<numOfBytes)){
			i2c_stop();
			_delay_ms(10);
			slaveAddress = SLA;
			actualAddress = (address + i) % 0x20000;
			page = (actualAddress>>16);
			highAddress = (actualAddress>>8);
			lowAddress = actualAddress;
			#ifdef EEPROM_EXTENDED_SIZE
				if(page!=0){				// addressing a byte inside first page
					slaveAddress += PAGE_1;
				}
			#endif
	
			if((i2c_start_address(slaveAddress+W))!=0){
				i2c_stop();
				return ERROR_CODE;
			}
	
			errorStatus |= i2c_sendData_ACK(highAddress);
			errorStatus |= i2c_sendData_ACK(lowAddress);
			
			if(errorStatus) return ERROR_CODE;
		}
		
		if(errorStatus){
			i2c_stop();
			return ERROR_CODE;
		}
	}
	
	i2c_stop();
	return 0;
}


uint32_t EEPROM_erase(uint32_t sizeKbit){
	
	uint8_t highAddress, lowAddress, page, errorStatus;
	uint32_t i, j, sizeByte, numberOfPages, actualAddress, slaveAddress;
	
	sizeByte = sizeKbit << 7;
	numberOfPages = sizeByte / EEPROM_PAGESIZE;
	
	slaveAddress = SLA+W;
	
	for(i=0; i<numberOfPages; i++){
		actualAddress = i * EEPROM_PAGESIZE;
		
		page = (actualAddress>>16);
		highAddress = (actualAddress>>8);
		lowAddress = actualAddress;
		
		//#ifdef EEPROM_EXTENDED_SIZE
			if( page!=0 )	slaveAddress = SLA+W+PAGE_1;
		//#endif
		
		errorStatus = i2c_start_address(slaveAddress);
		errorStatus |= i2c_sendData_ACK(highAddress);
		errorStatus |= i2c_sendData_ACK(lowAddress);
		
		if(errorStatus){
			i2c_stop();
			return(ERROR_CODE);
		}
		
		for(j=0;j<EEPROM_PAGESIZE;j++){
			errorStatus |= i2c_sendData_ACK(0xFF);
			
			if(errorStatus){
				i2c_stop();
				return ERROR_CODE;
			}
		}
		i2c_stop();
		_delay_ms(5);
	}
	return 0;
}
