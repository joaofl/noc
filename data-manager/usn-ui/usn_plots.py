# Copyright (c) 2015 João Loureiro
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
# Author: João Loureiro <joflo@isep.ipp.pt>

__author__ = 'João Loureiro <joflo@isep.ipp.pt>'


import os
import numpy as np
import pylab
import matplotlib.pyplot as plt
from itertools import cycle


def bar(data, filename="0", show=False, title = "", label_x = "", label_y = "", legends='', x_size = 6.5, y_size = 3.1, x_lim = [] , y_lim = []):

    w = 0.9
    n_plots = 3

    pylab.figure(title, figsize=(x_size, y_size), dpi=120, facecolor='w', edgecolor='w')
    ticks = []
    i = 0
    for d in data:
        axes = plt.bar(d[0]-w/2, float(d[1]), w/n_plots, facecolor='blue')
        axes = plt.bar(d[0]-w/2 + w/n_plots, float(d[2]), w/n_plots, facecolor='green')
        axes = plt.bar(d[0]-w/2 + 2* w/n_plots, float(d[3]), w/n_plots, facecolor='red')
        # ya = axes.get_yaxis()
        # ya.set_major_locator(MaxNLocator(integer=True))
        # ticks.append("")
        # ticks.append(d[0])
        i+=1


    axes = plt.bar(0,0,0, facecolor='blue', label=legends[0])
    axes = plt.bar(d[0]-w/2 + w/n_plots, float(d[2]), w/n_plots, facecolor='green', label=legends[1])
    axes = plt.bar(d[0]-w/2 + 2* w/n_plots, float(d[3]), w/n_plots, facecolor='red', label=legends[2])


    pylab.xlabel(label_x)
    pylab.ylabel(label_y)
    pylab.xlim([-0.2, 4.9])
    # pylab.title(title)
    pylab.legend(loc=1, fontsize=10)
    pylab.grid(True)
    # Tweak spacing to prevent clipping of ylabel
    # pylab.subplots_adjust(left=0.15)

    pylab.gca().get_yaxis().get_major_formatter().set_powerlimits((0, 0))
    ax = pylab.gca()
    # ax.set_xticklabels(ticks)
    # for label in ax.get_xticklabels() + ax.get_yticklabels():
    #     label.set_fontsize(18)

    pylab.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)

    if filename!='0':
        dir = filename[:filename.rfind("/")]
        if not os.path.exists(dir):
            os.makedirs(dir)
        pylab.savefig(filename)
        print("Plot saved in " + filename)
    if show == True:
        plt.show()

    plt.show()


def histogram(data, filename='0', title = "", lable_x = "", lable_y = "", x_size = 9, y_size = 5, bins = 30, show=False, bin_width=0):

    values = []
    offset = data[0][1] #take the initial time
    r = 1e6
    x_down = data[0][1]/r
    x_up = data[-1][1]/r

    # offset = 0
    for line in data:
        values.append(int(np.round((line[1] - offset) / r)))

    bin_width = int(bin_width/r)

    bins=range(min(values), max(values) + 2*bin_width, bin_width)


    # example data
    mu = np.mean(values) # mean of distribution
    sigma = np.std(values) # standard deviation of distribution


    # the histogram of the data
    pylab.figure(title, figsize=(x_size, y_size), dpi=120, facecolor='w', edgecolor='w')
    n, bins, patches = plt.hist(values, bins, facecolor='blue', alpha=0.9) #, histtype='step')
    #normed=1, facecolor='blue', alpha=0.9, cumulative=True, histtype='step'
    # add a 'best fit' line
    # y = mlab.normpdf(bins, mu, sigma)
    # plt.plot(bins, y, 'r--')
    plt.xlabel(lable_x, fontsize=20)
    plt.ylabel(lable_y, fontsize=20)
    # pylab.xlim([x_up, x_down])
    # pylab.ylim([0,400])
    plt.title(title)

    # Tweak spacing to prevent clipping of ylabel
    plt.subplots_adjust(left=0.15)

    ax = pylab.gca()
    # ax.set_xticklabels([0,0,2,4,6,8,10])
    for label in ax.get_xticklabels() + ax.get_yticklabels():
      label.set_fontsize(18)

    pylab.tight_layout()

    if filename!='0':
        dir = filename[:filename.rfind("/")]
        if not os.path.exists(dir):
            os.makedirs(dir)
        pylab.savefig(filename)
    if show == True:
        plt.show()

