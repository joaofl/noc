#!/usr/bin/python
# -*- coding: utf-8 -*-


import serialtmc
import serial
import random
import pickle
from time import sleep, time

import numpy as np
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt

def print_progress(i, n_samples):
    print(str(round(((i/n_samples)*100),2)) + '% ')


# measurement loop
n_samples = 100
br = 3000000
context = 'relay-delay-uc-pc'
measurements = []




pck = [0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02]
# a = random.random
output_dir = '/home/joao/noc-data/hw-measurements/'
fn = output_dir + context + '-' + str(n_samples/1000) + 'ks@' + str(br/1000000) + 'Mbps' + '.data'

#connect to FPGA serial
serial_port = serial.Serial(port='/dev/ttyUSB1', baudrate=br)
print('Connected = ' + str(serial_port.isOpen()))

serial_port.parity = serial.PARITY_NONE
serial_port.bytesize = serial.EIGHTBITS
serial_port.stopbits = serial.STOPBITS_ONE


i = 0

while i < n_samples:
    i += 1
    sleep_time = (random.randint(0,100) / 100)

    ti = time()
    serial_port.write(pck)
    d = serial_port.read()
    tf = time()

    t = round((tf - ti) * 1e6, 2)

    print(str(t) + ' us')


# pickle.dump(measurements, open( fn, 'wb' ))

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