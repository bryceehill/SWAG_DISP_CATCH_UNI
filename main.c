#include <msp430.h> 
#include "serial_handler.h"
#include "uart.h"
#include "main.h"
#include <stdint.h>

//#include "uart.h"


/**
 * main.c
 * 1.0 - SW2
 * 1.1 - RX0
 * 1.2 - TX0
 * 1.3 - SW1
 * 1.4 - RX1
 * 1.5 - TX1
 * 1.6 - SCL
 * 1.7 - SDA
 * 2.0 - SIG_C1
 * 2.1 - SIG_C2
 * 2.2 - SIG_C3
 * 2.3 - SIG_C4
 * 2.4 - SIG_C5
 * 2.5 - PWM_K
 * 2.6 - SW3
 * 2.7 - NC
 * 3.0 - NC
 * 3.1 - SIG_R1
 * 3.2 - SIG_R2
 * 3.3 - SIG_R3
 * 3.4 - SIG_R4
 * 3.5 - SIG_R5
 * 3.6 - SIG_R6
 * 3.7 - SIG_R7
 *
 */

char gridArray[5] = {0x7F, 0x04, 0x18, 0x04, 0x7F};
char currChar = ' ';
int isrCount = 0,scrollCount = 0;

int main(void){
    WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
    led_init();
    //    switch_init();
    timer_ISR_init();
    uart_init(4); // 9600 baud rate = 4 input
    uart_sw_init(); // Software uart init
    BCSCTL1 = CALBC1_16MHZ;      // Set range
    DCOCTL = CALDCO_16MHZ;      // SMCLK = DCO = 1MHz
    uint8_t c;
    int replyVal=0;
    while(1){
        if(eos_flag){
            replyVal = serial_handler (rx_data_str , 0, rx_flag, tx_data_str);
            if (replyVal)
                uart_write_string(0,replyVal);
            rx_flag = 0;
            eos_flag = 0;

        }
        if (eos_flag_sw){
            replyVal = serial_handler (rx_data_str_sw ,1, rx_flag_sw, tx_data_str_sw+1);  // THe +1 in this line and the line below is due to a SW uart Glitch IDK
            if (replyVal)
                uart_write_string_sw(0,replyVal+1);

            rx_flag_sw = 0;
            eos_flag_sw = 0;


        }


    }
}

void shiftIn(char * arrIn, char * arrOut,int shiftNum, int inDir){
    // This takes the bytes from arrIn and puts them into arrout.  Direction is dependent upon inDir that can swap direction
    int k;
    if (inDir){ // direct copy values from arrIn to Arr out swapping order
        for (k=0;k<shiftNum;k++){
            arrOut[k] = arrIn[shiftNum-k];
        }
    }
    else{     // direct copy of values
        for (k=0;k<shiftNum;k++){
            arrOut[k] = arrIn[k];
        }
    }

}

void shiftArr(char * arrIn,int byteNum,int shiftNum,int shiftDir){
    int k,n;
    if (shiftDir){      // Shift to the right
        for (n=0;n<byteNum;n++){
            for (k=0;k<shiftNum-1;k++){

                arrIn[shiftNum-k]=arrIn[shiftNum-k-1];
            }
        }
    }
    else{  //shift to the left
        for (n=0;n<byteNum;n++){
            for (k=0;k<shiftNum-1;k++){
                arrIn[k]=arrIn[k+1];
            }
        }

    }

}

