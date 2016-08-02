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



# matplotlib.style.use('bmh')
# ['ggplot', 'bmh', 'grayscale', 'fivethirtyeight', 'dark_background']
#mpl.rcParams.update(mpl.rcParamsDefault)


def main ():

    global options, args

    # dir = '/noc-data/nw5x4cWC_ANALYSIS_ALL_TO_ONE/out/'
    dir = '/noc-data/nw5x4cWCA_2_TO_1/out/'


    home = expanduser("~")

    if options.inputfile == None:
        options.inputfile = home + dir + 'packets-trace-netdevice.csv'
    if options.outputdir == None:
        options.outputdir = home + dir

    # if not os.path.exists(options.outputdir):
        # os.makedirs(options.outputdir)


    data = files_io.load_list(options.inputfile)
    if (len(data)) == 0:
        print('Log file is empty')
        exit(1)

    # Could get it from the config file
    max_x = max( data[:,trace.x_absolute].astype(int) ) +1
    max_y = max( data[:,trace.y_absolute].astype(int) ) +1
    #


    pck_duration = ( float(options.packet_size) / float(options.baudrate) ) * 1e9


    ################# Extact from the Simulation data log ###################

    def show_hop(node_x, node_y, sw_in):

        x_transmitted_simul = []
        y_transmitted_simul = []
        y_received_simul = []
        x_received_simul = []

        traced_y = []
        traced_x = []

        received_simul = numpy.zeros([max_y, max_x])
        transmitted_simul = numpy.zeros([max_y, max_x])

        for line in data:
            abs_x = int( line[trace.x_absolute] )
            abs_y = int( line[trace.y_absolute] )


            time_slot = int(line[trace.time]) / pck_duration

            # t = int (round(time_slot,0)) - 1
            t = time_slot

            #Build the matrix for the density map
            if line[trace.operation] == 'r' or line[trace.operation] == 'g':
                received_simul[ abs_y, abs_x ] += 1
                # log_received.append([abs_y, abs_x])

                # Build the cumulative arrival/departure for an specific node
                if abs_x == node_x and abs_y == node_y:
                    x_received_simul.append(t)
                    y_received_simul.append(received_simul[node_y, node_x])

                    #point there at what time the traced packed passed by
                    if line[trace.protocol_app] == '6':
                        traced_x.append(t)
                        traced_y.append(received_simul[node_y, node_x])
            ################# Transmitted ####################

            #Build the matrix for the density map
            elif line[trace.operation] == 't': # or line[trace.operation] == 'g':
                transmitted_simul[ abs_y, abs_x ] += 1
                # log_transmitted.append([abs_y, abs_x])

                #Build the cumulative arrival/departure curve
                if abs_x == node_x and abs_y == node_y:
                    x_transmitted_simul.append(t+1)
                    y_transmitted_simul.append(transmitted_simul[node_y, node_x])

                    # point there at what time the traced packed passed by
                    if line[trace.protocol_app] == '6':
                        traced_x.append(t + 1) #Plus one time slot, the time to finish transmitting
                        traced_y.append(transmitted_simul[node_y, node_x])


        ################# Now the same data but from the model ###################

        if len(x_received_simul) != 0:

            [received_model, transmitted_model_eted, transmitted_model_queue] = wca.calculate_node(sw_in)

            # print(fout)

            y_received_model = []
            x_received_model = []
            for e in received_model:
                y_received_model.append(e[1])
                x_received_model.append(e[0])

            y_transmitted_model_eted = []
            x_transmitted_model_eted = []
            for e in transmitted_model_eted:
                y_transmitted_model_eted.append(e[1])
                x_transmitted_model_eted.append(e[0])

            y_trasmitted_model_queue = []
            x_trasmitted_model_queue = []
            for e in transmitted_model_queue:
                y_trasmitted_model_queue.append(e[1])
                x_trasmitted_model_queue.append(e[0])


            # try:
            #     y_diff = numpy.subtract(y_received, y_transmitted).tolist()
            #     x_diff = x_transmitted


            # grab from the model here
            plots = [
                    x_received_simul, y_received_simul,
                    x_transmitted_simul, y_transmitted_simul,
                    x_received_model, y_received_model,
                    x_transmitted_model_eted, y_transmitted_model_eted,
                    # x_diff, y_diff,
                    x_trasmitted_model_queue, y_trasmitted_model_queue,
                    # traced_x, traced_y
                    ]

            fn = options.outputdir + 'cumulative_n' + str(node_x) + ',' + str(node_y) + '_sw' + str(sw_in) + '.pdf'
            plotCumulativeInOut(plots, filename=fn)

            # plotMatrix(transmitted)



        else:
            print('The node selected have received no packets.')

    def calculateWorstCase():
        routing = 'yx'

        # orig = [0, 0]
        # dest = [4, 3]

         # network load mapping
        size = [5, 4]
        f = [1, 0, 1]  # flow each node generate
        network = [size[0] * [f] for i in range(size[1])]

        fo = [0, 0, 0]  # the output from other neighbors (none at t=0)

        if routing == 'yx':
            for i in range(len(network)):  # iterate over Y
                fi = network[0][i]
                swi = [fo, fi]

                fo = wca.resulting_flow(swi)
                swo = [fo]

                n_pck = wca.burst_size(swi)
                ti = wca.time_taken(swi, n_pck, direction='in')

                n_pck = wca.burst_size(swo)
                to = wca.time_taken(swo, n_pck, direction='out')

                delay = to - ti

                print('i = ' + str(i) + ' swi = ' + str(swi) + 'swo = ' + str(swo) + 'ti = ' + str(ti) + ' to = ' + str(
                    to) + ' d= ' + str(delay))

                show_hop(1, 3 - i, swi)

                # for j in range(len(network[0])): #iterate over X
                #     print(i)

    # node_x, node_y = 4, 0

    n = 1 #number of packets released per node
    # fa = [1, 0, 1 * n]  #my own flow, whereas the flows comming from neighbors take one time cycle more
    fb = [0.1, 1, 5] #the ones from top
    fc = [0.8, 1, 20] #and the remaning, coming from west, thre columns with 4 nodes each

    # sw_in = [fa, fb]
    sw_in = [fb, fc]

    # calculateWorstCase()

    show_hop(1,0, sw_in)



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

