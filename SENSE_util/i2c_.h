/**
 * \file i2c.h
 * \brief I2C communication lib, header file.
 *
 * \date 16/08/2011
 * \author Stefano Cillo <cillino.25@gmail.com>
 * \version v0.1
 * 
 */

#ifndef I2C_H_
#define I2C_H_

#define	 START				0x08
#define  REPEAT_START		0x10

// Master Transmit Mode
#define  MT_SLA_ACK			0x18
#define  MT_SLA_NACK		0x20
#define  MT_DATA_ACK		0x28
#define  MT_DATA_NACK		0x30
// Master Receive Mode
#define  MR_SLA_ACK			0x40
#define  MR_SLA_NACK		0x48
#define  MR_DATA_ACK		0x50
#define  MR_DATA_NACK		0x58

#define  ARB_LOST			0x38

#define  ERROR_CODE			0x7e

#define TW_STATUS	TWSR
#define TW_CONTROL	TWCR

#define RX_ACK		NACK  // i chip EEPROM del tipo at24c rispondono con dei NACK

unsigned char i2c_start(void);
unsigned char i2c_start_address(unsigned char);
unsigned char i2c_repeatStart(void);
unsigned char i2c_sendAddress_ACK(unsigned char);
unsigned char i2c_sendAddress_NACK(unsigned char);
unsigned char i2c_sendData_ACK(unsigned char);
unsigned char i2c_sendData_NACK(unsigned char);
unsigned char i2c_receiveData_ACK(void);
unsigned char i2c_receiveData_NACK(void);
//unsigned char i2c_receiveData(void);
void i2c_stop(void);

#endif  // I2C_H_