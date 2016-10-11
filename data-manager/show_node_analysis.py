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

FLOW_ID = 0
FLOW_X = 1
FLOW_Y = 2
FLOW_ROUTE = -1
FLOW_BURSTNESS = 3
FLOW_OFFSET = 4
FLOW_SIZE = 5


def main ():


    ############################# Initial settings #######################
    global options, args

    home = expanduser("~")

    inputfile_queue_size = options.inputdir + '/queue-size-trace.csv'
    inputfile_flows = options.inputdir + '/flows-trace.csv'
    inputfile_packet_trace = options.inputdir + '/packets-trace-netdevice.csv'
    inputfile_flows_source = options.inputdir + '/flows-source.csv'

    # if not os.path.exists(options.outputdir):
        # os.makedirs(options.outputdir)

    trace_packets = files_io.load_list(inputfile_packet_trace)
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
        # exit(1

    trace_flows_source = files_io.load_list(inputfile_flows_source)
    if (len(trace_flows)) == 0:
        print('Flows source trace is empty or not found.')


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

    def simulation_arrival_departure(node_x, node_y, packet_n=0):

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
                if line[HEADER.protocol_app] == '1':
                    #Build the matrix for the density map
                    if line[HEADER.operation] == 'r': #or line[HEADER.operation] == 'g':
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



        return [
            simul_r_x, simul_r_y,
            simul_t_x, simul_t_y
                ]

    def model_arrival_departure():
        # else:
        #     print('The node selected have received no packets.')




        ################# Now the same trace_packets but from the model ###################
        #
        #
        #
        # sw_in = incomming_flows_ub[node_y][node_x] # + [nodes_flows_ub[node_y][node_x]]
        # sw_out = [outgoing_flow_lb[node_y][node_x]]
        #
        # if (count_t + count_r) > 0:
        #
        #     #######
        #     def transform(data):
        #         x = []
        #         y = []
        #         for e in data:
        #             y.append(e[1])
        #             x.append(e[0])
        #         return [x, y]
        #     #######
        #
        #     [model_in, model_out] = wca.calculate_node(sw_in, sw_out)
        #
        #     [model_in_x, model_in_y] = transform(model_in)
        #     [model_out_x, model_out_y] = transform(model_out)
        #
        #     t_in_lb, t_out_lb, n_in_lb, n_out_lb = \
        #         model_node_bounds(traced_flow=traced_flow_lb[node_y][node_x],
        #                           in_flows=incomming_flows_lb[node_y][node_x],
        #                           out_flow=outgoing_flow_lb[node_y][node_x],
        #                           packet_n=packet_n)
        #
        #     t_in_ub, t_out_ub, n_in_ub, n_out_ub = \
        #         model_node_bounds(traced_flow=traced_flow_ub[node_y][node_x],
        #                           in_flows=incomming_flows_ub[node_y][node_x],
        #                           out_flow=outgoing_flow_ub[node_y][node_x],
        #                           packet_n=packet_n)
        #
        #
        #
        #     model_traced_x = [t_in_ub, t_out_lb]
        #     model_traced_y = [n_in_ub, n_in_lb]
        return[]


    def intersects(flows_list, node_x, node_y):
        arriving_flows_list = []
        for i in range(len(flows_list)):
            r = flows_list[i][FLOW_ROUTE]
            if [node_x, node_y] in r:
                # j = r.index([node_x, node_y])
                arriving_flows_list.append(i)

        return  arriving_flows_list


    def model_propagate(flow_matrix, flows_list): #the list with the flows and the index of the one we are looking for
        index = 0
        f = flows_list[index]

        # xo = f[FLOW_X]
        # yo = f[FLOW_X]

        route = f[FLOW_ROUTE]

        for [x,y] in route: #which flows intersect the observed flow, at each hop
            index_list = intersects(flows_list,x, y)
            print(index, x, y, index_list)

            if index_list == [index]: #contains myself only, self to propagate
                print('')


            #else, if someone else, check out that flow




    def model_backpropagate(flows_list_all, flows_matrix_all, node_x, node_y):

        f_none = [-1,-1,-1]  # unknown
        f_zero = [0, 0, 0]   # no flows from that node

        flows_source = [[f_zero for _ in range(len(flows_matrix_all[0]))] for _ in range(len(flows_matrix_all))]
        flows_outgoing = [[f_none for _ in range(len(flows_matrix_all[0]))] for _ in range(len(flows_matrix_all))] #create an empty matrix

        flows_list_filtered = []

        # find filter only the flows that pass by the node of interest
        for i in range(len(flows_list_all)):
            l = flows_list_all[i]

            route = l[FLOW_ROUTE]
            x = l[FLOW_X]
            y = l[FLOW_Y]
            flow = [l[FLOW_BURSTNESS], l[FLOW_OFFSET], l[FLOW_SIZE]]

            #add flows of interest to a separate list
            # TODO: issue here is that I capture also nodes that may cross the node, and not actually interfeer
            if [node_x, node_y] in route:
                flows_list_filtered.append(l)
                flows_source[y][x] = flow


        #from this point on, work only with the contending flows
        for l in flows_list_filtered:
            route = l[FLOW_ROUTE]
            # x = l[FLOW_X]
            # y = l[FLOW_Y]
            # flow = [l[FLOW_BURSTNESS], l[FLOW_OFFSET], l[FLOW_SIZE]]

            for [x,y] in route: #which flows intersect the observed flow, at each hop
                index_list = intersects(flows_list_filtered,x, y)
                print(x, y, index_list)






        #find which of the intersecting flows are the farthest of its row
        xs = {}
        ys = {}
        # max_x = max([flows_list[i][FLOW_X] for i in arriving_flows_list])

        for i in flows_list_filtered:
            x = flows_list_all[i][FLOW_X]
            y = flows_list_all[i][FLOW_Y]

            if not x in xs:
                xs[x] = [y]
            else:
                xs[x].append(y)

            if not y in ys:
                ys[y] = [x]
            else:
                ys[y].append(x)

        if len(flows_list_filtered) == 0:
            print('No flows depart or cross through this node')
            return -1

        # applies to quadrant x+y+
        for y in sorted(ys, reverse=True): #From top to bottom
            for x in range(max(ys[y]), 0, -1): #at each line (y) with a flow sorce, start from the farthest one

                if x == max(ys[y]):
                    sw_in = [flows_source[y][x]]
                else:
                    sw_in = [flows_source[y][x], flows_outgoing[y][x+1]]

                flows_outgoing[y][x] = wca.resulting_flow(sw_in, analysis='eted')
                print(x,y, flows_outgoing[y][x])


        for y in range(max(ys.keys()), node_y -1, -1):
            if y == max(ys.keys()):
                sw_in = [flows_source[y][node_x], flows_outgoing[y][node_x+1]]
            else:
                sw_in = [flows_source[y][node_x], flows_outgoing[y][node_x+1], flows_outgoing[y+1][node_x]]

            flows_outgoing[y][node_x] = wca.resulting_flow(sw_in, analysis='eted')

            print(node_x, y, flows_outgoing[y][x])


        return [flows_source, flows_outgoing, sw_in]


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

    node_x = int(options.pos_x)
    node_y = int(options.pos_y)
    f_none = [0, 0, 0]  #nodes own output flow

    nw_flows_matrix = [[f_none for _ in range(max_x)] for _ in range(max_y)]
    nw_flows_list = []

    for l in trace_flows_source:
        id = int(l[FLOW_ID])
        x = int(l[FLOW_X])
        y = int(l[FLOW_Y])
        p = l[FLOW_ROUTE].split(';')
        r = []
        for i in range(0, len(p), 2):
            r.append([int(p[i]), int(p[i+1])])

        f = [float(l[FLOW_BURSTNESS]), float(l[FLOW_OFFSET]), float(l[FLOW_SIZE])]
        nw_flows_matrix[y][x] = f
        nw_flows_list.append([id, x, y] + f + [r])



    # model_propagate(nw_flows_matrix, nw_flows_list, 0)
    flows_map = model_backpropagate(nw_flows_list, nw_flows_matrix, node_x, node_y)

    # if flows_map == -1:
    #     print("Nothing found for the node [{},{}]".format(node_x, node_y))
    #     return
    #
    # flows_departures = flows_map[1]

    # incomming_flows_lb, outgoing_flow_lb, traced_flow_lb = model_flows_io(nw_flows_matrix, node_x, node_y)
    # incomming_flows_ub, outgoing_flow_ub, traced_flow_ub = model_flows_io(nodes_flows_ub, x, y)

    #Checking the values now
    # show_simul_flows(plot=True)

    r = simulation_arrival_departure(node_x,node_y)


    plots = [
        r[0], r[1], #arrivals
        r[2], r[3], #departures

        # model_in_x, model_in_y,
        # model_out_x, model_out_y,

        # simul_traced_x[(packet_n - 1) * 2:packet_n * 2], simul_traced_y[(packet_n - 1) * 2:packet_n * 2],
        #     simul_traced_x, simul_traced_y,
        #     model_traced_x, model_traced_y,
    ]

    # sw_in = flows_map[2]
    sw_in = [[1,1,1], [2,2,2]]
    sw_in_f = []
    for i in range(len(sw_in)):
        ff = [float('%.2f' % elem) for elem in sw_in[i]]
        if ff != [0.0, 0.0, 0.0]:
            sw_in_f.append(ff)

    # sw_out_f = flows_departures[node_y][node_x]
    sw_out_f = [3.0,3.0,3.0]

    fn = options.outputdir + 'cumulative_n_' + str(node_x) + ',' + str(node_y) + '_sw_' + str(sw_in_f) + ' = ' + str(sw_out_f) + '.pdf'
    plotCumulativeInOut(plots, filename=fn)


if __name__ == '__main__':
    try:
        start_time = time.time()
        parser = optparse.OptionParser(formatter=optparse.TitledHelpFormatter(), usage=globals()['__doc__'], version='$Id$')

        parser.add_option ('-v', '--verbose', action='store_true', default=False, help='verbose output')
        parser.add_option ('-i', '--inputdir', help='Dir containing the logs', default=None)
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
