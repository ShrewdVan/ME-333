#ifndef UTILITY__H__
#define UTILITY__H__

#include <xc.h>
#include <stdio.h>
#include "NU32DIP.h"

enum mode_t {IDLE, PWM, ITEST, HOLD, TRACK};

enum mode_t get_mode();
void print_mode();
void set_mode(enum mode_t new_mode);

#endif