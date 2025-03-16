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




void __ISR(_TIMER_3_VECTOR, IPL4SOFT) Timer3ISR(void){
    IFS0bits.T3IF = 0;
    switch (get_mode()){

        case IDLE:
        {
            OC1RS = 0;
            break;
        }

        case PWM:
        {
            if (PWM_requested > 0){
                OC1RS = (int)((float)(PWM_requested * PR2)/100.0);
                LATBbits.LATB12 = 1;
            } else{
                OC1RS = (int)((float)((-1) * PWM_requested * PR2)/100.0);
                LATBbits.LATB12 = 0;}
            break;
        }

        case ITEST:
        {
            if (Itest_counter == 100){
                Itest_counter = 0;
                E_accu_current = 0;

                set_mode(IDLE);
                break;
            }

            Real_Itest = INA219_read_current();
            Current_Actual_ITEST[Itest_counter] = Real_Itest;
            e_current = Current_Reference_ITEST[Itest_counter] - Real_Itest;
            E_accu_current += e_current;
            
            new_current = Current_Kp * e_current + Current_Ki * E_accu_current;
            if (new_current > 0){
                LATBbits.LATB12 = 1;
            } else if (new_current < 0) {
                LATBbits.LATB12 = 0;
                new_current = abs(new_current);
            }
            if (new_current > 100.0){
                new_current = 100.0;
            }
            new_duty = (int)(new_current * PR2 / 100);
            OC1RS = new_duty;
            ++ Itest_counter;
            break;
        }

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

void ITEST_Current_Wave(){
    float Reference_Itest = 200.0;

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

    TRISBbits.TRISB12 = 0;
    LATBbits.LATB12 = 0;
    
}