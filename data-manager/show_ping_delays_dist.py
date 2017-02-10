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
import numpy as np
import matplotlib.pyplot as plt

#My stuff
import files_io


if __name__ == '__main__':
    # try:
    start_time = time.time()
    parser = optparse.OptionParser(formatter=optparse.TitledHelpFormatter(), usage=globals()['__doc__'], version='$Id$')

    parser.add_option('--verbose', action='store_true', default=False, help='verbose output')
    parser.add_option('--inputdir', help='Dir containing the logs', default=None)
    parser.add_option('--outputdir', help='', default="")
    parser.add_option('--basedir', help='Base dir for simulation output files', default='')
    parser.add_option('--showplots', help='Show plots', default='True')
    parser.add_option('--shaper', help='Which traffic shaping policy to use', default='A')
    parser.add_option('--size_x', help='network size', default=0)
    parser.add_option('--size_y', help='network size', default=0)
    parser.add_option('--pos_x', help='node to analyse', default=0)
    parser.add_option('--pos_y', help='node to analyse', default=0)
    parser.add_option('--port', help='port to analyse', default=0)
    parser.add_option('--sinks_n', help='number of sinks', default=1)
    parser.add_option('--baudrate', help='baudrate utilized', default=3000000)
    parser.add_option('--packet_size', help='packet_size in bits', default=16*10)
    parser.add_option('--size_neighborhood', help='neighborhood size', default=0)

    (options, args) = parser.parse_args()
    if options.verbose: print (time.asctime())




    # inputfile_ping_delay = options.inputdir + '/ping-delay-log.csv'
    inputfile_ping_delay = "/home/joao/noc-data/INTERNAL_DELAY_nw51x1/out/ping-delay-log.csv"
    inputfile_ping_delay_hw = "/home/joao/Documentos/PhD/Artigos/NS3 Workshop'17/experiments/Logs/test_multihop_2_hops_1000_tests.csv"
    outputfile_ping_delay_dist = options.outputdir + 'ping-delay-dist.pdf'


    trace_ping_delay = files_io.load_numerical_list(inputfile_ping_delay)
    if (len(trace_ping_delay)) == 0:
        print('Ping delays not found not found.')
        exit(1)

    trace_ping_delay_hw = files_io.load_numerical_list(inputfile_ping_delay_hw, 2)
    if (len(trace_ping_delay)) == 0:
        print('Ping delays from hw not found not found.')
        # exit(1)

    # ax.grid(True)

    pck_dur = 53333

    normalize = True
    filter = True

    if filter:
        v_min = 0
        v_max = 10000000000000000

        nl = []
        for v in trace_ping_delay_hw:
            if v_min < v < v_max:
                nl.append(v)
        trace_ping_delay_hw = nl

        nl = []
        for v in trace_ping_delay:
            if v_min < v < v_max:
                nl.append(v)
        trace_ping_delay = nl

    if normalize:
        trace_ping_delay[:] = [x / pck_dur for x in trace_ping_delay]
        trace_ping_delay_hw[:] = [(x / pck_dur) for x in trace_ping_delay_hw]


    # -------------------
    # fig, ax = plt.subplots(1, 1, figsize=(6.5, 3.1), dpi=120, facecolor='w', edgecolor='w')
    # # ax.set_xlabel('Forward Delay ($\mu s$)')
    # ax.set_xlabel('Transmission time slot (TTS)')
    # ax.set_ylabel('Normalized Density')
    #
    # ax.hist(trace_ping_delay, bins='fd', facecolor='lightblue', alpha=1, normed=1, cumulative=0, log=0, align='mid', label='Histogram SIM')
    #
    # ax.legend(loc='best', frameon=True)#, prop={'size':12})
    # fig.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
    # file_out = outputfile_ping_delay_dist
    # # plt.savefig(dir + file_out)
    #
    # # --------------------
    # fig, ax = plt.subplots(1, 1, figsize=(6.5, 3.1), dpi=120, facecolor='w', edgecolor='w')
    # # ax.set_xlabel('Forward Delay ($\mu s$)')
    # ax.set_xlabel('Transmission time slot (TTS)')
    # ax.set_ylabel('Normalized Density')
    #
    # ax.hist(trace_ping_delay_hw, bins='fd', facecolor='lightblue', alpha=1, normed=1, cumulative=0, log=0, align='mid', label='Histogram HW')
    #
    # ax.legend(loc='best', frameon=True)#, prop={'size':12})
    # fig.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
    # file_out = outputfile_ping_delay_dist
    # # plt.savefig(dir + file_out)
    # plt.show()


    all_data = [trace_ping_delay, trace_ping_delay_hw]

    fig, axes = plt.subplots(nrows=1, ncols=2, figsize=(9, 4))

    axes[0].violinplot(all_data,
                       showmeans=False,
                       showmedians=True)
    # axes[0].set_title('violin plot')

    # plot box plot
    axes[1].boxplot(all_data)
    # axes[1].set_title('box plot')

    # adding horizontal grid lines
    for ax in axes:
        ax.yaxis.grid(True)
        ax.set_xticks([y+1 for y in range(len(all_data))])
        # ax.set_xlabel('xlabel')
        ax.set_ylabel('Transmission Time Slot (TTS)')

    # add x-tick labels
    plt.setp(axes, xticks=[y+1 for y in range(len(all_data))],
             xticklabels=['SIM', 'HW'])
    plt.show()








