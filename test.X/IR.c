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

unsigned int *CNflagPtr = 0;

#define DEBOUNCE_DELAY 1000000
#define MAX_CYCLES_SINCE_PUSHED 50 //experimentally determined
#define MAX_CYCLES_SINCE_RELEASED 30

uint8_t currentMode = CHANGE_CHANNEL;

//Necessary so that the pointer can be shared between IO.c and IR.c,
//while keeping the code design somewhat modular, and keeping different
//functionality separate.
void ChannelChanger_Init(unsigned int *CNFptr){
    CNflagPtr = CNFptr;
}

void IR_CN_check(void){
    
    //some experimentation shows 100 ms is a good interval to allow it to
    //recognize that both buttons have been pressed without an overly noticeable delay.
    delay_us_32(100000);
    
    uint8_t RA4pushed = (PORTAbits.RA4==0) ? 1 : 0;
    uint8_t RB4pushed = (PORTBbits.RB4==0) ? 1 : 0;
    
    if(*CNflagPtr == 1){
        if (RA4pushed == 1 && RB4pushed == 1) {
            //Disp2String("Both buttons pushed!\n\r");
            Disp2String("\n\r");
            Both_Buttons_Pushed();
        } else if(RB4pushed == 1){
            //Disp2String("Up pushed!\n\r"); 
            Disp2String("\n\r");
            Up_Button_Pushed();
        } else if(RA4pushed == 1){
            //Disp2String("Down pushed!\n\r");
            Disp2String("\n\r");
            Down_Button_Pushed();
        }
    }
    
    return;
}

//Helper function
void Both_Buttons_Pushed(){
    uint8_t pushedOnlyOnce = 1;
    uint8_t pushedTwice = 0;
    
    uint16_t cyclesSincePushed = 0;
    uint16_t cyclesSinceReleased = 0;
    
    do{
        Hold_Low(1000000);
        Hold_Low(1000000);
        
        if(pushedOnlyOnce == 1){
            if(PORTAbits.RA4==1 || PORTBbits.RB4==1){
                pushedOnlyOnce = 0;
                cyclesSinceReleased++;

                Hold_Low(DEBOUNCE_DELAY);
                Hold_Low(DEBOUNCE_DELAY);
                
                //Keeping this here for debugging the Rx part. It will be removed eventually
                //Disp2String("cyclesSinceReleased = ");
                //Disp2Hex((int) cyclesSinceReleased);
                //Disp2String("\n\r");
            }else{
                cyclesSincePushed++;
                
                Hold_Low(DEBOUNCE_DELAY);
                Hold_Low(DEBOUNCE_DELAY);
                
                //Keeping this here for debugging the Rx part. It will be removed eventually
                //Disp2String("cyclesSincePushed = ");
                //Disp2Hex((int) cyclesSincePushed);
                //Disp2String("\n\r");
                if(cyclesSincePushed == MAX_CYCLES_SINCE_PUSHED){
                    //Transmit Power on/off
                    Disp2String("POWER_ON_OFF\n\r");
                    Tx_HexCode(POWER_ON_OFF);
                    return;
                }
            }
        }else{
            if(PORTAbits.RA4==0 && PORTBbits.RB4==0){
                //The buttons have been pushed twice
                //Set to volume mode.
                Volume_Mode();
                return;
            }
            else{
                cyclesSinceReleased++;
                
                Hold_Low(DEBOUNCE_DELAY);
                Hold_Low(DEBOUNCE_DELAY);

                //Keeping this here for debugging the Rx part. It will be removed eventually
                //Disp2String("cyclesSinceReleased = ");
                //Disp2Hex((int) cyclesSinceReleased);
                //Disp2String("\n\r");
                if(cyclesSinceReleased == MAX_CYCLES_SINCE_RELEASED){                    
                    //Set to channel mode.
                    Channel_Mode();
                    return;
                }
            }
        }
        
    }while((pushedTwice == 0) && (cyclesSincePushed < MAX_CYCLES_SINCE_PUSHED) && (cyclesSinceReleased < MAX_CYCLES_SINCE_RELEASED));
    
}

void Up_Button_Pushed(){
    if(currentMode == CHANGE_CHANNEL)
        Channel_Up();
    else
        Volume_Up();
}

