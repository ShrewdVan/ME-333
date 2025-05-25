#ifndef CURRENT_CONTROL__H__
#define CURRENT_CONTROL__H__

#include <xc.h> // processor SFR definitions
#include <sys/attribs.h> // __ISR macro
#include "NU32DIP.h"

extern volatile int PWM_requested;
extern volatile float Current_Kp;
extern volatile float Current_Ki;
volatile float E_accu_current;
float Current_Reference_ITEST[100];
float Current_Actual_ITEST[100];

volatile float e_current;

void ITEST_Current_Wave();
void current_control_Startup();

# endif