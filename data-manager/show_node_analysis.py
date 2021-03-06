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


#Libs
import sys, os, optparse
import time
import numpy
import matplotlib.pyplot as plt
import copy
from itertools import cycle
from joblib import Parallel, delayed

#My stuff
import packet_structure as HEADER
import others.analysis_wc as wca
import others.wcanalysis_node as wca_new
import files_io



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

# DIRECTION_MASK_NONE             = 0b00000000
# DIRECTION_MASK_E                = 0b00000001
# DIRECTION_MASK_S                = 0b00000010
# DIRECTION_MASK_W                = 0b00000100
# DIRECTION_MASK_N                = 0b00001000
# DIRECTION_MASK_L                = 0b00010000
# DIRECTION_MASK_ALL              = 0b00011111
# DIRECTION_MASK_ALL_EXCEPT_LOCAL = 0b00001111



if __name__ == '__main__':
    # try:
    start_time = time.time()
    parser = optparse.OptionParser(formatter=optparse.TitledHelpFormatter(), usage=globals()['__doc__'], version='$Id$')

    parser.add_option('--verbose', action='store_true', default=False, help='verbose output')
    parser.add_option('--inputdir', help='Dir containing the logs', default=None)
    parser.add_option('--outputdir', help='', default=None)
    parser.add_option('--basedir', help='Base dir for simulation output files', default='')
    parser.add_option('--showplots', help='Show plots', default='True')
    parser.add_option('--shaper', help='Which traffic shaping policy to use', default='A')
    parser.add_option('--size_x', help='network size', default=0)
    parser.add_option('--size_y', help='network size', default=0)
    parser.add_option('--pos_x', help='node to analyse', default=1)
    parser.add_option('--pos_y', help='node to analyse', default=1)
    parser.add_option('--port', help='port to analyse', default=2)
    parser.add_option('--sinks_n', help='number of sinks', default=1)
    parser.add_option('--baudrate', help='baudrate utilized', default=3000000)
    parser.add_option('--packet_size', help='packet_size in bits', default=16*10)
    parser.add_option('--size_neighborhood', help='neighborhood size', default=0)

    (options, args) = parser.parse_args()
    if options.verbose: print (time.asctime())

    ############################# Global vars and settings #######################
    recursion_limit_g = 10000
    sys.setrecursionlimit(recursion_limit_g)

    flow_model_list_g = []
    flow_model_dict_g = {}
    counter_iteration_g = 0
    counter_recursion_g = 0
    rounding_digits_g = 3

    # inputfile_queue_size = options.inputdir + '/queue-size-trace.csv'
    # inputfile_flows = options.inputdir + '/flows-trace.csv'
    inputfile_packet_trace = options.inputdir + '/packets-trace-netdevice.csv'
    inputfile_flows_source = options.inputdir + '/flows-source.csv'

    inputfile_results = options.outputdir + 'results.csv'

    input_results = files_io.load_list(inputfile_results)
    if (len(input_results)) > 0: #simulation has been run on this scenario already, skip it
        print('Scenario already run.')
        # exit(1)

    trace_packets = files_io.load_list(inputfile_packet_trace)
    if (len(trace_packets)) == 0:
        print('Log file {} is empty'.format(inputfile_packet_trace))
        exit(1)

    trace_flows_source = files_io.load_list(inputfile_flows_source)
    if (len(trace_flows_source)) == 0:
        print('Flows source trace is empty or not found.')
        exit(1)

    # trace_queue_size = files_io.load_list(inputfile_queue_size)
    # if (len(trace_queue_size)) == 0:
    #     print('Queue size trace is empty or not found.')
    #     # exit(1)

    # trace_flows = files_io.load_list(inputfile_flows)
    # if (len(trace_flows)) == 0:
    #     print('Flows trace is empty or not found.')
    #     # exit(1


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

    def mask_to_port(p):
        # DIRECTION_E = 0, // east
        # DIRECTION_S = 1, // south
        # DIRECTION_W = 2, // west
        # DIRECTION_N = 3, // north
        # DIRECTION_L = 4 // Internal, local
        r = 0
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

    def port_to_mask(p):
        # DIRECTION_E = 0, // east
        # DIRECTION_S = 1, // south
        # DIRECTION_W = 2, // west
        # DIRECTION_N = 3, // north
        # DIRECTION_L = 4 // Internal, local
        r = 0
        if p == 0:
            r = 0b00000001
        if p == 1:
            r = 0b00000010
        if p == 2:
            r = 0b00000100
        if p == 3:
            r = 0b00001000
        if p == 4:
            r = 0b00010000
        return r

    def round_array(data, digits):
        data_r = []
        for l in data:
            data_r.append(round(l, digits))
        return data_r


    def simulation_measure_eted():
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

    def simulation_arrival_departure(node_x, node_y, protocol=1):
        ################# Get trace_packets from simulation logs ##################
        x_arrival = []
        y_arrival = []
        x_arrival.append(0)
        y_arrival.append(0)

        x_departure = []
        y_departure = []
        x_departure.append(0)
        y_departure.append(0)

        count_r = 0;
        count_t = 0;

        for line in trace_packets:
            log_x = int(line[HEADER.x_absolute])
            log_y = int(line[HEADER.y_absolute])
            log_protocol = int(line[HEADER.app_protocol])
            log_t = float(line[HEADER.time]) / pck_duration

            if log_x == node_x and log_y == node_y and log_protocol == protocol:
                ################# Received ####################
                #Build the matrix for the density map
                if line[HEADER.operation] == 'r' or line[HEADER.operation] == 'g':
                    # Build the cumulative arrival for an specific node
                    count_r += 1
                    x_arrival.append(log_t)
                    y_arrival.append(count_r)

                ################# Transmitted ####################
                elif line[HEADER.operation] == 't': # or line[trace.operation] == 'g':
                    #Build the cumulative departure curve
                    count_t += 1
                    x_departure.append(log_t+1)
                    y_departure.append(count_t)

        return x_arrival, y_arrival, x_departure, y_departure

    # def model_arrival_departure(sw_in, sw_out):
    #     [x_arrival, y_arrival] = wca.arrival_departure(sw_in)
    #     [x_departure, y_departure] = wca.arrival_departure(sw_out)
    #
    #     return [x_arrival, y_arrival, x_departure, y_departure] #, x_queue, y_queue, x_eted, y_eted]
    #
    def subtract(x_f1, y_f1, x_f2, y_f2, inverted=False):
        def f(value, x_f, y_f):
            if inverted == False:
                i = 1
                while i < len(x_f) and x_f[i] <= value:
                    i += 1
                y = y_f[i - 1]
            else:
                i = len(x_f) - 2
                while i >= 0 and x_f[i] >= value:
                    i -= 1
                y = y_f[i + 1]

            return y

        if [x_f1, y_f1, x_f2, y_f2] == [[0],[0],[0],[0]]:
            return [[0], [0]]

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
    #
    #


    def model_propagate(f_index=0): #the list with the flows and the index of the one we are looking for
        def competing_flows(flows_list, node_x, node_y, port):
            arriving_flows_list = []
            for i in range(len(flows_list)):  # Check all the flows that compete for a specific node/output port
                route = flows_list[i][3]
                if [node_x, node_y, port] in route:
                    arriving_flows_list.append(i)

            # Return the indexes of the flows
            return arriving_flows_list

        global counter_iteration_g
        counter_iteration_g += 1
        # print(counter_iteration_g)

        if counter_iteration_g == recursion_limit_g:
            print('Maximum of {} recursions depth reached'.format(recursion_limit_g))
            # exit(0)

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
                f_out = wca.resulting_flow(sw_in, model=options.shaper)
                route_flows_base[i + 1] = f_out
                route_flows_index_base[i + 1] = index_list
                i += 1

                # organaze data in a hashtable to allow easy access by coordinates
                # if not (x, y) in flow_dict_g:
                flow_model_dict_g[x, y] = [sw_in, f_out, p]

            else:
                model_propagate(sorted(unknown_list)[0])
                # return
                # else

        # print(route_flows_base)
        return


    # def profile_node(x,y):
    #
    #     # print(x,y, sim_delay_matrix_g, sim_queue_matrix_g, model_delay_matrix_g, model_queue_matrix_g)
    #
    #     sw_in = flow_model_dict_g[x,y][0]
    #     sw_out = flow_model_dict_g[x,y][1]
    #
    #     ####### SIMULATION
    #     [x_arrival_sim, y_arrival_sim, x_departure_sim, y_departure_sim] = simulation_arrival_departure(x, y)
    #
    #     x_arrival_sim = round_array(x_arrival_sim, rounding_digits_g)
    #     x_departure_sim = round_array(x_departure_sim, rounding_digits_g)
    #
    #     x_sim_queue, y_sim_queue = subtract(x_arrival_sim, y_arrival_sim, x_departure_sim, y_departure_sim)
    #     x_sim_delay, y_sim_delay  = subtract(x_arrival_sim, y_arrival_sim, x_departure_sim, y_departure_sim, inverted=True)
    #
    #     sim_max_queue = max(y_sim_queue)
    #     sim_max_delay = max(x_sim_delay)
    #
    #     sim_results = [
    #         sim_max_queue, sim_max_delay,
    #         x_arrival_sim, y_arrival_sim,
    #         x_departure_sim, y_departure_sim,
    #         x_sim_queue, y_sim_queue,
    #         x_sim_delay, y_sim_delay
    #     ]
    #
    #
    #     ####### MODEL
    #     [x_arrival_model, y_arrival_model, x_departure_model, y_departure_model] = model_arrival_departure(sw_in, sw_out)
    #
    #     x_arrival_model = round_array(x_arrival_model, rounding_digits_g)
    #     x_departure_model = round_array(x_departure_model, rounding_digits_g)
    #
    #     x_model_queue, y_model_queue = subtract(x_arrival_model, y_arrival_model, x_departure_model, y_departure_model)
    #     x_model_delay, y_model_delay = subtract(x_arrival_model, y_arrival_model, x_departure_model, y_departure_model, inverted=True)
    #
    #     model_max_queue = max(y_model_queue)
    #     model_max_delay = max(x_model_delay)
    #
    #     model_results = [
    #         model_max_queue, model_max_delay,
    #         x_arrival_model, y_arrival_model,
    #         x_departure_model, y_departure_model,
    #         x_model_queue, y_model_queue,
    #         x_model_delay, y_model_delay
    #     ]
    #
    #
    #     return [x, y, sim_results, model_results]

    def plot_hist(data, filename=None):
        fig, ax = plt.subplots(nrows=2, ncols=1, figsize=(6.5, 3.1), dpi=120, facecolor='w', edgecolor='w')

        # ax0, ax1 = ax.flat

        y_label = ['Model', 'Simulation']

        for i in range(len(ax)):
            axi = ax[i]

            axi.hist(data[i], bins='fd', facecolor='lightblue', alpha=1, normed=1, cumulative=0, log=0, align='mid', label='Measured data')
            axi.hist(data[i], bins='fd', facecolor='lightblue', alpha=1, normed=1, cumulative=0, log=0, align='mid', label='Measured data')

            axi.set_ylabel(y_label[i])
            axi.grid(True)

        ax[-1].set_xlabel('Per hop delay distribution')

        # x = np.linspace(d_min, d_max, d_count)
        #
        # param = uniform.fit(d)
        # # y = uniform.pdf(x, param[0], param[1])
        # y = uniform.cdf(x, param[0], param[1])
        # ax.plot(x, y, 'r--', color='darkred', label='Uniform distribution fit')

        # plt.xlim([d_min - abs(2*d_min), d_max + abs(2*d_min)])

        # ax.legend(loc='upper left', frameon=True, prop={'size': 12})
        fig.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
        # file_out = file_in + '-cdf.eps'


        if filename != None:
            dir = filename[:filename.rfind("/")]
            if not os.path.exists(dir):
                os.makedirs(dir)
            plt.savefig(filename)
    
    def plot_box(data, filename=None):
        title = ""
        lable_x = ""
        lable_y = ""
        x_size = 9
        y_size = 5
        plt.figure(title, figsize=(x_size, y_size), dpi=80, facecolor='w', edgecolor='w')
        plt.xlabel(lable_x, fontsize=20)
        plt.ylabel(lable_y, fontsize=20)
        # if y_lim != []:

        # plt.ylim(y_lim)

        # cem = (0,2)
        # plt.boxplot(data, conf_intervals=[cem, cem, cem, cem, cem, cem])
        plt.boxplot(data)

        # ax = plt.gca()
        # ax.set_xticklabels([1,2,3,4,5,6,7])
        # for label in ax.get_xticklabels() + ax.get_yticklabels():
        #     label.set_fontsize(18)

        plt.tight_layout()

        if filename != None:
            dir = filename[:filename.rfind("/")]
            if not os.path.exists(dir):
                os.makedirs(dir)
            plt.savefig(filename)

    def plot_matrix(data, title='', show=True, region=[0,0,0,0], filename = None):
        data_cropped = []
        if region != [0,0,0,0]:
            [min_x, max_x, min_y, max_y] = region
            for m in data:
                m_cropped = [line[min_x:max_x+1] for line in m[min_y:max_y+1]]
                data_cropped.append(m_cropped)
            data = data_cropped

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

        plt.figure(title, figsize=(6.1, 3.2), dpi=120, facecolor='w', edgecolor='w')

        for m in range(len(data)):
            y_size = len(data[0])
            x_size = len(data[0][0])

            ax = plt.subplot(1, 2, m+1)
            ax.set_title(lable_x[m], fontsize=10)
            im = ax.imshow(data[m], cmap=plt.get_cmap(cmap), interpolation='nearest',
                           origin='lower', vmin=min_valeu, vmax=max_value)

            plt.yticks(numpy.arange(0, y_size, dtype=numpy.int), fontsize=8)
            plt.xticks(numpy.arange(0, x_size, dtype=numpy.int), fontsize=8)

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
                             size = 7,
                             weight='bold',
                             color='darkgrey'
                             )

        # plt.xlabel(lable_x)
        # plt.ylabel(lable_y)

        plt.tight_layout(pad=0.3, w_pad=1, h_pad=0.9)

        if filename != None:
            dir = filename[:filename.rfind("/")]
            if not os.path.exists(dir):
                os.makedirs(dir)
            plt.savefig(filename)
            print("Plot matrix saved in " + filename)

        if show == True:
            # plt.show()
            plt.draw()

    def plot(axis, filename=None, show = True):
        x_size = 6.5
        y_size = 3.1
        x_lim = None
        y_lim = None

        lines = ["-", "-", "--", "--", "-", "--", "-", "--"]
        markers = ["", "", "", "", "", "", "", ""]
        colours = ['lightgreen', 'yellow', 'black', 'grey', 'cyan', 'magenta', 'purple']
        labels = [
            'Sim arrival',
            'Sim departures',
            'Model arrivals',
            'Model departures',
            'Sim queue',
            'Model queue',
            'Sim delay',
            'Model delay',
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
            # plt.draw()


    ################################# Running ####################################

    ####### Reading input params
    nw_size_x = int(options.size_x)
    nw_size_y = int(options.size_y)
    node_x = int(options.pos_x)
    node_y = int(options.pos_y)

    try:
        port = int(options.port)
    except:
        if options.port.startswith('north'):
            port = 3
        if options.port.startswith('south'):
            port = 1
        if options.port.startswith('east'):
            port = 0
        if options.port.startswith('west'):
            port = 2

    pck_duration = ( float(options.packet_size) / float(options.baudrate) ) * 1e9

    if [nw_size_x, nw_size_y] == [0,0]:
        print('Invalid network size {}'.format([nw_size_x, nw_size_y]))
        exit(0)

    ####### Extracting data
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

    ############## Going through the calculations to present the data

    list_model_max_queue = []
    list_model_max_delay = []
    list_sim_max_queue = []
    list_sim_max_delay = []
    total_time_model = 0
    total_time_sim = 0

    for (x, y) in flow_model_dict_g:

        sw_in = flow_model_dict_g[x,y][0]
        sw_out = flow_model_dict_g[x,y][1]

        ## Convert list in dict
        for i, f in enumerate(sw_in):
            sw_in[i] = {'burstness': f[0], 'offset': f[1], 'msgsize': f[2]}
        sw_out = {'burstness': sw_out[0], 'offset': sw_out[1], 'msgsize': sw_out[2]}

        ##Start the caclulation

        ######## MODEL
        # t_in, n_in = wca_new.calculate_flow(sw_in)
        # t_out, n_out = wca_new.calculate_flow([sw_out])

        # t_queue, n_queue = wca_new.calculate_queue(t_in, n_in, t_out, n_out)
        # t_delay, n_delay = wca_new.calculate_delay(t_in, n_in, t_out, n_out)



        t_math_in, n_math_in = wca_new.get_points(sw_in)
        t_math_out, n_math_out = wca_new.get_points([sw_out])

        max_queue = max(wca_new.calculate_queue_model(t_math_in, n_math_in, t_math_out, n_math_out))
        max_delay = max(wca_new.calculate_delay_model(t_math_in, n_math_in, t_math_out, n_math_out))

        # model_results = [
        #     max_queue, max_delay,
        #     t_in, n_in,
        #     t_out, n_out,
        #     t_queue, n_queue,
        #     t_delay, n_delay
        # ]

        list_model_max_queue.append(max_queue)
        list_model_max_delay.append(max_delay)

        ####### SIMULATION
        [t_in_sim, n_in_sim, t_out_sim, n_out_sim] = simulation_arrival_departure(x, y)

        # x_arrival_sim = round_array(t_in_sim, rounding_digits_g)
        # x_departure_sim = round_array(t_out_sim, rounding_digits_g)

        t_queue_sim, n_queue_sim = subtract(t_in_sim, n_in_sim, t_out_sim, n_out_sim)
        t_delay_sim, n_delay_sim  = subtract(t_in_sim, n_in_sim, t_out_sim, n_out_sim, inverted=True)

        max_queue_sim = max(n_queue_sim)
        max_delay_sim = max(t_delay_sim)

        # sim_results = [
        #     max_queue_sim, max_delay_sim,
        #     t_in_sim, n_in_sim,
        #     t_out_sim, n_out_sim,
        #     t_queue_sim, n_queue_sim,
        #     t_delay_sim, n_delay_sim
        # ]

        list_sim_max_queue.append(max_queue_sim)
        list_sim_max_delay.append(max_delay_sim)

        wca_new.plot(t_in_sim, n_in_sim, t_out_sim, n_out_sim, t_math_in, n_math_in, t_math_out, n_math_out, "whatever")
        # exit()

        if [x, y] == [node_x, node_y]:
            plots_arrival_departure = \
            total_time_model = t_math_out[-1]
            total_time_sim = t_out_sim[-1]










    ####### MODEL ARRIVAL / DEPARTURE
    # nw_matrix = [[0 for x in range(nw_size_x)] for y in range(nw_size_y)]
    #
    # model_queue_matrix_g = copy.deepcopy(nw_matrix)
    # model_delay_matrix_g = copy.deepcopy(nw_matrix)
    #
    # sim_queue_matrix_g = copy.deepcopy(nw_matrix)
    # sim_delay_matrix_g = copy.deepcopy(nw_matrix)
    #
    # sim_eted_matrix = copy.deepcopy(nw_matrix)
    # model_eted_matrix = copy.deepcopy(nw_matrix)

    ####### FIND OUT THE MAXIMUM

    # nw_region_list_x = []
    # nw_region_list_y = []
    # list_model_max_queue = []
    # list_model_max_delay = []
    # list_sim_max_queue = []
    # list_sim_max_delay = []
    # total_time_model = 0
    # total_time_sim = 0
    
    # plots_arrival_departure = profile_node(node_x, node_y)

    # results = Parallel(n_jobs=-1)(delayed(profile_node)(x, y) for (x, y) in flow_model_dict_g)
    # results = Parallel(n_jobs=2)(delayed(profile_node)(x, y) for (x, y) in flow_model_dict_g)
    #
    # for r in results:
    #     x, y = r[0], r[1]
    #     nw_region_list_x.append(x)
    #     nw_region_list_y.append(y)
    #
    #     [
    #         sim_max_queue, sim_max_delay,
    #         x_arrival_sim, y_arrival_sim,
    #         x_departure_sim, y_departure_sim,
    #         x_sim_queue, y_sim_queue,
    #         x_sim_delay, y_sim_delay
    #     ] = r[2]
    #
    #     [
    #         model_max_queue, model_max_delay,
    #         x_arrival_model, y_arrival_model,
    #         x_departure_model, y_departure_model,
    #         x_model_queue, y_model_queue,
    #         x_model_delay, y_model_delay
    #     ] = r[3]
    #
    #     model_queue_matrix_g[y][x] = model_max_queue
    #     # list_model_max_queue.append(model_max_queue)
    #     list_model_max_queue += y_model_queue
    #
    #     sim_queue_matrix_g[y][x] = sim_max_queue
    #     # list_sim_max_queue.append(sim_max_queue)
    #     list_sim_max_queue += y_sim_queue
    #     #I'm actually apending arrays to each other, containing the whole queue size over time
    #
    #
    #     model_delay_matrix_g[y][x] = model_max_delay
    #     # list_model_max_delay.append(model_max_delay)
    #     list_model_max_delay += x_model_delay
    #     sim_delay_matrix_g[y][x] = sim_max_delay
    #     # list_sim_max_delay.append(sim_max_delay)
    #     list_sim_max_delay += x_sim_delay
    #
    #     if [x, y] == [node_x, node_y]:
    #         plots_arrival_departure = \
    #             [
    #                 x_arrival_sim, y_arrival_sim,
    #                 x_departure_sim, y_departure_sim,
    #                 x_arrival_model, y_arrival_model,
    #                 x_departure_model, y_departure_model,
    #                 # x_sim_queue, y_sim_queue,
    #                 # x_sim_delay, y_sim_delay,
    #                 # x_model_queue, y_model_queue,
    #                 # x_model_delay, y_model_delay
    #             ]
    #         total_time_model = x_departure_model[-1]
    #         total_time_sim = x_departure_sim[-1]
    #
    # nw_region = [min(nw_region_list_x), max(nw_region_list_x), min(nw_region_list_y), max(nw_region_list_y)]
    #
    #
    # ####### Calcultate max end-to-end delay by summing max per hop delay
    # for f in flow_model_list_g:
    #     xo, yo, po = f[3][0]
    #     for i in range(len(f[3])):
    #         [x, y, p] = f[3][i]
    #         model_eted_matrix[yo][xo] += model_delay_matrix_g[y][x]
    #         sim_eted_matrix[yo][xo] += sim_delay_matrix_g[y][x]
    #
    # sim_msg_eted_matrix = copy.deepcopy(sim_eted_matrix)
    # model_msg_eted_matrix = copy.deepcopy(model_eted_matrix)
    #
    # for e in flow_model_dict_g:
    #     (x, y) = e
    #     beta = flow_model_dict_g[e][0][0][0]
    #     msg_size = flow_model_dict_g[e][0][0][2]
    #     try:
    #         sim_msg_eted_matrix[y][x] += msg_size / beta
    #         model_msg_eted_matrix[y][x] += msg_size / beta
    #     except:
    #         sim_msg_eted_matrix[y][x] += 0
    #         model_msg_eted_matrix[y][x] += 0
    #
    # ####### Gethering eted from logfiles
    #
    # flow_trace_dict = simulation_measure_eted()
    # sim_flow_trace_eted_matrix = copy.deepcopy(nw_matrix)
    #
    # for f in flow_trace_dict:
    #     [xo, yo] = flow_trace_dict[f][2:4] #get elements 2 and 3
    #     dt = float(flow_trace_dict[f][1] - flow_trace_dict[f][0]) / pck_duration
    #     sim_flow_trace_eted_matrix[yo][xo] = dt



    ######## Producing output results
    str_results = ''
    str_results += str(max(list_sim_max_queue)) + ','
    str_results += str(max(list_model_max_queue)) + ','

    str_results += str(max(list_sim_max_delay)) + ','
    str_results += str(max(list_model_max_delay)) + ','

    str_results += str(total_time_sim) + ','
    str_results += str(total_time_model) + ','

    [beta, offset, msg_size] = flow_model_dict_g[node_x, node_y][1]
    str_results += '{},{},{}'.format(beta, offset, msg_size)

    print('Max queue: {}'.format(max(list_sim_max_queue)))
    print('Total time: {}'.format(total_time_sim))

    # Naming the output file
    sw_in = flow_model_dict_g[node_x, node_y][0]
    sw_out_f = flow_model_dict_g[node_x, node_y][1]
    sw_in_f = []
    for i in range(len(sw_in)):
        ff = [float('%.2f' % elem) for elem in sw_in[i].values()]
        if ff != [0.0, 0.0, 0.0]:
            sw_in_f.append(ff)

    sw_out_f = [float('%.2f' % elem) for elem in sw_out_f]
    str_out = ''
    for l in shaping_dict:
        str_out += str(l) + ',' + str(shaping_dict[l]) + '\n'
    str_out = str_out.replace('[', '').replace(']','').replace('(','').replace(')','').replace(' ','')

    output_tx = ''
    for f in flow_model_list_g:
        output_tx += ("--------------------------------------------------------------------------\n")
        output_tx += ("Route:\t {}\n".format(f[3]))
        output_tx += ("Flows:\t {}\n".format(f[4]))
        output_tx += ("Intersections:\t {}\n".format(f[5]))
    output_tx += ("--------------------------------------------------------------------------\n")
    output_tx += ('Iterations required: {}\n'.format(counter_iteration_g))
    output_tx += ('Total number of flows: {}\n'.format(len(flow_model_list_g)))

    ######## Savinf results

    fn1 = options.outputdir + 'shaping_config.csv'
    files_io.write(str_out, fn1)
    print('Traffic shaping parameters saved at ' + fn1)

    fn2 = options.outputdir + 'flows_model.csv'
    files_io.write(output_tx, fn2)
    print('Flows model saved at ' + fn2)

    fn3 = options.outputdir + 'results.csv'
    files_io.write(str_results, fn3)
    print('Results saved at ' + fn3)

    # fn = options.outputdir + 'CC[' + str(node_x) + ',' + str(node_y) + ']sw' + str(sw_in_f).replace(' ', '') + '=' + str(sw_out_f).replace(' ', '') + '.pdf'
    # fn3 = fn.replace('CC', 'cumulative-ad')
    # fn4 = fn.replace('CC', 'matrix-max-queue')
    # fn5 = fn.replace('CC', 'matrix-max-delay')
    # fn6 = fn.replace('CC', 'matrix-max-eted')
    # fn7 = fn.replace('CC', 'matrix-measured-eted')
    # fn8 = fn.replace('CC', 'dist-delay')
    # fn9 = fn.replace('CC', 'dist-queue')


    ####### Plotting results

    # show = bool(options.showplots)
    # show = False

    # plot_hist([list_model_max_delay, list_sim_max_delay], filename=fn9)
    # plot_hist([list_model_max_queue, list_sim_max_queue], filename=fn8)
    # plt.show()


    # plot_matrix([model_queue_matrix_g, sim_queue_matrix_g], show=show, title='Maximum per-hop queueing', region=nw_region, filename=fn4)
    # plot_matrix([model_delay_matrix_g, sim_delay_matrix_g], show=show, title='Maximum per-hop delay', region=nw_region, filename=fn5)
    # plot_matrix([model_msg_eted_matrix, sim_msg_eted_matrix], show=show, title='Maximum end-to-end delay', region=nw_region, filename=fn6)
    # plot_matrix([model_msg_eted_matrix, sim_flow_trace_eted_matrix], show=show, title='Measured end-to-end delay', region=nw_region, filename=fn7)
    # plot(plots_arrival_departure, filename=fn3, show=show)

    # if show:
    # plt.show()


    # if options.verbose:
    print (time.asctime())
    # if options.verbose:
    print ('Total execution time (s):')
    # if options.verbose:
    print (time.time() - start_time)
    sys.exit(0)

    # except (KeyboardInterrupt, e): # Ctrl-C
    #     raise e
    # except SystemExit, e: # sys.exit()
    #     raise e

    # except (Exception):
    #     print ('ERROR, UNEXPECTED EXCEPTION')
    #     print (str(Exception))
    #     traceback.print_exc()
    #     os._exit(1)
