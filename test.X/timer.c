/*
 * File:   timer.c
 * Author: Owner
 *
 * Created on September 15, 2018, 3:16 PM
 */


#include <p24F16KA101.h>

#include "xc.h"
#include "ChangeClk.h"
#include "UART2.h"
#include "IR_Rx.h"
#include <math.h>

// Timer 2 interrupt subroutine
 void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void)
 {
    //Disp2String("T2 Timeout interrupt triggered");
    IFS0bits.T2IF=0; //Clear timer 2 interrupt flag
    T2CONbits.TON=0;
    // TMR2flag = 1;
 }

 // Timer 3 interrupt subroutine
 void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void)
 {
    //Disp2String("T3 Timeout interrupt triggered");
    //Reset_Counter();
    IFS0bits.T3IF=0; //Clear timer 3 interrupt flag
    T2CONbits.TON=0;
    // TMR2flag = 1;
 }
 
void Init_Timer_16bit() {
    T2CONbits.TCS = 0;
    T2CONbits.T32 = 0;
    T2CONbits.TSIDL = 0; //For simplicity
    // T2CONbits.TGATE = 0;
    // T2CONbits.TCKPS = 0b00;
    IEC0bits.T2IE = 1;
    IPC1bits.T2IP = 0b111;
} 

void Init_Timer_32bit() {
    T2CONbits.T32 = 1;

    T3CONbits.TCS = 0;
    T3CONbits.TSIDL = 0; //For simplicity
    
    T2CONbits.TCS = 0;
    T2CONbits.TSIDL = 0; //For simplicity

    // T2CONbits.TGATE = 0;
    // T2CONbits.TCKPS = 0b00;
    IEC0bits.T3IE = 1;
    IPC2bits.T3IP = 0b111;
} 

void delay_ms(int time_ms){
    if(time_ms<1) return;
    
    Init_Timer_16bit();    
    
    PR2 = (16 * time_ms);
    T2CONbits.TON = 1;
    Idle();
}

//void delay_us(uint32_t time_us){
void delay_us(int time_us){
    if(time_us<1) return;
    
    Init_Timer_16bit();
    
    // NewClk(8); // Can't get it to work properly if I include here, so done in main
    T2CONbits.TCKPS = 0b00; // divide by 1
    
    PR2 = (4 * time_us);
    T2CONbits.TON = 1;
    Idle();
    
}

void delay_us_32(uint32_t time_us){
    if(time_us<1) return;
    
    Init_Timer_32bit();
    
    T2CONbits.TCKPS = 0b00; // divide by 1
    
    // MSW into PR3
    uint32_t msw = (4 * time_us) >> 16;
    msw = msw & 0x0000ffff;
    uint32_t lsw = (4 * time_us) & 0x0000ffff;
        
    PR2 = lsw;
    PR3 = msw;
    T2CONbits.TON = 1;
    Idle();
    
}

