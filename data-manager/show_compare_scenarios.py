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
                  label_x='', log=False, y_lim=[]):

        x_size = 4
        y_size = 3

        fig, ax = plt.subplots(figsize=(x_size, y_size), dpi=110, facecolor='w', edgecolor='w')
        fig.canvas.set_window_title(title)

        # http://matplotlib.org/mpl_examples/color/named_colors.pdf
        colors = ['green', 'blue', 'seagreen', 'darkcyan',
                    'darkgreen', 'darkcyan', 'darkblue', 'burlywood']
        colourcycler = cycle(colors)

        hatches = ['x','/','//']
        hatchcycler = cycle(hatches)

        index = np.arange(len(x))
        # bar_width = 0.35
        opacity = 0.9
        x = np.asarray(x)

        dim = len(x)
        w = 1.5
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

        if y_lim != []:
            plt.ylim(y_lim)

        plt.xlabel(label_x)
        plt.ylabel(label_y)
        if (log):
            plt.yscale('log')

        plt.xticks(x + w / len(fx_list)/2, x_ticks)
        plt.legend(loc=0, fontsize=10)

        plt.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)

        if filename is not None:
            dir = filename[:filename.rfind("/")]
            if not os.path.exists(dir):
                os.makedirs(dir)
            plt.savefig(filename)
            print("Plot saved in " + filename)

        if show == True:
            plt.draw()


    def plot_multiple_lines(x, fx_list, axis_label=None, x_ticks=[], filename=None, show=False, title='', label_y='',
                  label_x='', log=False, y_lim=None, step=False, mark=['']):

        x_size = 3.6
        y_size = 3
        # logscale = True

        # lines = ["-", "--", "-.", ":"]
        # lines = ["-"]
        # dashes = [ [100,1],[1,1],[4,2],[8,4] ]
        dashes = ['-']
        linecycler = cycle(dashes)


        makercycler = cycle(mark)

        fig, ax = plt.subplots(figsize=(x_size, y_size), dpi=110, facecolor='w', edgecolor='w')
        fig.canvas.set_window_title(title)

        if log == True:
            plt.yscale('log')

        for i in range(len(fx_list)):
            axi = ax
            if step == False:
                axi.plot(x, fx_list[i], next(makercycler), label=axis_label[i], markevery=6, markersize=8, linewidth=5)
            else:
                axi.step(x, fx_list[i], next(makercycler), label=axis_label[i], markevery=6, markersize=8, linewidth=5)
            # axi.plot(axis, data[i], 'o', linestyle=next(linecycler))

        ax.set_xlabel(label_x)
        ax.set_ylabel(label_y)

        plt.tight_layout(pad=0.5, w_pad=0.3, h_pad=0.3)
        plt.legend(loc=0, prop={'size':6})#, fontsize=11)
        plt.grid(False)

        if y_lim != []:
            plt.ylim(y_lim)

        # ax = plt.gca()
        # ax.set_xticklabels(axis)

        plt.xlim([x[0]-0.01, x[-1]+0.01])
        # plt.locator_params(axis='x', nbins=len(x))

        if x_ticks != []:
            plt.xticks(x_ticks)

        plt.legend(loc=0, fontsize=11)

        if filename is not None:
            dir = filename[:filename.rfind("/")]
            if not os.path.exists(dir):
                os.makedirs(dir)
            plt.savefig(filename)
            print("Plot saved in " + filename)

        if show == True:
            plt.draw()


    def plot_multiple_planes(x, y, z_array, x_ticks='', filename=None, show=False, title='', label_y='',
                  label_x='', label_z='', log=False, y_lim=None, step=False, mark='', color='blue'):
        x_size = 7
        y_size = 3

        fig, ax = plt.subplots(figsize=(x_size, y_size), dpi=110, facecolor='w', edgecolor='w', subplot_kw={'projection': '3d'})

        if x_ticks != []:
            plt.xticks(x_ticks)

        plt.gca().invert_yaxis()
        # plt.gca().invert_xaxis()

        ax.set_xlabel(label_x)
        ax.set_zlabel(label_z)
        ax.set_ylabel(label_y)

        x_mesh = [x for _ in y]
        y_mesh = [[y[i] for _ in range(len(x))] for i in range(len(y))]

        for z in z_array:
            ax.plot_wireframe(x_mesh, y_mesh, z, color=color, alpha=0.7)
            ax.set_zlim(0,2000)
            # ax.plot_surface(x_mesh, y_mesh, z, color=next(colourcycler), alpha=0.7)
        # ax.plot_wireframe(x_mesh, y_mesh, )

        plt.tight_layout()
        plt.draw()

        if filename is not None:
            dir = filename[:filename.rfind("/")]
            if not os.path.exists(dir):
                os.makedirs(dir)
            plt.savefig(filename)
            print("Plot saved in " + filename)


    def get_scenario(array_result_files, shaper_in, beta_in, n_size_in, context_in='WCA_CLUSTER_'):
        for file in sorted(array_result_files):
            folder_name = file.split('/')[-4]
            i = folder_name.rfind('b')
            context = folder_name[0:i]
            j = folder_name.rfind('nw')
            beta = folder_name[i+1:j]
            i = folder_name.rfind('s')
            shaper = folder_name[i+1:i+3]
            i = folder_name.rfind('n')
            n = int(folder_name[i + 1])

            if beta == beta_in and n == n_size_in and shaper == shaper_in and context == context_in:
                r = files_io.load_line(file).split(',')
                return [float(v) for v in r]

        return None

    def compare_routing_protocols():

        base_dir = options.basedir

        protocols = ['XY', 'YX', 'CW', 'CO']
        lines = ['N/S', 'E/W']

        #Data manually captured from following scenario:
        #  beta=1, ms = 1, o=dist, nw=21x11, one sink in center (10,5)
        # ports north and south report the same values, and also e/w, so they are grouped
        ns_d = [107,10,57, 52]
        ns_q = [20,1,10, 11]

        ew_d = [20,112,62, 61]
        ew_q = [1,10,5, 6]

        x = [i for i in range(len(protocols))]

        ys = [ns_d, ew_d]
        fn = base_dir + 'post/routing_delay_comparison.pdf'
        plot_bars(x, ys, axis_label=lines, x_ticks=protocols, log=False, y_lim=[],
                  filename=fn, label_y='Total time (TTS)', label_x='Routing protocol', title='Total time')

        ys = [ns_q, ew_q]
        fn = base_dir + 'post/routing_max_queue_comparison.pdf'
        plot_bars(x, ys, axis_label=lines, x_ticks=protocols, log=False, y_lim=[0,21],
                  filename=fn, label_y='Max queue size', label_x='Routing protocol', title='Max queue')

        return None

    ############## SCRIPT FROM HERE ####################################################################################

    # compare_routing_protocols()
    # plt.show()
    # exit(0)

    base_dir = options.basedir

    array_result_files = files_io.find_multiple_files(base_dir, 'results.csv')
    if (len(array_result_files)) == 0:
        print('No result files (results.csv) found inside {}'.format(base_dir))
        exit(1)

    # Table of indexes
    # 0    sim_max_queue,
    # 1    model_max_queue,
    INDEX_MODEL_MAX_QUEUE = 1
    # 2    sim_max_delay,
    # 3    model_max_delay,
    # 4    sim_total_time,
    # 5    model_total_time
    INDEX_MODEL_TOTAL_TIME = 5
    INDEX_RESULTING_BETA = 6
    INDEX_RESULTING_OFFS = 7
    INDEX_RESULTING_MSGS = 8
    # 6    resulting_beta
    # 7    final_offset
    # 8    final_ms

    n_size_mask = [1,2,3,4,5]
    # n_size_mask = [1,5]
    # n_size_mask = [3]

    # beta_mask = [b / 100 for b in range(1, 101, 2)]
    beta_mask = [b/100 for b in np.logspace(0, 2, 30)]
    # beta_mask = sorted([(101-b)/100 for b in np.logspace(0, 2, 30)])
    # beta_mask = [0.01, 0.02, 0.04, 0.05, 0.06, 0.08, 0.1, 0.5, 1]
    # beta_mask_str = ['0.01', '0.02', '0.04', '0.05', '0.06', '0.08', '0.10', '0.50', '1.00']
    beta_mask_str = ['{:0.04f}'.format(b) for b in beta_mask]


    # beta_mask = ['0.10', '0.20', '0.30', '0.40', '0.50', '0.60', '0.70', '0.80', '0.90', '1.00']
    # shapers_mask_str = ['BU', 'RL', 'TD', 'TL', 'SIM']
    shapers_mask_str = ['BU', 'TL', 'RL', 'SIM']
    shapers_mask_str_plot = ['Min-O', 'Max-S', 'LQ', 'SIM']

    context_mask_str = ['WCA_CLUSTER_', 'WCA_FULL_']
    # context_mask_str = ['WCA_FULL_']


    # for n in n_size_mask:
    #     fx_list = []
    #     for b in beta_mask:
    #         fxi_beta = []
    #         for s in shapers_mask:
    #             if s == 'SIM':
    #                 index = what - 1
    #                 s = shapers_mask[0]  # any other one
    #             else:
    #                 index = what
    #
    #             results = get_scenario(array_result_files, s, b, n, context_in=context_mask_str[0]) #return a single value
    #             if results is not None:
    #                 fxi_beta.append(results[index])
    #
    #         fx_list.append(fxi_beta)
    #
    #     x = [i for i in range(len(shapers_mask))]
    #     fn = base_dir + 'post/shaper_vs_' + fncontext + '_n_' + str(n) + '.pdf'
    #
    #     plot_bars(x, fx_list, axis_label=beta_mask, x_ticks=shapers_mask, log=use_log_scale,
    #               filename=fn, label_y=label_y, label_x='Shaping scheme', title='n={}'.format(n), y_lim=y_lim)




    # new_beta_mask = [ beta_mask[0] , beta_mask[int(len((beta_mask_str))/3)] , beta_mask[-1] ]
    # new_beta_mask_str = ['{:0.04f}'.format(b) for b in new_beta_mask]

    i_beta = int(len((beta_mask_str))/3) #pick the beta value to show on the 2d plots
    i_beta = -1

    for s in ['TL']:
    # for each s, a new plot will be generated
        fx_list_util = {s:[] for s in context_mask_str}
        fx_list_delay = {s:[] for s in context_mask_str}
        fx_list_queue = {s:[] for s in context_mask_str}

        for b in beta_mask_str: #, 'SIM']:
            if s == 'SIM':
                i_queue = INDEX_MODEL_MAX_QUEUE - 1
                i_delay = INDEX_MODEL_TOTAL_TIME -1
                s = shapers_mask_str[0] #any other one
            else:
                i_queue = INDEX_MODEL_MAX_QUEUE
                i_delay = INDEX_MODEL_TOTAL_TIME


            for context in context_mask_str:

                fxi_util = []
                fxi_delay = []
                fxi_queue = []

                for n in n_size_mask:
                    results = get_scenario(array_result_files, s, b, n, context_in=context)
                    if results is not None:
                        fxi_delay.append(results[i_delay])
                        fxi_queue.append(results[i_queue])
                        fxi_util.append(results[INDEX_RESULTING_MSGS] / results[i_delay])
                    else:
                        print('Expected scenario beta={}, shaper={}, nsize={} not found'.format(b, s, n))

                fx_list_util[context].append(fxi_util)
                fx_list_delay[context].append(fxi_delay)
                fx_list_queue[context].append(fxi_queue)

        x = [int(n) for n in n_size_mask]

        # axis_labels = [context[4:-1] for context in context_mask_str]
        axis_labels = ['$\\phi_3$', '$\\phi_4$']

        # plot it, but for a single value of beta
        fn = base_dir + 'post/n_size_vs_total_time_b_{}.pdf'.format(beta_mask_str[i_beta])
        lines = [fx_list_delay[context][i_beta] for context in context_mask_str]
        plot_multiple_lines(x, lines, axis_label=axis_labels, x_ticks=n_size_mask, log=False,
                            filename=fn, label_y='Total time (TTS)', label_x='Neigborhood size $(n_{radius})$', title='s={}'.format(s),
                            y_lim=[], mark=['*', 'x'])

        #plot it, but for a single value of beta
        fn = base_dir + 'post/n_size_vs_max_queue_b_{}.pdf'.format(beta_mask_str[i_beta])
        lines = [fx_list_queue[context][i_beta] for context in context_mask_str]
        plot_multiple_lines(x, lines, axis_label=axis_labels, x_ticks=n_size_mask, log=False,
                            filename=fn, label_y='Max queue size', label_x='Neigborhood size $(n_{radius})$', title='s={}'.format(s),
                            y_lim=[], mark=['*', 'x'])

        # colors = ['blue', 'green', 'darkblue', 'burlywood']
        colors = ['black']
        colourcycler = cycle(colors)
        # fn = base_dir + 'post/n_size_vs_beta_vs_total_time_b_{}.pdf'.format(b)
        # planes = [fx_list_delay[context] for context in context_mask_str]
        for context in context_mask_str:
            fn = base_dir + 'post/n_size_vs_beta_vs_total_time_{}.pdf'.format(context)
            plot_multiple_planes(x, beta_mask, [fx_list_delay[context]], x_ticks=x, filename=fn, label_y='Burstiness $(\\beta)$',
                                 label_x='Neigborhood size $(n_{radius})$', label_z='Total time (TTS)', title='s={}'.format(s),
                                 color=next(colourcycler))


    plt.show()
    # exit(1)

    n_size_mask = [1, 5]

    for context in context_mask_str:
        for n in n_size_mask:
            fx_list_util = []
            fx_list_delay = []
            fx_list_queue = []

            for s in shapers_mask_str:
                if s == 'SIM':
                    i_queue = INDEX_MODEL_MAX_QUEUE - 1
                    i_delay = INDEX_MODEL_TOTAL_TIME -1
                    s = shapers_mask_str[0] #any other one
                else:
                    i_queue = INDEX_MODEL_MAX_QUEUE
                    i_delay = INDEX_MODEL_TOTAL_TIME

                fxi_util = []
                fxi_delay = []
                fxi_queue = []

                for b in beta_mask_str:
                    results = get_scenario(array_result_files, s, b, n, context_in=context) #return a single value
                    if results is not None:
                        fxi_delay.append(results[i_delay])
                        fxi_queue.append(results[i_queue])
                        fxi_util.append(results[INDEX_RESULTING_MSGS] / results[i_delay])
                    else:
                        print('Expected scenario beta={}, shaper={}, nsize={} not found'.format(b, s, n))

                fx_list_util.append(fxi_util)
                fx_list_delay.append(fxi_delay)
                fx_list_queue.append(fxi_queue)

            x = [i for i in range(len(beta_mask_str))]



            # fn = base_dir + 'post/beta_vs_total_time_n_{}.pdf'.format(n)
            # plot_bars(x, fx_list_delay, axis_label=shapers_mask_str,  x_ticks=beta_mask_str, log=True,
            #           filename=fn, label_y='Total time (TTS)', label_x='Burstiness (beta)', title='n={}'.format(n), y_lim=[10, 16000])
            fn = base_dir + 'post/{}_beta_vs_total_time_n_{}.pdf'.format(context, n)
            plot_multiple_lines(beta_mask, fx_list_delay, axis_label=shapers_mask_str_plot,  x_ticks=[], log=True,
                filename=fn, label_y='Total time (TTS)', label_x='Burstiness $(\\beta)$', title='n={}_{}'.format(context,n), y_lim=[1e1,2e4], mark=['o', '^', 'x', '+'])


            # fn = base_dir + 'post/beta_vs_max_queue_n_{}.pdf'.format(n)
            # plot_bars(x, fx_list_queue, axis_label=shapers_mask_str_plot,  x_ticks=beta_mask_str, log=False,
            #           filename=fn, label_y='Max. queue size', label_x='Burstiness (beta)', title='n={}'.format(n), y_lim=[0, 31])
            fn = base_dir + 'post/{}_beta_vs_max_queue_n_{}.pdf'.format(context, n)
            plot_multiple_lines(beta_mask, fx_list_queue, axis_label=shapers_mask_str_plot,  x_ticks=[], log=False,
                filename=fn, label_y='Max queue size', label_x='Burstiness $(\\beta)$', title='n={}_{}'.format(context,n), y_lim=[0, 45], step=True, mark=['o', '^', 'x', '+'])


            # fn = base_dir + 'post/beta_vs_link_utilization_n_{}.pdf'.format(n)
            # plot_bars(x, fx_list_util, axis_label=shapers_mask_str_plot,  x_ticks=beta_mask_str, log=False,
            #           filename=fn, label_y='Link utilization', label_x='Burstiness (beta)', title='n={}'.format(n), y_lim=[0,1])
            fn = base_dir + 'post/{}_beta_vs_link_utilization_n_{}.pdf'.format(context, n)
            plot_multiple_lines(beta_mask, fx_list_util, axis_label=shapers_mask_str_plot, log=False,
                filename=fn, label_y='Link utilization', label_x='Burstiness $(\\beta)$', title='n={}_{}'.format(context,n).format(n), y_lim=[0,1], mark=['o', '^', 'x', '+'])


        # plot_multiple_lines(x, fx_list, axis_label=shapers_mask,  x_ticks=beta_mask, log=use_log_scale,
        #           filename=fn, label_y=label_y, label_x='Burstiness (beta)', title='n={}'.format(n))

    plt.show()
