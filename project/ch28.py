# chapter 28 in python

# sudo apt-get install python3-pip
# python3 -m pip install pyserial
# sudo apt-get install python3-matplotlib

import serial
from genref import genRef
import matplotlib.pyplot as plt 
from statistics import mean 
ser = serial.Serial('/dev/tty.usbserial-1410',230400)
print('Opening port: ')
print(ser.name)


def read_plot_matrix():
    n_str = ser.read_until(b'\n');  # get the number of data points to receive
    n_int = int(n_str) # turn it into an int
    print('Data lengeth = ' + str(n_int))
    ref = []
    data = []
    data_received = 0
    while data_received < n_int:
        dat_str = ser.read_until(b'\n');  # get the data as a string, ints seperated by spaces
        dat_int = list(map(float,dat_str.split())) # now the data is a list
        ref.append(dat_int[0])
        data.append(dat_int[1])
        data_received = data_received + 1
    meanzip = zip(ref,data)
    meanlist = []
    for i,j in meanzip:
        meanlist.append(abs(i-j))
    score = mean(meanlist)
    t = range(len(ref)) # index array
    plt.plot(t,ref,'r-',t,data,'b-')
    plt.title('Average Current Error = ' + str(score))
    plt.ylabel('Current (sensor count)')
    plt.xlabel('index')
    plt.show()

def read_plot_matrix_Track():
    n_str = ser.read_until(b'\n');  # get the number of data points to receive
    n_int = int(n_str) # turn it into an int
    print('Data lengeth = ' + str(n_int))
    ref = []
    data = []
    data_received = 0
    while data_received < n_int:
        dat_str = ser.read_until(b'\n');  # get the data as a string, ints seperated by spaces
        dat_int = list(map(float,dat_str.split())) # now the data is a list
        ref.append(dat_int[0])
        data.append(dat_int[1])
        data_received = data_received + 1
    meanzip = zip(ref,data)
    meanlist = []
    for i,j in meanzip:
        meanlist.append(abs(i-j))
    score = mean(meanlist)
    t = [i / 200.0 for i in range(len(ref))]  
    plt.plot(t,ref,'r-',t,data,'b-')
    plt.title('Average Angle Error = ' + str(score))
    plt.ylabel('Angle (degree)')
    plt.xlabel('index')
    plt.show()

