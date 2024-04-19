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
 * ****************************************************************************
 * Includes also improvements from Joby Taffey and fixes from Colin Funnell
 * as posted here:
 *
 * http://blog.hodgepig.org/tag/msp430/
 ******************************************************************************/

#include <msp430.h>
//#include <legacymsp430.h>
#include <stdbool.h>

#include "uart.h"


//Button states vectors
volatile char SWState[] = {1, 1, 1};
volatile char StateChange[] = {0, 0, 0};

/**
 * TXD on P1.1
 */
#define TXD BIT5

/**
 * RXD on P1.2
 */
#define RXD BIT4

/**
 * CPU freq.
 */
#define FCPU 			1600000

/**
 * Baudrate
 */
#define BAUDRATE 		9600

/**
 * Bit time
 */
#define BIT_TIME        1667  //(FCPU / BAUDRATE)

/**
 * Half bit time
 */
#define HALF_BIT_TIME   833   //(BIT_TIME / 2)

#define Q_BIT_TIME     416  // quarter bit time

#define THREE_Q_BIT_TIME  1250  // 3/4 bit time

/**
 * Bit count, used when transmitting byte
 */
static volatile uint8_t bitCount;
static volatile uint8_t txBitCount;


/**
 * Value sent over UART when uart_putc() is called
 */
static volatile unsigned int TXByte;

/**
 * Value received once hasRecieved is set
 */
static volatile unsigned int RXByte;

/**
 * Status for when the device is receiving
 */
static volatile bool isReceiving = false;
static volatile bool isTransmitting = false;

static volatile int sync = 1;
static volatile int rxQueue=1;
static volatile int txQueue=1;

/**
 * Lets the program know when a byte is received
 */
static volatile bool hasReceived = false;

static volatile unsigned int part_val;

//#define uart_max 100
unsigned char tx_data_str_sw[uart_max], rx_data_str_sw[uart_max], rx_flag_sw=0, eos_flag_sw=0;

void uart_write_string_sw(int vals, int vale){
    int i;                                  // writes a string from global variable tx_data_str.  vals is starting pointer and vale is the ending value
    for(i=vals;i<vale;i++){
        uart_putc(tx_data_str_sw[i]);

    }
    while (!(IFG2&UCA0TXIFG));
    uart_putc(10);

}

void uart_sw_init(void)
{
    //    P1SEL |= TXD;
    P1DIR |= TXD;
    P1OUT |= TXD;

    P1IES |= RXD; 		// RXD Hi/lo edge interrupt
    P1IFG &= ~RXD; 		// Clear RXD (flag) before enabling interrupt
    P1IE  |= RXD; 		// Enable RXD interrupt
}

bool uart_getc(uint8_t *c)
{
    if (!hasReceived) {
        return false;
    }

    *c = RXByte;
    hasReceived = false;

    return true;
}

void uart_putc(uint8_t c)
{
    TXByte = c;

    while(isTransmitting); 					// Wait for RX completion
    P1OUT |= TXD;                           // Initialize the TX line high
    isTransmitting = true;
    sync = 1;
    txQueue = 1;
    TACTL = TASSEL_2 + MC_2;        // SMCLK, continuous mode

    txBitCount = 9; 						// Load Bit counter, 8 bits + ST/SP
    if (isReceiving == false){              // if it's already receiving just use existing clock sequence
        CCR0 = TAR; 							// Initialize compare register
        CCR0 += BIT_TIME; 						// Set time till first bit
        CCTL0 =  CCIE;        // Disable TX and enable interrupts
    }
    _BIS_SR( GIE);                 // Enter LPM0 w/ interrupt
    //    __delay_cycles(10000);
    //    P1OUT &=~ TXD;
    //    CCTL0 = CCIS_0 + OUTMOD_0 + CCIE + OUT; // Set signal, intial value, enable interrupts
    //
    //    while ( CCTL0 & CCIE ); 				// Wait for previous TX completion
}

void uart_puts(const char *str)
{
    if(*str != 0) uart_putc(*str++);
    while(*str != 0) uart_putc(*str++);
}

unsigned int get_part(void){
    return part_val;
}