def histogram_multiple(data, filename='0', title = "", lable_x = "", lable_y = "", legends='', x_size = 6.5, y_size = 3.1, bins = 30, show=False, bin_width=0):

    value = []
    values = []
    offset = data[0][0][1] #take the initial time
    r = 1e6
    lines = []

    # offset = 0
    i=2
    for list in data:
        for line in list:
            value.append(int(np.round((line[1] - offset) / r)))

        values.append(value)
        value = []
        line = (0, (i, i/2)) #definition of a linestyle. dash length and interspace length
        lines.append( line )
        i += 4


    linecycler = cycle(lines)

    bin_width = int(bin_width/r)
    bins=range(min(values[0]), max(values[0]) + 2*bin_width, bin_width)


    # the histogram of the data
    pylab.figure(figsize=(x_size, y_size), dpi=120, facecolor='w', edgecolor='w')
    n, bins, patches = pylab.hist(values, bins, facecolor='blue', alpha=0.9, histtype='step', label=legends)

    for patch in patches:
        patch[0]._linestyle = next(linecycler)

    pylab.xlabel(lable_x)
    pylab.ylabel(lable_y)
    # pylab.xlabel(lable_x)
    # pylab.ylabel(lable_y)
    # pylab.xlim([x_up, x_down])
    pylab.ylim([0,1050])
    # pylab.title(title)
    pylab.legend(loc=2, fontsize=10)
    pylab.grid(True)
    # Tweak spacing to prevent clipping of ylabel
    # pylab.subplots_adjust(left=0.15)

    ax = pylab.gca()
    # ax.set_xticklabels([0,0,2,4,6,8,10])
    # for label in ax.get_xticklabels() + ax.get_yticklabels():
    #   label.set_fontsize(18)

    pylab.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)

    if filename!='0':
        dir = filename[:filename.rfind("/")]
        if not os.path.exists(dir):
            os.makedirs(dir)
        pylab.savefig(filename)
        print("Plot saved in " + filename)
    if show == True:
        plt.show()

# def histogram

def matrix(data, filename="0", show=False, title = "", lable_x = "", lable_y = "", x_size = 3.5, y_size = 3.5):

    pylab.figure(title, figsize=(x_size, y_size), dpi=120, facecolor='w', edgecolor='w')
    plt.imshow(data, cmap=pylab.get_cmap('hot_r'), interpolation='nearest')

    plt.xlabel(lable_x)
    plt.ylabel(lable_y)

    pylab.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)

    if filename!='0':
        dir = filename[:filename.rfind("/")]
        if not os.path.exists(dir):
            os.makedirs(dir)
        pylab.savefig(filename)

    # plt.ion()
    # plt.show()

    elif show==True:
        plt.show()

def plotxy(data, filename="0", show=False, title = "", label_x = "", label_y = "", x_size = 6.5, y_size = 3.1, x_lim = [] , y_lim = [], logscale=True, marker=[], step=True):

    s = len(data)
    x = [(row[0]) for row in data]
    y = [(row[1]) for row in data]

    fig, ax1 = plt.subplots(figsize=(x_size, y_size), dpi=120, facecolor='w', edgecolor='w')
    # pylab.figure(fig)
    # pylab.figure(figsize=(x_size, y_size), dpi=120, facecolor='w', edgecolor='w')

    # t = np.arange(0.01, 10.0, 0.01)
    # s1 = np.exp(t)

    if step:
        ax1.step(x, y, 'g', where='post', lw=2)
    else:
        ax1.plot(x,y, '-o')
    ax1.set_xlabel(label_x)
    if x_lim != []: pylab.xlim(x_lim)
    if y_lim != []: pylab.ylim(y_lim)

    if logscale == True:
        pylab.yscale('log')

    # Make the y-axis label and tick labels match the line color.
    ax1.set_ylabel(label_y, color='black')
    # for tl in ax1.get_yticklabels():
    #     tl.set_color('b')


    pylab.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
    pylab.grid(True)

    if filename != '0':
        dir = filename[:filename.rfind("/")]
        if not os.path.exists(dir):
            os.makedirs(dir)
        pylab.savefig(filename)

    if show == True:
        plt.show()
    # pylab.close()

def plotxy_multiple(data_list, filename="0", show=False, title = "", label_x = "", label_y = "",
                    x_size = 6.5, y_size = 3.1, x_lim = [] , y_lim = [], legend=[], marker=[],
                    plot_type='step', logscale=False, legend_position=1):

    lines = ['-' , '--' , '-.' , ':' , '' ]

    dots = ["D", "o", "x", "d"]
    dotcycler = cycle(dots)

    fig, ax1 = plt.subplots(figsize=(x_size, y_size), dpi=120, facecolor='w', edgecolor='w')

    if x_lim != []: pylab.xlim(x_lim)
    if y_lim != []: pylab.ylim(y_lim)

    l=2

    # for d in data_list:
    #     line = (0, (l, l/2))
    #     lines.append( line )
    #     l += 3

    linecycler = cycle(lines)

    i=0
    for data in data_list:
        x = [(row[0]) for row in data]
        y = [(row[1]) for row in data]

        if plot_type == 'plot':
            ax1.plot(x, y, next(dotcycler), lw=2, label=legend[i], linestyle='-')
        else:
            ax1.step(x, y, where='post', lw=2, label=legend[i], linestyle=next(linecycler))

        if logscale == True:
            pylab.yscale('log')
        i+=1


    ax1.set_xlabel(label_x)
    ax1.set_ylabel(label_y)

    pylab.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
    pylab.grid(True)
    pylab.legend(loc=legend_position, fontsize=10)

    if plot_type=='plot':
        pylab.locator_params(axis='x', nbins=len(x))

    if filename != '0':
        dir = filename[:filename.rfind("/")]
        if not os.path.exists(dir):
            os.makedirs(dir)
        pylab.savefig(filename)

    if show == True:
        plt.show()

    return 0

