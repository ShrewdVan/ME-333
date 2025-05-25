#include "sr04.h"
#include <xc.h>
#include <stdio.h>


int main() {
    unsigned int timeout = 120000000;
    unsigned int Start_Time;
    NU32DIP_Startup();
    SR04_Startup();

    while (1) {
        float distance = SR04_read_meters(timeout);
        char buffer[50]; 
        sprintf(buffer, "The distance is %.2f meters\r\n", distance);
        NU32DIP_WriteUART1(buffer); 

        Start_Time = _CP0_GET_COUNT(); 
        while ((_CP0_GET_COUNT() - Start_Time) < 5000000);
        }
        
    return 0;
}