void Down_Button_Pushed(){
    if(currentMode == CHANGE_CHANNEL)
        Channel_Down();
    else
        Volume_Down();
}

//helper Function
void Tx_HexCode(uint32_t hexCode){
    //Preprocess the hexcode so that it doesn't need to waste
    //clock cycles and potentially throwing off the frequency when transmitting.
    
    //Start bit
    Pulse_High(4500);
    Hold_Low(4500);
    
    int i = 0;
    for(i = 31; i>=0; i--){
        if((hexCode >> i) % 2 == 0){
            Transmit_Zero();
        }else{
            Transmit_One();
        }
    }
    
    //Stop bit
    Pulse_High(560);
    Hold_Low(560);

    
    //Uncomment when debugging
    Disp2String("Transmitting Hexcode: ");
    Disp2Hex32(hexCode);
    Disp2String("\n\r");
    
}

void Transmit_Zero(void){
    Pulse_High(560);
    Hold_Low(560);
}

void Transmit_One(void){
    Pulse_High(560);
    Hold_Low(1690);
}

void Pulse_High(long int pulseWidth_us){
    //38 kHz corresponds to a period of 26.3 us
    //In other words, 13 us ON, and 13 us OFF.
    int numberOfCycles = pulseWidth_us / 26;
    int i;
    for(i=0; i<numberOfCycles; i++){
        LATBbits.LATB9 = 1;
        __delay32(45);
        LATBbits.LATB9 = 0;
        __delay32(45);
    }
}

void Hold_Low(long int pulseWidth_us){
    //38 kHz corresponds to a period of 26.3 us
    //In other words, 13 us ON, and 13 us OFF.
    int numberOfCycles = pulseWidth_us / 26;
    LATBbits.LATB9 = 0;
    int i;
    for(i=0; i<numberOfCycles; i++){
        __delay32(80);
    }
}

void Channel_Up(){
    //Uncomment when debugging
    Disp2String("CHANNEL_UP\n\r");    

    Tx_HexCode(CHANNEL_UP);
    //channel = (channel == MAX_CHANNEL) ? MIN_CHANNEL : channel + 1; 
}

void Channel_Down(){
    //Uncomment when debugging
    Disp2String("CHANNEL_DOWN\n\r");    

    Tx_HexCode(CHANNEL_DOWN);    
    //channel = (channel == MIN_CHANNEL) ? MAX_CHANNEL : channel - 1; 
}

void Volume_Up(){
    //Uncomment when debugging
    Disp2String("VOLUME_UP\n\r");    

    Tx_HexCode(VOLUME_UP);    
    //volume = (volume == 100) ? 100 : volume + 1; 
}

void Volume_Down(){
    //Uncomment when debugging
    Disp2String("VOLUME_DOWN\n\r");    

    Tx_HexCode(VOLUME_DOWN);    
    //volume = (volume == 0) ? 0 : volume - 1; 
}

void Volume_Mode(){
    currentMode = CHANGE_VOLUME;
    Disp2String("VOLUME MODE\n\r");
}

void Channel_Mode(){
    currentMode = CHANGE_CHANNEL;
    Disp2String("CHANNEL MODE\n\r");
}

// Rx Functions
void IR_CN_check_Rx(void){
    
    //some experimentation shows 100 ms is a good interval to allow it to
    //recognize that both buttons have been pressed without an overly noticeable delay.
    delay_us_32(100000);
    
    uint8_t RA4pushed = (PORTAbits.RA4==0) ? 1 : 0;
    uint8_t RB4pushed = (PORTBbits.RB4==0) ? 1 : 0;
    
    if(*CNflagPtr == 1){
        if (RA4pushed == 1 && RB4pushed == 1) {
            //Disp2String("Both buttons pushed!\n\r");
            Disp2String("\n\r");
            Both_Buttons_Pushed();
        } else if(RB4pushed == 1){
            //Disp2String("Up pushed!\n\r"); 
            Disp2String("\n\r");
            Up_Button_Pushed();
        } else if(RA4pushed == 1){
            //Disp2String("Down pushed!\n\r");
            Disp2String("\n\r");
            Down_Button_Pushed();
        }
    }
    
    return;
}



