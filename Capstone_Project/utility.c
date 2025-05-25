// This code is used for storing mode related stuff. There are totally 5 mode, IDLE, PWM, ITEST, HOLD, and TRACK
# include "utility.h"

// "mode" is a static mode, and can only be queried by using the get_mode() function listed below.
static volatile enum mode_t mode;

// Function created for other c file to query the current mode
enum mode_t get_mode(){
    return mode;
}

// Function used to send info to the python, telling the user with the showm str
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

// Function used to switch the mode
void set_mode(enum mode_t new_mode) {
    mode = new_mode;
}
