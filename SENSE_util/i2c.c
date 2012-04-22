/**
 * \file i2c.c
 * \brief I2C communication library, main file.
 *
 * \date 16/08/2011
 * \author Stefano Cillo <cillino.25@gmail.com>
 * \version v0.1
 * 
 */

#include "i2c.h"


unsigned char i2c_start(void)
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);		//Send START condition
	
	while (!(TWCR & (1<<TWINT)));   		//Wait for TWINT flag set. This indicates that the
											//START condition has been transmitted
	
	if (((TWSR & 0xF8) == START) || ((TWSR & 0xF8) == REPEAT_START))
		return 0;
	else
		return 1;
}

unsigned char i2c_start_address(unsigned char address)
{
	uint8_t twst;
	
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTA);		// Prepare and send START condition
	
	while (!(TWCR & (1<<TWINT)));   		//Wait for TWINT flag set. This indicates that the
											//START condition has been transmitted
	twst = TWSR & 0xF8;
	
	if ((twst != START) && (twst != REPEAT_START))
		return 2;
	
	TWDR = address;
	TWCR = (1<<TWINT)|(1<<TWEN);
	
	while(!(TWCR & (1<<TWINT)));
	
	//twst = TW_STATUS & 0xF8;
	twst = TWSR;
	
	if ( (twst != MT_SLA_ACK) && (twst != MR_SLA_NACK) )
		return 1;
	
	return 0;
}

unsigned char i2c_repeatStart(void)
{
 
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); 		//Send START condition
    
	while (!(TWCR & (1<<TWINT)));
	
	if ((TWSR & 0xF8) == REPEAT_START)
		return(0);
	else
		return(1);
}


unsigned char i2c_sendAddress_ACK(unsigned char address)	// lsb of ADDRESS is R/W bit || R/W =1 --> Read operation!
{
	unsigned char STATUS;
   
	if((address & 0x01) == 0)		// == EEPROM_W
		STATUS = MT_SLA_ACK;		// Write (master transmit)
	else
		STATUS = MR_SLA_ACK; 		// Read (master receive)
   
	TWDR = address;						//Load SLA_W/SLA_R into TWDR Register. Clear TWINT bit
	TWCR = (1<<TWINT)|(1<<TWEN);
	TWCR |= (1<<TWEA);		//in TWCR to start transmission of address

	while (!(TWCR & (1<<TWINT)));		//Wait for TWINT flag set. This indicates that the
										//SLA+W has been transmitted, and ACK/NACK has been received.
	//return TWSR;
	if ((TWSR & 0xF8) == STATUS)
		return(0);
	else 
		return(1);
}

unsigned char i2c_sendAddress_NACK(unsigned char address)	// lsb of ADDRESS is R/W bit || R/W =1 --> Read operation!
{
	unsigned char STATUS;
   
	if((address & 0x01) == 0)		// == EEPROM_W
		STATUS = MT_SLA_NACK;		// Write (master transmit)
	else
		STATUS = MR_SLA_NACK; 		// Read (master receive)
   
	TWDR = address;					//Load SLA_W/SLA_R into TWDR Register. Clear TWINT bit
	TWCR = (1<<TWINT)|(1<<TWEN);		//in TWCR to start transmission of address

	while (!(TWCR & (1<<TWINT)));		//Wait for TWINT flag set. This indicates that the
										//SLA+W has been transmitted, and ACK/NACK has been received.
	//return TWSR;
	if ((TWSR & 0xF8) == STATUS)
		return(0);
	else 
		return(1);
}

unsigned char i2c_sendData_ACK(unsigned char data)
{
	TWDR = data; 
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	
	while (!(TWCR & (1<<TWINT)));
	
	if (((TWSR & 0xF8) == MT_DATA_ACK)||((TWSR & 0xF8) == MT_DATA_ACK))
		return(0);
	else
		return(1);
}
unsigned char i2c_sendData_NACK(unsigned char data)
{
	TWDR = data; 
	TWCR = (1<<TWINT)|(1<<TWEN);
	
	while (!(TWCR & (1<<TWINT)));
	
	if ((TWSR & 0xF8) == MT_DATA_NACK)
		return(0);
	else
		return(1);
}



unsigned char i2c_receiveData_ACK(void)
{
	unsigned char data;
  
	TWCR = (1<<TWEA)|(1<<TWINT)|(1<<TWEN);	// Enable ACKnowledge
  
	while (!(TWCR & (1<<TWINT)));
	
	if ((TWSR & 0xF8) != MR_DATA_ACK)
		return(ERROR_CODE);
  
	data = TWDR;
	return(data);
}


unsigned char i2c_receiveData_NACK(void)
{
	unsigned char data;
  
	TWCR = (1<<TWINT)|(1<<TWEN);
  
	while (!(TWCR & (1<<TWINT)));
	
	if ((TWSR & 0xF8) != MR_DATA_NACK)
		return(ERROR_CODE);
  
	data = TWDR;
	return(data);
}
/*
unsigned char i2c_receiveData(void)
{	
	#if RX_ACK==ACK
		TWCR = (1<<TWINT)|(1<<TWEN);
	#elif RX_ACK==NACK
		TWCR = (1<<TWEA)|(1<<TWINT)|(1<<TWEN);	// Enable ACKnowledge
	#else
	#endif
		
	while (!(TWCR & (1<<TWINT)));
	
	if (((TWSR & 0xF8) != MR_DATA_NACK)||((TWSR & 0xF8) != MR_DATA_ACK))
		return(ERROR_CODE);
	
	return(TWDR);
}*/
 	
void i2c_stop(void)
{
	TWCR =  (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);	  //Transmit STOP condition
	while(TWCR & (1<<TWSTO));
}  