has_quit = False
# menu loop
while not has_quit:
    print('PIC32 MOTOR DRIVER INTERFACE')
    # display the menu options; this list will grow
    print('\ta: Read current sensor in ADC count\n \tb: Read current sensor in mA count\n '
        '\tc: Encoder counts\n \td: Encoder count in degree\n \te: Reset encoder count\n '
        '\tf: Set PWM (-100 to 100)\n \tg: Change the Current PI control gain parameters\n \th: Get the Current PI control gain parameters\n'
        '\ti: Change the Position PID control gain parameters\n \tj: Get the Position PID control gain parameters\n' 
        '\tk: Itest Mode\n \tl: Go to desired deg and hold\n \tm: Load the Cubic Trajectory\n'
        '\tn: Load the Step Trajectory\n \tp: Unpower the motor\n \to: Execute trajectory\n'
        '\tr: Current mode\n \tq: Quit\n') # '\t' is a tab
    # read the user's choice
    selection = input('\nENTER COMMAND: ')
    selection_endline = selection+'\n'
     
    # send the command to the PIC32
    ser.write(selection_endline.encode()); # .encode() turns the string into a char array
    
    # take the appropriate action
    # there is no switch() in python, using if elif instead

    if (selection == 'a'):
        ser.reset_input_buffer()

        n_str = ser.read_until(b'\n')  # get the incremented number back
        n_int = int(n_str) # turn it into an int
        print('Current in ADC count: ' + str(n_int) + '\n') # print it to the screen   
    
    
    elif (selection == 'b'):
        ser.reset_input_buffer()

        n_str = ser.read_until(b'\n')  # get the incremented number back
        n_int = float(n_str) # turn it into an int
        print('Current in mA count: ' + str(n_int) + '\n') # print it to the screen
    
    
    elif (selection == 'c'):
        ser.reset_input_buffer()

        n_str = ser.read_until(b'\n')
        n_int = int(n_str)
        print('Encoder counts: ' + str(n_int) + '\n')
    
    
    elif (selection == 'd'):
        ser.reset_input_buffer()

        n_str = ser.read_until(b'\n')  # get the incremented number back
        n_int = float(n_str) # turn it into an int
        print('Encoder Degree: ' + str(n_int) + '\n') # print it to the screen
    
    
    elif (selection == 'e'):
        ser.reset_input_buffer()

        print("Encoder count has been reset")
    
    
    elif (selection == 'f'):
        ser.reset_input_buffer()

        print("Chosse the PWM duty cycle from -100 to 100")
        n_str = input('Enter number: ')
        n_int = int(n_str)
        ser.write((str(n_int)+'\n').encode())
   
   
    elif (selection == 'g'):
        ser.reset_input_buffer()

        print("Adjust the value of current Proportional and Integral error gain")
        n_str = input('Enter the value for Kp: ')
        n_float = float(n_str)
        ser.write((str(n_float)+'\n').encode())
        n_str = ser.read_until(b'\n')

        n_str = input('Enter the value for Ki: ')
        n_float = float(n_str)
        ser.write((str(n_float)+'\n').encode())
        n_str = ser.read_until(b'\n')

        print("Complete adjusting current control error gain")

   
    elif (selection == 'h'):
        ser.reset_input_buffer()

        n_str = ser.read_until(b'\n').decode().strip()
        k_p = float(n_str)
        n_str = ser.read_until(b'\n').decode().strip()
        k_i = float(n_str)
        print("Kp(current): " + str(k_p) + "\n")
        print("Ki(current): " + str(k_i) + "\n")

    elif (selection == 'i'):
        ser.reset_input_buffer()

        print("Adjust the value of Position Proportional, Integral, and Derivative error gain")
        n_str = input('Enter the value for Kp: ')
        n_float = float(n_str)
        ser.write((str(n_float)+'\n').encode())
        n_str = ser.read_until(b'\n')

        n_str = input('Enter the value for Ki: ')
        n_float = float(n_str)
        ser.write((str(n_float)+'\n').encode())
        n_str = ser.read_until(b'\n')

        n_str = input('Enter the value for Kd: ')
        n_float = float(n_str)
        ser.write((str(n_float)+'\n').encode())
        n_str = ser.read_until(b'\n')

        print("Complete adjusting position control error gain")

    elif (selection == 'j'):
        ser.reset_input_buffer()

        n_str = ser.read_until(b'\n').decode().strip()
        k_p = float(n_str)
        n_str = ser.read_until(b'\n').decode().strip()
        k_i = float(n_str)
        n_str = ser.read_until(b'\n').decode().strip()
        k_d = float(n_str)
        print("Kp(Position): " + str(k_p) + "\n")
        print("Ki(Position): " + str(k_i) + "\n")
        print("Kd(Position): " + str(k_d) + "\n")

    elif (selection == 'k'):  

        read_plot_matrix()    

    elif (selection == 'l'):
        ser.reset_input_buffer()

        n_str = input("Enter the desired degree:")
        n_float = float(n_str)
        ser.write((str(n_float)+'\n').encode())
        n_str = ser.read_until(b'\n')

    elif (selection == 'm'):
        ser.reset_input_buffer()

        ref = genRef('cubic')
        #print(len(ref))
        t = range(len(ref))
        plt.plot(t,ref,'r*-')
        plt.ylabel('ange in degrees')
        plt.xlabel('index')
        plt.show()
        # send 
        ser.write((str(len(ref))+'\n').encode())
        for i in ref:
            ser.write((str(i)+'\n').encode())
        n_str = ser.read_until(b'\n').decode()
        print('Complete Loading the cubic Trajectory')

    elif (selection == 'n'):
        ser.reset_input_buffer()

        ref = genRef('step')
        #print(len(ref))
        t = range(len(ref))
        plt.plot(t,ref,'r*-')
        plt.ylabel('ange in degrees')
        plt.xlabel('index')
        plt.show()
        # send 
        ser.write((str(len(ref))+'\n').encode())
        for i in ref:
            ser.write((str(i)+'\n').encode())

    elif (selection == 'o'):
        ser.reset_input_buffer()    

        read_plot_matrix_Track()      
   
    elif (selection == 'p'):
        ser.reset_input_buffer()

        print('Waiting response...')
        n_str = ser.read_until(b'\n')
        print('Complete Unpowering')
    
    
    elif (selection == 'q'):
        ser.reset_input_buffer()

        print('Exiting client')
        has_quit = True; # exit client
        # be sure to close the port
        ser.close()
    
    elif (selection == 'r'):
        ser.reset_input_buffer()

        received_str = ser.read_until(b'\n').decode().strip() 
        print(received_str)

    else:
        ser.reset_input_buffer()

        print('Invalid Selection ' + selection_endline)



