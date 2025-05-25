# include "encoder.h"
# include "nu32dip.h"
# include "utility.h"
# include "ina219.h"
# include <xc.h>
# include <math.h>
# include "current_control.h"
# include <sys/attribs.h> // __ISR macro
# include "position_control.h"

int volatile PWM_requested = 0; 
volatile float Current_Kp = 0.00;
volatile float Current_Ki = 0.00;
float Current_Reference_ITEST[100];
float Current_Actual_ITEST[100];
volatile float E_accu_current = 0.00;
volatile float e_current = 0.00;
static volatile int Itest_counter = 0;
static volatile float Real_Itest = 0.00;
static volatile float Real_HOLD = 0.00;
static volatile float new_current = 0.00;
static volatile int new_duty = 0;



// ISR function for the 20kHZ current control
void __ISR(_TIMER_3_VECTOR, IPL4SOFT) Timer3ISR(void){
    IFS0bits.T3IF = 0;
    switch (get_mode()){
        // When mode is IDLE, set PWM = 0 to make sure motor will not rotate 
        case IDLE:
        {
            OC1RS = 0;
            break;
        }
        // Mapping the percentage given by user to the PR2 and adjust the OC1RS, controlling the rotating speed
        // Set another DIO output to control the direction of the motor by setting the it to be 1 for counter-clockwise and 0 for clockwise
        case PWM:
        {
            if (PWM_requested > 0){
                // Calculate the OC1RS corresponding to the persentage
                OC1RS = (int)((float)(PWM_requested * PR2)/100.0);
                // Set the direction
                LATBbits.LATB12 = 1;
            } else{
                // Calculate the OC1RS corresponding to the persentage
                OC1RS = (int)((float)((-1) * PWM_requested * PR2)/100.0);
                // Set the direction
                LATBbits.LATB12 = 0;}
            break;
        }

        // When the mode is ITEST, Itest_counter is responsible for turning the mode back to IDLE when requried number of sample is collected
        case ITEST:
        {
            // When the number hits 100, set the mode back to IDLE 
            if (Itest_counter == 100){
                // clear the counter for the next ITEST 
                Itest_counter = 0;
                // clear the intergal error for the next ITEST or TRACK
                E_accu_current = 0;

                set_mode(IDLE);
                break;
            }

            // Real current
            Real_Itest = INA219_read_current();
            // Store the real current
            Current_Actual_ITEST[Itest_counter] = Real_Itest;
            // Calculate the error
            e_current = Current_Reference_ITEST[Itest_counter] - Real_Itest;
            // Accumulate the integral error
            E_accu_current += e_current;

            // Calculate the required current for the next step
            new_current = Current_Kp * e_current + Current_Ki * E_accu_current;

            // Same procedure as PWM 
            if (new_current > 0){
                LATBbits.LATB12 = 1;
            } else if (new_current < 0) {
                LATBbits.LATB12 = 0;
                new_current = abs(new_current);
            }
            // Cap Algrothim for current to avoid a invalid input
            if (new_current > 100.0){
                new_current = 100.0;
            }
            new_duty = (int)(new_current * PR2 / 100);
            OC1RS = new_duty;
            ++ Itest_counter;
            break;
        }

        // When the mode is HOLD, use the shared variable "Required_current_HOLD" calculated by position control to get the latest reuqired current
        // Code is identical to the ITEST expect the required current is now given by position control instead of ITEST waveform
        case HOLD:
        {
            Real_HOLD = INA219_read_current();
            e_current = Required_current_HOLD - Real_HOLD;
            E_accu_current += e_current;

            new_current = Current_Kp * e_current + Current_Ki * E_accu_current;
            if (new_current > 0){
                LATBbits.LATB12 = 1;
            } else if (new_current < 0) {
                LATBbits.LATB12 = 0;
                new_current = fabs(new_current);
            }
            if (new_current > 100.0){
                new_current = 100.0;
            }
            new_duty = (int)(new_current * PR2 / 100);
            OC1RS = new_duty;
            break;

        }

        // When the mode is TRACK, basically it's like multiple HOLD cases stack up. For current control loop there's no difference, therefore
        //   the code would also unchanged.
        case TRACK:
        {
            Real_HOLD = INA219_read_current();
            e_current = Required_current_HOLD - Real_HOLD;
            E_accu_current += e_current;

            new_current = Current_Kp * e_current + Current_Ki * E_accu_current;
            if (new_current > 0){
                LATBbits.LATB12 = 1;
            } else if (new_current < 0) {
                LATBbits.LATB12 = 0;
                new_current = fabs(new_current);
            }
            if (new_current > 100.0){
                new_current = 100.0;
            }
            new_duty = (int)(new_current * PR2 / 100);
            OC1RS = new_duty;
            
            break;
        }
        

    }

}

// Function used to generate the desired current for the ITEST
void ITEST_Current_Wave(){
    
    float Reference_Itest = 200.0;
    // When the Reference_Generator_Counter hits 25, 50, and 75, convert the sign
    for (int Reference_Generator_Counter = 0; Reference_Generator_Counter <= 99; Reference_Generator_Counter++){
        if (Reference_Generator_Counter <= 24){
            Current_Reference_ITEST[Reference_Generator_Counter] = Reference_Itest;
        } 
        else if (Reference_Generator_Counter <= 49){  
            Current_Reference_ITEST[Reference_Generator_Counter] = - Reference_Itest;
        } 
        else if (Reference_Generator_Counter <= 74){
            Current_Reference_ITEST[Reference_Generator_Counter] = Reference_Itest;
        }
        else {
            Current_Reference_ITEST[Reference_Generator_Counter] = - Reference_Itest;
        }
    }
}


// Startup function to initialize the current control loop
void current_control_Startup(){
    RPA0Rbits.RPA0R = 0b0101;
    // Timer 2 for the PWM control with a frequency of 20KHz
    T2CONbits.TCKPS = 0;
    T2CON = 0; 
    OC1CON = 0;
    OC1CONbits.OC32 = 0;
    OC1CONbits.OCTSEL = 0;
    PR2 = 2399;
    TMR2 = 0;
    OC1CONbits.OCM = 0b110;
    OC1R = 600;
    OC1RS = 600;
    T2CONbits.ON = 1;
    OC1CONbits.ON = 1;

    //Timer 3 for a 5kHz ISR in current control loop
    T3CONbits.TCKPS = 0;
    PR3 = 9599;
    TMR3 = 0;
    T3CONbits.ON = 1;
    IPC3bits.T3IP = 4;
    IPC3bits.T3IS = 0;  
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1;

    // Pin RB12 is set to be a DIO output to control the direction of the motor
    TRISBbits.TRISB12 = 0;
    LATBbits.LATB12 = 0;
    
}
