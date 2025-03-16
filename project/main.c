# include "encoder.h"
# include "nu32dip.h"
# include "utility.h"
# include "ina219.h"
# include <xc.h>
# include "current_control.h"
# include "position_control.h"

#define BUF_SIZE 200
#define NU32_LED1 TRISBbits.TRISB4  
#define NU32_LED2 TRISBbits.TRISB5





int main()
{
    // INA219_Startup();
    NU32DIP_Startup(); // cache on, min flash wait, interrupts on, LED/button init, UART init
    set_mode(IDLE);
    //UART2_Startup();
    ITEST_Current_Wave();
    //current_control_Startup();
    char buffer[BUF_SIZE];
    NU32_LED1 = 1; // turn off the LEDs
    NU32_LED2 = 1;

    __builtin_disable_interrupts();
    current_control_Startup();
    position_control_Startup();
    UART2_Startup();
    INA219_Startup();
    __builtin_enable_interrupts();


    while(1)
    {
        NU32_LED2 = 1;
        NU32DIP_ReadUART1(buffer,BUF_SIZE); // we expect the next character to be a menu command
        
        switch (buffer[0]) {

            case 'a':
            {
                signed short current_ADC = readINA219(0x04);
                char m[50];
                sprintf(m,"%d\r\n",current_ADC);
                NU32DIP_WriteUART1(m); // send encoder count to client

                break;
            }

            case 'b':
            {
                float current_ma = INA219_read_current();
                char m[50];
                sprintf(m,"%.2f\r\n",current_ma);
                NU32DIP_WriteUART1(m); // send encoder count to client

                break;
            }

            case 'c':
            {
                WriteUART2("a");
                while(!get_encoder_flag()){}
                set_encoder_flag(0);
                char m[50];
                int p = get_encoder_count();
                sprintf(m,"%d\r\n",p);
                NU32DIP_WriteUART1(m); // send encoder count to client

                break;
            }

            case 'd': // encoder count in degree
            {   
                WriteUART2("a");
                while(!get_encoder_flag()){}
                set_encoder_flag(0);
                char m[50];
                float angle = (float)get_encoder_count()/4;
                sprintf(m,"%.2f\r\n",angle);
                NU32DIP_WriteUART1(m); // send encoder count to client

                break;
            }

            case 'e':
            {
                WriteUART2("b");

                break;
            }

            case 'f':
            {
                char m[50];
                set_mode(PWM);
                NU32DIP_ReadUART1(buffer, BUF_SIZE);
                sscanf(buffer, "%d", &PWM_requested);
                sprintf(m,"y\r\n");
                NU32DIP_WriteUART1(m);

                break;

            }

            case 'g':
            {
                char m[50];
                NU32DIP_ReadUART1(buffer, BUF_SIZE);
                sscanf(buffer, "%f", &Current_Kp);
                sprintf(m,"y1\r\n");
                NU32DIP_WriteUART1(m);

                NU32DIP_ReadUART1(buffer, BUF_SIZE);
                sscanf(buffer, "%f", &Current_Ki);
                sprintf(m,"y2\r\n");
                NU32DIP_WriteUART1(m);   

                break;
                      
            }

            case 'h':
            {
                char m[50];
                sprintf(m,"%.3f\r\n",Current_Kp);
                NU32DIP_WriteUART1(m);

                int cl = _CP0_GET_COUNT();
                while (_CP0_GET_COUNT() - cl < 2400000) {;}

                sprintf(m,"%.3f\r\n",Current_Ki);
                NU32DIP_WriteUART1(m);

                break;
            }

            case 'i':
            {
                char m[50];
                NU32DIP_ReadUART1(buffer, BUF_SIZE);
                sscanf(buffer, "%f", &Position_Kp);
                sprintf(m,"y1\r\n");
                NU32DIP_WriteUART1(m);

                NU32DIP_ReadUART1(buffer, BUF_SIZE);
                sscanf(buffer, "%f", &Position_Ki);
                sprintf(m,"y2\r\n");
                NU32DIP_WriteUART1(m);    

                NU32DIP_ReadUART1(buffer, BUF_SIZE);
                sscanf(buffer, "%f", &Position_Kd);
                sprintf(m,"y3\r\n");
                NU32DIP_WriteUART1(m); 

                break;                
            }

            case 'j':
            {
                char m[50];
                sprintf(m,"%.3f\r\n",Position_Kp);
                NU32DIP_WriteUART1(m);

                int cl = _CP0_GET_COUNT();
                while (_CP0_GET_COUNT() - cl < 2400000) {;}

                sprintf(m,"%.6f\r\n",Position_Ki);
                NU32DIP_WriteUART1(m);

                cl = _CP0_GET_COUNT();
                while (_CP0_GET_COUNT() - cl < 2400000) {;}

                sprintf(m,"%.3f\r\n",Position_Kd);
                NU32DIP_WriteUART1(m);  

                break;
            }

            case'k':
            {
                char m[50];
                sprintf(m,"100\r\n");
                NU32DIP_WriteUART1(m);
                set_mode(ITEST);
                while (get_mode() == ITEST){
                    ;
                }


                for (int i = 0; i < 100; i++) {
                    sprintf(m, "%+.2f %+.2f\r\n", \
                    Current_Reference_ITEST[i], Current_Actual_ITEST[i]);
                    NU32DIP_WriteUART1(m);

                    _CP0_SET_COUNT(0); 
                    while (_CP0_GET_COUNT() < 240000) {;} 
                }

                break;
            }
            
            case 'l':
            {
                Required_current_HOLD = 0;
                E_accu_current = 0;
                E_accu_position = 0;
                e_priv = 0;

                char m[50];
                NU32DIP_ReadUART1(buffer, BUF_SIZE);
                sscanf(buffer, "%f", &Required_angle);
                set_mode(HOLD);
                sprintf(m,"y\r\n");
                NU32DIP_WriteUART1(m);

                break;
            }

            case 'm':
            {
                set_mode(IDLE);
                NU32DIP_ReadUART1(buffer, BUF_SIZE);
                sscanf(buffer, "%d", &N);
                float temp = 0;

                for (int j = 0; j < N; j++){
                    NU32DIP_ReadUART1(buffer, BUF_SIZE);
                    sscanf(buffer, "%f", &temp);
                    Reference_Track[j] = temp;
                }
                char m[50];
                sprintf(m,"y\r\n");
                NU32DIP_WriteUART1(m);
                break;
            }

            case 'n':
            {
                set_mode(IDLE);
                NU32DIP_ReadUART1(buffer, BUF_SIZE);
                sscanf(buffer, "%d", &N);
                float temp = 0;

                for (int j = 0; j < N; j++){
                    NU32DIP_ReadUART1(buffer, BUF_SIZE);
                    sscanf(buffer, "%f", &temp);
                    Reference_Track[j] = temp;
                }
                char m[50];
                sprintf(m,"y\r\n");
                NU32DIP_WriteUART1(m);
                break;
            }

            case 'o':
            {
                Required_current_HOLD = 0;
                E_accu_current = 0;
                E_accu_position = 0;
                e_priv = 0;  

                set_mode(TRACK);
                while (get_mode() == TRACK){;}

                char m[50];
                sprintf(m,"%d\r\n", N);
                NU32DIP_WriteUART1(m);

                for (int k = 0; k < N; k++) {
                    sprintf(m, "%+.2f %+.2f\r\n", \
                    Reference_Track[k], Actual_Track[k]);
                    NU32DIP_WriteUART1(m);
                }

                break;         

            }

            case 's':
            {
                char mn[50];
                for (int ij = 0; ij < N; ij++) {
                    sprintf(mn, "Index %d: %.2f\r\n", ij, Reference_Track[ij]);
                    NU32DIP_WriteUART1(mn);
                }
                break;
            }

            case 'p':
            {
                char m[50];
                set_mode(IDLE);
                sprintf(m,"y\r\n");
                NU32DIP_WriteUART1(m);

                break;

            }

            case'v':// Query the result from the position control PID
            {
                char m[50];
                sprintf(m,"%.5f,%d,%d,%d\r\n",Required_current_HOLD,E_accu_position,e, E_dot);
                NU32DIP_WriteUART1(m);

                break;
            }

            case 'r':
            {
                print_mode();

                break;
            }

            default:
            {
                NU32_LED2 = 0; // turn on LED2 to indicate an error
                LATBbits.LATB5 = 0;
                _CP0_SET_COUNT(0);
                while (_CP0_GET_COUNT() < 24000000) {;}

                break;
            }
        }
    }
    return 0;
}
