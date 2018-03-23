#!/usr/bin/env python3

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

import sys, os, traceback, optparse, time, cProfile
import files_io
import packet_structure as trace
import scipy as sp
import numpy as np

import matplotlib.pyplot as plt

from PyQt5.QtWidgets import * #QWidget, QProgressBar,QPushButton, QApplication, QLabel, QCheckBox
from PyQt5.QtGui import *
from PyQt5.QtCore import QTimer, Qt, QRectF

import packet_structure as HEADER
import network_viewer
from node_graphics import Node



class NOCAnim(QWidget):

    def __init__(self):
        super().__init__()

        self.viewers = []

        self.initUI()
        # self.initDataFirstRun()
        self.initData()

    def initUI(self):

        self.timer = QTimer(self)
        self.timer.timeout.connect(self.timerEvent)

        self.btn = QPushButton('Start', self)
        self.btn.clicked.connect(self.doActionStartStop)

        self.cbStep = QCheckBox('Step', self)
        self.cbLoop = QCheckBox('Loop', self)

        self.btn2 = QPushButton('Reload', self)
        self.btn2.clicked.connect(self.doActionReload)

        self.btnView = QPushButton('Open view', self)
        self.btnView.clicked.connect(self.doActionOpenView)

        self.pbar = QProgressBar(self)

        self.tb_time_tts = QLabel('0 tts   ', self)
        self.tb_time_tts.setFont(QFont("Monospace", 12, QFont.Bold))

        self.tb_time_ns = QLabel('0 ns    ', self)
        self.tb_time_ns.setFont(QFont("Monospace", 12, QFont.Bold))

        # self.tb_refresh_label = QLabel('Refresh', self)
        # self.tb_refresh_label.setFont(QFont("Monospace", 10, QFont.Bold))

        self.tb_refresh = QLabel('50ms', self)
        self.tb_refresh.setFont(QFont("Monospace", 10, QFont.Bold))

        self.refresh_slider = QSlider(Qt.Horizontal)
        self.refresh_slider.setRange(1,2000)
        self.refresh_slider.setTickInterval(10)
        self.refresh_slider.valueChanged.connect(self.doChangeRefreshRate)

        hbox_l1 = QHBoxLayout()
        hbox_l1.addWidget(self.btn)
        hbox_l1.addWidget(self.cbStep)
        hbox_l1.addWidget(self.cbLoop)
        hbox_l1.addWidget(self.btn2)
        hbox_l1.addWidget(self.tb_refresh)
        hbox_l1.addWidget(self.refresh_slider)
        hbox_l1.addStretch(1)
        hbox_l1.addWidget(self.btnView)

        hbox_l2 = QHBoxLayout()
        hbox_l2.addWidget(self.pbar)
        hbox_l2.addWidget(self.tb_time_ns)
        hbox_l2.addWidget(self.tb_time_tts)

        vbox = QVBoxLayout()
        vbox.addLayout(hbox_l1)
        vbox.addLayout(hbox_l2)

        self.setLayout(vbox)
        self.setMinimumWidth(800)
        # self.setGeometry(200, 100, 1200, 900)

        self.setWindowTitle('NoC Anim')
        self.show()

    def initData(self):

        global options, args

        self.packetTrace = files_io.load_list(options.inputdir + '/packets-trace-netdevice.csv')
        if (len(self.packetTrace) == 0):
            print ('No input file provided')
            return -1

        self.networkSize = [int(options.size_x), int(options.size_y)]
        self.network = [[Node] * self.networkSize[0] for i in range(self.networkSize[1])]

        ps = int(options.packet_size)
        br = int(options.baudrate)
        self.packetDuration = ( ps / br ) * 1e9

        self.nextT = 0
        self.t_next = 0
        self.previous_index = 0

        self.refresh_slider.setValue(1)
        self.doChangeRefreshRate()

    def doChangeRefreshRate(self):
        self.anim_refresh_rate = self.refresh_slider.value()
        self.tb_refresh.setText(str(self.anim_refresh_rate / 1) + ' ms')
        self.timer.setInterval(self.anim_refresh_rate)
        # self.graphics.setma

    def doActionStartStop(self):
        if self.timer.isActive():
            self.timer.stop()
            self.btn.setText('Start')
        else:
            self.timer.start(self.anim_refresh_rate)
            self.btn.setText('Stop')


    def doActionReload(self):
        self.timer.stop()
        self.btn.setText('Start')
        self.nextT = 0
        self.t_next = 0
        self.previous_index = 0
        self.pbar.setValue(0)
        self.initData()

        # self.anim_aux.resetNetwork()
        # self.anim_aux.scene.update()
        [_.resetNetwork() for _ in self.viewers]
        [_.scene.update() for _ in self.viewers]

    def doActionOpenView(self):
        # self.viewers = []
        v = network_viewer.SensorAnim(self.networkSize)
        v.actionCallback = self.clickCallback

        self.viewers.append(v)
        self.viewers[-1].show()




    def clickCallback(self, x, y, port):
        # print(' do smt')
        #
        # script = '/home/joao/Repositorios/ns-3-dev/src/noc/data-manager/show_node_analysis.py'
        # args = ' --inputdir=' + options.inputdir + ' --outputdir=' + options.outputdir + \
        #        ' --pos_x=' + str(x) + ' --pos_y=' + str(y) + ' --port=' + str(port) + ' ' + \
        #        ' --size_x=' + options.size_x + ' --size_y=' + options.size_y
        #
        #
        # cmd = 'python3.5 ' + script + args
        # print('Command executed: ' + cmd)
        #
        # # launch_thread = True
        #
        # # if (launch_thread == True):
        #     # _thread.start_new_thread(os.system, (cmd,))
        # # else:
        # os.system(cmd)


        ################# Get trace_packets from simulation logs ##################
        x_arrival = [0]
        y_arrival = [0]

        x_departure = [0]
        y_departure = [0]

        count_r = 0;
        count_t = 0;

        protocol = 1

        for line in self.packetTrace:
            log_x = int(line[HEADER.x_absolute])
            log_y = int(line[HEADER.y_absolute])
            log_protocol = int(line[HEADER.app_protocol])
            log_t = float(line[HEADER.time]) / self.packetDuration

            if log_x == x and log_y == y and log_protocol == protocol:
                ################# Received ####################
                # Build the matrix for the density map
                if line[HEADER.operation] == 'r' or line[HEADER.operation] == 'g':
                    # Build the cumulative arrival for an specific node
                    count_r += 1
                    x_arrival.append(log_t)
                    y_arrival.append(count_r)

                ################# Transmitted ####################
                elif line[HEADER.operation] == 't':  # or line[trace.operation] == 'g':
                    # Build the cumulative departure curve
                    count_t += 1
                    x_departure.append(log_t + 1)
                    y_departure.append(count_t)

        # print(x_arrival, y_arrival, x_departure, y_departure)
        # return x_arrival, y_arrival, x_departure, y_departure

        self.plot(x_arrival, y_arrival, x_departure, y_departure, x, y)

    def plot(sellf, t_in, n_in, t_out, n_out, x, y):

        ######################## Plot #########################
        fig, ax_main = plt.subplots(figsize=(3.6, 3), dpi=110, facecolor='w', edgecolor='w')

        # ax_main.step(t_queue, n_queue, '-', where='post', label='Queue')
        # ax_main.step(t_delay, n_delay, '-', where='post', label='Delay')

        ax_main.step(t_in, n_in, '-', color='green', where='post', label='SIM')
        # ax_main.plot(t_math_in, n_math_in, '--', color='blue', label=model)

        ax_main.step(t_out, n_out, '-', color='lightgreen', where='post', label='')
        # ax_main.plot(t_math_out, n_math_out, '--', color='blue', label='')


        ax_main.set_xlabel("Transmission time slot (TTS)")
        ax_main.set_ylabel("Cumulative packet count")
        # ax_main.set_xlim([3.5,31])

        # plt.xlim([-1, 32])

        plt.tight_layout(pad=0.5, w_pad=0.3, h_pad=0.3)
        plt.legend(loc=4, fontsize=11)
        # plt.legend(loc=0, prop={'size': 6})
        plt.grid(False)

        filename = options.outputdir + "sim-arrival-departure-node({},{}).pdf".format(x,y)
        plt.savefig(filename)
        plt.show()
        print("Plot saved in " + filename)

    def timerEvent(self):

        lastT = int (self.packetTrace[-1][trace.time])

        # self.anim_aux.resetPorts()
        [_.resetPorts() for _ in self.viewers]

        previous_index = self.previous_index


        for i in range(previous_index, len(self.packetTrace)):
            self.previous_index = i
            current_trans = self.packetTrace[i]

            if int(current_trans[trace.time]) <= self.t_next:
                x = int(current_trans[trace.x_absolute])
                y = int(current_trans[trace.y_absolute])

                if current_trans[trace.operation] == 's': #Data sensed at the source
                    v = int(current_trans[trace.sensor_value])
                    # [viewer.setNode(x, y, 'sensor', v) for viewer in self.viewers]
                    continue
                elif current_trans[trace.operation] == 'S': #Sensed data as calculated by the sink
                    v = int(current_trans[trace.sensor_value])
                    x_origin = int(current_trans[trace.sensor_value + 1])
                    y_origin = int(current_trans[trace.sensor_value + 2])
                    [viewer.setNode(x_origin, y_origin, 'sensor', v) for viewer in self.viewers]

                    data_matrix[(x_origin, y_origin)] = v
                    continue

                elif current_trans[trace.operation] == 'c':
                    [viewer.setNode(x, y, 'core_rx',  1) for viewer in self.viewers]
                elif current_trans[trace.operation] == 'g':
                    [viewer.setNode(x, y, 'core_tx',  1) for viewer in self.viewers]

                elif current_trans[trace.operation] == 'r':
                    if int(current_trans[trace.direction]) == trace.DIRECTION_N:
                        [viewer.setNode(x, y, 'north_rx', 1) for viewer in self.viewers]
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_S:
                        [viewer.setNode(x, y, 'south_rx', 1) for viewer in self.viewers]
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_E:
                        [viewer.setNode(x, y, 'east_rx',  1) for viewer in self.viewers]
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_W:
                        [viewer.setNode(x, y, 'west_rx',  1) for viewer in self.viewers]

                    # [viewer.setNode(x, y, text=current_trans[trace.queue_size]) for viewer in self.viewers]

                elif current_trans[trace.operation] == 't':
                    if int(current_trans[trace.direction]) == trace.DIRECTION_N:
                        [viewer.setNode(x, y, 'north_tx',  1) for viewer in self.viewers]
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_S:
                        [viewer.setNode(x, y, 'south_tx',  1) for viewer in self.viewers]
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_E:
                        [viewer.setNode(x, y, 'east_tx',  1) for viewer in self.viewers]
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_W:
                        [viewer.setNode(x, y, 'west_tx',  1) for viewer in self.viewers]

                # if current_trans[trace.app_protocol] == '6':
                #     [viewer.setNode(x, y, led = 1) for viewer in self.viewers]


            else:
                self.t_next = int(current_trans[trace.time]) + self.packetDuration * 0.1
                break


        t = int (self.packetTrace[self.previous_index - 1][trace.time])
        self.pbar.setValue((t / lastT) * 100)

        tts = round(t / self.packetDuration, 2)
        self.tb_time_tts.setText('{0:.2f}tts'.format(tts))

        time = round(t / 1000, 2)
        self.tb_time_ns.setText('{0:.2f}µs'.format(time))

        if self.previous_index == len(self.packetTrace) - 1:
            if self.cbLoop.isChecked():
                self.doActionReload()
                self.doActionStartStop()
            else:
                self.doActionStartStop()

        if self.cbStep.isChecked():
            self.doActionStartStop()

        # self.graphics.update()
        # self.anim_aux.scene.update()

        plot_data_profile()

        [v.scene.update() for v in self.viewers]
        #self.update()

