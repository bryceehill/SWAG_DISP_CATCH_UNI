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
signed int buttoncounter[3];//vector for counting how long each button has been pressed
char gridArray[5] = {};
char currChar = ' ';
int isrCount = 0,scrollCount = 0;
int a_val, k_val, scrollPos=0, scrollNum, scrollMax, cat_val;
int ModeSelect = 2;
unsigned int swLFDiagCount = 0; swCNDiagCount = 0; swRTDiagCount = 0; // Counters for each time Left, Center, and Right Pushbuttons are pressed (used in Test Mode)
int lockoutTimer = 0;
int a=0,k=0;
int game_spd = 6;
int game_count = 0;
int playerpos = 0;
volatile char myVec[10];
int main(void){
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    led_init();
    switch_init();
    timer_ISR_init();
    uart_init(4); // 9600 baud rate = 4 input
    uart_sw_init(); // Software uart init
    BCSCTL1 = CALBC1_16MHZ;      // Set range
    DCOCTL = CALDCO_16MHZ;      // SMCLK = DCO = 1MHz

    int player;
    unsigned int g;
    unsigned int g2;
    unsigned int s;

    int gameover;
    int game_scroll = 0;


    //text scroll and graphic display variables

    char disp_string[] = "   Welcome to the Tech-Matrix! Go to mtech.edu/electrical-engineering/ today!  |";//Green
    char two_char[11] = {0,0,0,0,0,0,0,0,0,0,0};
    char anim_frames[] = {0x01 , 0x7F, 0x40, 0x40 ,0x7F, 0x01, 0x01, 0x7F, 0x40, 0x40, 0x7F, 0x01,0x01,0x7F,0x40,0x40,0x7F,0x01,0x01,0x7F,0x40,0x40,0x7F,0x01,0x01,0x7F,0x40,0x40,0x7F,0x01};
    char game_layout[] = {0x00,0x00,0x00,0x00,0x67,0x00,0x00,0x00,0x4F,0x00,0x00,0x00,0x73,0x00,0x00,0x00,0x7C,0x00,0x00,0x00,0x4F,0x00,0x00,0x00,0x73,0x00,0x00,0x00,0x7C,0x00,0x00,0x00,0x00};
    char game_safezone[] = {0x10,0x20,0x40,0x04,0x02,0x01,0x08,0x77,0x6F,0x5F,0x7B,0x7E,0x7D};
    //char anim_frames[] = {0x7F, 0x04, 0x18, 0x04, 0x7F, 0x7F, 0x7F,0x7F,0x7F,0x7F,0x7F, 0x04, 0x18, 0x04, 0x7F};
    char scrollLast=0;
    a_val = 0;
    while (scrollNum != '|')
        scrollNum = disp_string[a_val++];
    scrollNum=a_val;
    scrollMax = scrollNum*6;



    uint8_t c;
    int replyVal=0;
    while(1){
        if(eos_flag){
            __delay_cycles(1000);
            replyVal = serial_handler(rx_data_str , 0, rx_flag, tx_data_str);
            if (replyVal>1)
                uart_write_string(0,replyVal);

            lockoutTimer = 0;//reset lockout
            ModeSelect = 1;
            rx_flag = 0;
            eos_flag = 0;


        }
        if (eos_flag_sw){
            replyVal = serial_handler(rx_data_str_sw ,1, rx_flag_sw, tx_data_str_sw+1);  // THe +1 in this line and the line below is due to a SW uart Glitch IDK
            if (replyVal)
                uart_write_string_sw(0,replyVal+1);

            lockoutTimer = 0;//reset lockout
            ModeSelect = 1;
            rx_flag_sw = 0;
            eos_flag_sw = 0;


        }

        if (lockoutTimer > LOCKOUT_DURATION) {
            lockoutTimer = LOCKOUT_DURATION+1;
            if(ModeSelect == 1){
                ModeSelect = 2;// if device is still in Mode 1 without receiving new data, put TechMatrix into Mode 2
            }
        }

        /*Checks the how long each of the buttons have been pressed once released, if the value exceeds the ButtonThreshlod
         *(defined in main.h) the button registers as a long press, otherwise it is a short one.
         */
        if(buttoncounter[0] < 0){
            buttoncounter[0]*=-1;//returns the button counter back to a positive value
            if(buttoncounter[0] > ButtonThreshold){//long press code for right button
                swRTDiagCount++;//testing variable
            }
            else if(buttoncounter[0] < ButtonThreshold){//short press code for right button
                playerpos --;
                if (playerpos < -3){
                    playerpos = 3;
                }
            }
            buttoncounter[0] = 0;
            swRTDiagCount++;//testing variable
        }


        if(buttoncounter[1] < 0){
            buttoncounter[1]*=-1;
            if(buttoncounter[1] > ButtonThreshold){//long press code for center button
                ModeSelect++;//move to next mode
                scrollPos = 0;
                if(ModeSelect>4){
                    ModeSelect = 2;
                }
            }
            else if(buttoncounter[1] < ButtonThreshold){
                //short press code for center button
            }
            buttoncounter[1] = 0;
            swCNDiagCount++;//testing variable
        }

        if(buttoncounter[2] < 0){
            buttoncounter[2]*=-1;
            if(buttoncounter[2] > ButtonThreshold){
                //long press code for left button
            }
            else if(buttoncounter[2] < ButtonThreshold){
                //short press code for left button
                playerpos ++;
                if (playerpos > 3){
                    playerpos = -3;
                }
            }
            buttoncounter[2] = 0;
            swLFDiagCount++;//testing variable
        }

        if (ModeSelect == 2){
            // Stand alone text scroll mode
            if (scrollCount >= 2000) {
                __delay_cycles(100000);
                scrollCount = 0;
                a_val = scrollPos / 6;
                charMap(disp_string[a_val], two_char);
                a_val = scrollPos/6;
                charMap(disp_string[a_val + 1], two_char + 6);
                a_val = scrollPos%6;
                tx_data_str[0] = '%';
                tx_data_str[1] = '*';
                tx_data_str[2] = 'b';
                tx_data_str[3] = '0';
                tx_data_str[4] = '1';
                tx_data_str[5] = 0x30 + (gridArray[0] >> 4);
                tx_data_str[6] = 0x30 + (gridArray[0] & 0x0F);
                uart_write_string(0, 7);

                for (k_val = 0; k_val < 5; k_val++) {
                    gridArray[k_val] = two_char[k_val + a_val];
                }
                scrollPos++;
                if (scrollPos > scrollMax)
                    scrollPos = 0;
            }
        }



        if (ModeSelect == 3){// Game
            player = 0x08;
            if (scrollCount >= 2000) {
                scrollCount = 0;
                g2 = scrollPos / game_spd;
                // uart_write_string(0, 7);

                for (g = 0; g < 5; g++) {
                    gridArray[g] = game_layout[g + g2];
                    gridArray[1] = player + game_layout[1 + g2];
                    if (playerpos == 1){
                        player = 0x10;
                    }
                    if (playerpos == 2){
                        player = 0x20;
                    }
                    if (playerpos == 3){
                        player = 0x40;
                    }
                    if (playerpos == 0){
                        player = 0x08;
                    }
                    if (playerpos == -1){
                        player = 0x04;
                    }
                    if (playerpos == -2){
                        player = 0x02;
                    }
                    if (playerpos == -3){
                        player = 0x01;
                    }
                }
                scrollPos++;
                if ((g+g2) > (sizeof(game_layout))){
                    scrollPos = 0;
                    g = 0;
                    // g2 = 0;
                    game_count++;

                    if (game_count == 2)
                    {
                        game_spd--;
                        game_count = 0;
                    }

                    if(game_spd == 0)
                    {
                        game_spd = 1;
                    }
                }
                for (s = 0; s < 14; s++)
                {
                    if(game_safezone[s] == gridArray[1])
                    {
                        gameover = 0;
                        break;
                    }
                    else
                    {
                        gameover = 1;
                    }
                }
                if (gameover == 1)
                {
                    ModeSelect = -1;
                }
            }
        }
        if (ModeSelect == -1)//game over screen
        {
            unsigned int g;
            scrollPos = 0;
            scrollCount = 0;
            playerpos = 0;
            game_spd = 6;
            for (g = 0; g < 3; g++)
            {
                gridArray[0] = 0x63;
                gridArray[1] = 0x14;
                gridArray[2] = 0x08;
                gridArray[3] = 0x14;
                gridArray[4] = 0x63;
                _delay_cycles(8000000);
                gridArray[0] = 0x00;
                gridArray[1] = 0x00;
                gridArray[2] = 0x00;
                gridArray[3] = 0x00;
                gridArray[4] = 0x00;
                _delay_cycles(8000000);
            }
            ModeSelect = 3;
        }
        if (ModeSelect == 4){//Graphic display
            int k;
            for (k=0; k<30;k=k+5){
                gridArray[0] = anim_frames[k+4];
                gridArray[1] = anim_frames[k+3];
                gridArray[2] = anim_frames[k+2];
                gridArray[3] = anim_frames[k+1];
                gridArray[4] = anim_frames[k];
                char temp = gridArray[4];

                tx_data_str[0] = '%';
                tx_data_str[1] = '*';
                tx_data_str[2] = 'B';
                tx_data_str[3] = '0';
                tx_data_str[4] = '1';
                tx_data_str[5] = (gridArray[0] >> 4);
                tx_data_str[6] = (gridArray[0] & 0x0F);
                uart_write_string(0, 7);
                __delay_cycles(2000000);
            }
            scrollPos = 0;      //stand alone text scroll (mode 2)

        }
        if(ModeSelect < 1){
            ModeSelect = 2;//places Tech-Matrix into mode 2 if it ever falls into invalid mode #
        }
        // if(ModeSelect == 5){ // User text message Scroll

        // }
        // if(ModeSelect == 6){ // User Graphic Scroll 
            
        // }
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
    volatile char bytesIn[5];
    volatile int strBeg, strEnd,i,k,byteNum,byteRet,shiftNum,dispShift;
    volatile char tempChar;
    char cmpStr[2],byteOut[5],byteChar[5];
    cmpStr[0]='%';
    cmpStr[1]='*';
    strBeg = strComp(stringIn,&cmpStr,strLen+1,2);
    if (strBeg == 0xFFFF)
        return;
    cmpStr[0]=13;
    strEnd = strComp(stringIn,&cmpStr,strLen+2,1);
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
    if (stringIn[strBeg+2] == 'B'){     // Bytes of data to display
        // Save this one for later
        byteNum = twoCharConv (stringIn+strBeg+3);
        convNibble(stringIn+strBeg+5,byteOut,5);
        if (byteNum>5){     // Pass on the other info.
            stringOut[0] = stringIn[0+strBeg];
            stringOut[1] = stringIn[1+strBeg];
            stringOut[2] = stringIn[2+strBeg];
            num2Char2Digit(byteNum-5,stringIn+3);
            for (k = 0;k<byteNum-5;k++){
                stringOut[k+5] = stringIn[15+strBeg];
                stringOut[k+6] = stringIn[16+strBeg];
            }
        }
        if (byteNum > 5)
            byteNum =5;
        for (k = 0;k < byteNum; k++){ // step the existing grid to the side
            if (uartdir){
                gridArray[5-k] = gridArray[4-k];
            }
            else{
                gridArray[k] = gridArray[k+1];
            }
        }
        for (k = 0;k < byteNum; k++){   // move in the new data
            if (uartdir){
                gridArray[byteNum-k] = byteOut[k];
            }
            else{
                gridArray[k+byteNum] = byteOut[k];
            }
        }
        if (byteNum>5)  // Can't be bytenum if it's 5
            return byteNum*2+3;


    }

    if (stringIn[strBeg+2] == 'P'){  // Byte  "Push"
        int offsetnum = 5;
        byteNum = twoCharConv (stringIn+strBeg+3);
        if(byteNum/2<offsetnum){offsetnum = byteNum/2;}
        for(k=0;k<5;k++){
            stringOut[k] = stringIn[k];
        }
        shiftIn(strBeg+stringIn+offsetnum+5, strBeg+stringOut+5, offsetnum, uartdir);
        for(k=0;k<byteNum;k++){//if we ever do upstream, this code needs to be configured for it (currently downstream only)
            stringOut[strBeg+5+offsetnum] = ((gridArray[k] & 0xF0) >> 4) + 0x30;
            stringOut[strBeg+5+offsetnum] = ((gridArray[k] & 0x0F)) + 0x30;
        }
        for (k=0;k<byteNum/2;k++){
            bytesIn[k] = (stringIn[strBeg+k*2+5]-0x30)<<4+(stringIn[strBeg+k*2+5]-0x30);
        }
        shiftIn(bytesIn, gridArray,byteNum/2, uartdir);
        return strLen;

    }

    if (stringIn[strBeg+2] == 'C'){    // Characters of data to display
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
        int byteNum = stringIn[strBeg+3];   // number of bytes N
        int byteLoc = stringIn[strBeg+4];    // location of writning bits M
        int byteStart = stringIn[strBeg+5];  // location of first B
        volatile char k;
        int regStart;
        volatile int mode;

        int MAXCHARFLASH = 100;           // max amount of letters 100
        int MAXGRAPHFLASH = 150;        // max amount of graphic frames ,,1 frame = 5 bytes,,

        for (k = 0;k < byteNum;k++){
            myVec[k] = stringIn[strBeg+5+k];

        }

        FCTL2 = FWKEY + FSSEL0 +
                (FN1*16);             // MCLK/3 for Flash Timing Generator

        if (byteLoc == 0){
            regStart = regCHARStart;

        }

        else if (byteLoc == 1){
            regStart = regGRAPHStart;

        }

        mode = memory_mode(0,regStart,myVec,byteNum);//  Open memory and write (0) or read (1)
    }
    
    //        }
    //    else if (stringIn[strBeg+2] == 'R'){    // Read request
    //
    //        }
    //    else if (stringIn[strBeg+2] == 'W'){    // Response from read request
    //
    //        }

    if (stringIn[strBeg+2] == 'T'){    // Diagnostic Mode
        ModeSelect = 7; //Enter Test Mode
        if (stringIn[strBeg+3] == 'L'){ //Test LED Matrix
            //for(a=0;a<5;a++){
            //for(k=0;k<7;k++){
            test_sequence(1,4);
            // __delay_cycles(20000000);
            //}
            //}
        }
        if (stringIn[strBeg+3] == 'U'){//Test Uart Channels
            if(stringIn[strBeg+4] == 'U'){//Test Channel 1
                tx_data_str[0] = 'U';
                tx_data_str[1] = 'A';
                tx_data_str[2] = 'R';
                tx_data_str[3] = 'T';
                tx_data_str[4] = 'U';
                tx_data_str[5] = 'O';
                tx_data_str[6] = 'K';
                uart_write_string(0, 7);

            }
            if(stringIn[strBeg+4] == 'D'){//Test Channel 2
                tx_data_str[0] = 'U';
                tx_data_str[1] = 'A';
                tx_data_str[2] = 'R';
                tx_data_str[3] = 'T';
                tx_data_str[4] = 'D';
                tx_data_str[5] = 'O';
                tx_data_str[6] = 'K';
                uart_write_string_sw(0, 7);
            }
        }
        if (stringIn[strBeg+3] == 'S'){ //Test Switches}
            if(stringIn[strBeg+4] == 'L'){ //Test switch 1
                char SLcountTemp = num2str(swLFDiagCount, stringOut+1);
                tx_data_str[0] = SLcountTemp;
                uart_write_string(0,0);
                swLFDiagCount = 0; //reset the diagnostic switch counts

            }
            if(stringIn[strBeg+4] == 'C'){//Test switch 2
                num2str(swCNDiagCount, stringOut+1);
                swCNDiagCount = 0; //reset the diagnostic switch counts
                return 7;
            }
            if(stringIn[strBeg+4] == 'R'){//Test switch 3
                num2str(swRTDiagCount, stringOut+1);
                swRTDiagCount = 0; //reset the diagnostic switch counts
                return 7;
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
    // returns a value of where in the string the first occurrence happens
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

void num2str(int val, char * outStr){
    volatile int temp,prev;
    unsigned int divider=10000;
    volatile int n=1,z=0,neg=0;
    outStr[0]='0';
    if (val<0){
        neg=1;
        val=val*(-1);
        outStr[0]='-';
    }
    prev=0;
    for(n=1;n<6;n++){
        temp=(val-prev)/divider;


        outStr[n]=temp+0x30;
        prev=prev+(temp*divider);


        divider=divider/10;
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
    lockoutTimer ++;

    if (isrCount>4)
        isrCount=0;
    TA1CCR0 +=1000;                            // Add Offset to CCR0
    char checkstate[3];
    char checkchange[3];
    //checking the state and change of the buttons
    SwitchStateGet(checkstate, checkchange);//calls function in uart.c
    //button 1
    if(checkstate[0] == 0){//if button is being pressed, increment counter
        buttoncounter[0]++;
    }
    if(checkchange[0]){//if a change in button state has occurred, see if it went active or inactive
        if(checkstate[0] == 0){//button is inactive, reset counter
            buttoncounter[0] = 0;
        }
        else{
            buttoncounter[0]*=-1;//button is active make counter value negative for checking in the main loop
        }
    }
    //button 2
    if(checkstate[1] == 0){
        buttoncounter[1]++;
    }
    if(checkchange[1]){
        if(checkstate[1] == 0){
            buttoncounter[1] = 0;
        }
        else{
            buttoncounter[1]*=-1;
        }
    }

    //button 3
    if(checkstate[2] == 0){
        buttoncounter[2]++;
    }
    if(checkchange[2]){
        if(checkstate[2] == 0){
            buttoncounter[2] = 0;
        }
        else{
            buttoncounter[2]*=-1;
        }
    }
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
int memory_mode(int setparam, int regStart, char * inVec, int byteNum){

    //read\write ; what reg to start; inforvector; number of bytes;
    char *Flash_ptr;
    char *Bytenum_ptr;
    volatile char temp,k;

    Flash_ptr = (char *) regStart+1;
    ByteNum_ptr = (char *) regStart;
    *ByteNum_ptr = byteNum;
    if (setparam==0){   //Set the mode to active mode
        //write to memory this mode
        FCTL2 = FWKEY + FSSEL0 +
                (FN1*16);             // MCLK/3 for Flash Timing Generator
        FCTL1 = FWKEY + ERASE;                    // Set Erase bit
        FCTL3 = FWKEY;                            // Clear Lock bit
        *Flash_ptr = 1;                           // Dummy write to erase Flash segment

        FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
        *Flash_ptr=0xAA;
        for (k = 0;k<byteNum;k++){
            temp = inVec[k];
            *Flash_ptr = temp;
            temp = *Flash_ptr;
            Flash_ptr++;
        }
        FCTL1 = FWKEY;                            // Clear WRT bit
        FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
        return 0;
    }
    else if(setparam==1){      //Set mode to Actve mode
        // read from memory
       
        for (k=0;k<byteNum;k++){
            myVec[k] = *(Flash_ptr+k);

        }

        return 1;
    }
}
