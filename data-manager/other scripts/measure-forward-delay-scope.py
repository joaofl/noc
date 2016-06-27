#!/usr/bin/python
# -*- coding: utf-8 -*-


import serialtmc
import serial
import random
import pickle
from time import sleep

import numpy as np
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt

def print_progress(i, n_samples):
    print(str(round(((i/n_samples)*100),2)) + '% ')


# measurement loop
n_samples = 100000
br = 3000000
context = 'relay-delay-uc-parallel'
measurements = []




pck = [0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02]
# a = random.random
output_dir = '/home/joao/noc-data/hw-measurements/'
fn = output_dir + context + '-' + str(n_samples/1000) + 'ks@' + str(br/1000000) + 'Mbps' + '.data'

#connect to FPGA serial
serial_port = serial.Serial(port='/dev/ttyUSB2', baudrate=br)
print('Connected = ' + str(serial_port.isOpen()))

serial_port.parity = serial.PARITY_NONE
serial_port.bytesize = serial.EIGHTBITS
serial_port.stopbits = serial.STOPBITS_ONE


# initialise device
connect_rigol = True

if connect_rigol == True:
    instr =  serialtmc.instrument() # Rigol DS1052E
    instr.connect('/dev/ttyUSB1', 9600)
    inf = instr.ask('*IDN?')
    print('Equipament found: '+ str(inf))



i = 0
retry = 0
failure = 0
while i < n_samples:

    serial_port.write(pck)

    if connect_rigol == True:
        try:
            sleep_time = 0.5 + (random.randint(0,100) / 100)
            sleep(sleep_time)
            # d1 = instr.ask(':MEASURE:PDELAY? CHAN1')
            d1 = instr.ask(':MEASURE:NDELAY? CHAN1')
            sleep(.1)
            instr.write(':KEY:RUN')
            #Try to convert
            fd1 = float(d1)
            # fd1 = fd1 - pck_duration_t
            #Continues if it is a valid value
            measurements.append(fd1)
            print_progress(i, n_samples)
            print('o:' + str(d1) + ' c:' + str(fd1))
            i += 1
            sleep(.1)


            if i % 20 == 0: #save the file every X cycles
                pickle.dump(measurements, open( fn, 'wb' ))
                print('File ' + fn + ' updated')
        except:
            sleep(.1)
            sd1 = str(d1)
            failure += 1
            if (sd1[2] == '<'):
                d1 = d1[1:-1]
                fd1 = float(d1)
                print_progress(i, n_samples)
                print('Below range of measurement ' + sd1 + '. Value stored was ' + str(fd1))
                measurements.append(fd1)
                i += 1

            else:
                print('Failed to convert ' + sd1)
                retry += 1

                # if retry == 10:
                #     break
                # sleep(2)




    # sleep(0.1) #seconds

pickle.dump(measurements, open( fn, 'wb' ))
print('File ' + fn + ' updated')

print('Number of failures: ' + str(failure))

if connect_rigol == True:
    instr.write(':KEY:LOCK DIS')
    instr.disconnect()

print('Complete.')

serial_port.close()



# example data
# mu = 100  # mean of distribution
# sigma = 15  # standard deviation of distribution
# x = mu + sigma * np.random.randn(10000)
#
# num_bins = 10
# # # the histogram of the data
# n, bins, patches = plt.hist(measurements, num_bins, normed=1, facecolor='green', alpha=0.5)
# # # add a 'best fit' line
# # y = mlab.normpdf(bins, mu, sigma)
# # plt.plot(bins, y, 'r--')
# # plt.xlabel('Smarts')
# # plt.ylabel('Probability')
# # plt.title(r'Histogram of IQ: $\mu=100$, $\sigma=15$')
#
# # Tweak spacing to prevent clipping of ylabel
# plt.subplots_adjust(left=0.15)
# plt.show()