def plot2xy(data1, data2, filename="0", show=False, title = "", label_x = "", label_y1 = "", label_y2 = "",
            x_size = 6.5, y_size = 3.1, x_lim = [] , y_lim = [], logscale=False, legend=[], two_axis=False):

    #consider that x is the same for both, and extract frmo the first only

    lines = ["-","--","-.",":"]
    linecycler = cycle(lines)

    s = len(data1)
    x = [(row[0]) for row in data1]
    y1 = [(row[1]) for row in data1]
    y2 = [(row[1]) for row in data2]



    fig, ax1 = plt.subplots(figsize=(x_size, y_size), dpi=120, facecolor='w', edgecolor='w')
    # pylab.figure(fig)
    # pylab.figure(figsize=(x_size, y_size), dpi=120, facecolor='w', edgecolor='w')

    # t = np.arange(0.01, 10.0, 0.01)
    # s1 = np.exp(t)



    if logscale == True:
        pylab.yscale('log')

    ax1.plot(x, y1, 'o', linestyle=next(linecycler), label=legend[0])
    ax1.set_xlabel(label_x)
    # Make the y-axis label and tick labels match the line color.
    ax1.set_ylabel(label_y1)
    # for tl in ax1.get_yticklabels():
    #     tl.set_color('b')

    if x_lim != []: pylab.xlim(x_lim)
    if y_lim != []: pylab.ylim(y_lim)
    pylab.legend(loc=2, fontsize=11)

    if two_axis == True:
        ax2 = ax1.twinx()
    else:
        ax2 = ax1

    # s2 = np.sin(2*np.pi*t)
    ax2.plot(x, y2, 'Dr', linestyle=next(linecycler), label=legend[1])
    ax2.set_ylabel(label_y2)
    # ax2.set_ylabel(label_y2, color='g')
    # for tl in ax2.get_yticklabels():
    #     tl.set_color('r')

    if x_lim != []: pylab.xlim(x_lim)
    if y_lim != []: pylab.ylim(y_lim)

    pylab.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
    pylab.legend(loc=1, fontsize=11)
    pylab.grid(True)

    # ax = pylab.gca()
    # ax.set_xticklabels(x)

    pylab.locator_params(axis='x', nbins=len(x))

    if filename != '0':
        dir = filename[:filename.rfind("/")]
        if not os.path.exists(dir):
            os.makedirs(dir)
        pylab.savefig(filename)
        print("Plot saved in " + filename)

    if show == True:
        plt.show()
    # pylab.close()

# def plot2(self,  data1, data2, filename='', title = "", label_x = "", label_y1 = "", label_y2 = "", x_size = 7, y_size = 3):
#
#     s = len(data1)
#     x = np.linspace(1, s, s)
#     fig, ax1 = plt.subplots(figsize=(x_size, y_size), dpi=120, facecolor='w', edgecolor='w')
#     # pylab.figure(fig)
#     # pylab.figure(figsize=(x_size, y_size), dpi=120, facecolor='w', edgecolor='w')
#
#     # t = np.arange(0.01, 10.0, 0.01)
#     # s1 = np.exp(t)
#     ax1.plot(x, data1, 'yo-')
#     ax1.set_xlabel(label_x)
#     # Make the y-axis label and tick labels match the line color.
#     ax1.set_ylabel(label_y1, color='y')
#     # for tl in ax1.get_yticklabels():
#     #     tl.set_color('b')
#
#     ax2 = ax1.twinx()
#     # s2 = np.sin(2*np.pi*t)
#     ax2.plot(x, data2, 'gx-')
#     ax2.set_ylabel(label_y2, color='g')
#     # for tl in ax2.get_yticklabels():
#     #     tl.set_color('r')
#
#     pylab.tight_layout(pad=2, w_pad=1, h_pad=1)
#
#     if filename != '':
#         pylab.savefig(filename)
#
#     plt.show()
#     pylab.close()
#
#
#
#     return