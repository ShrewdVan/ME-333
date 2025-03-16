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

void __ISR(_TIMER_4_VECTOR, IPL5SOFT) Timer4ISR(void){
    IFS0bits.T4IF = 0;
    switch (get_mode()){

        case HOLD:
        {
            WriteUART2("a");
            while(!get_encoder_flag()){}
            set_encoder_flag(0);
            Current_Encoder_Count = get_encoder_count();

            Required_encoder_count = (int)(Required_angle * 4);
            e = Required_encoder_count - Current_Encoder_Count;
            E_accu_position += e;
            E_dot = e - e_priv;
            e_priv = e;
            Required_current_HOLD = (float)(Position_Kp * e + Position_Ki * E_accu_position + Position_Kd * E_dot);

            break;
        }

        case TRACK:
        {

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