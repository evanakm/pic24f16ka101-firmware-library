/* 
 * File:   main.c
 * Author: Evan Meikleham, based on code by Rushi Vyas
 *
 */


#include "xc.h"
#include <p24fxxxx.h>
#include <p24F16KA101.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>

#define FCY 4000000UL
#include <libpic30.h>

#include "ChangeClk.h"
#include "UART2.h"
#include "Timer.h"
//#include "IO.h"
//#include "IR.h"
//#include "IR_Rx.h"
#include "Comparator.h"
//#include "ADC.h"
//#include "CTMU.h"
//#include "SenseCapApp.h"

//// CONFIGURATION BITS ////

// Code protection 
#pragma config BSS = OFF // Boot segment code protect disabled
#pragma config BWRP = OFF // Boot sengment flash write protection off
#pragma config GCP = OFF // general segment code protecion off
#pragma config GWRP = OFF

// CLOCK CONTROL 
#pragma config IESO = OFF    // 2 Speed Startup disabled
#pragma config FNOSC = FRC  // Start up CLK = 8 MHz
#pragma config FCKSM = CSECMD // Clock switching is enabled, clock monitor disabled
#pragma config SOSCSEL = SOSCLP // Secondary oscillator for Low Power Operation
#pragma config POSCFREQ = MS  //Primary Oscillator/External clk freq betwn 100kHz and 8 MHz. Options: LS, MS, HS
#pragma config OSCIOFNC = ON  //CLKO output disabled on pin 8, use as IO. 
#pragma config POSCMOD = NONE  // Primary oscillator mode is disabled

// WDT
#pragma config FWDTEN = OFF // WDT is off
#pragma config WINDIS = OFF // STANDARD WDT/. Applicable if WDT is on
#pragma config FWPSA = PR32 // WDT is selected uses prescaler of 32
#pragma config WDTPS = PS1 // WDT postscler is 1 if WDT selected

//MCLR/RA5 CONTROL
#pragma config MCLRE = OFF // RA5 pin configured as input, MCLR reset on RA5 diabled

//BOR  - FPOR Register
#pragma config BORV = LPBOR // LPBOR value=2V is BOR enabled
#pragma config BOREN = BOR0 // BOR controlled using SBOREN bit
#pragma config PWRTEN = OFF // Powerup timer disabled
#pragma config I2C1SEL = PRI // Default location for SCL1/SDA1 pin

//JTAG FICD Register
#pragma config BKBUG = OFF // Background Debugger functions disabled
#pragma config ICS = PGx2 // PGC2 (pin2) & PGD2 (pin3) are used to connect PICKIT3 debugger

// Deep Sleep RTCC WDT
#pragma config DSWDTEN = OFF // Deep Sleep WDT is disabled
#pragma config DSBOREN = OFF // Deep Sleep BOR is disabled
#pragma config RTCOSC = LPRC// RTCC uses LPRC 32kHz for clock
#pragma config DSWDTOSC = LPRC // DeepSleep WDT uses Lo Power RC clk
#pragma config DSWDTPS = DSWDTPS7 // DSWDT postscaler set to 32768 


// GLOBAL VARIABLES
unsigned int temp;
unsigned int i;


// MACROS
#define Nop() {__asm__ volatile ("nop");}
#define ClrWdt() {__asm__ volatile ("clrwdt");}
#define Sleep() {__asm__ volatile ("pwrsav #0");}   // set sleep mode
#define Idle() {__asm__ volatile ("pwrsav #1");}
#define dsen() {__asm__ volatile ("BSET DSCON, #15");}



/*
 * 
 */

unsigned int CNflag = 0;
        
int main(void) {
     //Clock output on REFO
     TRISBbits.TRISB15 = 0;  // Set RB15 as output for REFO
     REFOCONbits.ROEN = 1; // Ref oscillator is enabled
     REFOCONbits.ROSSLP = 0; // Ref oscillator is disabled in sleep
     REFOCONbits.ROSEL = 0; // Output base clk showing clock switching
     REFOCONbits.RODIV = 0b0000;
     
     // Switch clock: 32 for 32kHz, 500 for 500 kHz, 8 for 8MHz 
     NewClk(32); //
     OSCTUNbits.TUN = 0b000000;
     
     // LED on RB7
     //TRISBbits.TRISB4 = 0; // set RB7 as output for LED
     //LATBbits.LATB8 = 1;
     
    //TRISBbits.TRISB8 = 0;    // Sets RB8 as output
    
    //ChannelChanger_Init(&CNflag);
    //CN_init(&CNflag);
    //Receiver_Init(&CNflag);
    //CN_init_for_Rx(&CNflag);
            
    //TRISBbits.TRISB9 = 0;    // Sets RB8 as output
     
    //CVRCONbits.CVREN = 1;
    //CVRCONbits.CVROE = 1;
    //CVRCONbits.CVRSS = 0;

    //CN_init(&CNflag);
    
    //CM_init();
    //TRISBbits.TRISB13 = 0; // Sets RB13 as output

    //Divider_Init(&CNflag);
    //CVREFinit(1.8);
    
    //ADC1_init();
    
    //CTMUinit();
    //ADC1_for_CTMU_init();
     
    //SenseCapAppInit();
    //TRISBbits.TRISB8 = 0;
    
    //float capResult = 0.0;
    //uint8_t flag = 0;
     
    Final_Exam_CM_init();
    
    while(1)
    {
        //delay_ms(1000);
        //Disp2String("Blink\n\r");
        //delay_ms(1000);
        //Disp2String("Initialized.\n\r");
        //delay_ms(1000);
        Idle();
    }
    return 0;
}
