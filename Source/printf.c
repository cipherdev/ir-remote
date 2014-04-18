/*
 * printf.c
 *
 *  Created on: Apr 18, 2014
 *      Author: Anh Huy
 *
 *	This is a printf.c file
 *	It requires the following UART functions to be implemented in the UART.
 *
 *	uart_putc()
 *	uart_puts()
 *
*/

#include <stdarg.h>
#include "uart_fifo.h"
#include "printf.h"
/*
 * divider[]
 * This is the dividers to convert a decimal value into the char needed
 */
static const unsigned long divider[] = {
    1000000000,
     100000000,
      10000000,
       1000000,
        100000,
         10000,
          1000,
           100,
            10,
             1,
};

/*
 * convert_dec
 * This is the dividers to convert a decimal value into the char needed. Uses the UART while doing so.
 * INPUT: number to convert and divider array.
 * RETURN: None
 */
void convert_dec(unsigned long x, const unsigned long *dp)
{
    char c;
    unsigned long d;
    if(x)
    {
        while(x < *dp) ++dp;
        do
        {
            d = *dp++;
            c = '0';
            while(x >= d) ++c, x -= d;

            uart_putc(c);
        }
        while(!(d & 1));
    }
    else
    {
        uart_putc('0');
    }
    return;
}

/*
 * convert_hex(unsigned)
 * Hex look up table for converting.
 * INPUT: number to convert.
 * RETURN: number converted.
 */
unsigned convert_hex(unsigned n)
{
    static const char hex[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    return hex[n & 15];
}

/*
 *uart_printf(char*)
 *Converts and formats values to be sent via char UART. Works similar to normal printf function.
 *INPUT: Char* EX: uart_printf("DATA: %i\r\n", datvar);
 *RETURN: None
 */
void uart_printf(char *format, ...)
{
    char c;
    int i;
    long l;

    va_list list;														//Make the arguement list
    va_start(list, format);

    while(c = *format++)												//run through the input till the end.
    {
        if(c == '%')													//% denotes the variable format character
        {
            switch(c = *format++)
            {
                case 's':                       						// strings
                    uart_puts(va_arg(list, char*));
                    break;

                case 'c':                       						// chars
                    uart_putc(va_arg(list, char));
                    break;

                case 'i':                       						// signed ints
                	i = va_arg(list, int);
                    if(i < 0)
                    {
                    	i = -i;
                     	uart_putc('-');
                    }
                    convert_dec((unsigned)i, divider + 5);
                    break;

                case 'u':                      							// unsigned ints
                    i = va_arg(list, int);
                    convert_dec((unsigned)i, divider + 5);
                    break;

                case 'l':                       						// signed longs
                    l = va_arg(list, long);
                    if(l < 0)
                    {
                    	l = -l;
                    	uart_putc('-');
                    }
                    convert_dec((unsigned long)l, divider);
                    break;

                case 'n':                       						// unsigned longs
                    l = va_arg(list, long);
                    convert_dec((unsigned long)l, divider);
                    break;

                case 'x':                       						// 16bit Hex
                    i = va_arg(list, int);
                    uart_putc(convert_hex(i >> 12));
                    uart_putc(convert_hex(i >> 8));
                    uart_putc(convert_hex(i >> 4));
                    uart_putc(convert_hex(i));
                    break;

                case 0:
                	return;
                default:
                	uart_putc(c);										//can't find formating. just print it.
            }
        }
        else
        {
        	uart_putc(c);
        }
    }
    va_end(a);
    return;
}



