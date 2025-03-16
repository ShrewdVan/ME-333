#ifndef POSITION_CONTROL__H__
#define POSITION_CONTROL__H__

#include <xc.h> // processor SFR definitions
#include <sys/attribs.h> // __ISR macro
#include "NU32DIP.h"

volatile float Position_Kp;
volatile float Position_Ki;
volatile float Position_Kd;
volatile float Required_current_HOLD;
volatile float Required_angle;
volatile int E_accu_position;
volatile int e_priv;
volatile float Reference_Track[2100];
volatile float Actual_Track[2100];
volatile int N;
//
volatile int e;
volatile int E_dot;
volatile int Current_Encoder_Count;

void position_control_Startup();

# endif