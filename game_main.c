#include <msp430.h> 
#include "serial_handler.h"
#include "main.h"

//#include <stdint.h>

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

//Mode 1: Catch data from upstream device
//Mode 2: Stand alone text scroll mode
//Mode 3: Graphic display
//Mode 4: Game
//Mode 5: User created text scroll
//Mode 6: User created graphic display
//Mode 7: Test mode (for assembly and troubleshooting)

char gridArray[5] = {0x7F, 0x04, 0x18, 0x04, 0x7F};
int isrCount = 0, scrollCount = 0;
#define LOCKOUT_DURATION 5000 // Lockout duration in milliseconds
int lockoutTimer = 0;
int ModeSelect = 2;// sets the TechMatrix to the stand alone Text scroll on startup
int playerpos = 0;
#define ModeSelect_Duration 20000 // Timer duration for changing mode (press SW2)
int ModeTimer = 0;


int main(void) {
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer
    led_init();
    switch_init();
    timer_ISR_init();
    uart_init(4);
    //uint8_t c;
    int a_val, k_val, scrollPos=0, scrollNum, scrollMax, cat_val;
    char disp_string[] = "   Welcome to the TechMatrix! Go to mtech.edu/electrical-engineering/ today!  |";//Green
    char two_char[11] = {0,0,0,0,0,0,0,0,0,0,0};
    char anim_frames[] = {0x01, 0x7F, 0x40, 0x40 ,0x7F, 0x01, 0x01, 0x7F, 0x40, 0x40, 0x7F, 0x01,0x01,0x7F,0x40,0x40,0x7F,0x01,0x01,0x7F,0x40,0x40,0x7F,0x01,0x01,0x7F,0x40,0x40,0x7F,0x01};
    char game_layout[] = {0x00,0x00,0x00,0x00,0x67,0x00,0x00,0x00,0x4F,0x00,0x00,0x00,0x73,0x00,0x00,0x00,0x7C,0x00,0x00,0x00,0x4F,0x00,0x00,0x00,0x73,0x00,0x00,0x00,0x7C,0x00,0x00,0x00};
    char game_safezone[] = {0x10,0x20,0x40,0x04,0x02,0x01,0x08,0x77,0x6F,0x5F,0x7B,0x7E,0x7D};
    int game_spd = 6;
    char scrollLast=0;
    a_val = 0;
    while (scrollNum != '|')
        scrollNum = disp_string[a_val++];
    scrollNum=a_val;
    scrollMax = scrollNum*6;

    while (1) {
        if (eos_flag) {

            ModeSelect = 1;
            //Reset lockout timer
            lockoutTimer = 0;
            // Controlled mode (catch)
            __delay_cycles(100000);
            rx_flag = 0;
            eos_flag = 0;
            char temp = gridArray[0];
            temp |= BIT7;
            gridArray[0] = gridArray[1];
            gridArray[1] = gridArray[2];
            gridArray[2] = gridArray[3];
            gridArray[3] = gridArray[4];
            cat_val = rx_data_str[5] & 0x0F;
            cat_val <<= 4;
            cat_val |= (rx_data_str[6] & 0x0F);
            gridArray[4] = cat_val;

            tx_data_str[0] = '%';
            tx_data_str[1] = '*';
            tx_data_str[2] = 'B';
            tx_data_str[3] = '0';
            tx_data_str[4] = '1';
            tx_data_str[5] = 0x30 + (temp >> 4);
            tx_data_str[6] = 0x30 + (temp & 0x0F);
            uart_write_string(0, 7);
            scrollPos = 0;//reset stand alone text scroll (mode 2)
        }
        if (lockoutTimer > LOCKOUT_DURATION) {
            lockoutTimer = LOCKOUT_DURATION+1;
            if(ModeSelect == 1){
                ModeSelect = 2;// if device is still in Mode 1 without receiving new data, put TechMatrix into Mode 2
            }
        }
        if (ModeSelect == 2){
            // Stand alone text scroll mode (send)
            if (scrollCount >= 2000) {
                scrollCount = 0;
                a_val = scrollPos / 6;
                charMap(disp_string[a_val], two_char);
                a_val = scrollPos/6;
                charMap(disp_string[a_val + 1], two_char + 6);
                a_val = scrollPos%6;
                tx_data_str[0] = '%';
                tx_data_str[1] = '*';
                tx_data_str[2] = 'B';
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
        if (ModeSelect == 3){//Graphic display
            unsigned int k;
            for (k=0; k<30;k=k+5){
                gridArray[0] = anim_frames[k];
                gridArray[1] = anim_frames[k+1];
                gridArray[2] = anim_frames[k+2];
                gridArray[3] = anim_frames[k+3];
                gridArray[4] = anim_frames[k+4];
                __delay_cycles(2000000);
            }
            scrollPos = 0;      //stand alone text scroll (mode 2)

        }
        if (ModeSelect == 4){// Game
            unsigned int g;
            unsigned int g2;
            unsigned int s;
            int player;
            int gameover;
            int game_scroll = 0;
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
                if (scrollPos > (6*sizeof(game_layout)-24)){
                    scrollPos = 0;
                    g = 0;
                    game_spd--;
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
            ModeSelect = 4;
        }
    }
}

void charMap(int valin, char * mapOut){

    // needs
    int k, n;
    char tempval, holder;
    for (k = 0; k < 5; k++) {
        holder = mapArray[valin - 0x20][k];
        tempval = 0;
        for (n = 0; n < 8; n++) {
            tempval <<= 1;
            if (holder & (1 << n)) {
                tempval += 1;
            }
        }
        tempval >>= 1;
        mapOut[k] = tempval;
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

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    if (P1IFG & BIT0){
        if (lockoutTimer < LOCKOUT_DURATION){
            ModeSelect = 1;
        }
        else{
            ModeTimer ++;
            ModeSelect ++;
            if (ModeSelect > 4){
                ModeSelect = 2;
            }
        }

        P1IFG &= ~BIT0;                           // P1.0 IFG cleared
    }
    if (P1IFG & BIT3){
        playerpos --;
        if (playerpos < -3){
            playerpos = -3;
        }
        P1IFG &= ~BIT3;                           // P1.4 IFG cleared
        while (P1IFG & BIT3)
           {
               _delay_cycles(40000);
           }
    }
}



// Port 1 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    if (P2IFG & BIT6){
        playerpos ++;
        if (playerpos > 3){
            playerpos = 3;
        }
        P2IFG &= ~BIT6;                           // P1.0 IFG cleared
        while (P2IFG & BIT6)
        {
            _delay_cycles(40000);
        }
    }

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
    lockoutTimer ++;
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
