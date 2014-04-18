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

#define T05 300
#define T65 T05*13
#define T2 T05*4
#define T3 T05*6

unsigned int irSignal = 0;
unsigned int bitCounter = 0;
unsigned int key5 = 0;
int run = 0x01;

void main(void) {
	WDTCTL = WDTPW + WDTHOLD;
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;

	P1DIR &= ~BIT1;
	P1SEL = BIT1;

	P2OUT &= ~0xFF;
	P2DIR |= 0xFF;
	P2SEL =0;
	TACTL = TASSEL_2 | MC_2;
	CCTL0 = CM_2 | CCIS_0 | CAP | CCIE;

	__bis_SR_register(LPM0_bits + GIE);
}

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
					break;
				case 20: 	// mute on all led P2
					P2OUT |= 0xFF;
					break;
				case 3:		// digit key 4
					P2OUT |= 0x0f;
					break;
				case 5:		//degit key 6
					P2OUT |= 0xf0;
					break;
				case 1:		// digit key 2
					P2OUT |= 0xFF;
					break;
				case 7:		//degit key 8
					P2OUT &= ~0xFF;
					break;
				case 9:		//degit key 8
					P1OUT |= BIT0;
					break;
				case 4:		//degit key 5
					for(;;){
						if(key5 == 0){
								_delay_cycles(500000);
								P2OUT = run;
								run <<=1;
								if(run>0x80)run = 0x01;
								key5++;
							}else if(key5==3)break;
						}
					break;
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
