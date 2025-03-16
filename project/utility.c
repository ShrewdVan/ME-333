# include "utility.h"


static volatile enum mode_t mode;

enum mode_t get_mode(){
    return mode;
}

void print_mode() {
    char result[70];

    switch (mode){
        case IDLE:
            sprintf(result,"The current mode is Idle\n\r");
            NU32DIP_WriteUART1(result);
            break;
        case PWM:
            sprintf(result,"The current mode is PWM\n\r");
            NU32DIP_WriteUART1(result);
            break;
        case ITEST:
            sprintf(result,"The current mode is ITEST\n\r");
            NU32DIP_WriteUART1(result);
            break;
        case HOLD:
            sprintf(result,"The current mode is Hold\n\r");
            NU32DIP_WriteUART1(result);
            break;
        case TRACK:
            sprintf(result,"The current mode is TRACK\n\r");
            NU32DIP_WriteUART1(result);
            break;
    }
}

void set_mode(enum mode_t new_mode) {
    mode = new_mode;
}
