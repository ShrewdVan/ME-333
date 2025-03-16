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

// This is the main file for the PIC motor, pick the feature by sending the PIC the corresponding letter



int main()
{

    // Cache on, min flash wait, interrupts on, LED/button init, UART init
    NU32DIP_Startup(); 
    // The default mode for the motor should be IDLE
    set_mode(IDLE);
    // Create the Waveform for the ITEST
    ITEST_Current_Wave();
    // The str used to receive the info sent from the python
    char buffer[BUF_SIZE];
    // There are two LED lights, green and yellow. Green led basically doesn't do something.
    //      I'm just too lazy to change that. Yellow led is the problem LED. When user type a char 
    //      with no matching feacture. the yellow led would bright for 1 sec.
    NU32_LED1 = 1; // turn off the LEDs
    NU32_LED2 = 1;

    // StartUp function assembly line
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

            // Read the current in sensor count
            case 'a':
            {
                signed short current_ADC = readINA219(0x04);
                char m[50];
                sprintf(m,"%d\r\n",current_ADC);
                NU32DIP_WriteUART1(m); // send encoder count to client

                break;
            }

            // Read the current in mA count
            case 'b':
            {
                float current_ma = INA219_read_current();
                char m[50];
                sprintf(m,"%.2f\r\n",current_ma);
                NU32DIP_WriteUART1(m); // send encoder count to client

                break;
            }

            // Read the encoder in its default count
            case 'c':
            {
                // Send the char 'a' to the encoder
                WriteUART2("a");
                // Waiting encoder responding
                while(!get_encoder_flag()){}
                // Clear the flag
                set_encoder_flag(0);
                // Receive the data
                char m[50];
                int p = get_encoder_count();
                sprintf(m,"%d\r\n",p);
                NU32DIP_WriteUART1(m); // send encoder count to client

                break;
            }

            // Read the encoder in degree count
            case 'd': 
            {   
                // Send the char 'a' to the encoder
                WriteUART2("a");
                // Waiting encoder responding
                while(!get_encoder_flag()){}
                // Clear the flag
                set_encoder_flag(0);
                // Receive the data
                char m[50];
                // The encoder have a scale of 4, divide the encoder count by 4 to get the real deg
                float angle = (float)get_encoder_count()/4;
                sprintf(m,"%.2f\r\n",angle);
                NU32DIP_WriteUART1(m); // send encoder count to client

                break;
            }

            // Reset the encoder count
            case 'e':
            {
                // Send the char 'b' to the encoder to reset
                WriteUART2("b");

                break;
            }

            // Set the motor velocity
            case 'f':
            {
                char m[50];
                // Switch mode to PWM
                set_mode(PWM);
                // Receive the asked data from user
                NU32DIP_ReadUART1(buffer, BUF_SIZE);
                sscanf(buffer, "%d", &PWM_requested);
                // Indicator info 
                sprintf(m,"y\r\n");
                NU32DIP_WriteUART1(m);

                break;

            }

            // Change the current control error gain
            case 'g':
            {
                char m[50];
                // Accept the first value sent from user 
                NU32DIP_ReadUART1(buffer, BUF_SIZE);
                // Assign the value to the current proportional error gain
                sscanf(buffer, "%f", &Current_Kp);
                // indicator info
                sprintf(m,"y1\r\n");
                NU32DIP_WriteUART1(m);

                // Accept the second value sent from user 
                NU32DIP_ReadUART1(buffer, BUF_SIZE);
                // Assign the value to the current integral error gain
                sscanf(buffer, "%f", &Current_Ki);
                // indicator info
                sprintf(m,"y2\r\n");
                NU32DIP_WriteUART1(m);   

                break;
                      
            }

            // Query the current error gain parameters
            case 'h':
            {
                // Send the current proportional error gain 
                char m[50];
                sprintf(m,"%.3f\r\n",Current_Kp);
                NU32DIP_WriteUART1(m);

                // Wait for 0.1 sec (Might be unnecessary)
                int cl = _CP0_GET_COUNT();
                while (_CP0_GET_COUNT() - cl < 2400000) {;}

                // Send the current integral error gain
                sprintf(m,"%.3f\r\n",Current_Ki);
                NU32DIP_WriteUART1(m);

                break;
            }

            // Change the position control error gain
            case 'i':
            {
                char m[50];
                // Accept the first value sent from user 
                NU32DIP_ReadUART1(buffer, BUF_SIZE);
                // Assign the value to the position proportional error gain
                sscanf(buffer, "%f", &Position_Kp);
                sprintf(m,"y1\r\n");
                NU32DIP_WriteUART1(m);

                // Accept the second value sent from user 
                NU32DIP_ReadUART1(buffer, BUF_SIZE);
                // Assign the value to the position intergal error gain
                sscanf(buffer, "%f", &Position_Ki);
                sprintf(m,"y2\r\n");
                NU32DIP_WriteUART1(m);    

                // Accept the third value sent from user 
                NU32DIP_ReadUART1(buffer, BUF_SIZE);
                // Assign the value to the position derivative error gain
                sscanf(buffer, "%f", &Position_Kd);
                sprintf(m,"y3\r\n");
                NU32DIP_WriteUART1(m); 

                break;                
            }

            // Query the position error gain parameters
            case 'j':
            {
                char m[50];
                // Send the position proportional error gain 
                sprintf(m,"%.3f\r\n",Position_Kp);
                NU32DIP_WriteUART1(m);

                // Wait for 0.1 sec
                int cl = _CP0_GET_COUNT();
                while (_CP0_GET_COUNT() - cl < 2400000) {;}

                // Send the position integral error gain 
                sprintf(m,"%.6f\r\n",Position_Ki);
                NU32DIP_WriteUART1(m);

                 // Wait for 0.1 sec
                cl = _CP0_GET_COUNT();
                while (_CP0_GET_COUNT() - cl < 2400000) {;}

                // Send the position derivative error gain 
                sprintf(m,"%.3f\r\n",Position_Kd);
                NU32DIP_WriteUART1(m);  

                break;
            }

            // ITEST
            case'k':
            {
                char m[50];
                // Tell the python there are totally 100 sample points
                sprintf(m,"100\r\n");
                NU32DIP_WriteUART1(m);
                // Switch the mode to ITEST
                set_mode(ITEST);
                // Collect the data
                while (get_mode() == ITEST){
                    ;
                }


                // Send the data to python after complete collecting the data
                for (int i = 0; i < 100; i++) {
                    sprintf(m, "%+.2f %+.2f\r\n", \
                    Current_Reference_ITEST[i], Current_Actual_ITEST[i]);
                    NU32DIP_WriteUART1(m);

                    // Wait for 0.1 sec to avoid any data leakage
                    _CP0_SET_COUNT(0); 
                    while (_CP0_GET_COUNT() < 240000) {;} 
                }

                break;
            }

            // Make motor rotate to a certain angle and stop there
            case 'l':
            {
                // Initialize the error stuff, make sure no influence from the last procedure
                Required_current_HOLD = 0;
                E_accu_current = 0;
                E_accu_position = 0;
                e_priv = 0;

                char m[50];
                // Receive the required angle from the user
                NU32DIP_ReadUART1(buffer, BUF_SIZE);
                // Assign the value to the shared variable
                sscanf(buffer, "%f", &Required_angle);
                // Set mode to HOLD
                set_mode(HOLD)；
                // Indicator info
                sprintf(m,"y\r\n");
                NU32DIP_WriteUART1(m);

                break;
            }

            // Load the cubic trajectory
            case 'm':
            {
                set_mode(IDLE);
                // Query the number of sample points
                NU32DIP_ReadUART1(buffer, BUF_SIZE);
                sscanf(buffer, "%d", &N);
                // variable used to temperailly store the data from python
                float temp = 0;

                // Receive data to the array Reference_Track
                for (int j = 0; j < N; j++){
                    NU32DIP_ReadUART1(buffer, BUF_SIZE);
                    sscanf(buffer, "%f", &temp);
                    Reference_Track[j] = temp;
                }
                char m[50];
                // Indicator info
                sprintf(m,"y\r\n");
                NU32DIP_WriteUART1(m);
                break;
            }

            // Load the step trajectory
            case 'n':
            {
                set_mode(IDLE);
                // Query the number of sample points
                NU32DIP_ReadUART1(buffer, BUF_SIZE);
                sscanf(buffer, "%d", &N);
                // variable used to temperailly store the data from python
                float temp = 0;

                // Receive data to the array Reference_Track
                for (int j = 0; j < N; j++){
                    NU32DIP_ReadUART1(buffer, BUF_SIZE);
                    sscanf(buffer, "%f", &temp);
                    Reference_Track[j] = temp;
                }
                char m[50];
                // Indicator info
                sprintf(m,"y\r\n");
                NU32DIP_WriteUART1(m);
                break;
            }

            // Executre the Trajectory
            case 'o':
            {
                // Initialize the error stuff, make sure no influence from the last procedure
                Required_current_HOLD = 0;
                E_accu_current = 0;
                E_accu_position = 0;
                e_priv = 0;  

                // Set the mode to TRACK
                set_mode(TRACK);
                // Data Collecting
                while (get_mode() == TRACK){;}

                char m[50];
                // Tell the python the number of sample points
                sprintf(m,"%d\r\n", N);
                NU32DIP_WriteUART1(m);

                // Data sending loop
                for (int k = 0; k < N; k++) {
                    sprintf(m, "%+.2f %+.2f\r\n", \
                    Reference_Track[k], Actual_Track[k]);
                    NU32DIP_WriteUART1(m);
                }

                break;         

            }

            // This is an additional feature making sure the data has been stored in corresponding array
            case 's':
            {
                char mn[50];
                for (int ij = 0; ij < N; ij++) {
                    sprintf(mn, "Index %d: %.2f\r\n", ij, Reference_Track[ij]);
                    NU32DIP_WriteUART1(mn);
                }
                break;
            }

            // Unpower the motor
            case 'p':
            {
                char m[50];
                // Set the mode back to default
                set_mode(IDLE);
                // Indicator info
                sprintf(m,"y\r\n");
                NU32DIP_WriteUART1(m);

                break;

            }

            // This is a debugging feature to monitor the position control and current control are working
            case'v':
            {
                char m[50];
                // Query the result from the position control PID
                sprintf(m,"%.5f,%d,%d,%d\r\n",Required_current_HOLD,E_accu_position,e, E_dot);
                NU32DIP_WriteUART1(m);

                break;
            }

            // Get the current mode
            case 'r':
            {
                print_mode();

                break;
            }

            // Default case, when a letter with no matching feature is asked
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
