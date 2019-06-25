/*
 * File:   IR.c
 * Author: Owner
 *
 * Created on October 10, 2018, 5:18 PM
 */


#include <p24F16KA101.h>

#define FCY 4000000UL
#include <libpic30.h>

#include "xc.h"
#include "timer.h"
#include "UART2.h"
#include "IR.h"
#include "IR_Rx.h"
#include <math.h>

unsigned int *CNflagPtr_Rx = 0;

uint32_t byteRead = 0;

//Not needed for the Tx part, only the Rx part.
unsigned int channel = 2;
unsigned int volume = 50;

uint32_t changeTimes[68];
uint8_t counter = 0;

uint8_t successfulSignal = 0;
uint8_t poweredOn = 0;

//Necessary so that the pointer can be shared between IO.c and IR.c,
//while keeping the code design somewhat modular, and keeping different
//functionality separate.
void Receiver_Init(unsigned int *CNFptr){
    CNflagPtr_Rx = CNFptr;
    Reset_Counter();
    Prime_Timer(); //Flushes garbage out of registers
}

void Prime_Timer(){
    
    TMR3HLD = 0;
    TMR2 = 0;
    
    Init_Timer_32bit();

    PR3 = 0x000f;
    PR2 = 0xffff;
    T2CONbits.TON = 1;

}

void IR_CN_check_for_Rx(void){
        
    uint32_t tmr = TMR3;

    //Disp2String("tmr3hld =");
    //Disp2Hex32(tmr);
    //Disp2String("\n\r");

    
    if(counter != 0 && counter <= 68){
        tmr = (tmr << 16) + TMR2;
        changeTimes[counter-1] = tmr;
        
        //Disp2String("tmr =");
        //Disp2Hex32(tmr);
        //Disp2String("\n\r");
    }

    //Disp2String("counter =");
    //Disp2Hex(counter);
    //Disp2String("\n\r");
    
    if(counter == 68){
        Disp2String("Signal received\n\r");
        
        //int i;
        //for(i=0;i<68;i++){
        //    Disp2Hex32(changeTimes[i]);
        //}
        
        uint32_t rxCode = Decode_Rx_Signal();    
        Check_HexCode(rxCode);
        
        Reset_Counter();

        return;
    }
    
    counter++;

    //Disp2String("counter =");
    //Disp2Hex(counter);
    //Disp2String("\n\r");
    
    Prime_Timer();    
}

uint32_t Decode_Rx_Signal(){
    successfulSignal = 0;
    uint32_t res = 0;
    
    if (changeTimes[0] < START_BIT_TIME_LOW || changeTimes[0] > START_BIT_TIME_HIGH){
        Disp2String("Fail point one");
        Disp2String("\n\r");
        return res;
    }
    
    if (changeTimes[1] < START_BIT_TIME_LOW || changeTimes[1] > START_BIT_TIME_HIGH){
        Disp2String("Fail point two");
        Disp2String("\n\r");
        return res;
    }
    
    int i;
    for(i = 2; i < 66; i++){
        if(i % 2 == 0){
            if(changeTimes[i] < ZERO_BIT_TIME_LOW || changeTimes[i] > ZERO_BIT_TIME_HIGH){
                Disp2String("Fail point three");
                Disp2Hex(i);
                Disp2String("\n\r");
                return res;
            }
        }
        else{
            if(changeTimes[i] > ZERO_BIT_TIME_LOW && changeTimes[i] < ZERO_BIT_TIME_HIGH){
                res = res << 1;
            }
            else if(changeTimes[i] > ONE_BIT_TIME_LOW && changeTimes[i] < ONE_BIT_TIME_HIGH){
                res = (res << 1) + 1;
            }
            else{
                Disp2String("Fail point four");
                Disp2Hex(i);
                Disp2String("\n\r");
                return res;
            }
        }
    }
    
    if(changeTimes[66] < ZERO_BIT_TIME_LOW || changeTimes[66] > ZERO_BIT_TIME_HIGH){
        Disp2String("Fail point five");
        Disp2String("\n\r");
        return res;
    }

    successfulSignal = 1;    
    return res;
    
}


void Check_HexCode(uint32_t hexCode){
    switch(hexCode){
        case CHANNEL_UP:
            Channel_Up_Rx();
            break;
        case CHANNEL_DOWN:
            Channel_Down_Rx();
            break;
        case VOLUME_UP:
            Volume_Up_Rx();
            break;
        case VOLUME_DOWN:
            Volume_Down_Rx();
            break;
        case POWER_ON_OFF:
            Power_On_Off_Rx();
            break;
        case 0:
            break;
        default:
            Disp2String("Command unrecognized: \n\r");
            Disp2Hex32(hexCode);
            Disp2String("\n\r");
            Prime_Timer(); //Resetting the timer.
            Reset_Counter(); //Some issues of data persisting in the buffer.
           break;
    }
}


void Channel_Up_Rx(){
    //Uncomment when debugging
    Disp2String("CHANNEL_UP\n\r");
    
    if(poweredOn == 0){
        Disp2String("TV is off.");
        return;
    }
    
    channel = (channel == MAX_CHANNEL) ? MAX_CHANNEL : channel + 1;
    Disp2String("Now on channel ");
    Disp2Hex(channel);
    Disp2String("\n\r");
}

void Channel_Down_Rx(){
    //Uncomment when debugging
    Disp2String("CHANNEL_DOWN\n\r");    

    if(poweredOn == 0){
        Disp2String("TV is off.");
        return;
    }

    channel = (channel == MIN_CHANNEL) ? MIN_CHANNEL : channel - 1; 
    Disp2String("Now on channel ");
    Disp2Hex(channel);
    Disp2String("\n\r");
}

void Volume_Up_Rx(){
    //Uncomment when debugging
    Disp2String("VOLUME_UP\n\r");    

    if(poweredOn == 0){
        Disp2String("TV is off.\n\r");
        return;
    }

    volume = (volume == 100) ? 100 : volume + 1; 
    Disp2String("Now at volume ");
    Disp2Hex(volume);
    Disp2String("\n\r");
}

void Volume_Down_Rx(){
    //Uncomment when debugging
    Disp2String("VOLUME_DOWN\n\r");

    if(poweredOn == 0){
        Disp2String("TV is off.\n\r");
        return;
    }
    
    volume = (volume == 0) ? 0 : volume - 1; 
    Disp2String("Now at volume ");
    Disp2Hex(volume);
    Disp2String("\n\r");
}

void Power_On_Off_Rx(){
    //Uncomment when debugging
    Disp2String("POWER_ON_OFF\n\r");    

    if(poweredOn == 0){
        Disp2String("Turning on TV.\n\r");
        poweredOn = 1;
    }
    else{
        Disp2String("Turning off TV.\n\r");
        poweredOn = 0;        
    }
}

void Reset_Counter(){
    counter = 0;
    successfulSignal = 0;
}
