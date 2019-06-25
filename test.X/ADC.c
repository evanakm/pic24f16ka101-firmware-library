#include <p24F16KA101.h>

//#define FCY 4000000UL
//#include <libpic30.h>

#include "xc.h"
#include "UART2.h"
#include "timer.h"
#include "ADC.h"

///// ADC interrupt subroutine
void __attribute__((interrupt, no_auto_psv)) _ADC1Interrupt(void)
{
    //Disp2String("Debug point 1.1.1\n\r");
    //printOutGraphDecimal(ADC1BUF0);
    printOutGraphBinary(ADC1BUF0);
    IFS0bits.AD1IF = 0;      // Clear the ADC1 Interrupt Flag
}

void ADC1_init(){
    
    //Read on AD5 (and only on AD5)
    AD1CSSLbits.CSSL5 = 1; //irrelevant since CSCNA = 0
    AD1CSSLbits.CSSL0 = 0;
    AD1CSSLbits.CSSL1 = 0;
    AD1CSSLbits.CSSL2 = 0;
    AD1CSSLbits.CSSL3 = 0;
    AD1CSSLbits.CSSL4 = 0;
    AD1CSSLbits.CSSL10 = 0;
    AD1CSSLbits.CSSL11 = 0;
    AD1CSSLbits.CSSL12 = 0;
    
    AD1PCFGbits.PCFG5 = 0; //Analog read
    
    AD1CHSbits.CH0NB = 0;
    AD1CHSbits.CH0NA = 0;

    AD1CHSbits.CH0SB = 0b0101;
    AD1CHSbits.CH0SA = 0b0101;
    
    AD1CON2bits.VCFG = 0b000;
    AD1CON2bits.OFFCAL = 0;
    AD1CON2bits.CSCNA = 0;
    AD1CON2bits.SMPI = 0b0000;
    AD1CON2bits.BUFM = 0;
    AD1CON2bits.ALTS = 0;   
    
    AD1CON1bits.FORM = 0b00;
    AD1CON1bits.SSRC = 0b111;
    AD1CON1bits.ASAM = 0;
    
    AD1CON3bits.ADRC = 1; //So that it can run in sleep mode
    AD1CON3bits.SAMC = 0b11111;
    AD1CON3bits.ADCS = 0b00000;
    
    TRISAbits.TRISA3 = 1;

    //Interrupts
    IPC3bits.AD1IP =  0x5; //Priority       
    IFS0bits.AD1IF = 0; // clear interrupt flag
    IEC0bits.AD1IE = 1; //enable
    //IEC0bits.AD1IE = 0;
    
    AD1CON1bits.SAMP = 0;
    AD1CON1bits.DONE = 0;
    
    AD1CON1bits.ADON = 1;

}

void do_ADC(void){
    AD1CON1bits.SAMP = 1; //Start sampling
}

void printOutGraph(unsigned int value){
    // Let's use 32 bins to represent the graph.
    
    // Since this is a 10-bit integer, we divide by 2^5
    // to get the 5 MSB.
    unsigned int fourMSB = value >> 5;

    char bar[34];
    bar[0] = '|';
    bar[33] = '\0';
    
    int i = 0;
    for(i = 0; i<32; i++){
        if(i < fourMSB){
            bar[i+1] = '-';
        }else if(i == fourMSB){
            bar[i+1] = '|'; 
        }
        else{
            bar[i+1] = '\0';
        }
    }
    
    Disp2String(bar);
}

void printOutGraphDecimal(unsigned int value){
    char val[5];
    sprintf(val, "%d", value);
    
    printOutGraph(value);

    Disp2String(val);
    Disp2String("\n\r"); 
}

void printBits(uint8_t lsbToPrint, unsigned int value)
{
    if(lsbToPrint > 16) return;
    char temp[17];
    
    int i;
    for(i = 0; i< lsbToPrint; i++){
        if(((value >> (lsbToPrint - 1 - i)) & 0x1) == 0)
            temp[i] = '0';
        else
            temp[i] = '1';
    }
    temp[i] = '\0';
    Disp2String(temp);
}

void printOutGraphBinary(unsigned int value){
    //char val[5];
    //sprintf(val, "%b", value);
    
    printOutGraph(value);

    Disp2String("0b");
    printBits(10, value);
    //Disp2String(val);
    Disp2String("\n\r"); 
}

void printOutGraphHex(unsigned int value){
    char val[5];
    sprintf(val, "%x", value);
    
    printOutGraph(value);

    Disp2String("0x");
    Disp2String(val);
    Disp2String("\n\r"); 
}
