
import serial
ser = serial.Serial('/dev/tty.usbserial-1410', 230400)
print('Opening port: ')
print(ser.name)

ser.write(b'0.5 0.015 1000\n') # Kp Ki eintmax

sampnum = 0
read_samples = 10
ADCval = []
ref = []
oc = []
eint = []


while read_samples > 1:
    data_read = ser.read_until(b'\n',50)
    data_text = str(data_read,'utf-8')
    data = list(map(int,data_text.split()))

    if(len(data)==3):
        read_samples = data[0]
        ADCval.append(data[1])
        ref.append(data[2])
        sampnum = sampnum + 1

    if(len(data)==5):
        read_samples = data[0]
        ADCval.append(data[1])
        ref.append(data[2])
        oc.append(data[3]*1024/2400)
        eint.append(data[4])
        sampnum = sampnum + 1

# plot it
import matplotlib.pyplot as plt 
t = range(len(ADCval)) # time array
if(len(data)==3):
    plt.plot(t,ADCval,'r-',t,ref,'b-')
if(len(data)==5):
    plt.plot(t,ADCval,'r*-',t,ref,'b*-',t,oc,'k*-',t,eint,'g*-')
plt.ylabel('value')
plt.xlabel('sample')
plt.show()


# be sure to close the port
ser.close()