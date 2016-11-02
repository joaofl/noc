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

#Reference to the fields from simulator's flow source log file
FLOW_NODE_ID = 0
FLOW_X = 1
FLOW_Y = 2
FLOW_BURSTNESS = 3
FLOW_OFFSET = 4
FLOW_SIZE = 5
FLOW_ROUTE = 6

DIRECTION_MASK_NONE             = 0b00000000
DIRECTION_MASK_E                = 0b00000001
DIRECTION_MASK_S                = 0b00000010
DIRECTION_MASK_W                = 0b00000100
DIRECTION_MASK_N                = 0b00001000
DIRECTION_MASK_L                = 0b00010000
DIRECTION_MASK_ALL              = 0b00011111
DIRECTION_MASK_ALL_EXCEPT_LOCAL = 0b00001111


def main ():


    ############################# Initial settings #######################
    global options, args
    global flow_model_list_g
    flow_model_list_g = []
    global flow_model_dict_g
    flow_model_dict_g = {}

    global counter_iteration_g
    counter_iteration_g = 0
    global counter_recursion_g
    counter_recursion_g = 0
    global pck_duration

    # home = expanduser("~")

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
    if (len(trace_flows_source)) == 0:
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
            plot_matrix(simul_eted_max)
            plot_matrix(simul_eted_min)

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



        plot_matrix(simul_q_max)
        plot_matrix(simul_r_count)

    def simulation_eted():
        flow_trace_dict = {}
        for line in trace_packets:
            id = int(line[HEADER.id])
            abs_x = int(line[HEADER.x_absolute])
            abs_y = int(line[HEADER.y_absolute])
            t = int(line[HEADER.time])

            if not id in flow_trace_dict:
                flow_trace_dict[id] = [t, t, abs_x, abs_y, abs_x, abs_y] #ti, tf, xy orig, xy dest
            else:
                flow_trace_dict[id][1] = t #update tf and xy dest
                flow_trace_dict[id][4] = abs_x
                flow_trace_dict[id][5] = abs_y

        return flow_trace_dict

    def simulation_arrival_departure(node_x, node_y, packet_n=0):
        ################# Get trace_packets from simulation logs ##################
        x_arrival = []
        y_arrival = []
        x_arrival.append(0)
        y_arrival.append(0)

        x_departure = []
        y_departure = []
        x_departure.append(0)
        y_departure.append(0)
        # simul_traced_y = []
        # simul_traced_x = []

        count_r = 0;
        count_t = 0;

        for line in trace_packets:
            abs_x = int(line[HEADER.x_absolute])
            abs_y = int(line[HEADER.y_absolute])

            t = float(line[HEADER.time]) / pck_duration
            # t = time_slot

            if abs_x == node_x and abs_y == node_y:
                ################# Received ####################
                if line[HEADER.app_protocol] == '1':
                    #Build the matrix for the density map
                    if line[HEADER.operation] == 'r' or line[HEADER.operation] == 'g':
                        # Build the cumulative arrival for an specific node
                        count_r += 1
                        x_arrival.append(t)
                        y_arrival.append(count_r)
                        #point there at what time the traced packed passed by
                        # if line[HEADER.app_protocol] == '6':
                        #     simul_traced_x.append(t)
                        #     simul_traced_y.append(count_r)

                    ################# Transmitted ####################
                    elif line[HEADER.operation] == 't': # or line[trace.operation] == 'g':
                        #Build the cumulative departure curve
                        count_t += 1
                        x_departure.append(t+1)
                        y_departure.append(count_t)
                        # point there at what time the traced packed passed by
                        # if line[HEADER.app_protocol] == '6':
                        #     simul_traced_x.append(t + 1) #Plus one time slot, the time to finish transmitting
                        #     simul_traced_y.append(count_t)

        return x_arrival, y_arrival, x_departure, y_departure


    # def profile_delay(x_arrival, y_arrival, x_departure, y_departure):
    #     delay = []
    #     prod = []
    #
    #     for i in range(0, len(y_arrival)):
    #         delay.append(x_departure[i] - x_arrival[i])
    #         prod.append(y_arrival[i])
    #
    #     return delay, prod

    def subtract(x_f1, y_f1, x_f2, y_f2, inverted=False):
        def f(value, x_f, y_f):
            if inverted == False:
                i = 1
                while i < len(x_f) and x_f[i] <= value:
                    i += 1
                y = y_f[i - 1]
            else:
                i = len(x_f) - 1
                while i > 0 and x_f[i] >= value:
                    i -= 1
                y = y_f[i + 1]
            return y

        if inverted==False:
            y_fs = []
            x_fs = sorted(list(set(x_f1 + x_f2)))
            for i in range(0, len(x_fs)):
                x = x_fs[i]
                p_in = f(x, x_f1, y_f1)
                p_out = f(x, x_f2, y_f2)
                diff = p_in - p_out
                y_fs.append(diff)
        else:
            y_fs = sorted(list(set(y_f1 + y_f2)))
            x_fs = []
            for i in range(0, len(y_fs)):
                y = y_fs[i]
                p_in = f(y, y_f1, x_f1)
                p_out = f(y, y_f2, x_f2)
                diff = p_out - p_in
                x_fs.append(diff)

        return x_fs, y_fs

    def model_arrival_departure(sw_in, sw_out):
        #     print('The node selected have received no packets.'

        [x_arrival, y_arrival] = wca.arrival_departure(sw_in)
        [x_departure, y_departure] = wca.arrival_departure(sw_out)

        return [x_arrival, y_arrival, x_departure, y_departure] #, x_queue, y_queue, x_eted, y_eted]


    def competing_flows(flows_list, node_x, node_y, port):
        arriving_flows_list = []
        for i in range(len(flows_list)): #Check all the flows that compete for a specific node/output port
            route = flows_list[i][3]
            if [node_x, node_y, port] in route:
                arriving_flows_list.append(i)

        #Return the indexes of the flows
        return  arriving_flows_list

    def model_propagate(f_index=0): #the list with the flows and the index of the one we are looking for
        global counter_iteration_g
        counter_iteration_g += 1

        route_base = flow_model_list_g[f_index][3]
        route_flows_base = flow_model_list_g[f_index][4] # a list containing the resulting flows at that route
        route_flows_index_base = flow_model_list_g[f_index][5] # a list with the indexes of the flows that generated the correncponding flow

        i = 0
        while i in range(len(route_base) -1): #which flows intersect the observed flow, at each hop
            [x, y, p] = route_base[i]
            index_list = competing_flows(flow_model_list_g, x, y, p)

            # First Check which are uknown
            sw_in = []
            sw_in_index = []
            unknown_list = []

            for j in index_list:
                route = flow_model_list_g[j][3]
                route_flows = flow_model_list_g[j][4] #get the complete route of the flow that intersects with the initial one
                route_flows_index = flow_model_list_g[j][5]
                hop_n = route.index([x,y,p]) #get the index of the point at each the flows intersect

                if route_flows[hop_n] == f_unknown:
                    unknown_list.append(j)
                else:
                    #Have to account a single time for flow already aggregated. by Using an id
                    if not route_flows_index[hop_n] in sw_in_index:
                        sw_in.append(route_flows[hop_n])
                        sw_in_index.append(route_flows_index[hop_n])


            #If all are known, calculate the resulting flow
            if len(unknown_list) == 0:
                f_out = wca.resulting_flow(sw_in)
                route_flows_base[i + 1] = f_out
                route_flows_index_base[i + 1] = index_list
                i += 1

                # organaze data in a hashtable to allow easy access by coordinates
                # if not (x, y) in flow_dict_g:
                flow_model_dict_g[x, y] = [sw_in, f_out]

            else:
                model_propagate(sorted(unknown_list)[0])
                # return
                # else

        # print(route_flows_base)
        return

    def mask_to_port(p):

        # DIRECTION_E = 0, // east
        # DIRECTION_S = 1, // south
        # DIRECTION_W = 2, // west
        # DIRECTION_N = 3, // north
        # DIRECTION_L = 4 // Internal, local

        if p == 0b00000001:
            r = 0
        if p == 0b00000010:
            r = 1
        if p == 0b00000100:
            r = 2
        if p == 0b00001000:
            r = 3
        if p == 0b00010000:
            r = 4

        return r


    def model_node_bounds(traced_flow, in_flows, out_flow, packet_n=0):
        swi = in_flows  # the incomming
        swo = [out_flow]
        # get t_in from the packet of interest, after, use t_out from previous iteration
        t_in = wca.time_taken(traced_flow, n_in=packet_n, direction='in')
        n_in = wca.produced_until(t_in, swi) + 1  # plus itself

        t_out = wca.time_taken(out_flow, n_in, direction='out')
        n_out = wca.produced_until(t_in, swo) + 1  # plus itself

        return t_in, t_out, n_in, n_out

    def plot_matrix(data, title='', show=True, region=[0,0,0,0]):

        data_cropped = []
        if region != [0,0,0,0]:
            [min_x, max_x, min_y, max_y] = region
            for m in data:
                m_cropped = [line[min_x:max_x+1] for line in m[min_y:max_y+1]]
                data_cropped.append(m_cropped)
            data = data_cropped

        filename = None;
        # show = True;
        # title = "What";
        lable_x = ['Model','Simulation'];
        lable_y = "";
        cmap =  'hot_r'#'inferno_r' #'hot_r, viridis_r'

        max_l = []
        min_l = []
        for m in data:
            for l in m:
                max_l.append(max(l))
                min_l.append(min(l))
        max_value = max(max_l)
        min_valeu = min(min_l)

        plt.figure(title, figsize=(6.1, 2.5), dpi=120, facecolor='w', edgecolor='w')

        for m in range(len(data)):
            y_size = len(data[0])
            x_size = len(data[0][0])

            ax = plt.subplot(1, 2, m+1)
            ax.set_title(lable_x[m], fontsize=10)
            im = ax.imshow(data[m], cmap=plt.get_cmap(cmap), interpolation='nearest',
                           origin='lower', vmin=min_valeu, vmax=max_value)

            plt.yticks(numpy.arange(0, y_size, dtype=numpy.int))
            plt.xticks(numpy.arange(0, x_size, dtype=numpy.int))

            # if m == 1:
            # plt.colorbar(im, ticks=numpy.linspace(min_valeu, max_value, num=6))

            for y in range(len(data[m])):
                for x in range(len(data[m][0])):
                    if type(data[m][y][x]) == float:
                        tx = '{:0.2f}'.format(data[m][y][x])
                    else:
                        tx = '{}'.format(data[m][y][x])

                    ax.text(x, y, tx,
                             horizontalalignment='center',
                             verticalalignment='center',
                             size = 8,
                             weight='bold',
                             color='darkgrey'
                             )

        # plt.xlabel(lable_x)
        # plt.ylabel(lable_y)

        plt.tight_layout(pad=2, w_pad=1, h_pad=0.9)

        if filename != None:
            dir = filename[:filename.rfind("/")]
            if not os.path.exists(dir):
                os.makedirs(dir)
            plt.savefig(filename)

        # plt.ion()
        # plt.show()

        elif show == True:
            plt.show()

    def plot(axis, filename=None, show = True):

        # global options

        # filename = options.outputdir + 'cumulative_ad.pdf'

        x_size = 6.5
        y_size = 3.1
        x_lim = None
        y_lim = None

        lines = ["-", "-", "--", "--", "-", "--", "-", "--"]
        markers = ["x", "x", "+", "+", "", "", "", ""]
        colours = ['lightgreen', 'yellow', 'black', 'grey', 'cyan', 'magenta', 'purple']
        labels = [
            'Arrivals',
            'Departures',
            'Arrivals',
            # 'Departures UP',
            # 'Arrivals LB',
            'Departures',
            'Queue',
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
            # ax_i.plot(axis[i], axis[i + 1], '-', linestyle=next(linecycler), label=next(labelcycler),
            #           color=next(colourcycler), marker=next(markercycler))

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
    # port = int(DIRECTION_MASK_S) #Has to be provided from the interface, by clicking at the port
    port = int(options.port)

    f_unknown = [-1, -1, -1]  #nodes own output flow
    nw_flows_list = []

    for k in range(len(trace_flows_source)):
        l = trace_flows_source[k]
        id = int(l[FLOW_NODE_ID])
        x = int(l[FLOW_X])
        y = int(l[FLOW_Y])
        f = [float(l[FLOW_BURSTNESS]), float(l[FLOW_OFFSET]), float(l[FLOW_SIZE])]
        p = l[FLOW_ROUTE].split(';')

        route = [] #keeps track of the route
        route_flows = [] #Keeps track of the resulting flows along the route
        route_flows_index = [] #it is an anrray that keeps track of which flows intercept you at each hop

        for i in range(0, len(p), 3):
            route.append([int(p[i]), int(p[i+1]), int(p[i+2])])
            route_flows.append(f_unknown)
            route_flows_index.append([])

        route_flows[0] = f #in the begining, you yours is known (the source)
        route_flows_index[0] = [k]

        nw_flows_list.append([id, x, y] + [route] + [route_flows] + [route_flows_index])


    # find filter only the flows that pass by the node of interest (decrease recursions required and memory usage)
    index = 0
    for f in nw_flows_list:
        route = f[3]
        if [node_x, node_y, port] in route:
            f[5][0] = index #reset the index for
            flow_model_list_g.append(f)
            index += 1

    if len(flow_model_list_g) == 0:
        print("Nothing found for port {} of node [{},{}]".format(port, node_x, node_y))
        exit(0)

    # Here the model is calculated, given the input flow's characteristics
    model_propagate()

    #Organize the data in a dict for easy access by coordinates
    shaping_dict = {}
    for f in flow_model_list_g:
        for i in range(len(f[3])-1):
            [x, y, p] = f[3][i]
            p = mask_to_port(p)
            shaping_dict[x, y, p] = f[4][i+1]



    ####### ARRIVAL / DEPARTURE
    sw_in = flow_model_dict_g[node_x, node_y][0]
    sw_out = [flow_model_dict_g[node_x, node_y][1]]

    [x_arrival_model, y_arrival_model, x_departure_model, y_departure_model] = model_arrival_departure(sw_in, sw_out)
    plots_model = [x_arrival_model, y_arrival_model, x_departure_model, y_departure_model]

    ####### MODEL QUEUE
    x_queue, y_queue = subtract(x_arrival_model, y_arrival_model, x_departure_model, y_departure_model)
    plots_model_queue = [x_queue, y_queue]

    ####### MODEL DELAY
    x_delay, y_delay = subtract(x_arrival_model, y_arrival_model, x_departure_model, y_departure_model, inverted=True)
    plots_model_delay = [x_delay, y_delay]

    ####### SIMULATION ARRIVAL / DEPARTURE
    x_arrival_sim, y_arrival_sim, x_departure_sim, y_departure_sim = simulation_arrival_departure(node_x, node_y)
    plots_simul = [x_arrival_sim, y_arrival_sim, x_departure_sim, y_departure_sim]

    ####### SIMULATION QUEUE
    qx, qy = subtract(x_arrival_sim, y_arrival_sim, x_departure_sim, y_departure_sim, inverted=False)
    plots_sim_queue = [qx, qy]

    ####### SIMULATION DELAY
    dx, dy = subtract(x_arrival_sim, y_arrival_sim, x_departure_sim, y_departure_sim, inverted=True)
    plots_sim_delay = [dx, dy]

    # plots = plots_simul + plots_model + plots_sim_queue + plots_model_queue + plots_sim_delay + plots_model_delay
    # plots = plots_simul + plots_model + plots_sim_delay + plots_model_delay
    plots = plots_simul + plots_model
    # plot(plots_sim_delay + plots_model_delay)
    # plot(plots_sim_queue + plots_model_queue)

    #Calculate the range
    #
    # x_range = []
    # y_range = []
    # for (x, y) in flow_dict_g:
    #     x_range.append(x)
    #     y_range.append(y)
    # nw_size_y = max(y_range) - min(y_range) + 1
    # nw_size_x = max(x_range) - min(x_range) + 1

    #Set the range fixed for now
    nw_size_y = 37
    nw_size_x = 73

    nw_region_list_x = []
    nw_region_list_y = []

    nw_matrix = []
    for y in range(nw_size_y):
        nw_matrix.append([])
        for x in range(nw_size_x):
            nw_matrix[y].append(0)

    model_queue_matrix = copy.deepcopy(nw_matrix)
    model_delay_matrix = copy.deepcopy(nw_matrix)

    sim_queue_matrix = copy.deepcopy(nw_matrix)
    sim_delay_matrix = copy.deepcopy(nw_matrix)

    ####### FIND OUT THE MAXIMUM

    for hop in flow_model_dict_g:
        sw_in = flow_model_dict_g[hop][0]
        sw_out = flow_model_dict_g[hop][1]

        (x, y) = hop

        nw_region_list_x.append(x)
        nw_region_list_y.append(y)

        ####### SIMULATION
        [x_arrival_sim, y_arrival_sim, x_departure_sim, y_departure_sim] = simulation_arrival_departure(x, y)

        x_sim_queue, y_sim_queue = subtract(x_arrival_sim, y_arrival_sim, x_departure_sim, y_departure_sim)
        x_sim_delay, y_sim_delay  = subtract(x_arrival_sim, y_arrival_sim, x_departure_sim, y_departure_sim, inverted=True)

        sim_queue_matrix[y][x] = max(y_sim_queue)
        sim_delay_matrix[y][x] = max(x_sim_delay)

        ####### MODEL
        [x_arrival_sim, y_arrival_sim, x_departure_sim, y_departure_sim] = model_arrival_departure(sw_in, sw_out)

        x_queue, y_queue = subtract(x_arrival_sim, y_arrival_sim, x_departure_sim, y_departure_sim)
        x_delay, y_delay = subtract(x_arrival_sim, y_arrival_sim, x_departure_sim, y_departure_sim, inverted=True)

        model_queue_matrix[y][x] = max(y_queue)
        model_delay_matrix[y][x] = max(x_delay)

        # print('Node: ' + str(hop))
        # print('Max delay: {}'.format(max_delay))
        # print('Max queue: {}'.format(max_queue))

    min_x = min(nw_region_list_x)
    max_x = max(nw_region_list_x)
    min_y = min(nw_region_list_y)
    max_y = max(nw_region_list_y)

    nw_region = [min_x, max_x, min_y, max_y]

    plot_matrix([model_queue_matrix, sim_queue_matrix], show=True, title='Maximum per-hop queueing', region=nw_region)
    plot_matrix([model_delay_matrix, sim_delay_matrix], show=True, title='Maximum per-hop delay', region=nw_region)

    ####### Calcultate max end-to-end delay by summing max per hop delay
    sim_eted_matrix = copy.deepcopy(nw_matrix)
    model_eted_matrix = copy.deepcopy(nw_matrix)

    for f in flow_model_list_g:
        xo, yo, po = f[3][0]
        for i in range(len(f[3])):
            [x, y, p] = f[3][i]
            model_eted_matrix[yo][xo] += model_delay_matrix[y][x]
            sim_eted_matrix[yo][xo] += sim_delay_matrix[y][x]

    sim_msg_eted_matrix = copy.deepcopy(sim_eted_matrix)
    model_msg_eted_matrix = copy.deepcopy(model_eted_matrix)

    for e in flow_model_dict_g:
        (x, y) = e
        beta = flow_model_dict_g[e][0][0][0]
        msg_size = flow_model_dict_g[e][0][0][2]
        sim_msg_eted_matrix[y][x] += msg_size / beta
        model_msg_eted_matrix[y][x] += msg_size / beta


    plot_matrix([model_msg_eted_matrix, sim_msg_eted_matrix], show=True, title='Maximum end-to-end delay', region=nw_region)


    ####### Gethering eted from logfiles

    flow_trace_dict = simulation_eted()
    sim_flow_trace_eted_matrix = copy.deepcopy(nw_matrix)

    for f in flow_trace_dict:
        [xo, yo] = flow_trace_dict[f][2:4] #get elements 2 and 3
        dt = float(flow_trace_dict[f][1] - flow_trace_dict[f][0]) / pck_duration
        sim_flow_trace_eted_matrix[yo][xo] = dt

    # plot_matrix([sim_flow_trace_eted_matrix], show=True)
    plot_matrix([model_msg_eted_matrix, sim_flow_trace_eted_matrix], show=True, title='Measured end-to-end delay', region=nw_region)
    ############## SAVING THE RESULTS ##########################

    # Naming the output file
    sw_in = flow_model_dict_g[node_x, node_y][0]
    sw_out_f = flow_model_dict_g[node_x, node_y][1]
    sw_in_f = []
    for i in range(len(sw_in)):
        ff = [float('%.2f' % elem) for elem in sw_in[i]]
        if ff != [0.0, 0.0, 0.0]:
            sw_in_f.append(ff)

    sw_out_f = [float('%.2f' % elem) for elem in sw_out_f]

    str_out = ''
    for l in shaping_dict:
        str_out += str(l) + ',' + str(shaping_dict[l]) + '\n'

    str_out = str_out.replace('[', '').replace(']','').replace('(','').replace(')','').replace(' ','')
    fn1 = options.outputdir + 'shaping_config.csv'
    files_io.write(str_out, fn1)

    output_tx = ''
    for f in flow_model_list_g:
        output_tx += ("--------------------------------------------------------------------------\n")
        output_tx += ("Route:\t {}\n".format(f[3]))
        output_tx += ("Flows:\t {}\n".format(f[4]))
        output_tx += ("Intersections:\t {}\n".format(f[5]))
    output_tx += ("--------------------------------------------------------------------------\n")
    output_tx += ('Iterations required: {}\n'.format(counter_iteration_g))
    output_tx += ('Total number of flows: {}\n'.format(len(flow_model_list_g)))

    fn2 = fn1.replace('shaping_config.csv', 'flows.csv')
    files_io.write(output_tx, fn2)

    # Ploting results
    fn = options.outputdir + 'cumulative (' + str(node_x) + ',' + str(node_y) + ') sw ' + str(sw_in_f) + ' = ' + str(sw_out_f) + '.pdf'
    plot(plots, filename=None)

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
        parser.add_option ('--port', help='port to analyse', default=0)
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
