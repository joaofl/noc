#!/usr/bin/python
# -*- coding: utf-8 -*-


import serialtmc
import serial
import time
import numpy
import matplotlib.pyplot as plot

# initialise device
instr =  serialtmc.instrument() # Rigol DS1102CD

instr.connect('/dev/ttyUSB1', 38400)

inf = instr.ask('*IDN?')
print('Equipament found: '+ str(inf))

# rawdata = instr.ask_raw(':WAV:DATA? CHAN2', 1024)

# read data
# instr.write(":STOP")
# Grab the data from channel 1
# instr.write(":WAV:POIN:MODE RAW")
#instr.write(":WAV:POIN:MODE NOR")


rawdata = instr.ask_raw(":WAV:DATA? CHAN1",1024)[10:]
rawdata2 = instr.ask_raw(":WAV:DATA? CHAN2",1024)[10:]# first ten bytes are header, so skip
data_size = len(rawdata)
#
# get metadata
frequency = float(instr.ask_raw(':MEASure:FREQuency? CHAN2'))

sample_rate = float(instr.ask(':ACQ:SAMP?'))
timescale = float(instr.ask(":TIM:SCAL?"))
timeoffset = float(instr.ask(":TIM:OFFS?"))
voltscale1 = float(instr.ask(':CHAN1:SCAL?'))
voltoffset1 = float(instr.ask(":CHAN1:OFFS?"))

voltscale2 = float(instr.ask(':CHAN2:SCAL?'))
voltoffset2 = float(instr.ask(":CHAN2:OFFS?"))

# show metadata
print ("Data size:      ", data_size)

print('Frequency:\t\t\t' + str(frequency))

print ("Sample rate:\t\t", sample_rate)
print ("Time scale:\t\t\t", timescale)
print ("Time offset:\t\t\t", timeoffset)

print ("Voltage offset ch1:\t\t\t", voltoffset1)
print ("Voltage scale ch1:\t\t\t", voltscale1)

print ("Voltage offset ch2:\t\t", voltoffset2)
print ("Voltage scale ch2:\t\t", voltscale2)
#
# # convert data from (inverted) bytes to an array of scaled floats
# # this magic from Matthew Mets
data = numpy.frombuffer(rawdata, 'B')
data = data * -1 + 255
data = (data - 130.0 - voltoffset1/voltscale1*25) / 25 * voltscale1

data2 = numpy.frombuffer(rawdata2, 'B')
data2 = data2 * -1 + 255
data2 = (data2 - 130.0 - voltoffset2/voltscale2*25) / 25 * voltscale2

# creat array of matching timestamps
time = numpy.linspace(timeoffset - 6 * timescale, timeoffset + 6 * timescale, num=len(data) )

# scale time series and label accordingly
if (time[-1] < 1e-3):
    time = time * 1e6
    tUnit = u"\u00B5\u0053"
elif (time[-1] < 1):
    time = time * 1e3
    tUnit = "mS"
else:
    tUnit = "S"
#
# instr.write(":RUN")
# instr.write(":KEY:FORC")


# Plot the data
plot.figure(1)
ax1 = plot.subplot(211)
plot.plot(time, data)
plot.title("Channel 1")
plot.ylabel("Voltage (V)")
plot.xlabel("Time (" + tUnit + ")")
plot.xlim(time[0], time[-1])

plot.subplots_adjust(hspace = 0.5)

plot.subplot(212, sharey=ax1)
plot.title("Channel 2")
plot.ylabel("Voltage (V)")
plot.xlabel("Time (" + tUnit + ")")
plot.xlim(time[0], time[-1])
#plot.ylim(ax1.get_ylim())
plot.plot(time, data2)

plot.show()

instr.disconnect()