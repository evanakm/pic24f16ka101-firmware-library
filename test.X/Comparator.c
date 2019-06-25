#include <p24F16KA101.h>

//#define FCY 4000000UL
//#include <libpic30.h>

#include "xc.h"
#include "UART2.h"
#include "timer.h"
#include "Comparator.h"
//#include "SenseCapApp.h"

#define V_SRC 3.25

char num[10];

unsigned int * CNflagPtr_Divider;

uint8_t zeroCrossings = 0;
uint8_t frequencyDivider = 5;
uint8_t frequencyDividerMax = 10;
uint8_t NN = 1;

void __attribute__((interrupt, no_auto_psv)) _CompInterrupt(void)
{
    PrintStatusOnChange();
    
    IFS1bits.CMIF = 0;		// clear IF flag
    CM1CONbits.CEVT=0; // Interrupts disabled till this bit is cleared
    CM2CONbits.CEVT=0; // Interrupts disabled till this bit is cleared
    Nop();

}

void PrintStatusOnChange(){
    delay_ms(200); //debouncing
    if(CM1CONbits.CEVT == 1){
        if(CM1CONbits.COUT == 0){
            Disp2String("OVER-CHARGE\n\r");
        }
        else{
            Disp2String("NORMAL CHARGE\n\r");            
        }
    }
    else if(CM2CONbits.CEVT == 1){
        if(CM2CONbits.COUT == 1){
            Disp2String("UNDER-CHARGE\n\r");
        }
        else{
            Disp2String("NORMAL CHARGE\n\r");            
        }    
    }
    CM1CONbits.CEVT=0;
    CM2CONbits.CEVT=0;
}

void SwitchOnZeroCrossings(){
    
    delay_ms(200); //debouncing
    zeroCrossings = ( zeroCrossings + 1 ) % frequencyDivider;
    if(zeroCrossings == 0){
        LATBbits.LATB13 = (1 - LATBbits.LATB13);            
    }
    
}

//Interrupts
void CM_init(){
    
    //Priority
    IPC4bits.CMIP =  0x5;
    
    IFS1bits.CMIF = 0; // clear interrupt flag.
    IEC1bits.CMIE = 1; // enable interrupt.
    
    CM2CONbits.CPOL = 0;
    
    CM2CONbits.CON = 1;
    CM2CONbits.COE = 1;
    CM2CONbits.CEVT = 0;
    CM2CONbits.EVPOL = 0b11; //Trigger on both zero crossings
   
    CM2CONbits.CREF = 1;
    CM2CONbits.CCH = 0b01;
}

void Final_Exam_CM_init(){
    
    TRISAbits.TRISA2 = 1;

    TRISAbits.TRISA3 = 1;
    TRISBbits.TRISB1 = 1;
    
    //Priority
    IPC4bits.CMIP =  0x5;
    
    IFS1bits.CMIF = 0; // clear interrupt flag.
    IEC1bits.CMIE = 1; // enable interrupt.
    
    CM1CONbits.CPOL = 0; //Do not invert
    CM2CONbits.CPOL = 0; //Do not invert
    
    CM1CONbits.CON = 1;
    CM1CONbits.COE = 1; //for debugging
    CM1CONbits.CEVT = 0;
    CM1CONbits.EVPOL = 0b11; //Trigger on both zero crossings

    CM2CONbits.CON = 1;
    CM2CONbits.COE = 1; //for debugging
    CM2CONbits.CEVT = 0;
    CM2CONbits.EVPOL = 0b11; //Trigger on both zero crossings    

    CM1CONbits.CREF = 1; //Externally reference
    CM1CONbits.CCH = 0b00; //C1INB

    CM2CONbits.CREF = 1; //Externally reference
    CM2CONbits.CCH = 0b10; //C2IND
    
    //CM2CONbits.CREF = 1;
    //CM2CONbits.CCH = 0b01;
}

