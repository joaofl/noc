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

    def plot2xy(axis, data, axis_label=None, filename=None, show=False, title='', label_y=''):
        label_x = "$n_{size}$"
        x_size = 6.5
        y_size = 3.1
        # logscale = True

        lines = ["-", "--", "-.", ":"]
        linecycler = cycle(lines)

        fig, ax = plt.subplots(figsize=(x_size, y_size), dpi=120, facecolor='w', edgecolor='w')
        fig.canvas.set_window_title(title)

        # if logscale == True:
        #     plt.yscale('log')

        for i in range(len(data)):
            axi = ax
            axi.plot(axis, data[i], 'o', linestyle=next(linecycler), label=axis_label[i])

        ax.set_xlabel(label_x)
        ax.set_ylabel(label_y)

        plt.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
        plt.legend(fontsize=11)
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
            plt.draw()

    def get_group(array_result_files, index, shaper_in='SA', beta_in='B0.1'):
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


    base_dir = options.basedir

    array_result_files = files_io.find_multiple_files(base_dir, 'results.csv')
    if (len(array_result_files)) == 0:
        print('No result files (results.csv) found inside {}'.format(base_dir))
        exit(1)

    # Table of indexes
    # 0    sim_max_queue,
    # 1    sim_max_delay,
    # 2    sim_total_time,
    # 3    model_max_queue,
    # 4    model_max_delay,
    # 5    model_total_time

    # Queue
    # fn = base_dir + 'post/' + shaper
    beta_in = 'B1.0'
    [x, s] = get_group(array_result_files, 0 , shaper_in='SA', beta_in=beta_in) # If shaping is disabled, sim should be the same for A B or C
    [x, mA] = get_group(array_result_files, 3, shaper_in='SA', beta_in=beta_in)
    [x, mB] = get_group(array_result_files, 3, shaper_in='SB', beta_in=beta_in)
    [x, mC] = get_group(array_result_files, 3, shaper_in='SC', beta_in=beta_in)
    plot2xy(x, [s, mA, mC], axis_label=['Sim', 'A', 'C'], show=True, title='Maximum hop queue')

    # Delay
    [x, s] = get_group(array_result_files, 1 , shaper_in='SA', beta_in=beta_in) # If shaping is disabled, sim should be the same for A B or C
    [x, mA] = get_group(array_result_files, 4, shaper_in='SA', beta_in=beta_in)
    [x, mB] = get_group(array_result_files, 4, shaper_in='SB', beta_in=beta_in)
    [x, mC] = get_group(array_result_files, 4, shaper_in='SC', beta_in=beta_in)
    plot2xy(x, [s, mA, mC], axis_label=['Sim', 'A', 'C'], show=True, title='Maximum hop delay')

    # Duration
    [x, s] = get_group(array_result_files, 2,  shaper_in='SA', beta_in=beta_in)  # If shaping is disabled, sim should be the same for A B or C
    [x, mA] = get_group(array_result_files, 5, shaper_in='SA', beta_in=beta_in)
    [x, mB] = get_group(array_result_files, 5, shaper_in='SB', beta_in=beta_in)
    [x, mC] = get_group(array_result_files, 5, shaper_in='SC', beta_in=beta_in)
    plot2xy(x, [s, mA, mC], axis_label=['Sim', 'A', 'C'], show=True, title='Total time')

    plt.show()