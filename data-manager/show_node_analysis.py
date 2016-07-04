#!/usr/bin/env python

# Copyright (c) 2015 Joao Loureiro
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Author: Joao Loureiro <joflo@isep.ipp.pt>

__author__ = 'Joao Loureiro <joflo@isep.ipp.pt>'


import sys, os, traceback, optparse
import time
import numpy
import matplotlib.pyplot as plt
import matplotlib as mpl
from itertools import cycle
import packet_structure as trace
import files_io
from os.path import expanduser

import others.analysis_wc as wca

import others.analysis_wc

home = expanduser("~")

# matplotlib.style.use('bmh')
# ['ggplot', 'bmh', 'grayscale', 'fivethirtyeight', 'dark_background']
#mpl.rcParams.update(mpl.rcParamsDefault)


def main ():

    global options, args

    dir = '/noc-data/nw3x3cWC_ANALYSIS_F724/out/'
    # dir = '/noc-data/nw3x3cWC_ANALYSIS_F123/out/'
    # dir = '/noc-data/nw3x3cWC_ANALYSIS_F222/out/'

    if options.inputfile == None:
        options.inputfile = home + dir + 'packets-trace-netdevice.csv'
    if options.outputdir == None:
        options.outputdir = home + dir

    if not os.path.exists(options.outputdir):
        os.makedirs(options.outputdir)


    data = files_io.load_list(options.inputfile)
    if (len(data)) == 0:
        print('Log file is empty')
        exit(1)

    #Could get it from the config file
    max_x = max( data[:,trace.x_absolute].astype(int) )
    max_y = max( data[:,trace.y_absolute].astype(int) )

    node_x, node_y = 1, 1

    axis_x_transmitted=[]
    axis_y_transmitted=[]
    axis_y_received = []
    axis_x_received = []

    # print list[:,trace.x_absolute]
    received = numpy.zeros([max_y + 1, max_x + 1])

    transmitted = numpy.zeros([max_y + 1, max_x + 1])

    log_received = []
    log_transmitted = []

    pck_duration = ( float(options.packet_size) / float(options.baudrate) ) * 1e9


    ################# Extact from the Simulation data log ###################

    for line in data:
        abs_x = int( line[trace.x_absolute] )
        abs_y = int( line[trace.y_absolute] )


        time_slot = int(line[trace.time]) / pck_duration

        t = int (round(time_slot,0)) - 1

        #Build the matrix for the density map
        if line[trace.operation] == 'r':
            received[ abs_y, abs_x ] += 1
            log_received.append([abs_y, abs_x])

            # Build the cumulative arrival/departure curve
            if abs_x == node_x and abs_y == node_y:
                axis_x_received.append(t)
                axis_y_received.append(received[node_y, node_x])

        #Build the matrix for the density map
        elif line[trace.operation] == 't': # or line[trace.operation] == 'g':
            transmitted[ abs_y, abs_x ] += 1
            log_transmitted.append([abs_y, abs_x])

            #Build the cumulative arrival/departure curve
            if abs_x == node_x and abs_y == node_y:
                axis_x_transmitted.append(t+1)
                axis_y_transmitted.append(transmitted[node_y, node_x])




    ################# Now the same data but from the model ###################

    x_bound = []

    if len(axis_x_received) != 0:
        x_bound = numpy.linspace(0, axis_x_transmitted[-1], num=axis_x_transmitted[-1] + 1)

        # for x in x_bound:
        [y_bound_received, y_bound_transmited] = wca.calculate_node(axis_x_transmitted[-1] + 1)

            # grab from the model here
    else:
        print('The node selected have received no packets.')



    plotCumulativeInOut(axis_x_received, axis_y_received, axis_x_transmitted, axis_y_transmitted, x_bound, y_bound_received, x_bound, y_bound_transmited)
    plotMatrix(transmitted)
    # plt.show()

    # received = numpy.flipud(received)
    # transmitted = numpy.flipud(transmitted)

    # print(received)
    # print('\n')
    # print(transmitted)


