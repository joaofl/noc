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
import noc_trace_structure as trace
import noc_io

# matplotlib.style.use('bmh')
# ['ggplot', 'bmh', 'grayscale', 'fivethirtyeight', 'dark_background']
#mpl.rcParams.update(mpl.rcParamsDefault)


def main ():

    global options, args

    if options.inputfile == None:
        options.inputfile = '/home/joao/usn-data/nw5x5s1n4/out/packets-trace-netdevice.csv'
    if options.inputconfigfile == None:
        options.inputconfigfile = '/home/joao/usn-data/config/input-config.c.csv'

    data = noc_io.load_list(options.inputfile)

    #Split the file here for parallel processing
    extract_flows(data)


def extract_flows(pck_trace):

    flow_trace_index = []
    flow_trace = []
    index_first = 0
    index_last = -1

    for i in range(0, len(pck_trace)):
        id = int(pck_trace[i][trace.id])

        if id not in flow_trace_index:
            flow_trace_index.append(id)
            flow_trace.append([id, i, i])

        else:
            index = flow_trace_index.index(id)
            flow_trace[index][2] = i


        if (i == len(pck_trace)): #last iteration
            id = int(pck_trace[index_first][trace.id])
            delta_ts = int(pck_trace[index_last][trace.time_slot]) - int(pck_trace[index_first][trace.time_slot])
            # delta_ts = int(pck_trace[index_last][trace.time_slot]) - int(pck_trace[index_first][trace.time_slot])
            flow_trace.append([id, delta_ts, ])

if __name__ == '__main__':
    try:
        start_time = time.time()
        parser = optparse.OptionParser(formatter=optparse.TitledHelpFormatter(), usage=globals()['__doc__'], version='$Id$')

        parser.add_option ('-v', '--verbose', action='store_true', default=False, help='verbose output')
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
