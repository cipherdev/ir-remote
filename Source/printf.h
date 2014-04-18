/*
 * printf.h
 *
 *  Created on: Apr 18, 2014
 *      Author: Anh Huy
 */

#ifndef PRINTF_H_
#define PRINTF_H_

/*
 *uart_printf(char*)
 *Converts and formats values to be sent via char UART. Works similar to normal printf function.
 *INPUT: Char* EX: uart_printf("DATA: %i\r\n", datvar);
 *RETURN: None
 */
void uart_printf(char *format, ...);


#endif /* PRINTF_H_ */
