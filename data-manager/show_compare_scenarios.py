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

    def plot_bars(x, fx_list, axis_label=None, x_ticks='', filename=None, show=False, title='', label_y='', log=False):
        x_size = 6
        y_size = 3

        fig, ax = plt.subplots(figsize=(x_size, y_size), dpi=110, facecolor='w', edgecolor='w')
        fig.canvas.set_window_title('Title')

        colours = ['green', 'yellow', 'blue', 'cyan', 'magenta', 'purple']
        colourcycler = cycle(colours)

        index = np.arange(len(x))
        # bar_width = 0.35
        opacity = 0.9
        x = np.asarray(x)

        dim = len(x)
        w = 0.60
        dimw = w / dim

        for i in range(len(fx_list)):
            rects1 = plt.bar(
                x + (i * dimw), fx_list[i], dimw,
                alpha=opacity,
                color=next(colourcycler),
                # yerr=std_men,
                # error_kw=error_config,
                label=axis_label[i]
                             )


        plt.xlabel('Shaping')
        plt.ylabel(label_y)
        if (log):
            plt.yscale('log')
        # plt.title('Scores by group and gender')
        # plt.xticks(index + bar_width, ('A', 'B', 'C'))
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

    def plot_multiple_lines(axis, data, axis_label=None, filename=None, show=False, title='', label_y=''):
        label_x = "$n_{size}$"
        x_size = 6
        y_size = 3
        # logscale = True

        lines = ["-", "--", "-.", ":"]
        linecycler = cycle(lines)

        fig, ax = plt.subplots(figsize=(x_size, y_size), dpi=110, facecolor='w', edgecolor='w')
        fig.canvas.set_window_title(title)

        # if logscale == True:
        #     plt.yscale('log')

        for i in range(len(data)):
            axi = ax
            axi.plot(axis, data[i], 'o', linestyle=next(linecycler), label=axis_label[i])
            # axi.plot(axis, data[i], 'o', linestyle=next(linecycler))

        ax.set_xlabel(label_x)
        ax.set_ylabel(label_y)

        plt.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
        plt.legend(loc=0, fontsize=11)
        plt.grid(True)


        # ax = plt.gca()
        # ax.set_xticklabels(axis)

        plt.xlim([axis[0]-0.1, axis[-1]+0.1])
        plt.locator_params(axis='x', nbins=len(axis))
        plt.legend(loc=2, fontsize=11)

        if filename is not None:
            dir = filename[:filename.rfind("/")]
            if not os.path.exists(dir):
                os.makedirs(dir)
            plt.savefig(filename)
            print("Plot saved in " + filename)

        if show == True:
            plt.draw()

    def filter(array_result_files, index, shaper_in, beta_in):
        y = []
        x = []

        for file in sorted(array_result_files):
            context = file.split('/')[-4]
            beta = context.split('_')[2]
            shaper = context.split('_')[3]
            i = context.rfind('n')
            # j = context.rfind('S')
            # txt = context[i + 1]  # + context[j+1]

            if shaper == shaper_in and beta == beta_in:
                # label.append(txt)
                x.append(int(context[i + 1]))
                r = files_io.load_line(file).split(',')
                y.append(float(r[index]))

        return [x,y]

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

    # Queue
    # fn = base_dir + 'post/' + shaper
    n_size_mask = [2,3,4,5]
    beta_mask = ['0.01', '0.05', '0.10', '0.50', '1.00']
    shapers_mask = ['BU', 'TD', 'RL', 'TL', 'SIM']
    # shapers_mask_old = ['SA', 'SB', 'SC']




    #Next add from all
    what = 1 # model_total_time
    n_size = 3 #something wrong with 4

    for n in n_size_mask:
        fx_list = []
        for b in beta_mask:
            fxi = []
            for s in shapers_mask:
                results = get_scenario(array_result_files, s, b, n) #return a single value

                if results is not None:
                    fxi.append(results[what])
                    fxi_sim = results[what - 1]

            fxi.append(fxi_sim) #sim goes last
            fx_list.append(fxi)

        x = [i for i in range(len(shapers_mask))]

        fn = base_dir + 'post/n' + str(n) + '_max_queue.pdf'
        plot_bars(x, fx_list, axis_label=beta_mask, x_ticks=shapers_mask, log=False, filename=fn)
        plt.draw()

    plt.show()


    #
    # # Here, do neighborgood size, in function of shaper used, in terms of delay/queue/duration
    # for beta_in in beta_mask:
    #     [x, s] = filter(array_result_files, 0, shaper_in='SA', beta_in=beta_in) # If shaping is disabled, sim should be the same for A B or C
    #     [x, mA] = filter(array_result_files, 3, shaper_in='SA', beta_in=beta_in)
    #     [x, mB] = filter(array_result_files, 3, shaper_in='SB', beta_in=beta_in)
    #     [x, mC] = filter(array_result_files, 3, shaper_in='SC', beta_in=beta_in)
    #     # plot2xy(x, [s, mA, mC], axis_label=['Sim', 'A', 'C'], show=True, title='Maximum hop queue')
    #
    #     fn = base_dir + 'post/' + beta_in + '_max_queue.pdf'
    #     plot_multiple_lines(x, [s, mA, mB, mC], axis_label=shapers_mask, show=True, title='Maximum hop queue', label_y='Max. queue size', filename=fn)
    #
    #     # Delay
    #     [x, s] = filter(array_result_files, 1, shaper_in='SA', beta_in=beta_in) # If shaping is disabled, sim should be the same for A B or C
    #     [x, mA] = filter(array_result_files, 4, shaper_in='SA', beta_in=beta_in)
    #     [x, mB] = filter(array_result_files, 4, shaper_in='SB', beta_in=beta_in)
    #     [x, mC] = filter(array_result_files, 4, shaper_in='SC', beta_in=beta_in)
    #     # plot2xy(x, [s, mA, mC], axis_label=['Sim', 'A', 'C'], show=True, title='Maximum hop delay')
    #
    #     fn = base_dir + 'post/' + beta_in + '_max_delay.pdf'
    #     plot_multiple_lines(x, [s, mA, mB, mC], axis_label=shapers_mask, show=True, title='Maximum hop delay', label_y='Max. hop delay (TTS)', filename=fn)
    #
    #     # Duration
    #     [x, s] = filter(array_result_files, 2, shaper_in='SA', beta_in=beta_in)  # If shaping is disabled, sim should be the same for A B or C
    #     [x, mA] = filter(array_result_files, 5, shaper_in='SA', beta_in=beta_in)
    #     [x, mB] = filter(array_result_files, 5, shaper_in='SB', beta_in=beta_in)
    #     [x, mC] = filter(array_result_files, 5, shaper_in='SC', beta_in=beta_in)
    #     # plot2xy(x, [s, mA, mC], axis_label=['Sim', 'A', 'C'], show=True, title='Total time')
    #
    #     fn = base_dir + 'post/' + beta_in + '_total_time.pdf'
    #     plot_multiple_lines(x, [s, mA, mB, mC], axis_label=shapers_mask, show=True, title='Total time', label_y='Total time (TTS)', filename=fn)
    #
    #
    #
    # plt.show()