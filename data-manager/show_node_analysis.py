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

    dir = '/noc-data/nw5x4cWCA_ALL_TO_ONE/out/'
    # dir = '/noc-data/nw5x4cWCA_2_TO_1/out/'
    # dir = '/noc-data/nw5x4cWCA_LINE_TO_ONE/out/'

    home = expanduser("~")

    if options.inputfile == None:
        options.inputfile = home + dir + 'packets-trace-netdevice.csv'
    if options.outputdir == None:
        options.outputdir = home + dir + 'post/'

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



    def show_hop(node_x, node_y, sw_lb, sw_ub):

        ################# Get data from simulation logs ##################

        simul_t_x = []
        simul_t_y = []

        simul_r_x = []
        simul_r_y = []

        traced_y = []
        traced_x = []

        simul_r_count = numpy.zeros([max_y, max_x])
        simul_t_count = numpy.zeros([max_y, max_x])

        simul_q_max = numpy.zeros([max_y, max_x])
        simul_eted = numpy.zeros([max_y, max_x])

        for line in data:
            abs_x = int( line[trace.x_absolute] )
            abs_y = int( line[trace.y_absolute] )

            time_slot = int(line[trace.time]) / pck_duration

            t = time_slot

            ################# Received ####################

            #Build the matrix for the density map
            if line[trace.operation] == 'r' or line[trace.operation] == 'g':
                simul_r_count[ abs_y, abs_x ] += 1

                # Log the maximum queue size
                if (int(line[trace.queue_size]) > simul_q_max[ abs_y, abs_x ]):
                    simul_q_max [ abs_y, abs_x ] = line[trace.queue_size]

                # Build the cumulative arrival/departure for an specific node
                if abs_x == node_x and abs_y == node_y:
                    simul_r_x.append(t)
                    simul_r_y.append(simul_r_count[node_y, node_x])

                    #point there at what time the traced packed passed by
                    if line[trace.protocol_app] == '6':
                        traced_x.append(t)
                        traced_y.append(simul_r_count[node_y, node_x])

            ################# Transmitted ####################

            #Build the matrix for the density map
            elif line[trace.operation] == 't': # or line[trace.operation] == 'g':
                simul_t_count[ abs_y, abs_x ] += 1
                # log_transmitted.append([abs_y, abs_x])

                # Log the maximum queue size
                if (int(line[trace.queue_size]) > simul_q_max[abs_y, abs_x]):
                    simul_q_max[abs_y, abs_x] = line[trace.queue_size]

                #Build the cumulative arrival/departure curve
                if abs_x == node_x and abs_y == node_y:
                    simul_t_x.append(t+1)
                    simul_t_y.append(simul_t_count[node_y, node_x])

                    # point there at what time the traced packed passed by
                    if line[trace.protocol_app] == '6':
                        traced_x.append(t + 1) #Plus one time slot, the time to finish transmitting
                        traced_y.append(simul_t_count[node_y, node_x])


        ################# Now the same data but from the model ###################

        if len(simul_r_x) != 0:

            [model_r_lb, model_t_lb] = wca.calculate_node(sw_lb)
            [model_r_ub, model_t_ub] = wca.calculate_node(sw_ub)

            # print(fout)

            #######
            def transform(data):
                x = []
                y = []
                for e in data:
                    y.append(e[1])
                    x.append(e[0])
                return [x, y]
            #######

            [model_r_lb_x, model_r_lb_y] = transform(model_r_lb)
            [model_t_lb_x, model_t_lb_y] = transform(model_t_lb)

            [model_r_ub_x, model_r_ub_y] = transform(model_r_ub)
            [model_t_ub_x, model_t_ub_y] = transform(model_t_ub)

            # try:
            #     y_diff = numpy.subtract(y_received, y_transmitted).tolist()
            #     x_diff = x_transmitted


            # grab from the model here
            plots = [
                    simul_r_x, simul_r_y,
                    simul_t_x, simul_t_y,
                    model_r_ub_x, model_r_ub_y,
                    # model_t_ub_x, model_t_ub_y,
                    # model_r_lb_x, model_r_lb_y,
                    model_t_lb_x, model_t_lb_y,
                    # x_diff, y_diff,
                    # x_trasmitted_model_queue, y_trasmitted_model_queue,
                    # traced_x, traced_y
                    ]

            fn = options.outputdir + 'cumulative_n' + str(node_x) + ',' + str(node_y) + '_sw' + str(sw_lb) + '.pdf'
            plotCumulativeInOut(plots, filename=fn)

            # plotMatrix(simul_q_max)



        else:
            print('The node selected have received no packets.')

    def calculateWorstCase(flow_lb, flow_ub, distance_x, distance_y):

        fo_lb = [0, 0, 0]  # the output from other neighbors (none at t=0)
        fo_ub = [0, 0, 0]  # the output from other neighbors (none at t=0)


        for y in range(distance_y[0], distance_y[1] -1, -1):  # iterate over X
            fi_lb = flow_lb
            fi_ub = flow_ub #Change the release delay to the worst

            swi_lb = [fo_lb, fi_lb]
            swi_ub = [fo_ub, fi_ub]


            fcolumn_lb = fo_lb
            fcolumn_ub = fo_ub

            fo_lb = wca.resulting_flow(swi_lb, analysis='eted')
            fo_ub = wca.resulting_flow(swi_ub, analysis='eted')

            print(str(swi_lb) + " " + str(fo_lb))

            show_hop(distance_x[0], y, swi_lb, swi_ub)



        for x in range(distance_x[0] - 1, distance_x[1] - 1, -1):  # iterate over X
            swi_lb = [fo_lb, fcolumn_lb, fi_lb] #it already includes its own input flow
            swi_ub = [fo_ub, fcolumn_ub, fi_ub] #it already includes its own input flow

            fo_lb = wca.resulting_flow(swi_lb, analysis='eted')
            fo_ub = wca.resulting_flow(swi_ub, analysis='eted')

            print(str(swi_lb) + " " + str(fo_lb))

            show_hop(x, distance_y[1], swi_lb, swi_ub)

    f_lb = [0.06, 0.2, 5]  #my own flow, whereas the flows comming from neighbors take one time cycle more
    f_ub = [0.07, 0, 5] #the ones from top

    calculateWorstCase(f_lb,f_ub, [4,1], [3,0])

    # show_hop(1,1, sw_in)

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

    lines =   ["-","-",":","--",":","--"]
    markers = ["","","","","","","D","D"]
    colours = ['lightgreen', 'yellow', 'black', 'black', 'darkgrey', 'darkgrey', 'purple']
    labels = [
        'Arrivals',
        'Departures',
        'Arrivals UB',
        # 'Departures UP',
        # 'Arrivals LB',
        'Departures LB',
        'WC Departure'
    ]

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
