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
# import matplotlib as mpl
import copy
from itertools import cycle
import packet_structure as HEADER
import files_io
from os.path import expanduser

import others.analysis_wc as wca

import others.analysis_wc



# matplotlib.style.use('bmh')
# ['ggplot', 'bmh', 'grayscale', 'fivethirtyeight', 'dark_background']
#mpl.rcParams.update(mpl.rcParamsDefault)


def main ():


    ############################# Initial settings #######################
    global options, args

    dir = '/noc-data/nw5x4cWCA_ALL_TO_ONE/out/'
    # dir = '/noc-data/nw5x4cWCA_2_TO_1/out/'
    # dir = '/noc-data/nw5x4cWCA_LINE_TO_ONE/out/'

    home = expanduser("~")

    if options.outputdir == None:
        options.outputdir = home + dir + 'post/'
    if options.inputfile == None:
        options.inputfile = home + dir + 'packets-trace-netdevice.csv'

    inputfile_queue_size = home + dir + 'queue-size-trace.csv'
    inputfile_flows = home + dir + 'flows-trace.csv'

    # if not os.path.exists(options.outputdir):
        # os.makedirs(options.outputdir)

    trace_packets = files_io.load_list(options.inputfile)
    if (len(trace_packets)) == 0:
        print('Log file is empty')
        exit(1)

    trace_queue_size = files_io.load_list(inputfile_queue_size)
    if (len(trace_queue_size)) == 0:
        print('Queue size trace is empty or not found.')
        # exit(1)

    trace_flows = files_io.load_list(inputfile_flows)
    if (len(trace_flows)) == 0:
        print('Flows trace is empty or not found.')
        # exit(1)

    # Could get it from the config file
    max_x = max(trace_packets[:, HEADER.x_absolute].astype(int)) + 1
    max_y = max(trace_packets[:, HEADER.y_absolute].astype(int)) + 1

    pck_duration = ( float(options.packet_size) / float(options.baudrate) ) * 1e9

    ################################## FUNCTIONS ###############################

    def distance(x1,y1,x2,y2):
        if x1 > x2:
            dx = abs(x1 - x2)
        else:
            dx = abs(x2 - x1)
        if y1 > y2:
            dy = abs(y1 - y2)
        else:
            dy = abs(y2 - y1)

        return dx + dy

    def show_simul_flows(x='all', y='all', n='all', plot=False):
        simul_eted_min = numpy.zeros([max_y, max_x])
        simul_eted_max = numpy.zeros([max_y, max_x])

        i = -1 #to start from zero
        t = 0

        for line in trace_flows:
            abs_x = int(line[HEADER.x_absolute])
            abs_y = int(line[HEADER.y_absolute])
            i += 1
            if line[HEADER.operation] == 'g':
                pck_id = line[HEADER.id]

                if x == abs_x and y == abs_y:
                    #get the last packet generated
                    t = float(line[HEADER.time]) / pck_duration


                for j in range(i+1 ,len(trace_flows)):

                    if trace_flows[j][HEADER.id] == pck_id:
                        eted = float(int(trace_flows[j][HEADER.time]) - float(line[HEADER.time])) / pck_duration

                        dist = distance(abs_x, abs_y,
                                        int(trace_flows[j][HEADER.x_absolute]), int(trace_flows[j][HEADER.y_absolute]))

                        etedn = round(eted / dist, 2)
                        eted = round(eted, 2)

                        print('eted=' + str(eted) + ' d=' + str(dist) + ' etedn=' + str(etedn))

                        if etedn > simul_eted_max[abs_y][abs_x]:
                            simul_eted_max[abs_y][abs_x] = etedn

                        if etedn < simul_eted_min[abs_y][abs_x] or simul_eted_min[abs_y][abs_x] == 0:
                            simul_eted_min[abs_y][abs_x] = etedn

        if plot:
            plotMatrix(simul_eted_max)
            plotMatrix(simul_eted_min)

        return(t)

    def show_simul_stats():

        ################# Get trace_packets from simulation logs ##################

        simul_q_max = numpy.zeros([max_y, max_x])

        simul_r_count = numpy.zeros([max_y, max_x])
        simul_t_count = numpy.zeros([max_y, max_x])

        for line in trace_queue_size:
            abs_x = int(line[HEADER.x_absolute])
            abs_y = int(line[HEADER.y_absolute])

            # Build the density map
            if line[HEADER.operation] == 'r' or line[HEADER.operation] == 'g':
                simul_r_count[abs_y, abs_x] += 1
            elif line[HEADER.operation] == 't':# or line[HEADER.operation] == 'c':
                simul_t_count[abs_y, abs_x] += 1

            # Log the maximum queue size
            if (int(line[HEADER.queue_size]) > simul_q_max[abs_y, abs_x]):
                simul_q_max[abs_y, abs_x] = line[HEADER.queue_size]

        ################# Now the same trace_packets but from the model ###################



        plotMatrix(simul_q_max)
        plotMatrix(simul_r_count)

    def show_node(node_x, node_y, packet_n=0):

        ################# Get trace_packets from simulation logs ##################

        simul_t_x = []
        simul_t_y = []

        simul_r_x = []
        simul_r_y = []

        simul_traced_y = []
        simul_traced_x = []

        count_r = 0;
        count_t = 0;

        for line in trace_packets:
            abs_x = int(line[HEADER.x_absolute])
            abs_y = int(line[HEADER.y_absolute])

            time_slot = int(line[HEADER.time]) / pck_duration

            t = time_slot

            if abs_x == node_x and abs_y == node_y:

                ################# Received ####################
                #Build the matrix for the density map
                if line[HEADER.operation] == 'r' or line[HEADER.operation] == 'g':
                    # Build the cumulative arrival/departure for an specific node
                    count_r += 1
                    simul_r_x.append(t)
                    simul_r_y.append(count_r)

                    #point there at what time the traced packed passed by
                    if line[HEADER.protocol_app] == '6':
                        simul_traced_x.append(t)
                        simul_traced_y.append(count_r)

                ################# Transmitted ####################
                elif line[HEADER.operation] == 't': # or line[trace.operation] == 'g':
                    #Build the cumulative arrival/departure curve
                    count_t += 1
                    simul_t_x.append(t+1)
                    simul_t_y.append(count_t)

                    # point there at what time the traced packed passed by
                    if line[HEADER.protocol_app] == '6':
                        simul_traced_x.append(t + 1) #Plus one time slot, the time to finish transmitting
                        simul_traced_y.append(count_t)


        ################# Now the same trace_packets but from the model ###################



        sw_in = incomming_flows_ub[node_y][node_x] # + [nodes_flows_ub[node_y][node_x]]
        sw_out = [outgoing_flow_lb[node_y][node_x]]

        if len(simul_r_x) != 0:

            #######
            def transform(data):
                x = []
                y = []
                for e in data:
                    y.append(e[1])
                    x.append(e[0])
                return [x, y]
            #######

            [model_in, model_out] = wca.calculate_node(sw_in, sw_out)

            [model_in_x, model_in_y] = transform(model_in)
            [model_out_x, model_out_y] = transform(model_out)

            t_in_lb, t_out_lb, n_in_lb, n_out_lb = \
                model_node_bounds(traced_flow=traced_flow_lb[node_y][node_x],
                                  in_flows=incomming_flows_lb[node_y][node_x],
                                  out_flow=outgoing_flow_lb[node_y][node_x],
                                  packet_n=packet_n)

            t_in_ub, t_out_ub, n_in_ub, n_out_ub = \
                model_node_bounds(traced_flow=traced_flow_ub[node_y][node_x],
                                  in_flows=incomming_flows_ub[node_y][node_x],
                                  out_flow=outgoing_flow_ub[node_y][node_x],
                                  packet_n=packet_n)



            model_traced_x = [t_in_ub, t_out_lb]
            model_traced_y = [n_in_ub, n_in_lb]

            plots = [
                    simul_r_x, simul_r_y,
                    simul_t_x, simul_t_y,

                    # model_in_x, model_in_y,
                    # model_out_x, model_out_y,

                # simul_traced_x[(packet_n - 1) * 2:packet_n * 2], simul_traced_y[(packet_n - 1) * 2:packet_n * 2],
                #     simul_traced_x, simul_traced_y,
                #     model_traced_x, model_traced_y,
                    ]

            fn = options.outputdir + 'cumulative_n' + str(node_x) + ',' + str(node_y) + '_sw' + str(sw_in) + '.pdf'
            plotCumulativeInOut(plots, filename=fn)

        else:
            print('The node selected have received no packets.')

        return n_in_ub



    def model_flows_io(nodes_flows, x_traced, y_traced):
        # resulting_flows = copy.deepcopy(nodes_flows)
        outgoing_flows = [[0 for _ in range(max_x)] for _ in range(max_y)]
        incoming_flows = [[0 for _ in range(max_x)] for _ in range(max_y)] #including mine
        traced_flows = [[0 for _ in range(max_x)] for _ in range(max_y)] #the flow that carries the packet we are tracing

        #Considering routing is y first, which holds for +x+y quadrant if using clockwise routing
        for x in range(len(nodes_flows[0]) -1, -1, -1):
            for y in range(len(nodes_flows) -1, -1, -1):

                if y == y_traced and x == x_traced:
                    traced_flows[y][x] = nodes_flows[y][x]
                elif y < y_traced and y > 0 and x == x_traced:
                    traced_flows[y][x] = outgoing_flows[y+1][x]
                elif (y == 0):
                    if x == x_traced:
                        traced_flows[y][x] = outgoing_flows[y + 1][x]
                    elif x < x_traced:
                        traced_flows[y][x] = outgoing_flows[y][x + 1]

                if (y == len(nodes_flows) -1): #node on the top edge. Only its own flow
                    sw_in = [[0,0,0]]
                elif (y < len(nodes_flows) -1 and y > 0): #myself and the node before myself
                    sw_in = [outgoing_flows[y+1][x]]
                elif (y == 0): #nodes in the sink's line. Get from right and top
                    if (x == len(nodes_flows[0])-1): #if on the right edge, it only gets from top
                        sw_in = [outgoing_flows[y + 1][x]]
                    elif (x < len(nodes_flows[0])-1):
                        sw_in = [outgoing_flows[y + 1][x], outgoing_flows[y][x+1]]

                sw_in.append(nodes_flows[y][x])
                sw_in = list(reversed(sw_in)) #tweak for better file names
                incoming_flows[y][x] = copy.deepcopy(sw_in) #add my own flow to calculate the output
                outgoing_flows[y][x] = wca.resulting_flow(sw_in, analysis='eted')

        return incoming_flows, outgoing_flows, traced_flows

    def model_node_bounds(traced_flow, in_flows, out_flow, packet_n=0):
        swi = in_flows  # the incomming
        swo = [out_flow]
        # get t_in from the packet of interest, after, use t_out from previous iteration
        t_in = wca.time_taken(traced_flow, n=packet_n, direction='in')
        n_in = wca.produced_until(t_in, swi) + 1  # plus itself

        t_out = wca.time_taken(out_flow, n_in, direction='out')
        n_out = wca.produced_until(t_in, swo) + 1  # plus itself

        return t_in, t_out, n_in, n_out

    def plotMatrix(data):

        filename = None;
        show = True;
        title = "";
        lable_x = "";
        lable_y = "";
        x_size = 8;
        y_size = 8;

        plt.figure(title, figsize=(x_size, y_size), dpi=120, facecolor='w', edgecolor='w')
        plt.imshow(data, cmap=plt.get_cmap('hot_r'), interpolation='nearest', origin='lower')

        # plt.colorbar()

        plt.xlabel(lable_x)
        plt.ylabel(lable_y)

        plt.tight_layout(pad=2, w_pad=1, h_pad=0.9)

        for y in range(data.shape[0]):
            for x in range(data.shape[1]):
                plt.text(x, y, '%d' % data[y, x],
                         horizontalalignment='center',
                         verticalalignment='center',
                         )

        if filename != None:
            dir = filename[:filename.rfind("/")]
            if not os.path.exists(dir):
                os.makedirs(dir)
            plt.savefig(filename)

        # plt.ion()
        # plt.show()

        elif show == True:
            plt.show()
            # plt.pause(0.001)
            # plt.draw()


    def plotCumulativeInOut(axis, filename=None):

        global options

        # filename = options.outputdir + 'cumulative_ad.pdf'
        show = True
        x_size = 6.5
        y_size = 3.1
        x_lim = None
        y_lim = None

        lines = ["-", "-", ":", "--", " ", " ", " ", " "]
        markers = ["", "", "", "", "o", "*", "o", "o"]
        colours = ['lightgreen', 'yellow', 'black', 'black', 'cyan', 'blue', 'purple', 'purple']
        labels = [
            'Arrivals',
            'Departures',
            'Arrivals UB',
            # 'Departures UP',
            # 'Arrivals LB',
            'Departures LB',
            'Traced',
            'Bounds'
        ]

        linecycler = cycle(lines)
        colourcycler = cycle(colours)
        labelcycler = cycle(labels)
        markercycler = cycle(markers)

        fig, ax_main = plt.subplots(figsize=(x_size, y_size), dpi=120, facecolor='w', edgecolor='w')

        for i in range(0, len(axis), 2):
            ax_i = ax_main
            ax_i.step(axis[i], axis[i + 1], '-', linestyle=next(linecycler), label=next(labelcycler), where='post',
                      color=next(colourcycler), marker=next(markercycler))

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


    ################################# Running ####################################

    f_lb = [0.05, 0, 5]  #nodes own output flow
    f_ub = [0.05, 0, 5]  #nodes own output flow

    nodes_flows_lb = [[f_lb for _ in range(max_x)] for _ in range(max_y)]
    nodes_flows_ub = [[f_ub for _ in range(max_x)] for _ in range(max_y)]
    # nodes_flows_lb[3][4] = [0.0625, 0, 10]

    x = int(options.pos_x)
    y = int(options.pos_y)
    # n_in = 2 #which packet of the message to trace and calculate eted to

    incomming_flows_lb, outgoing_flow_lb, traced_flow_lb = model_flows_io(nodes_flows_lb, x, y)
    incomming_flows_ub, outgoing_flow_ub, traced_flow_ub = model_flows_io(nodes_flows_ub, x, y)

    #Checking the values now
    # show_simul_flows(plot=True)

    show_node(x,y)


    # for yi in range(y, 0, -1):
    #     n_in = show_node(x, yi, packet_n=n_in)
    # for xi in range(x, 0, -1):
    #     n_in = show_node(xi, 0, packet_n=n_in)

    # n_in = show_node(x-1, y, packet_n=n_in)
    # n_in = show_node(x-2, y, packet_n=n_in)
    # n_in = show_node(x-3, y, packet_n=n_in)


if __name__ == '__main__':
    try:
        start_time = time.time()
        parser = optparse.OptionParser(formatter=optparse.TitledHelpFormatter(), usage=globals()['__doc__'], version='$Id$')

        parser.add_option ('-v', '--verbose', action='store_true', default=False, help='verbose output')
        parser.add_option ('-i', '--inputfile', help='input file containing the packet trace', default=None)
        # parser.add_option ('-c', '--inputconfigfile', help='config file containing the simulation parameters')
        parser.add_option ('-o', '--outputdir', help='', default=None)

        # parser.add_option ('-t', '--timeslotsize', help='time between two refreshes of the animator')
        parser.add_option ('-x', '--size_x', help='network size')
        parser.add_option ('-y', '--size_y', help='network size')
        parser.add_option ('--pos_x', help='node to analyse', default=0)
        parser.add_option ('--pos_y', help='node to analyse', default=0)
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
