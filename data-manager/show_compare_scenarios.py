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



import time
import os, optparse
import files_io
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
from itertools import cycle


if __name__ == '__main__':
    # try:
    start_time = time.time()
    parser = optparse.OptionParser(formatter=optparse.TitledHelpFormatter(), usage=globals()['__doc__'], version='$Id$')

    parser.add_option('--verbose', action='store_true', default=False, help='verbose output')
    parser.add_option('--inputdir', help='Dir containing the logs', default='')
    parser.add_option('--basedir', help='Base dir for simulation output files', default='')
    parser.add_option('--outputdir', help='', default='')
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

    ############################# Global vars and settings #######################

    def plot_bars(x, fx_list, axis_label=None, x_ticks='', filename=None, show=False, title='', label_y='',
                  label_x='', log=False, y_lim=None):

        x_size = 3
        y_size = 3

        fig, ax = plt.subplots(figsize=(x_size, y_size), dpi=110, facecolor='w', edgecolor='w')
        fig.canvas.set_window_title(title)

        # http://matplotlib.org/mpl_examples/color/named_colors.pdf
        colours = [ 'sage', 'darkkhaki' , 'lightslategray','lightgreen', 'lightblue', 'seagreen', 'darkcyan',
                    'darkgreen', 'darkcyan', 'darkblue', 'burlywood']
        colourcycler = cycle(colours)

        hatches = ['x','/','//']
        hatchcycler = cycle(hatches)

        index = np.arange(len(x))
        # bar_width = 0.35
        opacity = 0.9
        x = np.asarray(x)

        dim = len(x)
        w = 1
        dimw = w / dim

        for i in range(len(fx_list)):
            rects1 = plt.bar(
                x + (i * dimw), fx_list[i], dimw,
                alpha=opacity,
                color=next(colourcycler),
                # yerr=std_men,
                # error_kw=error_config,
                label=axis_label[i],
                hatch=next(hatchcycler)
                             )

        if y_lim is not None:
            plt.ylim(y_lim)

        plt.xlabel(label_x)
        plt.ylabel(label_y)
        if (log):
            plt.yscale('log')

        plt.xticks(x + w / 2, x_ticks)
        plt.legend(loc=0, fontsize=11)

        plt.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)

        if filename is not None:
            dir = filename[:filename.rfind("/")]
            if not os.path.exists(dir):
                os.makedirs(dir)
            plt.savefig(filename)
            print("Plot saved in " + filename)

        if show == True:
            plt.draw()

    def plot_bars3d(x, fx3d_list, axis_label=None, x_ticks='', filename=None, show=False, title='', label_y='', label_x='', log=False):
        x_size = 6
        y_size = 3

        fig = plt.figure()
        ax = fig.add_subplot(111, projection='3d')

        # fig, ax = plt.subplots(figsize=(x_size, y_size), dpi=110, facecolor='w', edgecolor='w')
        fig.canvas.set_window_title(title)

        colours = ['darkgreen', 'darkcyan', 'darkblue', 'burlywood', 'seagreen', 'confotableblue']
        colourcycler = cycle(colours)

        index = np.arange(len(x))
        # bar_width = 0.35
        opacity = 0.9
        x = np.asarray(x)

        dim = len(x)
        w = 0.60
        dimw = w / dim

        fx_list = fx3d_list[0]

        for n in range(len(fx3d_list)):
            for i in range(len(fx_list)):
                rects1 = ax.bar3d(
                    x + (i * dimw), n, fx_list[i],
                    dimw, dimw, 0,
                    alpha=opacity,
                    # color=next(colourcycler),
                    # yerr=std_men,
                    # error_kw=error_config,
                    # label=axis_label[i],
                    # hatch="/"
                                 )


        plt.xlabel(label_x)
        plt.ylabel('n')
        # if (log):
        #     plt.zscale('log')

        # plt.xticks(x + w / 2, x_ticks)
        # plt.legend(loc=0, fontsize=11)

        # plt.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)

        # plt.ylim([1e1, 16000])
        # plt.ylim([0, 31])

        # if filename is not None:
        #     dir = filename[:filename.rfind("/")]
        #     if not os.path.exists(dir):
        #         os.makedirs(dir)
        #     plt.savefig(filename)
        #     print("Plot saved in " + filename)

        # if show == True:
        #     plt.draw()

    def plot_multiple_lines(x, fx_list, axis_label=None, x_ticks='', filename=None, show=False, title='', label_y='', label_x='', log=False):
        label_x = "$n_{size}$"
        x_size = 6
        y_size = 3
        # logscale = True

        lines = ["-", "--", "-.", ":"]
        linecycler = cycle(lines)

        fig, ax = plt.subplots(figsize=(x_size, y_size), dpi=110, facecolor='w', edgecolor='w')
        fig.canvas.set_window_title(title)

        if log == True:
            plt.yscale('log')

        for i in range(len(fx_list)):
            axi = ax
            axi.plot(x, fx_list[i], 'o', linestyle=next(linecycler), label=axis_label[i])
            # axi.plot(axis, data[i], 'o', linestyle=next(linecycler))

        ax.set_xlabel(label_x)
        ax.set_ylabel(label_y)

        plt.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
        plt.legend(loc=0, fontsize=11)
        plt.grid(True)


        # ax = plt.gca()
        # ax.set_xticklabels(axis)

        plt.xlim([x[0]-0.1, x[-1]+0.1])
        plt.locator_params(axis='x', nbins=len(x))
        plt.legend(loc=2, fontsize=11)

        if filename is not None:
            dir = filename[:filename.rfind("/")]
            if not os.path.exists(dir):
                os.makedirs(dir)
            plt.savefig(filename)
            print("Plot saved in " + filename)

        if show == True:
            plt.draw()

    def get_scenario(array_result_files, shaper_in, beta_in, n_size_in):
        for file in sorted(array_result_files):
            context = file.split('/')[-4]
            i = context.rfind('b')
            j = context.rfind('nw')
            beta = context[i+1:j]
            i = context.rfind('s')
            shaper = context[i+1:i+3]
            i = context.rfind('n')
            n = int(context[i + 1])

            if beta == beta_in and n == n_size_in and shaper == shaper_in:
                r = files_io.load_line(file).split(',')
                return [float(v) for v in r]

        return None

    def compare_routing_protocols():

        base_dir = options.basedir

        protocols = ['XY', 'YX', 'CW']
        lines = ['N/S', 'E/W']

        #Data anotated from following scenario:
        #  beta=1, ms = 1, o=dist, nw=21x11, one sink in center (10,5)
        # ports north and south repost the same values, and also e/w, so they are grouped
        ns_d = [107,10,57]
        ns_q = [20,1,10]

        ew_d = [20,112,62]
        ew_q = [1,10,5]

        x = [i for i in range(len(protocols))]

        ys = [ns_d, ew_d]
        fn = base_dir + 'post/routing_delay_comparison.pdf'
        plot_bars(x, ys, axis_label=lines, x_ticks=protocols, log=False,
                  filename=fn, label_y='Total time (TTS)', label_x='Routing protocol', title='Total time')

        ys = [ns_q, ew_q]
        fn = base_dir + 'post/routing_max_queue_comparison.pdf'
        plot_bars(x, ys, axis_label=lines, x_ticks=protocols, log=False,
                  filename=fn, label_y='Max queue size', label_x='Routing protocol', title='Max queue')

        return None


    compare_routing_protocols()
    plt.show()
    exit(0)

    base_dir = options.basedir

    array_result_files = files_io.find_multiple_files(base_dir, 'results.csv')
    if (len(array_result_files)) == 0:
        print('No result files (results.csv) found inside {}'.format(base_dir))
        exit(1)

    # Table of indexes
    # 0    sim_max_queue,
    # 1    model_max_queue,
    # 2    sim_max_delay,
    # 3    model_max_delay,
    # 4    sim_total_time,
    # 5    model_total_time
    # 6    final_beta
    # 7    final_offset
    # 8    final_ms

    n_size_mask = [2,3,4,5]
    # n_size_mask = [3]
    beta_mask = ['0.01', '0.05', '0.10', '0.50', '1.00']
    shapers_mask = ['BU', 'RL', 'TD',  'TL', 'SIM']


    what = 1 # model_total_time

    if what == 5: # TOTAL TIME
        label_y = 'Total time (TTS)'
        use_log_scale = True
        fncontext = 'total_time'
        y_lim = [10, 16000]

    elif what == 1: # MAX QUEUE
        label_y = 'Max. queue size'
        use_log_scale = False
        fncontext = 'max_queue'
        y_lim = [0, 31]

    elif what == 6:
        label_y = 'Equivalent beta'
        use_log_scale = False
        fncontext = 'eq_beta'
        y_lim = []


    for n in n_size_mask:
        fx_list = []
        for b in beta_mask:
            fxi = []
            for s in shapers_mask:
                if s == 'SIM':
                    index = what - 1
                    s = shapers_mask[0]  # any other one
                else:
                    index = what

                results = get_scenario(array_result_files, s, b, n) #return a single value
                # if results is not None:
                fxi.append(results[index])

            fx_list.append(fxi)

        x = [i for i in range(len(shapers_mask))]

        fn = base_dir + 'post/shaper_vs_' + fncontext + '_n_' + str(n) + '.pdf'

        plot_bars(x, fx_list, axis_label=beta_mask, x_ticks=shapers_mask, log=use_log_scale,
                  filename=fn, label_y=label_y, label_x='Shaping scheme', title='n=' + str(n), y_lim=y_lim)


    fx3d_list = []
    for n in n_size_mask:
        fx_list = []
        for s in shapers_mask:
            if s == 'SIM':
                index = what - 1
                s = shapers_mask[0] #any other one
            else:
                index = what

            fxi = []

            for b in beta_mask:
                results = get_scenario(array_result_files, s, b, n) #return a single value

                if results is not None:
                    fxi.append(results[index])

            fx_list.append(fxi)

        x = [i for i in range(len(beta_mask))]

        fx3d_list.append(fx_list)

        fn = base_dir + 'post/beta_vs_' + fncontext + '_n_' + str(n) + '.pdf'

        plot_bars(x, fx_list, axis_label=shapers_mask,  x_ticks=beta_mask, log=use_log_scale,
                  filename=fn, label_y=label_y, label_x='Burstiness (beta)', title='n=' + str(n), y_lim=y_lim)
        # plot_multiple_lines(x, fx_list, axis_label=shapers_mask,  x_ticks=beta_mask, log=use_log_scale, filename=fn, label_y=label_y, label_x='Burstiness (beta)', title='n=' + str(n))

    # plot_bars3d(x, fx3d_list, axis_label=shapers_mask, x_ticks=beta_mask, log=use_log_scale, filename=fn, label_y=label_y,
    #           label_x='Burstiness (beta)', title='n=' + str(n))

    plt.show()
