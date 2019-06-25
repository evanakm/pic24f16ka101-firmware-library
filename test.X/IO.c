/*
 * File:   IO.c
 * Author: Owner
 *
 * Created on September 24, 2018, 5:55 PM
 */


#include <p24F16KA101.h>

#include "xc.h"
#include "timer.h"
#include "UART2.h"
//#include "IR.h"
//#include "IR_Rx.h"
#include "Comparator.h"


unsigned int * CN_flag; //Needs to be accessible from anything that #includes IO.h

void CN_check(void){
    
    //some experimentation shows 100 ms is a good interval to allow it to
    //recognize that both buttons have been pressed without an overly noticeable delay.
    delay_ms(1000);         
    //delay_us_32(100000);
    
    if(*CN_flag == 1){
        if (PORTAbits.RA4==0 && PORTBbits.RB4==0) {
            Disp2String("A4 was pushed. B4 was pushed!\n\r");
        } else if(PORTAbits.RA4==0 && PORTBbits.RB4==1){
            Disp2String("A4 was pushed!\n\r");        
        } else if(PORTAbits.RA4==1 && PORTBbits.RB4==0){
            Disp2String("B4 was pushed!\n\r");
        }
    }
    
    return;
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void){
	IFS1bits.CNIF = 0; // clear IF flag
    IEC1bits.CNIE = 0;
    
    if(PORTBbits.RB4==0 || PORTAbits.RA4==0)   
    //if(PORTBbits.RB2==0 || PORTBbits.RB2==1)   
    {
        *CN_flag = *CN_flag + 1; //Part of debouncing... CN_check only displays the first time.
        
        // THIS IS WHERE TO CALL THE APPROPRIATE FUNCTIONS...
        CN_check_Divider();
        //CN_check();
    }
    else{
        //Part of debouncing... reset when the buttons are released
        
        //Uncomment the next line when debugging.
        //Disp2String("Buttons released.");
        *CN_flag = 0;
    }
        
    IEC1bits.CNIE = 1;
	Nop();
	
}

void CN_init(unsigned int *CNFptr){
    CN_flag = CNFptr;
    
    TRISBbits.TRISB4 = 1; //reading from B4 (button)
    TRISAbits.TRISA4 = 1; //reading from A4 (button)
    
    //Priority
    IPC4bits.CNIP = 0b111;
    
    IFS1bits.CNIF = 0; // clear the interrupt flag.
    IEC1bits.CNIE = 1; // enable interrupt
    CNEN1bits.CN0IE = 1;
    CNEN1bits.CN1IE = 1;
    
    //A4 = CN0
    CNPU1bits.CN1PUE = 1; 
    CNPU1bits.CN0PUE = 1;
}

void CN_init_for_Rx(unsigned int *CNFptr){
    CN_flag = CNFptr;
    
    TRISBbits.TRISB2 = 1; //reading from B2 (Receiver)
    
    //Priority
    IPC4bits.CNIP = 0b110;
    
    IFS1bits.CNIF = 0; // clear the interrupt flag.
    IEC1bits.CNIE = 1; // enable interrupt
    CNEN1bits.CN6IE = 1;
    
    //A4 = CN0
    //CNPU1bits.CN1PUE = 1;
    //CNPU1bits.CN0PUE = 1;
}