##############################################################

data_matrix = {}
plot_enabled = False

if plot_enabled == True:
    plt.ion()
    ax = plt.gca()
    ax.set_autoscale_on(True)

    p1, = ax.plot([], [], marker= '', label='Top')
    p2, = ax.plot([], [], marker= '', label='Bottom')
    ax.legend(loc='best', frameon=True, prop={'size':12})
    ax.set_xlabel('Wing position (m)')
    ax.set_ylabel('Pressure (kPa)')
    # p2, = plt.subplot([], [])
    plt.show()

def plot_data_profile():
    import scipy as sp
    from scipy.interpolate import interp1d

    lines = []
    values = []

    size_x = int(options.size_x)
    size_y = int(options.size_y)

    wing_size = 1.1963 #meters
    sensors_interspace = (wing_size*2)/size_x

    sink_x = 20

    for y in range(0,size_y):
        line_top = []
        value_top = []
        line_bottom = []
        value_bottom = []

        for x in range(0, size_x): # get the top only (righ only +0 if sink is centered)
            v = data_matrix.get( (x, y) )
            if v != None:
                if x >= sink_x:
                    line_top.append(x - sink_x)
                    value_top.append(v/1000)
                if x <= sink_x:
                    line_bottom.append(-(x - sink_x))
                    value_bottom.append(v/1000)

        lines.append(line_top)
        values.append(value_top)

        lines.append(line_bottom)
        values.append(value_bottom)

    if len(lines[0]) > 4 and len(values[0]) > 4:
        #Interpolate here
        x = lines[0]
        y = values[0]

        func = sp.interpolate.interp1d(x, y, kind="cubic")

        x_vals = np.linspace(0, len(y) - 1, len(y) * 100, endpoint=False)
        y_vals = func(x_vals)

        x_vals = [val * sensors_interspace for val in x_vals]

        #Plot here
        p1.set_xdata(x_vals)
        p1.set_ydata(y_vals)


        x = lines[1]
        y = values[1]

        func = sp.interpolate.interp1d(x, y, kind="cubic")

        x_vals = np.linspace(0, len(y) - 1, len(y) * 100, endpoint=False)
        y_vals = func(x_vals)

        x_vals = [val * sensors_interspace for val in x_vals]

        #Plot here
        p2.set_xdata(x_vals)
        p2.set_ydata(y_vals)

        ax.relim()
        ax.autoscale_view(True, True, True)
        plt.draw()
        plt.pause(0.0001)


