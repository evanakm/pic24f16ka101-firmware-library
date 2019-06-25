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
#include "SenseCapApp.h"

uint32_t timeOfInterrupt;

void SenseCapAppInit(){
    CTMUinit();
    
    //For the comparator
    //Priority
    //IPC4bits.CMIP =  0x5;
    
    //IFS1bits.CMIF = 0; // clear interrupt flag.
    //IEC1bits.CMIE = 1; // enable interrupt.
    
    //CM2CONbits.CPOL = 0;
    
    //CM2CONbits.CON = 1;
    //CM2CONbits.COE = 1;
    //CM2CONbits.CEVT = 0;
    //CM2CONbits.EVPOL = 0b10; //Trigger on high to low
   
    //CM2CONbits.CREF = 1;
    //CM2CONbits.CCH = 0b01;
    
    //CVRCONbits.CVREN = 1;
    //CVRCONbits.CVROE = 1;
    //CVRCONbits.CVRSS = 0;

    //CVREFinit(2.0);

    //For the touchscreen application    
/*    TRISAbits.TRISA0 = 1;
    TRISAbits.TRISA1 = 1;
    TRISAbits.TRISA2 = 1;
    TRISAbits.TRISA3 = 1;

    TRISBbits.TRISB0 = 1;
    TRISBbits.TRISB1 = 1;
    TRISBbits.TRISB12 = 1;
    TRISBbits.TRISB13 = 1;
    TRISBbits.TRISB14 = 1;
    
    AD1PCFGbits.PCFG0 = 0;
    AD1PCFGbits.PCFG1 = 0;
    AD1PCFGbits.PCFG2 = 0;
    AD1PCFGbits.PCFG3 = 0;
    AD1PCFGbits.PCFG4 = 0;
    AD1PCFGbits.PCFG5 = 0;
    AD1PCFGbits.PCFG10 = 0;
    AD1PCFGbits.PCFG11 = 0;
    AD1PCFGbits.PCFG12 = 0;
*/
}

uint8_t CalculateCapacitance(float *capResult){
    //This requires an 8MHz clock and 32 bit timer!!
    //Disp2String("Attempting to calculate capacitance on pin 16...\n\r");
    //delay_us_32(100000);
    
    *capResult = 0.0;
    
    if(SmallCap(capResult) == 1) return 1;
    if(MediumCap(capResult) == 1) return 1;
    if(BigCap(capResult) == 1) return 1;
        
    Disp2String("Either no capacitor attached, or too big to measure.\n\r");
    
    return 0;
    
}

int SmallCap(float* cap){

    float res;
    //float cap;    
    
    CTMUICONbits.IRNG = 0b01; //.55 uA
    
    res = get_CTMU_result_with_delay(600); //
    if(res >= 3.1){
        Disp2String("Capacitor is < 100pF\n\r");
        return 1;
    } else if (res >= 0.25){
        // nominally, cap = 6e-4 * 5.5e-7 / res; in units of farads
        *cap = 0.33 / res; //In units of nanometers
        Disp2String("Capacitor value is ");
        printFloat(*cap);
        Disp2String(" nF\n\r");
        //*cap = *cap * 1e-9;
        return 1;
    }

    //Disp2String("Debug point 1\n\r");
    
    res = get_CTMU_result_with_delay(7000); //
    if (res >= 0.25){
        // nominally, cap = 7e-3 * 5.5e-7 / res; in units of farads
        *cap = 3.85 / res; //In units of nanofarads
        Disp2String("Capacitor value is ");
        printFloat(*cap);
        Disp2String(" nF\n\r");
        //*cap = *cap * 1e-9;
        return 1;
    }
    
    res = get_CTMU_result_with_delay(80000); //
    if (res >= 0.25){
        // nominally, cap = 8e-2 * 5.5e-7 / res; in units of farads
        *cap = 44 / res; //In units of nanofarads
        Disp2String("Capacitor value is ");
        printFloat(*cap);
        Disp2String(" nF\n\r");
        //*cap = *cap * 1e-9;        
        return 1;        
    }
    
    return 0;
}