# def plotCumulativeInOut(x1, y1, x2, y2, x3=None, y3=None, x4=None, y4=None, x5=None, y5=None, x6=None, y6=None):
def plotCumulativeInOut(axis, filename=None):

    global options

    # filename = options.outputdir + 'cumulative_ad.pdf'
    show=True
    x_size = 6.5
    y_size = 3.1
    x_lim = None
    y_lim = None

    lines =   ["-","-","--",":","-.", ".", ","]
    markers = ["","","","","","D","D"]
    colours = ['lightgreen', 'yellow', 'black', 'black', 'blue', 'red', 'purple']
    labels = ['Arrivals', 'Departures', 'Arrivals model', 'Departures model', 'Queue size', 'WC Arrival', 'WC Departure']

    linecycler = cycle(lines)
    colourcycler = cycle(colours)
    labelcycler = cycle(labels)
    markercycler = cycle(markers)


    fig, ax_main = plt.subplots(figsize=(x_size, y_size), dpi=120, facecolor='w', edgecolor='w')

    for i in range(0, len(axis), 2):
        ax_i = ax_main
        ax_i.step(axis[i], axis[i+1], '-', linestyle=next(linecycler), label=next(labelcycler), where='post', color=next(colourcycler), marker=next(markercycler))

    ax_main.set_xlabel("Transmission time slot (TTS)")
    ax_main.set_ylabel("Cumulative packet count")

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
        parser.add_option ('-x', '--size_x', help='network size', default=1)
        parser.add_option ('-y', '--size_y', help='network size', default=0)
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
