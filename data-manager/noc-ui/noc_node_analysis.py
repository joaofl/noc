#!/usr/bin/env python
"""
SYNOPSIS

    TODO helloworld [-h,--help] [-v,--verbose] [--version]

DESCRIPTION

    TODO This describes how to use this script. This docstring
    will be printed by the script if there is an error or
    if the user requests help (-h or --help).

EXAMPLES

    TODO: Show some examples of how to use this script.

EXIT STATUS

    TODO: List exit codes

AUTHOR

    __author__ = 'Joao Loureiro <joflo@isep.ipp.pt>'

LICENSE

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

VERSION

    $Id$
"""

import sys, os, traceback, optparse
import time
import re
import noc_plots as plot
import noc_io
import noc_trace_structure as trace
import numpy
import pylab
import matplotlib.pyplot as plt
from itertools import cycle

#from pexpect import run, spawn

def plotCumulativeInOut(data1, data2):

    filename="0"
    show=True
    title = ""
    label_x = ""
    label_y1 = ""
    label_y2 = ""
    x_size = 6.5
    y_size = 3.1
    x_lim = []
    y_lim = []
    logscale=False
    legend=['a','b']
    two_axis=False

    #consider that x is the same for both, and extract frmo the first only

    lines = ["-","--","-.",":"]
    linecycler = cycle(lines)

    s = len(data1)
    x = [(row[0]) for row in data1]
    y1 = [(row[1]) for row in data1]
    y2 = [(row[1]) for row in data2]



    fig, ax1 = plt.subplots(figsize=(x_size, y_size), dpi=120, facecolor='w', edgecolor='w')

    if logscale == True:
        pylab.yscale('log')

    ax1.step(x, y1, '-', linestyle=next(linecycler), label='r', where='post')
    ax1.set_xlabel(label_x)
    # Make the y-axis label and tick labels match the line color.
    ax1.set_ylabel(label_y1)
    # for tl in ax1.get_yticklabels():
    #     tl.set_color('b')

    if x_lim != []: pylab.xlim(x_lim)
    if y_lim != []: pylab.ylim(y_lim)

    if two_axis == True:
        ax2 = ax1.twinx()
    else:
        ax2 = ax1

    # s2 = np.sin(2*np.pi*t)
    ax2.step(x, y2, '-', linestyle=next(linecycler), label='t', where='post')
    ax2.set_ylabel(label_y2)

    if x_lim != []: pylab.xlim(x_lim)
    if y_lim != []: pylab.ylim(y_lim)

    pylab.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
    pylab.legend(loc=2, fontsize=11)
    pylab.grid(True)

    # ax = pylab.gca()
    # ax.set_xticklabels(x)

    # pylab.locator_params(axis='x', nbins=len(x))

    if filename != '0':
        dir = filename[:filename.rfind("/")]
        if not os.path.exists(dir):
            os.makedirs(dir)
        pylab.savefig(filename)
        print("Plot saved in " + filename)

    if show == True:
        plt.show()

def main ():

    global options, args
    options.inputfile = '/home/joao/usn-data/nw5x5s1n4/out/packets-trace-netdevice.csv'

    list = noc_io.load_list(options.inputfile)

    # list[:,trace.x_absolute] = int(list[:,trace.x_absolute])
    # list[:,trace.y_absolute] = int(list[:,trace.y_absolute])

    max_x = int(max(list[:,trace.x_absolute]))
    max_y = int(max(list[:,trace.y_absolute]))

    max_x = 5
    max_y = 5

    node_x, node_y = 1,0
    axis_x=[]
    axis_y=[]
    axis_y_received = []
    axis_x_received = []
    received_over_time = []
    transmitted_over_time = []
    # print 'max: ' + str(max_x) + ',' + str(max_y)

    # print list[:,trace.x_absolute]
    received = numpy.zeros([max_x + 1, max_y + 1])
    transmitted = numpy.zeros([max_x + 1, max_y + 1])
    for line in list:
        abs_x = int(line[trace.x_absolute])
        abs_y = int(line[trace.y_absolute])
        #Get the full picture
        if line[trace.operation] == 'r' or line[trace.operation] == 'g':
            received[ abs_y,abs_x ] += 1

        if line[trace.operation] == 't':
            transmitted[ abs_y,abs_x ] += 1

        if abs_x == node_x and abs_y == node_y:
            # axis_x.append(int(line[trace.time_slot]))
            # axis_y.append(transmitted[node_y, node_x])
            transmitted_over_time.append([int(line[trace.time_slot]),transmitted[node_y, node_x]])

            # axis_x_received.append(int(line[trace.time_slot]))
            # axis_y_received.append(received[node_y, node_x])
            received_over_time.append([int(line[trace.time_slot]),received[node_y, node_x]])

    received = numpy.flipud(received)
    transmitted = numpy.flipud(transmitted)
    # axis_x = numpy.
    print received
    print('\n')
    print transmitted

    # plot.plotxy(axis_x,axis_y, show=True)
    # plot.plotxy(axis_x_received,axis_y_received, show=True)
    plotCumulativeInOut(received_over_time, transmitted_over_time)















if __name__ == '__main__':
    try:
        start_time = time.time()
        parser = optparse.OptionParser(formatter=optparse.TitledHelpFormatter(), usage=globals()['__doc__'], version='$Id$')
        parser.add_option ('-v', '--verbose', action='store_true', default=False, help='verbose output')
        parser.add_option ('-i', '--inputfile', help='input file containing the packet trace')
        (options, args) = parser.parse_args()
        #if len(args) < 1:
        #    parser.error ('missing argument')
        if options.verbose: print time.asctime()
        main()
        if options.verbose: print time.asctime()
        if options.verbose: print 'Total execution time (s):',
        if options.verbose: print (time.time() - start_time)
        sys.exit(0)
    except KeyboardInterrupt, e: # Ctrl-C
        raise e
    except SystemExit, e: # sys.exit()
        raise e
    except Exception, e:
        print 'ERROR, UNEXPECTED EXCEPTION'
        print str(e)
        traceback.print_exc()
        os._exit(1)