int MediumCap(float* cap){
    float res;
    //float cap;
    
    //Bigger capacitors so bump up the current
    CTMUICONbits.IRNG = 0b10; //5.5 uA
    
    res = get_CTMU_result_with_delay(90000); //
    if (res >= 0.25){
        // nominally, cap = 9e-2 * 5.5e-6 / res; in units of farads
        *cap = 495 / res; //In units of nanofarads
        Disp2String("Capacitor value is ");
        printFloat(*cap);
        Disp2String(" nF\n\r"); //May get a value slightly over 1000nF, no big deal that it doesn't display in uF
        //*cap = *cap * 1e-9;
        return 1;        
    }

    res = get_CTMU_result_with_delay(1000000); //
    if (res >= 0.25){
        // nominally, cap = 5.5e-6 / res; in units of farads
        *cap = 5.5 / res; //In units of microfarads
        Disp2String("Capacitor value is ");
        printFloat(*cap);
        Disp2String(" uF\n\r"); //May get a value slightly over 1000nF, no big deal that it doesn't display in uF
        //*cap = *cap * 0.000001;
        return 1;        
    }    
    return 0;
}

int BigCap(float* cap) {
    float res;
    //float cap;
    
    //Finding larger capacitors so bump up the current again
    CTMUICONbits.IRNG = 0b11; //55 uA
    
    CTMUCONbits.EDG2STAT = 0;
    CTMUCONbits.EDG1STAT = 0;  
    CTMUCONbits.IDISSEN = 1;
    LATBbits.LATB8 = 1;
    delay_us_32(2000000);    
    LATBbits.LATB8 = 0;
    CTMUCONbits.IDISSEN = 0;
    
    res = get_CTMU_result_with_delay(1250000); //
    if (res >= 0.25){
        // nominally, cap = 1.25 * 5.5e-5 / res; in units of farads
        *cap = 68.75 / res; //In units of microfarads
        Disp2String("Capacitor value is ");
        printFloat(*cap);
        Disp2String(" uF\n\r");
        //*cap = *cap * 0.000001;
        return 1;
    }    

    CTMUCONbits.EDG2STAT = 0;
    CTMUCONbits.EDG1STAT = 0;  
    CTMUCONbits.IDISSEN = 1;
    LATBbits.LATB8 = 1;
    delay_us_32(2000000);    
    LATBbits.LATB8 = 0;
    CTMUCONbits.IDISSEN = 0;
    
    res = get_CTMU_result_with_delay(16000000); //
    if (res >= 0.25){
        // nominally, cap = 16 * 5.5e-5 / res; in units of farads
        *cap = 880 / res; //In units of microfarads
        Disp2String("Capacitor value is ");
        printFloat(*cap);
        Disp2String(" uF\n\r");//May get a value slightly over 1000uF, no big deal that it doesn't display in mF
        //*cap = *cap * 0.000001;
        return 1;
    }
    
    return 0;
}

void ScanAllChannelsAndDetermineButtonPressed(){
    float max = 0.0;
    float temp;
    uint8_t res = 99;
                
    AD1CHSbits.CH0SA = 0b0000; //Pin2/AN0/RA0
    if(CalculateCapacitance(&temp)==1){
        if(temp > 0.0001){
            Disp2String("Sensing on pin 2\n\r");
            //max = temp;
            //res = 0;
        }
    }    
    AD1CHSbits.CH0SA = 0b0001; //Pin3/AN1/RA1
    if(CalculateCapacitance(&temp)==1){
        if(temp > 0.0001){
            Disp2String("Sensing on pin 3\n\r");
            //max = temp;
            //res = 1;            
        }
    }    
    AD1CHSbits.CH0SA = 0b0010; //Pin4/AN2/RB0
    if(CalculateCapacitance(&temp)==1){
        if(temp > 0.0001){
            Disp2String("Sensing on pin 4\n\r");
            //max = temp;
            //res = 2;            
        }
    }    
    AD1CHSbits.CH0SA = 0b0011; //Pin5/AN3/RB1
    if(CalculateCapacitance(&temp)==1){
        if(temp > 0.0001){
            Disp2String("Sensing on pin 5\n\r");
            //max = temp;
            //res = 3;            
        }
    }    
    AD1CHSbits.CH0SA = 0b0100; //Pin7/AN4/RB2
    if(CalculateCapacitance(&temp)==1){
        if(temp > 0.0001){
            Disp2String("Sensing on pin 7\n\r");
            //max = temp;
            //res = 4;            
        }
    }    
    AD1CHSbits.CH0SA = 0b0101; //Pin8/AN5/RB3
    if(CalculateCapacitance(&temp)==1){
        if(temp > 0.0001){
            Disp2String("Sensing on pin 8\n\r");
            //max = temp;
            //res = 5;            
        }
    }    
    AD1CHSbits.CH0SA = 0b1010; //Pin17/AN10/RB14
    if(CalculateCapacitance(&temp)==1){
        if(temp > 0.0001){
            Disp2String("Sensing on pin 17\n\r");            
            //max = temp;
            //res = 10;            
        }
    }    
    AD1CHSbits.CH0SA = 0b1011; //Pin16/AN11/RB13
    if(CalculateCapacitance(&temp)==1){
        if(temp > 0.0001){
            Disp2String("Sensing on pin 16\n\r");
            //max = temp;
            //res = 11;            
        }
    }    
    AD1CHSbits.CH0SA = 0b1100; //Pin15/AN12/RB12
    if(CalculateCapacitance(&temp)==1){
        if(temp > 0.0001){
            Disp2String("Sensing on pin 15\n\r");
            //max = temp;
            //res = 12;            
        }
    }
        
}

