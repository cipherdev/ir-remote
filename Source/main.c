/*
 * main.c
 * Copyright 2014
 * HuyLe, Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 *  Created on: Apr 15, 2014
 *      Author: Anh Huy
 ****************************************************************************/
/*	>>>>>>>>>>>>>>>>>>>>>>>>>DEFINE KEY REMOTE <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	*************************************************************************
	*************************** ***LG ***************************************
	*************************************************************************
	*					#define _PW 0xf30cff00								*
	*					#define _UP 0xe51aff00								*
	*					#define _DW  0xe11eff00								*
	*					#define _R 0xe41bff00								*
	*					#define _L 0xe01fff00								*
	******************************Panasonic**********************************
	*					#define _X 0x0ac10110     x may lanh				*
 	*************************************************************************
	****************************** Sigma ************************************
	*************************************************************************
	*					#define _Power	0xb54acb04							*
	*					#define _Left 	0xbc43cb04							*
	*					#define _Right 	0xfe01cb04							*
	*					#define _Up 	0xf906cb04							*
	*					#define _Down 	0xf50acb04							*
	*					#define _Red 	0xa05fcb04							*
	*					#define _Green 	0xe21dcb04							*
 	*************************************************************************
	*********************************** Sony ********************************
	*************************************************************************
	*    Command		Function				Command		Function		*
	*		0			Digit key 1				20			Mute			*
	*		1			Digit key 2				21			Power			*
	*		2			Digit key 3				22			Reset			*
	*		3			Digit key 4				23			Audio Mode		*
	*		4			Digit key 5				24			Contrast +		*
	*		5			Digit key 6				25			Contrast -		*
	*		6			Digit key 7				26			Colour +		*
	*		7			Digit key 8				27			Colour -		*
	*		8			Digit key 9				30			Brightness +	*
	*		9			Digit key 0				31			Brightness -	*
	*		16			Channel +				38			Balance Left	*
	*		17			Channel -				39			Balance Right	*
	*		18			Volume +				47			Standby			*
	*		19			Volume -											*
	*************************************************************************/
#include "msp430g2553.h"
#include "uart_fifo.h"					//Your UART header
#include "printf.h"

#define T05 300
#define T65 T05*13
#define T2 T05*4
#define T3 T05*6

unsigned int irSignal = 0, bitCounter = 0;
unsigned int flag = 0;
unsigned int devRun = 0x01;
int so[13] = { 0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0x7F,0xFF,0xBF };

void showled(int a, int b)
{
	P2OUT = so[a];
	P1OUT |= b;
	__delay_cycles(5000);
	P1OUT &=~ b;
}
void main(void) {
	WDTCTL = WDTPW + WDTHOLD;
	BCSCTL1 = CALBC1_8MHZ;
	DCOCTL = CALDCO_8MHZ;
//	P1DIR |=0xF0;
	P1DIR &= ~BIT1;
	P1SEL = BIT1;

	P2OUT &= ~0xFF;
	P2DIR |= 0xFF;
	P2SEL =0;
	//TACTL = TASSEL_2 | MC_2;
	TACTL = TASSEL_2 | MC_2 | ID_3;
	CCTL0 = CM_2 | CCIS_0 | CAP | CCIE;
	__enable_interrupt();

	//__bis_SR_register(LPM0_bits + GIE);
	uart_init();
	uart_printf(">>>>>>>>>>>welcome to program control ir remote<<<<<<<<<<< \r\n");
}
#ifndef _debug
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void) {
	if (CCTL0 & CAP) {
		bitCounter++;
		CCR0 += T65;
		CCTL0 &= ~CAP;
	} else {
		switch (bitCounter) {
		case 0x1000:
			bitCounter = 0;
			switch (irSignal & 0x001F) {
				case 21: 	// power off all led P2
					P2OUT = 0x00;
					uart_printf("_turn off all led \r\n");
					break;
				case 20: 	// mute on all led P2
					P2OUT |= 0xFF;
					uart_printf("_turn on all led \r\n");
					break;
				case 3:		// digit key 4
					P2OUT |= 0x0f;
					break;
				case 5:		//degit key 6
					P2OUT |= 0xf0;
					break;
				case 1:		// digit key 2
					for(;;){
							_delay_cycles(500000);
							P2OUT =~ devRun;
							devRun >>=1;
							if(devRun < 0x01)devRun = 0x80;
							uart_printf("_debug move off right: %u\r\n",devRun);
							break;
						}
					break;
				case 7:		//degit key 8
					for(;;){
							_delay_cycles(500000);
							P2OUT = devRun;
							devRun >>=1;
							if(devRun < 0x01)devRun = 0x80;
							uart_printf("_debug move right: %u\r\n",devRun);
							break;
						}
					break;
				case 4:		//degit key 5
					for(;;){
							_delay_cycles(500000);
							P2OUT = devRun;
							devRun <<=1;
							if(devRun > 0x80)devRun = 0x01;
							uart_printf("_debug move left: %u\r\n",devRun);
							break;
						}
					break;
#ifdef _debug_key
				case 0:		//degit key 1
					for(;;){
							showled(2,0x10);
							showled(0,0x20);
							showled(1,0x40);
							showled(4,0x80);
					//		break;
						}
					break;
#endif
			}
			irSignal = 0;
			CCTL0 |= CAP;
			break;
		default:
			if (CCTL0 & SCCI) {
				CCR0 += T2;
			} else {
				irSignal |= bitCounter;
				CCR0 += T3;
			}
			bitCounter <<= 1;
			break;
		}
	}
}
#endif
