# include "encoder.h"
# include "nu32dip.h"
# include "utility.h"
# include "ina219.h"
# include <xc.h>
# include "current_control.h"
# include "position_control.h"

volatile float Position_Kp = 0.00;
volatile float Position_Ki = 0.00;
volatile float Position_Kd = 0.00;
volatile float Required_current_HOLD = 0.00;
volatile float Required_angle = 0.00;
volatile int E_accu_position = 0;
volatile int e_priv = 0;
volatile float Reference_Track[2100];
volatile float Actual_Track[2100];
volatile int N = 0;

volatile int Current_Encoder_Count = 0;
static volatile int Required_encoder_count = 0;
static volatile int Track_count;
//
volatile int e = 0;
volatile int E_dot = 0;

//ISR function for the 5 Hz position control loop
void __ISR(_TIMER_4_VECTOR, IPL5SOFT) Timer4ISR(void){
    IFS0bits.T4IF = 0;
    switch (get_mode()){

        // When the mode is HOLD, receive the value using the variable "Required_angle". 
        case HOLD:
        {
            // Procedure for reading the encoder count
            WriteUART2("a");
            while(!get_encoder_flag()){}
            set_encoder_flag(0);
            Current_Encoder_Count = get_encoder_count();

            // Turn the required angle into the encoder count
            Required_encoder_count = (int)(Required_angle * 4);
            // Calculate the error
            e = Required_encoder_count - Current_Encoder_Count;
            // Accumulate the integral error
            E_accu_position += e;
            // Calculate the derivative error
            E_dot = e - e_priv;
            // Update the e_priv for the next iteration of the E_dot
            e_priv = e;
            // Use all the error item above to calculate the required current and send to the current control loop
            Required_current_HOLD = (float)(Position_Kp * e + Position_Ki * E_accu_position + Position_Kd * E_dot);

            break;
        }

        // When the mode is TRACK, basically it's like stack multiple HOLD case together. everything would be the same with the HOLD mode but the required angle
        //     now is allocated by the designed waveform
        case TRACK:
        {

            // When the Track_count hits the length of the data, set the mode to HOLD to stop motor at the last angle. 
            // Refresh the Track_count for the next TRACK operation
            if (Track_count == N){
                set_mode(HOLD);
                Track_count = 0;

                break;
            }

            Required_angle = Reference_Track[Track_count];

            WriteUART2("a");
            while(!get_encoder_flag()){;}
            set_encoder_flag(0);
            Current_Encoder_Count = get_encoder_count();
            Actual_Track[Track_count] = (float)get_encoder_count()/4;

            Required_encoder_count = (int)(Required_angle * 4);
            e = Required_encoder_count - Current_Encoder_Count;
            E_accu_position += e;
            E_dot = e - e_priv;
            e_priv = e;
            Required_current_HOLD = (float)(Position_Kp * e + Position_Ki * E_accu_position + Position_Kd * E_dot);

            Track_count ++;
        }

    }

}

// Startup function for the position control loop
void position_control_Startup(){
    //Timer 4 for a 5kHz ISR in current control loop
    T3CONbits.TCKPS = 0;
    PR3 = 239999;
    TMR4 = 0;
    T4CONbits.ON = 1;
    IPC4bits.T4IP = 5;
    IPC4bits.T4IS = 0;  
    IFS0bits.T4IF = 0;
    IEC0bits.T4IE = 1;

}