/*
//This goes in the CM interrupt
void ReadTimerOnInterrupt(){
    Disp2String("Interrupt triggered!!\n\r");
    timeOfInterrupt = 0;
    timeOfInterrupt += TMR2;
    timeOfInterrupt += ((uint32_t) TMR3 << 16);
    IEC1bits.CMIE = 0; // disable interrupt.    

}

void CalculateCapacitanceUsingCVREF(){
    //This requires an 8MHz clock and 32 bit timer!!
    Disp2String("Attempting to calculate capacitance on pin 16...\n\r");
    //delay_us_32(100000);
    
    CTMUCONbits.CTMUEN = 1;
    AD1CON1bits.SAMP = 1;

    CTMUCONbits.IDISSEN = 0;
    
    IEC1bits.CMIE = 1; // enable interrupt.    
    
    float res;

    StopAndDrainCurrent();
    
    timeOfInterrupt = 0;
    
    Disp2String("Running small current\n\r");
    RunSmallCurrent();
    delay_us(5000000);
    if(timeOfInterrupt != 0){
        //This means it did not time out
        res = 5.5e-4 / 3.0;
        Disp2String("Capacitance = ");
        printFloat(res);
        Disp2String(" mF\n\r");
        return;
    }
    
    StopAndDrainCurrent();

    Disp2String("Running medium current\n\r");
    RunMediumCurrent();
    delay_us(5000000);
    if(timeOfInterrupt != 0){
        //This means it did not time out
        res = 5.5e-3 / 3.0;
        Disp2String("Capacitance = ");
        printFloat(res);
        Disp2String(" mF\n\r");
        return;
    }

    StopAndDrainCurrent();

    Disp2String("Running big current\n\r");
    RunBigCurrent();
    delay_us(5000000);
    if(timeOfInterrupt != 0){
        //This means it did not time out
        res = 5.5e-2 / 2.0;
        Disp2String("Capacitance = ");
        printFloat(res);
        Disp2String(" mF\n\r");
        return;
    }
    
    Disp2String("Either no capacitor attached, or too big to measure.\n\r");

    
}

void StopAndDrainCurrent() {
    CTMUCONbits.EDG2STAT = 0;
    CTMUCONbits.EDG1STAT = 0;  
    CTMUCONbits.IDISSEN = 1;
    LATBbits.LATB8 = 1;
    delay_us_32(2000000); //Discharge pin
    LATBbits.LATB8 = 0;
    CTMUCONbits.IDISSEN = 0;    
}

void RunSmallCurrent() {
    //Finding larger capacitors so bump up the current again
    CTMUICONbits.IRNG = 0b01; //.55 uA
    
    CTMUCONbits.EDG2STAT = 0;
    CTMUCONbits.EDG1STAT = 1;    
}

void RunMediumCurrent() {
    //Finding larger capacitors so bump up the current again
    CTMUICONbits.IRNG = 0b10; //5.5 uA
    
    CTMUCONbits.EDG2STAT = 0;
    CTMUCONbits.EDG1STAT = 1;    
}

void RunBigCurrent() {
    //Finding larger capacitors so bump up the current again
    CTMUICONbits.IRNG = 0b11; //.55 uA
    
    CTMUCONbits.EDG2STAT = 0;
    CTMUCONbits.EDG1STAT = 1;    
}
*/
