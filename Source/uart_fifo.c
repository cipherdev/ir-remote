/*
 * uart_fifo.c
 *
 *  Created on: Apr 18, 2014
 *      Author: Anh Huy
 *
 *This is a uart_fifo.c file of the FIFO UART Hardware demo using USCI on the MSP430G2553 microcontroller.
 *Set your baud rate in your terminal to 9600 8N1.
 *When using your TI MSP-430 LaunchPad you will need to cross the TXD and RXD jumpers.
 *
 */

#include <msp430g2553.h>
#include "uart_fifo.h"

#define LED BIT0
#define RXD BIT1
#define TXD BIT2

#define FIFO_SIZE 128

volatile unsigned char tx_char;				//This char is the most current char to go into the UART

volatile unsigned int rx_flag;				//Mailbox Flag for the rx_char.
volatile unsigned char rx_char;				//This char is the most current char to come out of the UART

volatile unsigned char tx_fifo[FIFO_SIZE];	//The array for the tx fifo
volatile unsigned char rx_fifo[FIFO_SIZE];  //The array for the rx fifo

volatile unsigned int tx_fifo_ptA;			//Theses pointers keep track where the UART and the Main program are in the Fifos
volatile unsigned int tx_fifo_ptB;
volatile unsigned int rx_fifo_ptA;
volatile unsigned int rx_fifo_ptB;

volatile unsigned int rx_fifo_full;
volatile unsigned int tx_fifo_full;

/*uart_init
* Sets up the UART interface via USCI
* INPUT: None
* RETURN: None
*/
void uart_init(void)
{
	P1SEL = RXD + TXD;					//Setup the I/O
	P1SEL2 = RXD + TXD;

    P1DIR |= LED; 						//P1.0 red LED. Toggle when char received.
    P1OUT |= LED; 						//LED off

	UCA0CTL1 |= UCSSEL_2; 				//SMCLK
										//8,000,000Hz, 9600Baud, UCBRx=52, UCBRSx=0, UCBRFx=1
	UCA0BR0 = 52;                  		//8MHz, OSC16, 9600
	UCA0BR1 = 0;                   	 	//((8MHz/9600)/16) = 52.08333
	UCA0MCTL = 0x10|UCOS16; 			//UCBRFx=1,UCBRSx=0, UCOS16=1
	UCA0CTL1 &= ~UCSWRST; 				//USCI state machine
	IE2 |= UCA0RXIE; 					//Enable USCI_A0 RX interrupt

	rx_flag = 0;						//Set rx_flag to 0

	tx_fifo_ptA = 0;					//Set the fifo pointers to 0
	tx_fifo_ptB = 0;
	rx_fifo_ptA = 0;
	rx_fifo_ptB = 0;

	tx_fifo_full = 0;
	rx_fifo_full = 0;

	return;
}

/*uart_getc
* Get a char from the UART. Waits till it gets one
* INPUT: None
* RETURN: Char from UART
*/
unsigned char uart_getc()					//Waits for a valid char from the UART
{
	unsigned char c;

	while (rx_flag == 0);		 			//Wait for rx_flag to be set

	c = rx_fifo[rx_fifo_ptA];				//Copy the fifo
	rx_fifo_ptA++;							//increase the fifo pointer

	if(rx_fifo_ptA == FIFO_SIZE)			//If pointer is equal to the max size roll it over
	{
		rx_fifo_ptA = 0;
	}

	if(rx_fifo_ptA == rx_fifo_ptB)			//if the pointers are the same we have no new data
	{
		rx_flag = 0;						//ACK rx_flag
	}
    return c;
}

/*uart_gets
* Get a string of known length from the UART. Strings terminate when enter is pressed or string buffer fills
* Will return when all the chars are received or a carriage return (\r) is received. Waits for the data.
* INPUT: Array pointer and length
* RETURN: None
*/
void uart_gets(char* Array, int length)
{
	unsigned int i = 0;

	while((i < length))					//Grab data till the array fills
	{
		Array[i] = uart_getc();
		if(Array[i] == '\r')				//If we receive a \r the master wants to end
		{
			for( ; i < length ; i++)		//fill the rest of the string with \0 nul. Overwrites the \r with \0
			{
				Array[i] = '\0';
			}
			break;
		}
		i++;
	}

    return;
}

/*uart_putc
* Sends a char to the UART. Will wait if the UART is busy
* INPUT: Char to send
* RETURN: None
*/
void uart_putc(unsigned char c)
{
	tx_char = c;						//Put the char into the tx_char
	tx_fifo[tx_fifo_ptA] = tx_char;		//Put the tx_char into the fifo
	tx_fifo_ptA++;						//Increase the fifo pointer
	if(tx_fifo_ptA == FIFO_SIZE)		//Check to see if the pointer is max size. If so roll it over
	{
		tx_fifo_ptA = 0;
	}
	if(tx_fifo_ptB == tx_fifo_ptA)		//fifo full
	{
		tx_fifo_full = 1;
	}
	else
	{
		tx_fifo_full = 0;
	}
	IE2 |= UCA0TXIE; 					//Enable USCI_A0 TX interrupt
	return;
}

/*uart_puts
* Sends a string to the UART. Will wait if the UART is busy
* INPUT: Pointer to String to send
* RETURN: None
*/
void uart_puts(char *str)					//Sends a String to the UART.
{
     while(*str) uart_putc(*str++);			//Advance though string till end
     return;
}

#pragma vector = USCIAB0TX_VECTOR			//UART TX USCI Interrupt
__interrupt void USCI0TX_ISR(void)
{
	UCA0TXBUF = tx_fifo[tx_fifo_ptB];		//Copy the fifo into the TX buffer
	tx_fifo_ptB++;

	if(tx_fifo_ptB == FIFO_SIZE)			//Roll the fifo pointer over
	{
		tx_fifo_ptB = 0;
	}
    if(tx_fifo_ptB == tx_fifo_ptA)			//Fifo pointers the same no new data to transmit
    {
    	IE2 &= ~UCA0TXIE; 					//Turn off the interrupt to save CPU
    }
}

#pragma vector = USCIAB0RX_VECTOR		//UART RX USCI Interrupt. This triggers when the USCI receives a char.
__interrupt void USCI0RX_ISR(void)
{
	rx_char = UCA0RXBUF;				//Copy from RX buffer, in doing so we ACK the interrupt as well
	rx_flag = 1;						//Set the rx_flag to 1

	rx_fifo[rx_fifo_ptB] = rx_char;		//Copy the rx_char into the fifo
	rx_fifo_ptB++;

	if(rx_fifo_ptB == FIFO_SIZE)		//Roll the fifo pointer over
	{
		rx_fifo_ptB = 0;
	}

	if(rx_fifo_ptB == rx_fifo_ptA)		//fifo full
	{
		rx_fifo_full = 1;
	}
	else
	{
		rx_fifo_full = 0;
	}

	P1OUT ^= LED;						//Notify that we received a char by toggling LED
}



