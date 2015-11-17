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
import numpy as np
import matplotlib.pyplot as plt
import matplotlib as mpl
from itertools import cycle
import packet_structure as pck
import flow_structure as flw
import files_io

# matplotlib.style.use('bmh')
# ['ggplot', 'bmh', 'grayscale', 'fivethirtyeight', 'dark_background']
#mpl.rcParams.update(mpl.rcParamsDefault)


def main ():

    global options, args

    if options.inputfile == None:
        options.inputfile = '/home/joao/noc-data/tests/out/packets-trace-netdevice.csv'
    if options.inputconfigfile == None:
        options.inputconfigfile = '/home/joao/noc-data/config/input-config.c.csv'

    data = files_io.load_list(options.inputfile)

    #Split the file here for parallel processing
    flows = extract_flows(data)

    # print(flows)
    print('Total of ', len(flows), ' flows')

    box_plot( [row[7] for row in flows] ) #[row[6] for row in flows]
    box_plot( [row[6] for row in flows] ) #[row[6] for row in flows]

def extract_flows(pck_trace):

    flow_trace_index = []
    flow_trace = []

    for i in range(0, len(pck_trace)):
        id = int(pck_trace[i][pck.id])

        if id not in flow_trace_index:
            flow_trace_index.append(id)
            flow_trace.append([id, i, i])

        else:
            index = flow_trace_index.index(id) #return the index for that id
            flow_trace[index][2] = i #and update the last seen packet of that flow


    flow_trace_log = []

    for line in flow_trace:
        id = line[0]
        pck_first = pck_trace[line[1]]
        pck_last = pck_trace[line[2]]

        delta_ts = int(pck_last[pck.time_slot]) - int(pck_first[pck.time_slot])
        x1 = int(pck_last[pck.x_absolute])
        y1 = int(pck_last[pck.y_absolute])
        x2 = int(pck_first[pck.x_absolute])
        y2 = int(pck_first[pck.y_absolute])
        distance = dist(x1,y1,x2,y2)

        if distance[2] == 0:
            print('Erro')
        else:
            flow_trace_log.append([id, delta_ts, x2, y2, x1, y1, distance[2], round(delta_ts / distance[2], 3) ]) #

    return flow_trace_log

def dist(xi, yi, xf, yf):
    # xi = np.abs(xi)
    # yi = np.abs(yi)

    if xi >= xf:  # calculates the distance
        dx = xi - xf
    else:
        dx = xf - xi
    if yi >= yf:
        dy = yi - yf
    else:
        dy = yf - yi

    return dx, dy, dx+dy

def box_plot(data):
    y_lim = 0
    filename=None
    title = ""
    lable_x = ""
    lable_y = ""
    x_size = 9
    y_size = 5


    plt.figure(title, figsize=(x_size, y_size), dpi=80, facecolor='w', edgecolor='w')
    plt.xlabel(lable_x, fontsize=20)
    plt.ylabel(lable_y, fontsize=20)
    # if y_lim != []:

    # plt.ylim([0,8])

    cem = (0,2)
    # plt.boxplot(data, conf_intervals=[cem, cem, cem, cem, cem, cem])
    plt.boxplot(data)

    ax = plt.gca()
    # ax.set_xticklabels([1,2,3,4,5,6,7])
    for label in ax.get_xticklabels() + ax.get_yticklabels():
        label.set_fontsize(18)

    plt.tight_layout()

    if filename is not None:
        plt.savefig(filename)

    plt.show()

if __name__ == '__main__':
    try:
        start_time = time.time()
        parser = optparse.OptionParser(formatter=optparse.TitledHelpFormatter(), usage=globals()['__doc__'], version='$Id$')

        parser.add_option ('-v', '--verbose', action='store_true', default=True, help='verbose output')
        parser.add_option ('-i', '--inputfile', help='input file containing the packet trace')
        parser.add_option ('-c', '--inputconfigfile', help='config file containing the simulation parameters')

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