int serial_handler ( char * stringIn, int uartdir,int strLen,char * stringOut){
    // Parse for the initial string and the end string
    volatile int strBeg, strEnd,i,k,byteNum,byteRet,shiftNum,dispShift;
    volatile char tempChar;
    char iString[] = "Tech Matrix";
    char cmpStr[2],byteOut[5],byteChar[5];
    cmpStr[0]='%';
    cmpStr[1]='*';
    strBeg = strComp(stringIn,cmpStr,strLen+1,2);
    if (strBeg == 0xFFFF)
        return;
    cmpStr[0]=10;
    strEnd = strComp(stringIn,cmpStr,strLen+2,1);
    if (strEnd == 0xFFFF)
        return;
    if (stringIn[strBeg+2] == 'b'){     // Bytes of data to display
        shiftIn(stringIn, stringOut, 4, 0); // copy first four bytes no questions asked
        byteNum = stringIn[strBeg+3];
        if (uartdir){   // Up-stream UART
            shiftIn(gridArray, stringIn+byteNum+4, 5, 0);   // copy the existing grid Array
            shiftIn(stringIn+4,gridArray,5,0);      // Move last 5 bytes into grid Array
            shiftIn(stringIn+9+strBeg, stringOut+4, byteNum, 0); // copy all bytes but shifted by 5

        }
        else{
            shiftIn(stringIn+4+strBeg, stringOut+9, byteNum, 0); // copy all bytes but shifted by 5
            shiftIn(gridArray, stringOut+4, 5, 0);   // copy the existing grid Array
            shiftIn(stringOut+4+byteNum,gridArray,5,0);      // Move last 5 bytes into grid Array
        }
        //        if (byteNum>5){  // Pass on other data
        //            stringOut[0] = stringIn[0+strBeg];
        //            stringOut[1] = stringIn[1+strBeg];
        //            stringOut[2] = stringIn[2+strBeg];
        //            stringOut[3] = stringIn[3+strBeg];
        //            for (k=0;k<byteNum-5;k++)  // move up the data to send
        //                stringOut[k+4]=stringIn[9+strBeg];
        //
        //            shiftIn(gridArray,stringOut+k+byteNum,5,uartdir);  // append data from existing display
        //
        //            shiftIn(stringIn+4+strBeg,gridArray,5,uartdir);  //  new Data to display
        //            return byteNum+4;
        //        }
        //        else{
        //            shiftArr(gridArray,byteNum,5,uartdir);
        //            shiftIn(stringIn+4+strBeg+5-byteNum,gridArray,byteNum,uartdir);  // New Data to display
        //        }
        return byteNum+4;



    }
    else if (stringIn[strBeg+2] == 'B'){     // Bytes of data to display
        // Save this one for later
        //        byteNum = twoCharConv (stringIn+strBeg+3);
        //        convNibble(stringIn+strBeg+5,byteOut,5);
        //        if (byteNum>5){     // Pass on the other info.
        //            stringOut[0] = stringIn[0+strBeg];
        //            stringOut[1] = stringIn[1+strBeg];
        //            stringOut[2] = stringIn[2+strBeg];
        //            num2Char2Digit(byteNum-5,stringIn+3);
        //            for (k = 0;k<byteNum-5;k++){
        //                stringOut[k+5] = stringIn[15+strBeg];
        //                stringOut[k+6] = stringIn[16+strBeg];
        //            }
        //        }
        //        if (byteNum > 5)
        //            byteNum =5;
        //        for (k = 0;k < byteNum; k++){ // step the existing grid to the side
        //            if (uartdir){
        //                gridArray[5-k] = gridArray[4-k];
        //            }
        //            else{
        //                gridArray[k] = gridArray[k+1];
        //            }
        //        }
        //        for (k = 0;k < byteNum; k++){   // move in the new data
        //            if (uartdir){
        //                gridArray[byteNum-k] = byteOut[k];
        //            }
        //            else{
        //                gridArray[k+byteNum] = byteOut[k];
        //            }
        //        }
        //        if (byteNum>5)  // Can't be bytenum if it's 5
        //            return byteNum*2+3;
        //

    }
    else if (stringIn[strBeg+2] == 'C'){    // Characters of data to display
        shiftIn(stringIn, stringOut, 5, 0); // copy first Five bytes no questions asked
        byteNum = twoCharConv (stringIn+strBeg+3);
        if (uartdir){

            shiftIn(&currChar,stringIn+byteNum+5,1,0);
            currChar = stringIn[strBeg+5];
            charMap(currChar, gridArray);
            shiftIn(stringIn+6+strBeg,stringOut+5,byteNum,0);  // updating data 2 send to next downstream or upstream

        }
        else{
            shiftIn(stringIn+5+strBeg,stringOut+6,byteNum-1,0);  // updating data 2 send to next downstream or upstream
            stringOut[5] = currChar;
            currChar = stringIn[4+byteNum+strBeg];
            charMap(currChar, gridArray);

        }
        //        //        tempChar = stringIn[strBeg+7];
        //
        //        //        shiftNum = stringIn[strBeg+5]-0x30;  // get the number of times to shift the data character
        //        //        dispShift = stringIn[strBeg+6]-0x30;  // get the number of times to shift the existing display
        //        //        shiftArr(gridArray,5,5,uartdir);  // number of characters to shift the existing display
        //        //        shiftIn(byteChar,gridArray+5-shiftNum,shiftNum,uartdir);  // New Data to display shifting in the desired number of bytes
        //        stringOut[0] = stringIn[0+strBeg];
        //        stringOut[1] = stringIn[1+strBeg];
        //        stringOut[2] = stringIn[2+strBeg];

            return byteNum+5;

    }
    //    else if (stringIn[strBeg+2] == 'C'){    // Characters of data to display
    //            charMap(stringIn[strBeg+7], byteChar);
    //            tempChar = stringIn[strBeg+7];
    //            byteNum = twoCharConv (stringIn+strBeg+3);
    //            shiftNum = stringIn[strBeg+5]-0x30;  // get the number of times to shift the data character
    //            dispShift = stringIn[strBeg+6]-0x30;  // get the number of times to shift the existing display
    //            shiftArr(gridArray,dispShift,5,uartdir);  // number of characters to shift the existing display
    //            shiftIn(byteChar,gridArray+5-shiftNum,shiftNum,uartdir);  // New Data to display shifting in the desired number of bytes
    //            stringOut[0] = stringIn[0+strBeg];
    //            stringOut[1] = stringIn[1+strBeg];
    //            stringOut[2] = stringIn[2+strBeg];
    //            num2Char2Digit(byteNum-1,stringOut+3);
    //            shiftArr(stringOut+5,1,byteNum,0);
    //
    //            return byteNum+4;
    //
    //        }
    else if (stringIn[strBeg+2] == 'F'){    // Write data to Flash Memory
        byteNum = stringIn[strBeg+3];   // number of bytes N
        byteLoc = stringIn[strBeg+4];    // location of writning bits M
       // flash_ptr = (char *) 0x1040;     // either 8 bits or 16 bits
        if (byteLoc == 0){
             Flash_ptr = (char *) 0x10FF; // pointer to available register 10ff-103 to 0x1099 // 256 bytes 010FFh to 01000h decending??
            // code to erase whatever is in these available registers when user hits send on phone. 1 byte = 1 charcter , 1 graphic frame = 5 bytes, limits 100-scroll 150-character 
            FCTL1 = FWKEY + ERASE;                    // Set Erase bit
		    FCTL3 = FWKEY;                            // Clear Lock bit
		    *Flash_ptr = 0;                           // Dummy write to erase Flash segment

		   FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
		   *Flash_ptr ++= 0xAA;
		   *Flash_ptr ++= 1;
		    FCTL1 = FWKEY;                            // Clear WRT bit
		    FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
        }
        else if (byteloc == 1){
              flash_ptr = (char *) 0x1098; // pointer to available register 1097-153 to 1000h
    }
    else if (stringIn[strBeg+2] == 'R'){    // Read request

    }
    else if (stringIn[strBeg+2] == 'W'){    // Response from read request

    }

    else if (stringIn[strBeg+2] == 'T'){    // Diagnostic Mode
	ModeSelect = 7; //Enter Test Mode
	if (stringIn[strBeg+3] == 'L'){ //Test LED Matrix 
	}
	if (stringIn[strBeg+3] == 'U'{//Test Uart Channels
		if(stringIn[strBeg+4] == '1'){//Test Channel 1
		char uartString[] = "UART 1 OK";
		uart_write_string(0,uartString);
		}
		if(stringIn[strBeg+4] == '2'){//Test Channel 2
		char uartString[] = "UART 2 OK";
		uart_write_string_sw(0,uartString);
		}
	}
	if (stringIn[strBeg+3] == 'S'){//Test Switches}
		if(stringIn[strBeg+4] == '1'){ //Test switch 1 
		}
		if(stringIn[strBeg+4] == '2'){//Test switch 2
		}
		if(stringIn[strBeg+4] == '3'){//Test switch 3
		}		
	}
	    if (stringIn[strBeg+3] == 'E'){ // Exit testing mode
		    ModeSelect = 2;
	    }
	    }
    return 0;
}

void convNibble(char * stringIn,char * byteOut, int num2Conv){
    // blindly converts two nibbles embedded in two bytes to a byte raw format
    int k;
    //    volatile char C1, C2;
    //    volatile int tempVal=0;
    for(k = 0;k<num2Conv;k++){
        //        C1=stringIn[k*2];
        //        C2 = stringIn[k*2+1];

        //        tempVal = (stringIn[k*2]-0x30)*10 + (stringIn[k*2+1]-0x30);
        byteOut[k] = (stringIn[k*2]-0x30)*10 + (stringIn[k*2+1]-0x30);
    }
}

void num2Char2Digit(int valIn,char * stringIn){
    //convert to a 2 digit char string.  Only 2 digits
    int tempVal=0;
    //    if (valIn<10){
    //        stringIn[0]='0';
    //        stringIn[1]=0x30+valIn;
    //    }
    tempVal = valIn/10;
    stringIn[0]=tempVal+0x30;
    valIn -= 10*tempVal;
    stringIn[1] = valIn+0x30;

}

int twoCharConv (char * stringIn){
    // simply and blindly creates a two digit int from two digit char
    return (stringIn[0]-0x30)*10+(stringIn[1]-0x30);
}

int strComp (char * stringIn, char * strPattern, int stringLen, int patternLen){
    // returns a value of where in the string the first occurance happens
    int i,k,diffVal;
    for (k=0;k<stringLen-patternLen;k++){
        diffVal = 0;
        for (i = 0;i<patternLen; i++){
            if (stringIn[k+i] != strPattern[i])
                diffVal++;
        }
        if (diffVal == 0)
            return k;
    }
    return 0xFFFF;  // no pattern detected

}


void charMap(int valin, char * mapOut){

    int k,n;
    char tempval,holder;
    for (k=0;k<5;k++){
        holder=mapArray[valin-0x20][k];
        tempval=0;
        for (n=0;n<8;n++){
            tempval<<=1;
            if (holder&(1<<n)){

                tempval+=1;

            }



        }
        tempval>>=1;
        mapOut[k]=tempval;
    }

}


void test_sequence(int a_in, int k_in){  // Simply lights each LED by itself
    P2OUT |= (BIT0+BIT1+BIT2+BIT3+BIT4);
    P3OUT &=~ (BIT1+BIT2+BIT3+BIT4+BIT5+BIT6+BIT7);
    P2OUT &=~ (1<<a_in);
    P3OUT |= (1<< (k_in+1));
}

void led_init(void){
    P2DIR |= (BIT0+BIT1+BIT2+BIT3+BIT4+BIT5);   // Annodes as outputs  (Active Low)
    P2OUT |= (BIT0+BIT1+BIT2+BIT3+BIT4);        // Annodes all off
    P2OUT |= BIT5;                             // PWM pin on (for brightness) On
    P3DIR |= (BIT1+BIT2+BIT3+BIT4+BIT5+BIT6+BIT7);  // Cathodes all as outputs
    P3OUT &=~ (BIT1+BIT2+BIT3+BIT4+BIT5+BIT6+BIT7);  // Cathodes all low (Active high)

}

void timer_ISR_init(void){
    TA1CCTL0 = CCIE;                             // CCR0 interrupt enabled
    TA1CCR0 = 0;
    TA1CTL = TASSEL_2 + MC_2;                  // SMCLK, contmode

    _BIS_SR( GIE);                 // Enter LPM0 w/ interrupt
}

//// Port 1 interrupt service routine
//#pragma vector=PORT1_VECTOR
//__interrupt void Port_1(void)
//{
//    if (P1IFG & BIT0){
//        P1IFG &= ~BIT0;                           // P1.0 IFG cleared
//    }
//    if (P1IFG & BIT3){
//        P1IFG &= ~BIT3;                           // P1.4 IFG cleared
//    }
//}
//
//
//// Port 1 interrupt service routine
//#pragma vector=PORT2_VECTOR
//__interrupt void Port_2(void)
//{
//    if (P2IFG & BIT6){
//        P2IFG &= ~BIT6;                           // P1.0 IFG cleared
//    }
//
//}

// Timer A0 interrupt service routine
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer_A1 (void)
{
    P2OUT |= (BIT0+BIT1+BIT2+BIT3+BIT4);
    P3OUT &=~ (BIT1+BIT2+BIT3+BIT4+BIT5+BIT6+BIT7);
    P2OUT &=~ (1<<isrCount);
    P3OUT |= gridArray[isrCount]<<1;
    scrollCount++;
    isrCount++;
    if (isrCount>4)
        isrCount=0;
    TA1CCR0 +=1000;                            // Add Offset to CCR0
}

void switch_init(void){
    P1DIR &=~ (BIT0+BIT3);  // INputs for switches
    P2SEL &=~ (BIT6+BIT7);              // Turn off oscillator
    P2SEL2 &=~ (BIT6+BIT7);              // Turn off oscillator
    P2DIR &=~ (BIT6);       // Input for switch
    P1OUT |= (BIT0+BIT3);   // Pull-UP for switches
    P2OUT |= (BIT6);    // Pull-UP for switches
    P1REN |= (BIT0+BIT3);   // Pulling resistors for switches
    P2REN |= (BIT6);    // Pulling resistor for switch
    P1IE |=(BIT0+BIT3);   // Hardware interrupt P1 switches
    P1IES |=(BIT0+BIT3);   // High to low edge trigger
    P2IE |=(BIT6);   // Hardware interrupt P1 switches
    P2IES |=(BIT6);   // High to low edge trigger
    P1IFG &= ~ (BIT0+BIT3);
    P2IFG &= ~BIT6;
    _BIS_SR( GIE);                 // Global interrupt enable
}

