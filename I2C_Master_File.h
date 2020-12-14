
/*
 * PIC18F4550 I2C Master Header File
 * http://www.electronicwings.com
 */ 

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef I2C_Master_H
#define	I2C_Master_H

#include <xc.h> // include processor files - each processor file is guarded.  

#define F_OSC 8000000                       /* define F_OSC for finding bit rate 
                                               here oscillator frequency is 8 MHz so define as 800000 */
#define I2C_CLOCK 100000                    /* I2C clock frequency is 100 kHz*/
#define BITRATE ((F_OSC/(4*I2C_CLOCK))-1)  /* find bit rate to assign this value to SSPADD register*/

void I2C_Ready();                           /* check and wait weather I2C is ready or not */
void I2C_Init();                            /* initialize I2C register configuration*/
char I2C_Start(char);                       /* send start pulse along with slave device address */
void I2C_Start_Wait(char);                  /* send start pulse along with slave device address 
                                             * and wait if not work */
char I2C_Repeated_Start(char);              /* send repeated start pulse along with device write address*/
char I2C_Stop();                            /* send stop pulse*/
char I2C_Write(unsigned char);              /* transmit/write data/address to slave device */
void I2C_Ack();                             /* send acknowledge from master for another read */
void I2C_Nack();                            /* send negative acknowledge from master for stop reading */
char I2C_Read(char flag);                   /* read data from slave devices*/

char I2C_Read(char flag)                 /*read data from location and 
                                         *send ack or nack depending upon parameter*/
{
        char buffer;
        RCEN = 1;                         /*enable receive*/
        while(!SSPSTATbits.BF);         /*wait for buffer full flag 
                                         *which is set when complete byte received*/
        buffer = SSPBUF;                  /*copy SSPBUF to buffer*/
       /*send acknowledgment or negative acknowledgment */        
        if(flag==0)
            I2C_Ack();
        else
            I2C_Nack();
        I2C_Ready();
        return(buffer);
}

void I2C_Init()
{
    TRISB0=1;							/*set up I2C lines by setting as input*/
	TRISB1=1;
	SSPSTAT=80;                         /*slew rate disabled, other bits are cleared*/
    SSPCON1=0x28;						/*enable SSP port for I2C Master mode,
                                         *clock = FOSC / (4 * (SSPADD+1))*/ 
	SSPCON2=0;
    SSPADD=BITRATE;                    /*clock 100 kHz*/  
    SSPIE=1;                            /*enable SSPIF interrupt*/
    SSPIF=0;
}


void I2C_Ready()
{
    while(!SSPIF);                  /* Wait for operation complete */
    SSPIF=0;                        /*clear SSPIF interrupt flag*/
}

void I2C_Start_Wait(char slave_write_address)
{
  while(1)
  {   
    SSPCON2bits.SEN=1;              /*send start pulse*/
    while(SSPCON2bits.SEN);         /*wait for completion of start pulse*/
    SSPIF=0;
	if(!SSPSTATbits.S)              /*wait till start bit is detected*/    
        continue;
    I2C_Write(slave_write_address); /*write slave device address with write to communicate*/
    if(ACKSTAT)
    {
        I2C_Stop();
        continue;
    }    
    break;        
  }
}

char I2C_Start(char slave_write_address)
{   
    SSPCON2bits.SEN=1;              /*send start pulse*/
    while(SSPCON2bits.SEN);         /*wait for completion of start */
	SSPIF=0;
    if(!SSPSTATbits.S)              /*wait till start bit is detected*/
    return 0;                       /*start failed*/    
    return (I2C_Write(slave_write_address));     /*write slave device address with write to communicate*/
     
}

char I2C_Repeated_Start(char slave_read_address)
{
    RSEN = 1;                         /*send repeated start pulse*/
    while(RSEN);                    /*wait for completion of repeated start pulse*/
    SSPIF = 0;
	if(!SSPSTATbits.S)             /*wait till start bit is detected*/
    return 0;                       /*start failed*/    
    I2C_Write(slave_read_address);      /*write slave device address with read to communicate*/
    if (ACKSTAT)
     return 1;
    else
     return 2;
}

char I2C_Stop()
{
    PEN = 1;                          /*stop communication*/
    while(PEN);                     /*wait for end of stop pulse*/
    if(!SSPSTATbits.P);             /*wait till stop bit is detected*/
    return 0;                       /*stop failed*/        
}

char I2C_Write(unsigned char data)
{
      SSPBUF = data;                  /*write data to SSPBUF*/
      I2C_Ready();
      if (ACKSTAT)                  /*check for acknowledge bit*/
        return 1;
      else
        return 2;
}

void I2C_Ack()
{
    ACKDT=0;					/*acknowledge data 1:NACK,0:ACK */
	ACKEN=1;					/*enable ACK to send*/	          
    while(ACKEN);
}

void I2C_Nack()
{
    ACKDT=1;					/*acknowledge data 1:NACK,0:ACK*/
	ACKEN=1;					/*enable ACK to send*/	          
    while(ACKEN);
}

#endif	/* I2C_Master_H */