def plotMatrix(data):

    filename=None; show=True; title = ""; lable_x = ""; lable_y = ""; x_size = 8; y_size = 8;

    plt.figure(title, figsize=(x_size, y_size), dpi=120, facecolor='w', edgecolor='w')
    plt.imshow(data, cmap=plt.get_cmap('hot_r'), interpolation='nearest', origin='lower')

    # plt.colorbar()

    plt.xlabel(lable_x)
    plt.ylabel(lable_y)

    plt.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)

    for y in range(data.shape[0]):
        for x in range(data.shape[1]):
            plt.text(x, y, '%d' % data[y, x],
                 horizontalalignment='center',
                 verticalalignment='center',
                 )

    if filename!=None:
        dir = filename[:filename.rfind("/")]
        if not os.path.exists(dir):
            os.makedirs(dir)
        plt.savefig(filename)

    # plt.ion()
    # plt.show()

    elif show==True:
        plt.show()
        # plt.pause(0.001)
        # plt.draw()

def plotCumulativeInOut(x1, y1, x2, y2, x3=None, y3=None, x4=None, y4=None):

    global options

    filename = options.outputdir + 'cumulative_ad.pdf'
    show=True
    x_size = 6.5
    y_size = 3.1
    x_lim = None
    y_lim = None


    lines = ["-","-","--",":","-."]
    colours = ['lightgreen', 'yellow', 'black', 'black']
    linecycler = cycle(lines)
    colourcycler = cycle(colours)

    fig, ax1 = plt.subplots(figsize=(x_size, y_size), dpi=120, facecolor='w', edgecolor='w')
    ax1.step(x1, y1, '-', linestyle=next(linecycler), label='Received', where='post', color=next(colourcycler))

    ax2 = ax1
    # s2 = np.sin(2*np.pi*t)
    ax2.step(x2, y2, '-', linestyle=next(linecycler), label='Transmitted', where='post', color=next(colourcycler))

    if x3 is not None and y3 is not None:
        ax3 = ax1
        ax3.step(x3, y3, '-', linestyle=next(linecycler), label='Upper bound', where='post', color=next(colourcycler))

    if x4 is not None and y4 is not None:
        ax4 = ax1
        ax4.step(x4, y4, '-', linestyle=next(linecycler), label='Lower bound', where='post', color=next(colourcycler))

    ax1.set_xlabel("Transmission time slot (TTS)")
    ax1.set_ylabel("Cumulative packet count")

    if x_lim is not None: plt.xlim(x_lim)
    if y_lim is not None: plt.ylim(y_lim)



    plt.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
    plt.legend(loc=0, fontsize=11)
    plt.grid(True)

    # ax = plt.gca()
    # ax.set_xticklabels(x)

    # plt.locator_params(axis='x', nbins=len(x))

    if filename is not None:
        dir = filename[:filename.rfind("/")]
        if not os.path.exists(dir):
            os.makedirs(dir)
        plt.savefig(filename)
        print("Plot saved in " + filename)

    if show == True:
        plt.show()

if __name__ == '__main__':
    try:
        start_time = time.time()
        parser = optparse.OptionParser(formatter=optparse.TitledHelpFormatter(), usage=globals()['__doc__'], version='$Id$')

        parser.add_option ('-v', '--verbose', action='store_true', default=False, help='verbose output')
        parser.add_option ('-i', '--inputfile', help='input file containing the packet trace', default=None)
        # parser.add_option ('-c', '--inputconfigfile', help='config file containing the simulation parameters')
        parser.add_option ('-o', '--outputdir', help='', default=None)

        parser.add_option ('-t', '--timeslotsize', help='time between two refreshes of the animator')
        parser.add_option ('-x', '--size_x', help='network size', default=3)
        parser.add_option ('-y', '--size_y', help='network size', default=3)
        parser.add_option ('-s', '--sinks_n', help='number of sinks', default=1)
        parser.add_option ('-b', '--baudrate', help='baudrate utilized', default=3000000)
        parser.add_option ('-p', '--packet_size', help='packet_size in bits', default=16*10)
        parser.add_option ('-n', '--size_neighborhood', help='neighborhood size', default=0)


        (options, args) = parser.parse_args()
        #if len(args) < 1:
        #    parser.error ('missing argument')
        if options.verbose: print (time.asctime())
        main()
        if options.verbose: print (time.asctime())
        if options.verbose: print ('Total execution time (s):')
        if options.verbose: print (time.time() - start_time)
        sys.exit(0)

    # except (KeyboardInterrupt, e): # Ctrl-C
    #     raise e
    # except SystemExit, e: # sys.exit()
    #     raise e

    except (Exception):
        print ('ERROR, UNEXPECTED EXCEPTION')
        print (str(Exception))
        traceback.print_exc()
        os._exit(1)
