#include <p24F16KA101.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//#define FCY 4000000UL
//#include <libpic30.h>

#include "xc.h"
#include "UART2.h"
#include "timer.h"
#include "ADC.h"
#include "CTMU.h"

void CTMUinit(){
    CTMUCONbits.CTMUEN = 0; //Disable, enable in do_CTMU
    CTMUCONbits.CTMUSIDL = 0; //Continue operation in idle mode
    CTMUCONbits.TGEN = 0; //Disable edge delay
    CTMUCONbits.EDGEN = 0; //Disable edges
    CTMUCONbits.EDGSEQEN = 0; //No edge sequence needed
    CTMUCONbits.IDISSEN = 0; //Do not discharge to ground.
    CTMUCONbits.CTTRIG = 0; //Trigger output disabled. Enable current sources in SW
    
    CTMUICONbits.ITRIM = 0b111010; //Nominal current output
    CTMUICONbits.IRNG = 0b10; //10X base current 
    
    CTMUCONbits.EDG1STAT = 0;
    CTMUCONbits.EDG2STAT = 1;
    
    ADC1_for_CTMU_init();
}

void ADC1_for_CTMU_init(){
    
    AD1CON1bits.ADON = 1; //Turn on ADC module
    
    //Read on AD11 (and only on AD5)
    AD1CSSLbits.CSSL0 = 0;
    AD1CSSLbits.CSSL1 = 0;
    AD1CSSLbits.CSSL2 = 0;
    AD1CSSLbits.CSSL3 = 0;
    AD1CSSLbits.CSSL4 = 0;
    AD1CSSLbits.CSSL5 = 0;
    AD1CSSLbits.CSSL10 = 0;
    AD1CSSLbits.CSSL11 = 0; //Omit from input scan
    AD1CSSLbits.CSSL12 = 0;
    
    AD1PCFGbits.PCFG11 = 0; //Analog read (set as analog pin)
    
    AD1CHSbits.CH0NB = 0;
    AD1CHSbits.CH0NA = 0; //Negative input is Vref-

    AD1CHSbits.CH0SB = 0b1011;
    AD1CHSbits.CH0SA = 0b1011; //Ch0 input is AN11/RB13
    
    AD1CON1bits.ADSIDL = 0; //Continue in idle
    AD1CON1bits.FORM = 0b00;
    AD1CON1bits.SSRC = 0b000; //Clearing SAMP bit starts conversion
    AD1CON1bits.ASAM = 0; //Start sampling when SAMP bit is set

    AD1CON2bits.VCFG = 0b000; //Vr+ = AVDD, Vr- = AVSS.
    AD1CON2bits.OFFCAL = 0;
    AD1CON2bits.CSCNA = 0; // Don't scan inputs
    AD1CON2bits.SMPI = 0b0000;
    AD1CON2bits.BUFM = 0; //Buffer configured as 16-bit word
    AD1CON2bits.ALTS = 0; //Always use MUXA as MUX

    AD1CON3bits.ADRC = 1; //So that it can run in sleep mode
    AD1CON3bits.SAMC = 0b11111;
    AD1CON3bits.ADCS = 0b11111;
    
    TRISBbits.TRISB13 = 1;

    //Interrupts
    IPC3bits.AD1IP =  0x5; //Priority       
    IFS0bits.AD1IF = 0; // clear interrupt flag
    //IEC0bits.AD1IE = 1; //enable
    IEC0bits.AD1IE = 0; //disable
        
}

void do_CTMU(){
    CTMUCONbits.CTMUEN = 1; //Disable, enable in do_CTMU
    
    AD1CON1bits.SAMP = 1;
    CTMUCONbits.EDG1STAT = 0;    
    CTMUCONbits.IDISSEN = 1;
    delay_ms(100);

    CTMUCONbits.IDISSEN = 0;    
    CTMUCONbits.EDG2STAT = 0;
    CTMUCONbits.EDG1STAT = 1;
    delay_ms(70);
    
    AD1CON1bits.SAMP = 0;
    while(AD1CON1bits.DONE == 0){}
    
    CTMUCONbits.EDG1STAT = 0;
    unsigned int raw = ADC1BUF0;
    float voltage = raw * 0.003173; //The coefficient is 3.25/1024
    printFloat(voltage);
    Disp2String("\n\r");

    //dtostrf( voltage, 1, 3, val );
    //sprintf(val, "%d\n\r", (int) voltage * 1000);
    //sprintf(val, "%.3f", voltage);
    //Disp2Hex(ADC1BUF0);
    //Disp2String(val);
    //AD1CON1bits.SAMP = 1;
    //AD1CON1bits.DONE = 0;
}

//based on do_CTMU, but the code is different enough not to be reusable.
float get_CTMU_result_with_delay(uint32_t delayTime){
    //This requires an 8MHz clock and a 32 bit timer
    CTMUCONbits.CTMUEN = 1;
    
    AD1CON1bits.SAMP = 1;
    CTMUCONbits.EDG1STAT = 0;
    CTMUCONbits.EDG2STAT = 0;    
    CTMUCONbits.IDISSEN = 1;
    delay_us_32(100000);

    CTMUCONbits.IDISSEN = 0;
    CTMUCONbits.EDG2STAT = 0;
    CTMUCONbits.EDG1STAT = 1;
    delay_us_32(delayTime);
    
    AD1CON1bits.SAMP = 0;
    while(AD1CON1bits.DONE == 0){}
    
    CTMUCONbits.EDG1STAT = 0;
    unsigned int raw = ADC1BUF0;
    
    CTMUCONbits.IDISSEN = 1; //Redundant but for protection, especially with larger caps.
    delay_us_32(100000);    
    
    return raw * 0.003173; //The coefficient is 3.25/1024
}

void printFloat(float val){
    //Courtesy of https://stackoverflow.com/questions/905928/using-floats-with-sprintf-in-embedded-c
    char str[20];

    char *tmpSign = (val < 0) ? "-" : "";
    float tmpVal = (val < 0) ? -val : val;

    int tmpInt1 = tmpVal;                  
    float tmpFrac = (tmpVal - tmpInt1) * 10000;
    int tmpInt2 = tmpFrac;

    // Print as parts, note that you need 0-padding for fractional bit.

    sprintf (str, "%s%d.%04d", tmpSign, tmpInt1, tmpInt2);
    Disp2String(str);
}

//Configure CTMU for SW control of current source
//Configure current sourcing pin as Analog Input for ADC
//Configure ADC sampling and time (previous project)
//Configure Timer as 32 bit or with prescaler to produce delays for different Capacitance values
//Turn on current source and timer
//Wait for timer to interrupt, Sample and measure Capacitor voltage using ADC
//Turn off Current and Discharge Capacitor using IDISSEN bit
//Compute Capacitance using 
//C = I(user-set) *  dT (user-set)/dV (measured)

