# from threading import Thread
import time
from time import sleep
import serial



class instrument:

    def __init__(self): #, dict):
        # self.dict = dict
        # self.serial_port_queue = []
        # self.thread = Thread
        self.initialize = True

    def connect(self, portname, baudrate):
        # self.serial_port.baudrate = 38400
        # self.serial_port.port = 5
        # self.serial_port.port = portname

        self.serial_port = serial.Serial(port=portname, baudrate=baudrate) #this command opens the port
        # , parity = serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, bytesize = serial.EIGHTBITS, timeout=1)
        # self.serial_port.open()

        try:
            if self.serial_port.isOpen() == True:
                self.serial_port.flushInput()
                msg = "Successfully connected to " + self.serial_port.portstr + "@" + str(self.serial_port.baudrate) + "bps"
                print(msg)

            else:
                msg = "Failed to open " + self.serial_port.portstr
                print(msg)
                exit(-1)
        except:
            msg = "Port " + self.serial_port.portstr + " busy or not found"
            print(msg)
            exit(-1)
            # self.timer_gd.stop()

        # self.statusbar.showMessage(msg, 0) # timeout 10s



    def disconnect(self):
        if self.serial_port.isOpen() == True:
            self.serial_port.close()
            self.serial_port_open = False
            msg = "Disconnected from " + self.serial_port.portstr
        else:
            self.serial_port_open = False
            msg = "Not connected to any serial port"

        print(msg)


    def ask(self, command):
        msg = ''
        if self.serial_port.isOpen() == True:
            self.serial_port.flushInput()
            text = str(command + '\n')
            self.serial_port.write(text.encode('ascii'))
            msg = "Sent: " + text

        else:
            msg = "Not connected"
        # print(msg)

        sleep(0.1)
        return self.read()

    def write(self, cmd):
        if self.serial_port.isOpen() == True:
            self.serial_port.flushInput()
            text = str(cmd + '\n')
            n = self.serial_port.write(text.encode('ascii'))

        return n


    def read(self):
        if self.serial_port.isOpen() == True:
            d = self.serial_port.readline()
        else:
            print("Not connected")

        sleep(0.1)
        return d#[2:-2] #remove the first and last 2 characters

    def ask_raw(self, command, num=1):
        msg = ''
        if self.serial_port.isOpen() == True:
            self.serial_port.flushInput()
            text = str(command + '\n')
            self.serial_port.write(text.encode('ascii'))
            msg = "Sent: " + text
        else:
            msg = "Not connected"
        # print(msg)

        sleep(0.1)
        return self.read_raw(num)

    def read_raw(self, num=1):
        if self.serial_port.isOpen() == True:
            d = self.serial_port.read(num)
        else:
            print("Not connected")

        sleep(0.1)
        return d





    # def read_serial_data(self):
    #     while 1:
    #         if self.serial_port_open == True:
    #             timestamp = time.clock()
    #             d = self.serial_port.readline()
    #             d = d.replace('\r', '')
    #             d = d.replace('\n', '')
    #             # s = 't=' + str(timestamp) + ' p=' + d
    #             # # s = s.replace('\n', '')
    #             self.serial_port_queue.append(d)
    #             # self.serial_port_queue.insert(0,d)
    #         else:
    #             sleep(10)
    #
    #
    # def pbSend_clicked(self, s=False):
    #
    #     if self.serial_port.isOpen() == True:
    #         if s == False:
    #             text = str(self.tbOutputData.text())
    #         else:
    #             text = s
    #         if text != '':
    #             self.serial_port.write(text)
    #         msg = "Data sent: " + text
    #     else:
    #         msg = "Not connected to any serial port"
    #
    #     self.statusbar.showMessage(msg, 2000)