/**
 * ISR for RXD
 */
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void){
    if (P1IFG & RXD){                   // RX has ensued
        isReceiving = true;
        sync = 1;                       // Synchronization is for the TX and RX if they happen simultaneously, or are off by 1/2 a clock cycle
        rxQueue = 1;                    //  Queue is for 1/2 sync purposes
        P1IE &= ~RXD; 					// Disable RXD interrupt
        P1IFG &= ~RXD; 					// Clear RXD IFG (interrupt flag)
        part_val = 3333;
        TACTL = TASSEL_2 + MC_2; 		// SMCLK, continuous mode
        if (isTransmitting){

            unsigned int txPart = CCR0 - TAR;    // find what is left on the timer
            part_val = txPart;
            if ((txPart< Q_BIT_TIME)||(txPart > THREE_Q_BIT_TIME)){
                sync = 0;                       // The two are not synced and the RX will proceed as half clock cycles
                rxQueue=0;
                txQueue=1;
            }

        }
        else{                               // just a receive sequence
            CCR0 = TAR; 					// Initialize compare register
            CCR0 += (HALF_BIT_TIME); 			// Set time till first bit
        }
        CCTL0 =  CCIE; 		// Disable TX and enable interrupts

        RXByte = 0; 					// Initialize RXByte
        bitCount = 9; 					// Load Bit counter, 8 bits + start bit
    }
    if (P1IFG & BIT0){
        P1IES ^= BIT0;
        if(P1IN & BIT0){
            SWState[1] = 1;//button is active
        }
        else{
            SWState[1] = 0;//button is not active
        }
        StateChange[1] = 1;//button change has occurred
        P1IFG &= ~BIT0;                           // P1.0 IFG cleared
    }
    if (P1IFG & BIT3){
        P1IES ^= BIT3;
        if(P1IN & BIT3){
            SWState[0] = 1;//button is active
        }
        else{
            SWState[0] = 0;//button is not active
        }
        StateChange[0] = 1;//button change has occurred
        P1IFG &= ~BIT3;                           // P1.4 IFG cleared
    }
}

/**
 * ISR for TXD and RXD
 */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    CCR0 += HALF_BIT_TIME;
    if (sync == 1)
        CCR0 += HALF_BIT_TIME;                      // Add Offset to CCR0 for half bit if they are in sync

    if(isReceiving && rxQueue) {
        if ( (P1IN & RXD) == RXD) { 		// If bit is set?
            RXByte |= BIT8; 				// Set the value in the RXByte
        }
        RXByte >>= 1;               // Shift the bits down
        bitCount --;
        if (bitCount==0){
            isReceiving = false;        // got to the end of the rx byte
            hasReceived = true;
            RXByte &= 0x00FF;           // Mask off the LSBits
            P1IFG &= ~RXD;         // Clear RXD (flag) before enabling interrupt
            P1IE  |= RXD;       // Enable RXD interrupt
            if (isTransmitting==0)
                CCTL0 &=~ CCIE;
            rx_data_str_sw[rx_flag_sw] = RXByte;
            rx_flag_sw++;
            if ((RXByte == 10)||(RXByte == 13)){
                eos_flag_sw = 1;
            }
        }

    }
    if (isTransmitting && txQueue){
        if (txBitCount == 0){
            P1OUT |= TXD;               // Stop Bit
            if (isReceiving == 0)       // Turn off the timer if it is not also still receiving
                CCTL0 &=~ CCIE;
            isTransmitting = false;

        }
        else if (txBitCount ==9){
            txBitCount--;
            P1OUT &=~ TXD;      // Start Bit
        }
        else if (txBitCount == 1){
            P1OUT &=~ TXD;      // Stop \Bit
            txBitCount--;
        }
        else{                       // output the TX bit one at a time for 8 bits LSBit first
            if (TXByte&BIT0)
                P1OUT |= TXD;
            else
                P1OUT &=~ TXD;
            TXByte >>=1;
            txBitCount--;
        }
    }
    if (sync == 0){
        txQueue^=1;         // using half bit time so the queues switch
        rxQueue^=1;
    }
}


// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    if (P2IFG & BIT6){
        P2IES ^= BIT6;
        if(P2IN & BIT6){
            SWState[2] = 1;//button is active
        }
        else{
            SWState[2] = 0;//button is not active
        }
        StateChange[2] = 1;//button change has occurred
        P2IFG &= ~BIT6;                           // P1.0 IFG cleared
    }

}

void SwitchStateGet(char *SwtchState, char *SWChange){//function to send the current states of the 3 pushbuttons
    SwtchState[0] = SWState[0];//states for buttons
    SwtchState[1] = SWState[1];
    SwtchState[2] = SWState[2];
    SWChange[0] = StateChange[0];//flags for changes in button states
    SWChange[1] = StateChange[1];
    SWChange[2] = StateChange[2];
    StateChange[0] = 0;//reset state change flags
    StateChange[1] = 0;
    StateChange[2] = 0;
}
