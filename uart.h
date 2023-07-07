/******************************************************************************
 * Software UART example for MSP430.
 *
 * Stefan Wendler
 * sw@kaltpost.de
 * http://gpio.kaltpost.de
 *
 * ****************************************************************************
 * The original code is taken form Nicholas J. Conn example:
 *
 * http://www.msp430launchpad.com/2010/08/half-duplex-software-uart-on-launchpad.html
 *
 * Origial Description from Nicholas:
 *
 * Half Duplex Software UART on the LaunchPad
 *
 * Description: This code provides a simple Bi-Directional Half Duplex
 * Software UART. The timing is dependant on SMCLK, which
 * is set to 1MHz. The transmit function is based off of
 * the example code provided by TI with the LaunchPad.
 * This code was originally created for "NJC's MSP430
 * LaunchPad Blog".
 *
 * Author: Nicholas J. Conn - http://msp430launchpad.com
 * Email: webmaster at msp430launchpad.com
 * Date: 08-17-10
 *****************************************************************************
 * Includes also improvements from Joby Taffey and fixes from Colin Funnell
 * as posted here:
 *
 * http://blog.hodgepig.org/tag/msp430/
 *****************************************************************************/

#ifndef __UART_H
#define __UART_H

#include <stdbool.h>
#include <stdint.h>

#define uart_max 50
extern unsigned char tx_data_str_sw[uart_max], rx_data_str_sw[uart_max],rx_flag_sw ,eos_flag_sw;

/**
 * Initialize soft UART
 */
void uart_sw_init(void);

/**
 * Read one character from UART non-blocking.
 *
 * @param[out]	*c	character received (if one was available)
 * @return			true if character received, false otherwise
 */
bool uart_getc(uint8_t *c);

/**
 * Write one chracter to the UART blocking.
 *
 * @param[in]	*c	the character to write
 */
void uart_putc(uint8_t c);


void uart_write_string_sw(int vals, int vale);


/**
 * Write string to the UART blocking.
 *
 * @param[in]	*str	the 0 terminated string to write
 */
void uart_puts(const char *str);

unsigned int get_part(void);

#endif

