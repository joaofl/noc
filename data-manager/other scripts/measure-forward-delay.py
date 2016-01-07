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


# measurement loop
n_samples = 10
br = 1500000
context = 'fpga-'
measurements = []
# a = random.random
fn = 'relay-delay-' + context + str(n_samples/1000) + 'ks@' + str(br/1000000) + 'Mbps' + '.data'

#connect to FPGA serial
serial_port = serial.Serial(port='/dev/ttyUSB0', baudrate=br)
print('Connected = ' + str(serial_port.isOpen()))


# initialise device
connect_rigol = True

if connect_rigol == True:
    instr =  serialtmc.instrument() # Rigol DS1052E
    instr.connect('/dev/ttyUSB1', 9600)
    inf = instr.ask('*IDN?')
    print('Equipament found: '+ str(inf))


for i in range(n_samples):

    serial_port.write(('0000000000000000').encode('ascii'))

    if connect_rigol == True:
        try:
            sleep(0.5)
            d1 = instr.ask(':MEASURE:PDELAY? CHAN1')
            sleep(.1)
            instr.write(':KEY:RUN')
            fd1 = float(d1)
            measurements.append(fd1)
            progress = round(((i/n_samples)*100),2)
            print(str(progress) + '% o:' + str(d1) + ' c:' + str(fd1))
            sleep(.1)

            if i % 20 == 0: #save the file every 10 cycles
                pickle.dump(measurements, open( fn, 'wb' ))
                print('File ' + fn + ' updated')
        except:
            print('Failed to convert ' + str(d1))
            sleep(2)


    # sleep(0.1) #seconds


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