class my_optparse(optparse.OptionParser):
    def error(self, msg):
        print("Error parsing some of the input arguments:")
        print(msg)
        pass


if __name__ == '__main__':

    try:
        start_time = time.time()
        parser = my_optparse(formatter=optparse.TitledHelpFormatter(), usage=globals()['__doc__'], version='$Id$')

        parser.add_option('--verbose', action='store_true', default=False, help='verbose output')
        parser.add_option('--inputdir', help='Dir containing the logs', default=None)
        parser.add_option('--outputdir', help='', default=None)
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
        parser.add_option('--packet_size', help='packet_size in bits', default=16 * 10)
        parser.add_option('--size_neighborhood', help='neighborhood size', default=0)


        (options, args) = parser.parse_args()
        #if len(args) < 1:
        #    parser.error ('missing argument')
        if options.verbose: print (time.asctime())

        app = QApplication(sys.argv)
        app.setWindowIcon(QIcon('others/grid2.png'))
        ex = NOCAnim()

        if options.verbose: print (time.asctime())
        if options.verbose: print ('Total execution time (s):')
        if options.verbose: print (time.time() - start_time)
        # sys.exit(0)

        r = app.exec_()
        sys.exit(r)
    # except (KeyboardInterrupt, e): # Ctrl-C
    #     raise e
    # except SystemExit, e: # sys.exit()
    #     raise e

    except (Exception):
        print ('ERROR, UNEXPECTED EXCEPTION')
        print (str(Exception))
        traceback.print_exc()
        os._exit(1)

