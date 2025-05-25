#include "sr04.h"
#include <xc.h>
#include <stdio.h>


// macros for the pins
#define TRIG LATBbits.LATB15
#define ECHO PORTBbits.RB14

// initialize the pins used by the SR04
void SR04_Startup(){
    ANSELA = 0; // turn off the analog input functionality that overrides everything else
    ANSELB = 0;
    TRISBbits.TRISB15 = 0; // B15 is TRIG, output from the PIC
    TRISBbits.TRISB14 = 1; // B14 is ECHO, input to the PIC
    TRIG = 0; // initialize TRIG to LOW
}


unsigned int SR04_read_raw(unsigned int timeout) {
    unsigned int start_time, end_time;
    unsigned int procedure_start_time;
    char buffer[50];

    // **TRIG trigger**
    TRIG = 1;
    unsigned int trig_start = _CP0_GET_COUNT();
    while(_CP0_GET_COUNT()-trig_start < 240){}
    TRIG = 0;

    // **Wait ECHO HIGH**
    while (ECHO == 0 ) {
    }

    start_time = _CP0_GET_COUNT();
    // **Wait ECHO LOW**
    while (ECHO == 1 && (_CP0_GET_COUNT() - start_time) < timeout) { 
    }


    end_time = _CP0_GET_COUNT();
    return end_time - start_time;
}


float SR04_read_meters(unsigned int timeout) {
    unsigned int passing_time = SR04_read_raw(timeout);
    float distance = (passing_time * 346.0f) / (2 * 20 * 1000000);
    return distance;
}