void CVREFinit(float vref){
    if (vref < 0.0) return;
    if (vref > V_SRC) return;
    
    /*sprintf(num, "%1.3f", vref);
    Disp2String("Inside CVREFinit. Requesting v = ");
    Disp2String(num);
    Disp2String("\n\r");*/
    
    
    // if vref is above 0.719 * V_SRC, the output will be 0.719* V_SRC
    if (vref > V_SRC * 0.719){
        CVRCONbits.CVRR = 0;
        CVRCONbits.CVR = 0xf;
        
        sprintf(num, "%d", CVRCONbits.CVR);
        Disp2String("CVR = ");
        Disp2String(num);
        Disp2String("; ");

        sprintf(num, "%d", CVRCONbits.CVRR);
        Disp2String("CVRR = ");
        Disp2String(num);
        Disp2String("\n\r");
        
        return;
    }
    
    float quarter_vsrc = 0.25*V_SRC;
    float half_vsrc = 0.50*V_SRC;
    float sixtyfourth_vsrc = V_SRC/64.0;

/*    sprintf(num, "%1.3f", quarter_vsrc);
    Disp2String("Inside CVREFinit. quarter_vsrc = ");
    Disp2String(num);
    Disp2String("\n\r");

    sprintf(num, "%1.3f", half_vsrc);
    Disp2String("Inside CVREFinit. half_vsrc = ");
    Disp2String(num);
    Disp2String("\n\r");

    sprintf(num, "%1.3f", sixtyfourth_vsrc);
    Disp2String("Inside CVREFinit. sixtyfourth_vsrc = ");
    Disp2String(num);
    Disp2String("\n\r");
*/    
    
    uint16_t i;
    //Disp2String("Debug Point 0.\n\r");
    
    if (vref > quarter_vsrc){

        //Disp2String("Debug Point 1.\n\r");
        
        CVRCONbits.CVRR = 0;
        
        //add the (1/64) V_SRC so that the intervals are centred at the outputs.
        for (i = 0; i < 16; i++ ){
            if (vref < (quarter_vsrc + (i / 16.0) * half_vsrc) + sixtyfourth_vsrc){
                CVRCONbits.CVR = i;

                sprintf(num, "%d", CVRCONbits.CVR);
                Disp2String("CVR = ");
                Disp2String(num);
                Disp2String("; ");

                sprintf(num, "%d", CVRCONbits.CVRR);
                Disp2String("CVRR = ");
                Disp2String(num);
                Disp2String("\n\r");
                
                return;
            }
        }
        
        // Should in theory not reach this point, but something may slip through
        // the cracks due to roundoff error.
        
        CVRCONbits.CVR = 15;

        sprintf(num, "%d", CVRCONbits.CVR);
        Disp2String("CVR = ");
        Disp2String(num);
        Disp2String("; ");

        sprintf(num, "%d", CVRCONbits.CVRR);
        Disp2String("CVRR = ");
        Disp2String(num);
        Disp2String("\n\r");

        return;

    }

    CVRCONbits.CVRR = 1;
    float two_thirds_vsrc = V_SRC * 2.0 / 3.0;
    float fortyeighth_vsrc = V_SRC/48.0;

    //Disp2String("Debug Point 2.\n\r");
    
    // Any case above 0.25 * V_SRC should haave been covered by the above if-block.
    // We only theoretically need to check until i = 3, but we check i = 4
    // in the corner case where one of the above checks fails because of
    // roundoff error.
    for (i = 0; i < 15; i++ ){
        //sprintf(num, "%d", i);
        //Disp2String(num);
        if (vref < (i / 16.0) * two_thirds_vsrc + fortyeighth_vsrc){
            CVRCONbits.CVR = i;
            
            sprintf(num, "%d", CVRCONbits.CVR);
            Disp2String("CVR = ");
            Disp2String(num);
            Disp2String("; ");

            sprintf(num, "%d", CVRCONbits.CVRR);
            Disp2String("CVRR = ");
            Disp2String(num);
            Disp2String("\n\r");
                        
            return;
        }
    }    
    
}

void Divider_Init(unsigned int *CNFptr){
    CNflagPtr_Divider = CNFptr; //For debouncing.
}


void CN_check_Divider(void){
    
    //some experimentation shows 100 ms is a good interval to allow it to
    //recognize that both buttons have been pressed without an overly noticeable delay.
    delay_ms(100);         
    //delay_us_32(100000);
    
    if(*CNflagPtr_Divider == 1){
        if (PORTAbits.RA4==0 && PORTBbits.RB4==0) {
            Disp2String("Both buttons pushed. Press only one.\n\r");
        } else if(PORTAbits.RA4==0 && PORTBbits.RB4==1){
            NN = (NN == frequencyDividerMax) ? 1 : NN + 1;
            sprintf(num, "%d", NN);
            Disp2String("N = ");
            Disp2String(num);
            Disp2String("\n\r");
        } else if(PORTAbits.RA4==1 && PORTBbits.RB4==0){
            frequencyDivider = NN;
            sprintf(num, "%d", frequencyDivider);
            Disp2String("Setting frequency divider to ");
            Disp2String(num);
            Disp2String("\n\r");
        }
    }
    
    return;
}
