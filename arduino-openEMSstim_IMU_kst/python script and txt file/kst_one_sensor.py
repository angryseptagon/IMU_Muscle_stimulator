    # Abhishek Kashyap and Hao Su
    # Plotting multiple sensors on KST

import serial
import time

SERIAL_PORT = 'COM3'  # COM# on windows or /tty.usb... on Linux/Mac
serial_port = serial.Serial(SERIAL_PORT, 115200)

with open("kst_one_sensor.txt", "w") as f:
    f.write('Time Sensor_value\n')

try:
    starting_time = time.time()  # time in milliseconds

    while 1:
        current_time = (time.time() - starting_time) * 1000  # millisecond
        current_value = str(serial_port.readline())
        # print "current time = ", current_time, ", reading = ", current_value

        # print current_time, "    ", serial_port.readline()

        with open("kst_one_sensor4.txt", "a") as f:
            f.write(current_value.replace('b\'','').replace('\r\n\'',''))
except KeyboardInterrupt:
    print ("Keyboard Interrupt")
    # f.close()   # not needed since file is opened using 'with open' 
    #.replace('b\'','').replace('\r\n\'','')

