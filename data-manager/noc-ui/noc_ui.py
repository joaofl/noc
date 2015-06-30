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

import csv
import sys
import os
import numpy as np
import cv2
import thread
from threading import Thread
from numpy.lib.npyio import savetxt
import pylab

from copy import copy, deepcopy

import Image
from PIL import Image

import matplotlib
import matplotlib.pyplot as plt

import matplotlib.mlab as mlab
import matplotlib.animation as animation

import noc_plots
import noc_io
import noc_calc

from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt4agg import NavigationToolbar2QTAgg as NavigationToolbar
from matplotlib.figure import Figure

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import uic, QtGui

from joblib import Parallel, delayed
import multiprocessing

from xml.etree import ElementTree as et

from subprocess import Popen, PIPE, call

from itertools import cycle




i = 0
j = 0
k = 0

form_class = uic.loadUiType("usn_ui.ui")[0]  # Load the UI


class MyWindowClass(QtGui.QWidget, form_class):
    def __init__(self, parent=None):
        QtGui.QMainWindow.__init__(self, parent)
        self.setWindowTitle('uSN Time Analysis Tool')
        self.setupUi(self)

        self.pbStart.clicked.connect(self.pbStart_clicked)
        self.pbStop.clicked.connect(self.pbStop_clicked)
        self.pbBoxPlot.clicked.connect(self.pbBoxPlot_clicked)
        self.pbPlotPDF.clicked.connect(self.pbPlotPDF_clicked)
        # self.pbLoadFiles.clicked.connect(self.pbLoadFiles_clicked)
        # self.pbExport.clicked.connect(self.pbExport_clicked)
        self.pbExtractFlows.clicked.connect(self.pbExtractFlows_clicked)
        self.pbGenerate.clicked.connect(self.pbGenerate_clicked)
        self.pbSelectWorkingDir.clicked.connect(self.pbSelectWorkingDir_clicked)
        self.pbSelectNs3Dir.clicked.connect(self.pbSelectNs3Dir_clicked)
        self.pbRunSimulation.clicked.connect(self.pbRunSimulation_clicked)
        self.pbPlotPacketTrace.clicked.connect(self.pbPlotPacketTrace_clicked)
        self.pbStepPacketTrace.clicked.connect(self.pbStepPacketTrace_clicked)
        self.pbExportAsImage.clicked.connect(self.pbExportAsImage_clicked)
        self.pbActivityHistogram.clicked.connect(self.pbActivityHistogram_clicked)
        self.pbActivityHistogramMultiple.clicked.connect(self.pbActivityHistogramMultiple_clicked)
        self.pbIOCompare.clicked.connect(self.pbIOCompare_clicked)
        self.pbIOCompareError.clicked.connect(self.pbIOCompareError_clicked)
        self.pbTrhoughput.clicked.connect(self.pbThroughput_clicked)
        self.pbTrhoughputMultiple.clicked.connect(self.pbTrhoughputMultiple_clicked)
        self.pbQueueSizeOverTimeMultiple.clicked.connect(self.pbQueueSizeOverTimeMultiple_clicked)
        self.pbQueueSizeOverTime.clicked.connect(self.pbQueueSizeOverTime_clicked)
        self.pbNumberTransmissionsDS.clicked.connect(self.pbpbNumberTransmissionsDS_clicked)





        self.pbAddSensortsToE2dFile.clicked.connect(self.pbAddSensortsToE2dFile_clicked)
        self.pbAddDataToSimulation.clicked.connect(self.pbAddDataToSimulation_clicked)

        self.timer_gd = QTimer(self)
        QObject.connect(self.timer_gd, SIGNAL("timeout()"), self.animate_gd)

        self.timer_ta = QTimer(self)
        QObject.connect(self.timer_ta, SIGNAL("timeout()"), self.timer_ta_animate)

        self.load_config()
        self.create_frames()
        # self.read_files()
        # self.initial_plot_setup()

    def load_config(self):

        #definitions for reading the values from the packet trace
        global DEF_TIME, DEF_FLOW_ID, DEF_CORD_X, DEF_CORD_Y, DEF_PCK_VALUE, DEF_PCK_CORD_X, DEF_PCK_CORD_Y

        DEF_TIME = 1
        DEF_FLOW_ID = 2
        DEF_CORD_X = 5
        DEF_CORD_Y = 6
        DEF_PCK_VALUE = 11
        DEF_PCK_CORD_X = 12
        DEF_PCK_CORD_Y = 13

        self.viewed = 0
        # self.output_figures_context = "-tests.eps"

        home = os.path.expanduser("~")
        working_dir_config = home + "/usn-data/config/"
        self.working_base_dir = home + "/usn-data/"

        file_name_cf = "input-config.c"  #c stands for configuration data
        file_extension = ".csv"
        full_path_cf = working_dir_config + file_name_cf + file_extension

        try:
            loaded_parameters = np.array(list(csv.reader(open(full_path_cf, "rb"), delimiter=' ')))
        except:
            QtGui.QMessageBox.warning(self, 'Error',
                  "A valid config file was not found at " + full_path_cf +
                  "\nGenerate a new config file to fix this issue",
                  QtGui.QMessageBox.Ok)
            return


        for i in range(loaded_parameters.size):
            parameter = loaded_parameters[0][i]

            try:
                p_name = parameter.split("=")[0]
                p_value = parameter.split("=")[1]
            except:
                continue

            if p_name == "--size_x":
                self.network_size_x = int(p_value)
                self.sbNetworkSizeX.setValue(int(p_value))
            elif p_name == "--size_y":
                self.network_size_y = int(p_value)
                self.sbNetworkSizeY.setValue(int(p_value))
            elif p_name == "--size_neighborhood":
                self.neighborhood_size = int(p_value)
                self.sbNeighborhoodSize.setValue(int(p_value))
            elif p_name == "--sinks_n":
                self.number_of_sinks = int(p_value)
                self.sbNumberofSinks.setValue(int(p_value))
            elif p_name == "--sampling_cycles":
                self.number_of_cycles = int(p_value)
                self.sbNumbeofCycles.setValue(int(p_value))
            elif p_name == "--sampling_period":
                self.period = int(p_value)
                self.sbPeriod.setValue(int(p_value))
            elif p_name == "--baudrate":
                self.baudrate = int(p_value)
                self.sbBaudrate.setValue(int(p_value))
            elif p_name == "--io_data_dir":
                self.working_dir = p_value
                self.working_context = "-" + self.working_dir.split('/')[4]
                self.tbWorkingDir.setText(p_value)


    def create_frames(self):
        # setup the plot area space for the time analysis
        self.frame_ta = self.wgPlot
        self.fig_ta = Figure()  #Figure((5.0, 4.0), dpi=100)
        self.canvas_ta = FigureCanvas(self.fig_ta)  #canvas for time_analysis
        self.canvas_ta.setParent(self.frame_ta)
        self.canvas_ta.setFocusPolicy(Qt.StrongFocus)
        self.canvas_ta.setFocus()

        self.mpl_toolbar_ta = NavigationToolbar(self.canvas_ta, self.frame_ta)
        vbox_ta = QtGui.QVBoxLayout()
        vbox_ta.addWidget(self.canvas_ta)
        vbox_ta.addWidget(self.mpl_toolbar_ta)
        self.frame_ta.setLayout(vbox_ta)


        # setup the plot area space for the data generator
        self.frame_gd = self.wgPlotGeneratedData  #frame for generating data on
        self.fig_gd = Figure()  #Figure((5.0, 4.0), dpi=100)
        self.canvas_gd = FigureCanvas(self.fig_gd)
        self.canvas_gd.setParent(self.frame_gd)
        self.canvas_gd.setFocusPolicy(Qt.StrongFocus)
        self.canvas_gd.setFocus()

        self.mpl_toolbar_gd = NavigationToolbar(self.canvas_gd, self.frame_gd)
        vbox_gd = QtGui.QVBoxLayout()
        vbox_gd.addWidget(self.canvas_gd)
        vbox_gd.addWidget(self.mpl_toolbar_gd)
        self.frame_gd.setLayout(vbox_gd)

        # setup the plot area space for the packet tracer
        self.frame_pt = self.wgPlotPacketTrace  #frame for generating data on
        self.fig_pt = Figure()  #Figure((5.0, 4.0), dpi=100)
        self.canvas_pt = FigureCanvas(self.fig_pt)
        self.canvas_pt.setParent(self.frame_pt)
        self.canvas_pt.setFocusPolicy(Qt.StrongFocus)
        self.canvas_pt.setFocus()

        self.mpl_toolbar_pt = NavigationToolbar(self.canvas_pt, self.frame_pt)
        vbox_pt = QtGui.QVBoxLayout()
        vbox_pt.addWidget(self.canvas_pt)
        vbox_pt.addWidget(self.mpl_toolbar_pt)
        self.frame_pt.setLayout(vbox_pt)

    def initial_plot_setup(self):
        #################### setup the initial plots ####################

        self.flows_transmission_time_updated = 0

        self.data = np.zeros((self.network_size_x, self.network_size_y))  #for initialization only

        self.data1 = np.zeros((self.network_size_x, self.network_size_y))  #the correspondent data of each subplot
        self.data2 = np.zeros((self.network_size_x, self.network_size_y))
        self.data3 = np.zeros((self.network_size_x, self.network_size_y))
        self.data4 = np.zeros((self.network_size_x, self.network_size_y))

        self.data1 = self.input_data[0]
        self.data2[0, 0] = 1

        self.max = np.max(self.data1)
        self.mean = self.max / 2
        self.data3 = np.ones((self.network_size_x, self.network_size_y)) * self.mean
        self.data3[0, 0] = self.max
        self.data3[0, 1] = 0

        self.data[0, 0] = 20  # this helps to set the plot ranges, which are automaticaly set at the first plot.
        #TODO: replace it to 'set range' functions instead

        self.fig_ta.clear()
        ### 1
        self.ax1 = self.fig_ta.add_subplot(2, 2, 1)
        self.ax1.axes.get_xaxis().set_visible(False)
        self.ax1.axes.get_yaxis().set_visible(True)
        self.ax1.set_title('Input Data')
        self.img1 = self.ax1.imshow(self.data1,cmap=pylab.get_cmap('hot_r'), interpolation='nearest')
        # self.img1 = plt.imshow(self.data1)

        ### 2
        self.ax2 = self.fig_ta.add_subplot(2, 2, 2)
        self.ax2.axes.get_xaxis().set_visible(False)
        self.ax2.axes.get_yaxis().set_visible(False)
        self.ax2.set_title('Events Detected')
        self.img2 = self.ax2.imshow(self.data2,cmap=pylab.get_cmap('hot_r'), interpolation='nearest')
        self.data2[0, 0] = 0
        # self.img2 = plt.imshow(self.data2)

        ## 3
        self.ax3 = self.fig_ta.add_subplot(2, 2, 3)
        self.ax3.axes.get_xaxis().set_visible(True)
        self.ax3.axes.get_yaxis().set_visible(True)
        self.ax3.set_title('Reconstructed Data')
        self.img3 = self.ax3.imshow(self.data3,cmap=pylab.get_cmap('hot_r'), interpolation='nearest')
        # self.img3 = plt.imshow(self.data3)

        ## 4
        self.ax4 = self.fig_ta.add_subplot(2, 2, 4)
        self.ax4.axes.get_xaxis().set_visible(True)
        self.ax4.axes.get_yaxis().set_visible(False)
        self.ax4.set_title('Transmissions Done')
        self.img4 = self.ax4.imshow(self.data,cmap=pylab.get_cmap('hot_r'), interpolation='nearest')
        # self.img44 = plt.imshow(self.data)


        self.canvas_ta.draw()

    def read_files(self):
        ###############################################
        # Addresses

        working_dir = self.tbWorkingDir.text()

        file_extension = ".csv"

        #fn - file name
        file_name_pck_trace = working_dir + "/out/packets-trace" + file_extension
        file_name_pck_trace_sink = working_dir + "/out/packets-sink-trace" + file_extension
        file_name_data_input = working_dir + "/in/input-data.s" + file_extension
        # file_name_config_input = working_dir + "in/input-config.c" + file_extension
        # output_filename = working_dir + "usn-time-simulation" + video_ext

        ###############################################
        # Var Init

        self.sink_x = int(self.network_size_x / 2)
        self.sink_y = int(self.network_size_y / 2)
        # self.max_packets = 100

        t_unit = int(1E9)  #nano seconds

        self.t_initial = int(1 * t_unit)
        self.t_final = int(10 * t_unit)  # in seconds ??
        self.t_step = int(.001 * t_unit)
        self.t_simulation_start = 1.1 * t_unit
        self.t_time_window = .05 * t_unit

        self.times = np.arange(self.t_initial, self.t_final, self.t_step)
        self.t_plot = 0

        self.input_data = []
        self.input_data_time = []
        self.flows_known = []
        self.flows_transmission_time = []
        input_data_single_frame = []
        ################################################


        #################### Reading the trace and data files ###################
        #read from the 3 files:

        #1st - Packets Trace
        self.pck_trace = np.array(list(csv.reader(open(file_name_pck_trace, "rb"), delimiter=',')))
        #2nd - Sink Packets Trace
        self.pck_trace_sink = np.array(list(csv.reader(open(file_name_pck_trace_sink, "rb"), delimiter=',')))


        #3rd - Input data
        i = 0
        j = 0
        k = 0

        for line in open(file_name_data_input):
            if line.startswith('@'):
                j += 1
                # i = 0
                # self.input_data_time.append(int(line.replace('@', '')))
                # self.input_data.append(zip(*np.array(input_data_single_frame)))
                self.input_data.append(np.array(input_data_single_frame))
                input_data_single_frame = []
            else:
                input_data_single_frame.append(np.array(line.strip().split(',')).astype('int'))

        self.input_data_time = np.linspace(0, self.number_of_cycles * self.period * 1000, self.number_of_cycles + 1)
        #TODO: this better be in the data file

    def pbSelectWorkingDir_clicked(self):
        dir = QFileDialog.getExistingDirectory(self, ("Open Directory"),
                                               self.tbWorkingDir.text(),
                                               QFileDialog.ShowDirsOnly | QFileDialog.DontResolveSymlinks);
        self.tbWorkingDir.setText(dir)

    def pbSelectNs3Dir_clicked(self):
        dir = QFileDialog.getExistingDirectory(self, ("Open Directory"),
                                               self.tbNs3Dir.text(),
                                               QFileDialog.ShowDirsOnly | QFileDialog.DontResolveSymlinks);
        self.tbNs3Dir.setText(dir)

    def pbRunSimulation_clicked(self):
        #self.pbGenerate_clicked()
        #TODO: replace the fixed string with the appropriated var
        self.teCommand.setText('Starting simulation. Please be patient')
        cmd = './waf --run "src/usn/examples/usn-example $(cat /home/joao/usn-data/config/input-config.c.csv)"'
        cmd_dir = self.tbNs3Dir.text()

        thread.start_new_thread(self.lunchSimulationProcess, (cmd,cmd_dir))
        with file(self.config_file, 'r') as f:
            self.teCommand.setText('Simulation running with the parameters:\n' + f.read())

        # self.lunchSimulationProcess(cmd,cmd_dir)



    def lunchSimulationProcess(self, cmd, cmd_dir):
        process = Popen(cmd, cwd=cmd_dir, universal_newlines=True, shell=True, stdout=PIPE)
        (output, err) = process.communicate()
        exit_code = process.wait()

        # self.pbExtractFlows_clicked()

    def pbGenerate_clicked(self):

        self.timer_gd.stop()

        # Read the user input config from the UI
        #This configs are important to the ns-3 simulation
        self.network_size_x = self.sbNetworkSizeX.value()
        self.network_size_y = self.sbNetworkSizeY.value()
        self.neighborhood_size = self.sbNeighborhoodSize.value()
        self.number_of_sinks = self.sbNumberofSinks.value()
        self.number_of_cycles = self.sbNumbeofCycles.value()
        self.period = self.sbPeriod.value()  # from milis to nano seconds
        self.baudrate = self.sbBaudrate.value()
        self.working_dir = self.tbWorkingDir.text()

        #This config is only utilized while generating the sensors data
        #which is caught by the ns-3 during the simulation
        self.sensor_scale = pow(2, self.sbSensorResolution.value())

        errors = 0

        #TODO: here, check, and possibly creat the in out folders, inside the working dir here

        #save the simulation parameters to a file, which is read by the simulator, and by the user interface
        #  results ploting

        file_name_id = "input-data.s"  #s stands for sets of data along the time
        file_name_cf = "input-config.c"  #c stands for configuration data
        file_name_aux_config = "input_config.c"
        file_extension = ".csv"

        home = os.path.expanduser("~")

        context_dir = "nw" + \
            str(self.network_size_x) + "x" + \
            str(self.network_size_y) + "s" + \
            str(self.number_of_sinks) + "n" + \
            str(self.neighborhood_size) + \
            str(self.tbContext.text())

        self.working_dir_config = home + "/usn-data/config/"
        # aux_config_dir = home + "/usn-data/"
        self.working_dir = home + "/usn-data/" + context_dir

        if not os.path.exists(self.working_dir_config):
            os.makedirs(self.working_dir_config)

        if not os.path.exists(self.working_dir + "/in" ):
            os.makedirs(self.working_dir + "/in" )

        self.config_file = self.working_dir_config + file_name_cf + file_extension
        full_path_id = self.working_dir + '/in/' + file_name_id + file_extension


        config = (
            " --size_x=" + str(self.network_size_x) +
            " --size_y=" + str(self.network_size_y) +
            " --size_neighborhood=" + str(self.neighborhood_size) +
            " --sinks_n=" + str(self.number_of_sinks) +
            " --sampling_cycles=" + str(self.number_of_cycles) +
            " --sampling_period=" + str(self.period) +
            " --baudrate=" + str(self.baudrate) +
            " --operational_mode=" + str(int(self.cbReadAll.isChecked())) +
            " --io_data_dir=" + str(self.working_dir)
        )


        with file(self.config_file, 'w') as configuration_data:
            configuration_data.write(config)  #TODO: check if this function generates error



        #the simulation then, be run like:
        #./waf --run "src/usn/examples/usn-example $(cat ~/usn-data/in/input-config.c.csv)"

        # Setup the initial plot
        self.input_data_generated = np.ones((self.network_size_x, self.network_size_y))
        self.input_data_generated[int(self.network_size_x) / 2][int(self.network_size_y) / 2] = self.sensor_scale

        self.axg = self.fig_gd.add_subplot(1, 1, 1)
        self.axg.axes.get_xaxis().set_visible(True)
        self.axg.axes.get_yaxis().set_visible(True)
        # self.axg.set_title('Input Sensors Data')
        self.imgg = self.axg.imshow(self.input_data_generated, interpolation='nearest', cmap=pylab.get_cmap('hot_r'))


        if self.rbGenerateFromEquation.isChecked():

            def f(x, y):
                return (np.sin(-x) * np.cos(y));  #offset to between 0 and max
                # return (np.exp(-x-y) * np.sin( 1 * np.pi * (x + y)))

            # example curve
            # plot exp(-x-y)*sin(2*pi*x*y), x=-2...0 y = -2...0

            x = np.linspace(-.5 * np.pi, .5 * np.pi, self.network_size_x)
            y = np.linspace(-.5 * np.pi, .5 * np.pi, self.network_size_y).reshape(-1, 1)

            self.input_data_list = []

            for i in range(self.number_of_cycles):
                x += np.pi / 30.  ## the steps in x and y
                y += np.pi / 30.

                # in each iteration, it creates the entire matrix
                self.input_data_list.append(f(x, y))

            self.input_data_list = self.normalize_data(self.input_data_list)
            self.write_input_data_to_disk(full_path_id, self.input_data_list)

            self.imgg.set_array(self.input_data_list[0])
            self.canvas_gd.draw()
            self.ig = 0
            self.timer_gd.start(200)

        elif self.rbGenerateFromCFD.isChecked():
            self.pbAddDataToSimulation_clicked()

        elif self.rbGenerateFromImage.isChecked():
            # im_file = str(QFileDialog.getOpenFileName(self, ("Select an image file"), self.tbWorkingDir.text() + '\..\images'))
            im_file = '/home/joao/usn-data/images/flow.png'
            self.input_data_list = []

            imarray = self.load_data_from_image(im_file) #todo: use open CV to load instead
            self.countour_image = self.find_contour(im_file, show=0)

            self.input_data_list.append(imarray)
            self.input_data_list.append(imarray)
            self.write_input_data_to_disk(full_path_id, self.input_data_list)

            self.imgg.set_array(self.input_data_list[0])
            self.canvas_gd.draw()
            self.ig = 0
            self.timer_gd.start(200)

        elif self.rbGenerateFromVideo.isChecked():
            # video_file = str(QFileDialog.getOpenFileName(self, ("Select a video file"), self.tbWorkingDir.text() + '\..\videos'))
            self.input_data_list = []
            video_file = '/home/joao/usn-data/videos/DNS_of_planar_jets_hd720.mp4'
            # self.input_data_list = []

            video_cap = cv2.VideoCapture(video_file)
            video_cap.set(cv2.cv.CV_CAP_PROP_POS_MSEC, self.sbTi.value()*1000)#at seconds 10

            success, image = video_cap.read()

            xi = self.sbXi.value()
            xf = self.sbXf.value()
            yi = self.sbYi.value()
            yf = self.sbYf.value()

            image = self.convert_image(image, xi=xi, xf=xf, yi=yi, yf=yf)
            self.input_data_list.append(image)
            image_ant = image

            i=0
            while i < self.number_of_cycles - 1:
                success, image = video_cap.read()
                image = self.convert_image(image, xi=xi, xf=xf, yi=yi, yf=yf)
                img_diff = np.mean(image-image_ant)
                if (np.abs(img_diff) < 0.005):
                    continue
                else:
                    i+=1
                    self.input_data_list.append(image)
                    image_ant = image

                # imarray = self.load_data_from_image(image)
                # image_out = np.ones((image.shape[1], image.shape[0])) * 255
                # image_out = cv2.cvtColor(image, cv2.cv.CV_RGB2GRAY);
                # self.input_data_list.append(self.convert_image(image))
                # image_ant = image

            self.input_data_list = noc_calc.normalize_data_array(self.input_data_list, self.sbSensorResolution.value())

            # self.countour_image = self.find_contour(im_file, show=0)

            self.write_input_data_to_disk(full_path_id, self.input_data_list)

            self.imgg.set_array(self.input_data_list[0])
            self.canvas_gd.draw()
            self.ig = 0
            self.timer_gd.start(200)

            filename = self.working_base_dir + "/plots/input-data"
            noc_plots.matrix(self.input_data_list[ 0], filename + "-ti.eps")
            noc_plots.matrix(self.input_data_list[-1], filename + "-tf.eps")


        if errors == 0:
            self.teCommand.setText('Configuration file created at "' + self.config_file +
                               '" with the following paramenters:\n' + config +
                               '\n\nData files created successfully at: "' + full_path_id + '"')
        else:
            self.teCommand.setText('Errors found while creating the input files for the simulation.')


        self.load_config()

    def normalize_data(self, data):
        #normalize the plot to the sensor resolution and range

        max_value = np.max(data)
        min_value = np.min(data)
        sens_scale = pow(2, self.sbSensorResolution.value())

        try:
            data = data.astype('float') # this conversion does not work when importing from CFD
        except:
            data = data

        data = (data + min_value * -1) / (np.abs(max_value) + np.abs(min_value))
        data *= sens_scale

        # for i in range(0, len(data)):
        #     data[i] = (data[i] + min_value * -1) / (np.abs(max_value) + np.abs(min_value))
        #     data[i] *= sens_scale
        #     # data[i] = np.uint16(data[i])

        return np.uint16(data)

    def convert_image(self, image, xi=0, yi=0, xf=500, yf=500):
        image_out = image[..., 0]*0.299 + image[..., 1]*0.587 + image[..., 2]*0.114

        image_out = image_out[yi:yf,xi:xf] #100:880,320:1100

        image_out = cv2.resize(image_out,(self.network_size_x, self.network_size_y), interpolation = cv2.INTER_CUBIC)

        return image_out

    def write_input_data_to_disk(self, full_path_id, data):
        with file(full_path_id, 'w') as output_data:
            input_data_snapshot_number = 0
            for input_data_snapshot in data:

                #TODO: check if this functions generates errors
                savetxt(output_data, input_data_snapshot, delimiter=',', fmt='%d')
                # metadata goes at after the data, as it is easier to read
                output_data.write('@' + str(input_data_snapshot_number) + '\n')  # starts with # and ends at @
                # print(np.max(d))
                # print(np.min(d))

                input_data_snapshot_number += 1


    def pbAddSensortsToE2dFile_clicked(self):

        xml_filename = self.tbE2dSimulationFile.text()
        F = open(xml_filename, mode='rw')

        #I access the XML file, and remove all the current sensors:
        tree = et.parse(F)
        model = tree.find('model')
        sensor = model.find('sensor')
        thermometers = sensor.findall('anemometer')

        for t in thermometers:
            sensor.remove(t)

        #Than, I add based on the parameters provided

        y_min = self.sbYmin.value()
        y_max = self.sbYmax.value()
        x_min = self.sbXmin.value()
        x_max = self.sbXmax.value()

        y_s = self.network_size_y
        x_s = self.network_size_x

        for y_cord in np.linspace(y_max, y_min, y_s):
            for x_cord in np.linspace(x_min, x_max, x_s):
                e = et.Element('anemometer', {'x':str(x_cord), 'y':str(y_cord)})
                e.tail = '\n'
                sensor.append(e)

        tree.write(xml_filename)
        F.close()

    def pbAddDataToSimulation_clicked(self):
        csv_filename = self.tbCsvSimulationData.text()
        y_s = self.network_size_y
        x_s = self.network_size_x

        #reads the output data from the cfd simulation

        in_data = np.array(list(csv.reader(open(csv_filename, "rb"), delimiter='\t')))


        out_data = []
        out_data_time = []
        i = 0
        j = 0

        #each line correspond to one full matrix
        for line in in_data:
            out_data_snapshot = np.zeros([x_s, y_s])
            for iterator, e in enumerate(line):
                if iterator == 0:
                    out_data_time.append(e)
                    continue
                out_data_snapshot[j][i] = e
                i+=1
                if i >= x_s:
                    i = 0
                    j+=1

            out_data.append(out_data_snapshot)
            i=0
            j=0

        #then, saves it and replaces the actual data file

        file_name_id = "input-data.s"  #s stands for sets of data along the time
        file_extension = ".csv"

        full_path_id = self.working_dir + '/in/' + file_name_id + file_extension

        self.input_data_list = self.normalize_data(out_data)

        self.write_input_data_to_disk(full_path_id, self.input_data_list)

        self.load_input_data_to_graphic()

    def load_input_data_to_graphic(self):
        # Setup the initial plot

        self.input_data_generated = np.ones((self.network_size_x, self.network_size_y))
        self.input_data_generated[int(self.network_size_x) / 2][int(self.network_size_y) / 2] = pow(2, self.sbSensorResolution.value())

        self.axg = self.fig_gd.add_subplot(1, 1, 1)
        self.axg.axes.get_xaxis().set_visible(True)
        self.axg.axes.get_yaxis().set_visible(True)
        self.axg.set_title('Input Sensors Data')
        self.imgg = self.axg.imshow(self.input_data_generated, interpolation='nearest')

        self.imgg.set_array(self.input_data_list[0])
        self.canvas_gd.draw()
        self.ig = 0
        self.timer_gd.start(200)

    def pbStart_clicked(self):
        # self.lbTime.setText("Teste")
        # plots the data with animation
        # ani = animation.FuncAnimation(self.fig, self.animate, np.arange(self.t_initial, self.t_final, self.t_step), interval=15, blit=True, repeat=False)
        self.t_plot = 0
        self.read_files()
        self.initial_plot_setup()
        self.timer_ta.start(100)

    def pbStop_clicked(self):
        self.timer_ta.stop()
        self.t_plot = 0


    def pbExtractFlows_clicked(self):

        #Indexes definition (enumeration)

        column_headers = ["n","id","ti","tf","td",
                          "th_min","th_max","th_mean",
                          "xi","xf","xd",
                          "yi","yf","yd","d"]


        working_dir = self.tbWorkingDir.text()
        file_extension = ".csv"
        file_name = working_dir + "/out/packets-trace" + file_extension

        #load form an specific file
        pck_trace = noc_io.load_list(file_name)


        if pck_trace == -1:
            QtGui.QMessageBox.error(self, 'Error', "Error loading the specified packet trace", QtGui.QMessageBox.Ok)
            return


        #TODO: turn this into parallel processing
        # what are your inputs, and what operation do you want to
        # perform on each input. For example...

        period = self.period * 1e3 # to nanoseconds
        spliting_points = []
        ti = int(1e9 + 2 * period)
        for i in range(len(pck_trace)-1):
            ta = int(pck_trace[i][noc_io.DEF_TIME])

            if ta - ti >= period:
                spliting_points.append(i)
                ti += period

        pck_trace_split = np.vsplit(pck_trace, spliting_points)

        num_cores = multiprocessing.cpu_count()

        results = Parallel(n_jobs=num_cores)(delayed(noc_io.flow_trace)(pck_trace_split[i]) for i in range(len(pck_trace_split)))

        flows = results[0]

        print len(results[0])

        for i in range(1, len(results)-1):
            flows = np.vstack((flows, results[i]))
            print len(results[i])

        # flows2 = usn_io.flow_trace(pck_trace)


        self.print_to_table(self.twFlowAnalysis, flows, column_headers)
        file_name = working_dir + "/out/flow-trace" + file_extension
        self.print_to_file(file_name, flows)

        # if self.cbExportCSV.checked == 1:

        return

    def pbLoadFiles_clicked(self):
        return


    def pbBoxPlot_clicked(self):

        #list the flow trace files

        #it first list all the flow trace files
        flows = []
        em_delay_files = []
        em_delay = []
        em_count = []
        for (dirpath, dirnames, filenames) in os.walk(self.working_base_dir):
            for file in filenames:
                if file.startswith("flow-trace"):
                    flows.append(os.path.join(dirpath, file))
                if file.startswith('value-announcement-delay'):
                    em_delay_files.append(os.path.join(dirpath, file))

        flows.sort()
        em_delay_files.sort()

        #then extract data from each one, putting them in bins
        data_transmission_time = []
        data_hop_time = []
        for flow_file in flows:
            bin_transmission_time = []
            bin_hop_time = []
            for line in np.array(list(csv.reader(open(flow_file, "rb"), delimiter=','))):
                bin_transmission_time.append(float(line[3]) / 1000000)
                bin_hop_time.append(float(line[6]) / 1000)

            data_transmission_time.append(bin_transmission_time)
            data_hop_time.append(bin_hop_time)

        #now, extract absolute maximum numbers about each scenario
        number_of_transmissions = []
        total_transmission_time = []
        i=0
        for line in data_transmission_time:
            total_transmission_time.append(np.max(line))
            number_of_transmissions.append([i, len(line)])
            i+=1

        i=0
        for file in em_delay_files:
            for line in np.array(list(csv.reader(open(file, "rb"), delimiter=','))):
                em_delay.append((float(line[1]) - float(line[0]))/ 1000000)
                em_count.append([i, float(line[2]) / 1000])
                i+=1

        # fn = "/home/joao/Documentos/Artigos/OPODIS'14/other-figures/number-of-transmissions.eps"
        # self.plot(number_of_transmissions, filename=fn, lable_x='n_hops', lable_y='event announcements count')
        #
        # fn = "/home/joao/Documentos/Artigos/OPODIS'14/other-figures/number-of-transmissions-em.eps"
        # self.plot(em_count, filename=fn, lable_x='n_hops', lable_y='event-monitoring count')

        # fn = "/home/joao/Documentos/Artigos/OPODIS'14/other-figures/number-of-transmissions.eps"
        # self.plot2(number_of_transmissions, em_count,filename=fn, label_x='n_hops', \
        #            label_y1='EA packet count', label_y2='EM packet count (10e-3)')

        number_of_transmissions.pop(0)
        em_count.pop(0)
        noc_plots.plot2xy(em_count, number_of_transmissions, label_x='Neighborhood size ($n_{hops}$)', label_y1='EM packet count (10-e3)',
                          label_y2='EA packet count', show=True, logscale=False, legend=['EM', 'EA'],
                          filename = self.working_base_dir + '/plots/em_x_ea.eps', two_axis=True)

        # fn = "/home/joao/Documentos/Artigos/OPODIS'14/other-figures/total-transmissions-time.eps"
        # self.plot(total_transmission_time, filename=fn, lable_x='n_hops', lable_y='total event-announcements time (ms)')
        #
        # fn = "/home/joao/Documentos/Artigos/OPODIS'14/other-figures/total-transmissions-time-em.eps"
        # self.plot(em_delay, filename=fn, lable_x='n_hops', lable_y='total event-monitoring time (ms)')

        # fn = "/home/joao/Documentos/Artigos/OPODIS'14/other-figures/total-transmissions-time.eps"
        # self.plot2(total_transmission_time, em_delay,filename=fn, label_x='n_hops', \
        #            label_y1='EA total time (ms)', label_y2='EM total time (ms)')

        # # self.box_plot(data_transmission_time,"", "n-hops", "time [us]")
        # fn = "/home/joao/Documentos/Artigos/OPODIS'14/other-figures/boxplot-event-announcement-total-time.eps"
        # self.box_plot(data_transmission_time, [0,.9],fn, "", "n-hops", "time [ms]")
        #
        # fn = "/home/joao/Documentos/Artigos/OPODIS'14/other-figures/boxplot-event-announcement-hop-time.eps"
        # self.box_plot(data_hop_time, [13,50],fn, "", "n-hops", "time [us]")

        # plt.hist(data_hop_time[2])
        # plt.show()

        # # open the file inside the address flows[3]
        # fn = "/home/joao/Documentos/Artigos/OPODIS'14/other-figures/transmissions-count-along-time.eps"
        # self.histogram(self.read_flow_trace_file(flows[3]), fn,"", "time [ms]", "number of transmissions")


    def pbActivityHistogram_clicked(self):
        flow = self.working_dir + '/out/flow-trace.csv'
        fn = self.working_dir + '/plots/activity-histogram.eps'
        noc_plots.histogram(self.read_flow_trace_file(flow), fn, "", "Time [ms]", "Number of transmissions", bin_width=self.period * 1e3)

    def pbActivityHistogramMultiple_clicked(self):
        files = noc_io.find_multiple_files(self.working_base_dir, 'flow-trace.csv')
        flows_list = []
        legends = []
        numbering = []

        for file in files:
            flows_list.append(self.read_flow_trace_file(file))
            fn = file.split('/')[4].split('nw101x101s1n')[1]#.split('0')[0]
            numbering.append(fn[0])
            # lg = "$n_{hops}$ = " + str(fn[0]) + " (" +  str(fn[4]) + str(fn[5]) + ")"
            # legends.append(lg)

        count = 0
        i = 0
        flows_ds_count = []

        # for flows in flows_list:
        #     for flow in flows:
        #         if flow[usn_io.DEF_PROTOCOL] == usn_io.DEF_DATA_SHARING:
        #             count += 1
        #
        #     flows_ds_count.append(count)
        #     i += 1
        #     count = 0
        #
        # fn = self.working_base_dir + '/plots/activity-ds-histogram.eps'
        # usn_plots.histogram(flows_ds_count, filename=fn, show=True)

        fn = self.working_base_dir + '/plots/activity-histogram.eps'
        noc_plots.histogram_multiple(flows_list, fn, "", "Time [ms]", "Number of DAs", legends=legends, bin_width=self.period * 1e3, show=True)

    def pbpbNumberTransmissionsDS_clicked(self):

        count_ds = 1
        count_da_fd = 1
        count_da_cl = 1
        legends = []

        i = 0
        packets_ds_count = []

        # plot here the duration of the DS phase
        packet_trace_files = noc_io.find_multiple_files(self.working_base_dir, 'packets-trace.csv')
        for packet_trace_file in packet_trace_files:
            packet_trace = noc_io.load_list(packet_trace_file)
            s = (packet_trace_file.split('/')[4].split('nw101x101s1n')[1])
            i = int(s[0])
            scenario = s[1] + s[2]
            for packet in packet_trace:
                if int(packet[noc_io.DEF_PROTOCOL]) == noc_io.DEF_DATA_SHARING:
                    count_ds += 1

                if int(packet[noc_io.DEF_PROTOCOL]) == noc_io.DEF_DATA_ANNOUCEMENT and scenario == 'FD':
                    count_da_fd += 1

                if int(packet[noc_io.DEF_PROTOCOL]) == noc_io.DEF_DATA_ANNOUCEMENT and scenario == 'CL':
                    count_da_cl += 1

            packets_ds_count.append((i, count_ds, count_da_fd, count_da_cl))

            # i += 1
            count_ds = 1
            count_da_fd = 1
            count_da_cl = 1


        legends = ['DS','DA (FD)','DA (CL)']

        print packets_ds_count
        fn = self.working_base_dir + '/plots/activity-ds.eps'
        noc_plots.bar(packets_ds_count, show=True, legends=legends, filename=fn, label_x='$n_{hops}$', label_y='Number of transmissions')



    def pbQueueSizeOverTime_clicked(self):
        queue = noc_io.load_list(self.working_dir + '/out/queue-size-over-time.csv')
        queue = np.delete(queue, (0), axis=0)

        t0 = float(queue[0][0])

        data1 = [ [ (float(v[0]) - t0) / 1000000, int(v[2]) ] for v in queue]
        data2 = [ [ (float(v[0]) - t0) / 1000000, int(v[1]) ] for v in queue]




        noc_plots.plotxy(data1, show=True, logscale=False,
                         filename = self.working_dir + '/plots/queue-size-over-time-p1.eps')
        noc_plots.plotxy(data2, show=True, logscale=False,
                         filename = self.working_dir + '/plots/queue-size-over-time-p0.eps')

    def pbQueueSizeOverTimeMultiple_clicked(self):
        files = noc_io.find_multiple_files(self.working_base_dir, 'queue-size-over-time.csv')

        data1 = []
        data2 = []
        data3 = []
        maxmax = []

        for file in files:
            i = int(file.split('/')[4].split('nw101x101s1n')[1])
            queue = noc_io.load_list(file)
            queue = np.delete(queue, (0), axis=0)

            max1 = np.max( [int(v[1]) for v in queue] )
            max2 = np.max( [int(v[2]) for v in queue] )

            data1.append([i, max1])
            data2.append([i, max2])
            data3.append([i, max1+max2])


        max = np.max(data3) + 4



        #
        # usn_plots.plot2xy(data1, data2, show=True, logscale=False, label_x='$n_{hops}$', label_y1='Maximum queue size',
        #                   label_y2='Maximum queue size', y_lim=[0, max], legend=['p0', 'p1'],
        #                   filename = self.working_base_dir + '/plots/queue-size-over-time.eps')

        noc_plots.plotxy_multiple([data1, data2, data3], show=True, logscale=False, label_x='Neighborhood size ($n_{hops}$)',
                          label_y='Maximum queue size',
                          y_lim=[0, max], legend=['p0', 'p1', 'p0 + p1'], plot_type='plot',
                          filename = self.working_base_dir + '/plots/queue-size-over-time.eps', legend_position=1)

    def pbIOCompare_clicked(self):

        data_in = noc_io.load_sensors_data(self.working_dir + '/in/input-data.s.csv', t=1)
        data_out = noc_io.load_sensors_data(self.working_dir + '/out/output-data-s-0.csv', t=0)

        data_out = noc_calc.trim(data_out, data_in.shape[1], data_in.shape[0])

        data_out_rebuilt = deepcopy(data_out)
        data_out_rebuilt = noc_calc.rebuild_plato(data_out_rebuilt)

        noc_plots.matrix(data_in, filename = self.working_dir + '/plots/input-data.eps')
        noc_plots.matrix(data_out, filename = self.working_dir + '/plots/output-data.eps') #show both
        noc_plots.matrix(data_out_rebuilt, filename = self.working_dir + '/plots/output-data-rebuilt.eps', show=True) #show both

        data_diff, mse = noc_calc.diff(data_in, data_out)

        print("\n mse = " + str(mse) + "\n")

        noc_io.write_single_value(mse, filename = self.working_dir + '/plots/mse.csv')

        noc_plots.matrix(data_diff, filename = self.working_dir + '/plots/data-diff.eps')

        ###################################################################################

    def pbIOCompareError_clicked(self):
        files = noc_io.find_multiple_files(self.working_base_dir, 'mse.csv')

        data_set1 = []
        data_set2 = []

        for file in files:
            s = (file.split('/')[4].split('nw101x101s1n')[1])
            i = int(s[0])
            data_set1.append([i, noc_io.load_single_value(file)])

        files = noc_io.find_multiple_files(self.working_base_dir, 'packets-trace.csv')

        for file in files:
            s = (file.split('/')[4].split('nw101x101s1n')[1])
            i = int(s[0])
            pck_trace = noc_io.load_list(file)
            # t = float(int(pck_trace[-1][1]) - int(pck_trace[0][1])) / 1000000
            t = float(int(pck_trace[-1][1]) - int(pck_trace[0][1])) / 8799
            data_set2.append([i, t])


        noc_plots.plot2xy(data_set2, data_set1, label_x='Neighborhood size ($n_{hops}$)', label_y2='Mean square error (MSE)',
                          label_y1='Max. end-to-end delay (TTS)', show=True, logscale=False, legend=['delay', 'MSE'],
                          filename = self.working_base_dir + '/plots/mse.eps', two_axis=True)
        #mean square error
        ####################################################################################

    def pbThroughput_clicked(self):
        pck_trace = noc_io.load_list(self.working_dir + '/out/packets-trace.csv')

        throughput = noc_calc.trhoughput(pck_trace)

        print 'Total packets exchanged: ' + str(np.sum(throughput, 0))

        # usn_plots.plotxy(throughput, show=True, filename = self.working_dir + '/plots/throughput.eps', x_lim=[1100000000,1115600000], y_lim=[0,42000])
        noc_plots.plotxy(throughput, show=True, logscale=True, filename = self.working_dir + '/plots/throughput.eps') #, x_lim=[9.05,9.06], y_lim=[0,42000])

    def pbTrhoughputMultiple_clicked(self):

        files = noc_io.find_multiple_files(self.working_base_dir, 'packets-trace.csv')
        legends = []
        throughputs = []

        for file in files:
            s = (file.split('/')[4].split('nw101x101s1n')[1])
            # i = int(s[0])
            legends.append('$n_{hops}=' + s[0] + '$')
            pck_trace = noc_io.load_list(file)
            throughput = noc_calc.trhoughput(pck_trace)
            throughputs.append(throughput)

        noc_plots.plotxy_multiple(throughputs, label_x='Total execution time (TTS) in time slot', label_y='Number of packets received',
                                  show=True, legend=legends, logscale=True,
                                  x_lim=[0,2600],
                                  filename = self.working_base_dir + '/plots/throughput.eps')

        noc_plots.plotxy_multiple(throughputs, label_x='Total execution time (TTS) in time slot', label_y='Number of packets received',
                                  show=True, legend=legends, x_lim=[0,60],y_lim=[1e2,1e5], logscale=True,
                                  filename = self.working_base_dir + '/plots/throughput-zoom.eps')


    def pbPlotPDF_clicked(self):

        dists_list = []
        legends = []
        mse = []
        lines = []
        # lines = ['solid', 'dashed', 'dotted', 'dashdot', (0, (3.0, 4.0)), (0, (8.0, 8.0))]
        # linecycler = cycle(lines)

        dists_files = noc_io.find_multiple_files(self.working_base_dir, 'contour-diff')

        for dir in dists_files:
            legends.append('n_hops=' + dir.split('/')[4].split('nw101x101s1n')[1])

        i=2
        j = 1
        for file in dists_files:
            dists = []
            dists_sq = []

            for d in np.array(list(csv.reader(open(file, "rb"), delimiter='\n'))):
                dists.append(float(d[0]))
                dists_sq.append(float(d[0]) * float(d[0]))

            dists_list.append(np.abs(np.array(dists) / 1752)) #devide by the size of the figure
            e = np.mean(np.array(dists_sq), 0)
            mse.append ( [j,e/1000] )
            line = (0, (i, i/2)) #definition of a linestyle. dash length and interspace length
            lines.append( line )
            j+=1
            i+=2

        linecycler = cycle(lines)

        # dists_list = dists_list / np.max(dists_list)

        # pylab.hist(dists, 50, normed=1, facecolor='green', alpha=0.5, cumulative=True, histtype='step')

        pylab.figure(figsize=(7, 3), dpi=120, facecolor='w', edgecolor='w')

        n, bins, patches = pylab.hist(dists_list, 50000, normed=1, facecolor='blue', alpha=0.9, cumulative=True, histtype='step',
                            # color=['r', 'g', 'b', 'y'],
                            label=legends)

        for patch in patches:
            patch[0]._linestyle = next(linecycler)

        pylab.ylim([-0.001, 1.05])
        pylab.xlim([-0.001, 0.08])
        pylab.xlabel('Error')
        pylab.ylabel('Cumulative density')
        pylab.tight_layout(pad=2, w_pad=1, h_pad=1)
        pylab.legend(loc=4, fontsize=10)
        pylab.grid(True)
        pylab.tight_layout(pad=1, w_pad=1, h_pad=1)
        pylab.savefig(self.working_base_dir + '/plots/pdf-error.eps')
        pylab.show()
        pylab.close()

        # usn_plots.plotxy(mse,show=True , filename = self.working_dir + '/plots/mse.eps', logscale=False, label_y="MSE (1e-3)", label_x='n_hops', step=False)

        files = noc_io.find_multiple_files(self.working_base_dir, 'packets-trace.csv')
        delay = []
        for file in files:
            i = int(file.split('/')[4].split('nw101x101s1n')[1])
            pck_trace = noc_io.load_list(file)
            # t = float(int(pck_trace[-1][1]) - int(pck_trace[0][1])) / 1000000
            t = float(int(pck_trace[-1][1]) - int(pck_trace[0][1])) / 8799
            delay.append([i, t])

        for e in delay:
            print str(e[1]) + ', ' + str(e[1]/delay[0][1] * 100) + '\n'

        delay.pop(0)

        noc_plots.plot2xy(delay, mse, label_x='Neighborhood size ($n_{hops}$)', label_y2='MSE (1e-3)',
                          label_y1='Max. end-to-end delay (TTS)', show=True, logscale=False, legend=['delay', 'MSE'],
                          filename = self.working_base_dir + '/plots/mse.eps', two_axis=True)

        return


    def box_plot(self, data, y_lim, filename, title = "", lable_x = "", lable_y = "", x_size = 9, y_size = 5):



        pylab.figure(title, figsize=(x_size, y_size), dpi=80, facecolor='w', edgecolor='w')
        pylab.xlabel(lable_x, fontsize=20)
        pylab.ylabel(lable_y, fontsize=20)
        # if y_lim != []:

        # pylab.ylim(y_lim)

        cem = (0,2)
        # pylab.boxplot(data, conf_intervals=[cem, cem, cem, cem, cem, cem])
        pylab.boxplot(data)

        ax = pylab.gca()
        ax.set_xticklabels([1,2,3,4,5,6,7])
        for label in ax.get_xticklabels() + ax.get_yticklabels():
            label.set_fontsize(18)

        pylab.tight_layout()

        if filename!='0':
            pylab.savefig(filename)

        pylab.show()

    def plot(self,  data, filename='', title = "", lable_x = "", lable_y = "", x_size = 7, y_size = 3):
        # if data_r is None:
        #     data_r = []

        s = len(data)
        pylab.figure(figsize=(x_size, y_size), dpi=120, facecolor='w', edgecolor='w')
        plt.plot(np.linspace(1, s, s), data, 'go-')
        pylab.tight_layout(pad=2, w_pad=1, h_pad=1)
        plt.xlabel(lable_x)
        plt.ylabel(lable_y)

        if filename != '':
            pylab.savefig(filename)

        plt.show()
        pylab.close()

        return


    def print_to_table(self, table, data, headers):
        c = 0
        table.setRowCount(0)
        table.setColumnCount(0)
        table.clear()

        table.setRowCount(len(data))
        table.setColumnCount(len(data[0]) + 1)
        table.setHorizontalHeaderLabels(headers)

        for l in data:
            itemh = QTableWidgetItem()
            itemh.setData(Qt.EditRole, int(c))
            table.setItem(c, 0, itemh)

            for i in range(0,len(l)):
                iteml = QTableWidgetItem()
                iteml.setData(Qt.EditRole, int(l[i]))
                table.setItem(c, i+1, iteml)
            c += 1

    def print_to_file(self, file_name, data):
        with file(file_name, 'w') as file_out:

            try:
                savetxt(file_out, data, delimiter=',', fmt='%d')
            except:
                QtGui.QMessageBox.error(self, 'Error', "Error writing file: " + file_name, QtGui.QMessageBox.Ok)
                return

    def read_flow_trace_file(self, file_name):

        flow_trace = []

        for line in np.array(list(csv.reader(open(file_name, "rb"), delimiter=','))):
            flow_trace_line = []
            for e in line:
                flow_trace_line.append(int(e))
            flow_trace.append(flow_trace_line)

        return flow_trace






    def print_timings_to_table(self):
        cc = 0

        self.tableWidget.setRowCount(len(self.flows_transmission_time))

        for l in self.flows_transmission_time:

            dist = noc_calc.dist(l[3], l[4], self.sink_x, self.sink_y)[2] #get the element 2 of the array returned by the f

            transmission_time = (l[2] - l[1])
            # self.listWidget.addItem(str(l[0]) + '\t(' + str(l[3]) + ',' + str(l[4]) + ') d=' + str(dist) + '\t' + str( (l[2] - l[1]) * 10E-6))

            self.tableWidget.setItem(cc, 0, QTableWidgetItem(str(cc)))  #counter
            self.tableWidget.setItem(cc, 1, QTableWidgetItem(str(l[0])))  #flow id
            self.tableWidget.setItem(cc, 2, QTableWidgetItem('d=' + str(dist) + '(' + str(l[3]) + ',' + str(l[4]) + ')'))  #distance to the sink
            self.tableWidget.setItem(cc, 3, QTableWidgetItem(str(transmission_time * 1E-6)))  #total transmission time
            self.tableWidget.setItem(cc, 4, QTableWidgetItem(str(l[6] * 1E-6)))  #min delay caused by one hop
            self.tableWidget.setItem(cc, 5, QTableWidgetItem(str(transmission_time / dist * 1E-6)))  #mean delay per node
            self.tableWidget.setItem(cc, 6, QTableWidgetItem(str(l[5] * 1E-6)))  #mmax delay caused by one hop
            cc += 1


    def timer_ta_animate(self):
        # while i < times.size:

        if self.t_plot < len(self.times) - 1:
            self.update_plot(self.times[self.t_plot])
            self.t_plot += 1

            # from this point on, it updates data only
            self.lbTime.setText("t = " + str(self.times[self.t_plot] * 1E-6) + " ms")

            if self.flows_transmission_time_updated == 1:
                self.flows_transmission_time_updated = 0
                self.print_timings_to_table()



        else:

            self.timer_ta.stop()

    ################## function called to animate the plots ########




    def update_plot(self, t):
        ## Array elements map definitions
        # global DEF_TIME, DEF_FLOW_ID, DEF_CORD_X, DEF_CORD_Y, DEF_PCK_VALUE, DEF_PCK_CORD_X, DEF_PCK_CORD_Y

        # global data1, data2, data3, data4
        global i, j, k



        ## traces for all the packets
        pck_trace_line = self.pck_trace[i]
        t_instant = int(pck_trace_line[DEF_TIME])
        flow_id = int(pck_trace_line[DEF_FLOW_ID])
        cord_x = int(pck_trace_line[DEF_CORD_X])
        cord_y = int(pck_trace_line[DEF_CORD_Y])

        ## trances for the sink only
        pck_trace_line_sink = self.pck_trace_sink[k]
        t_instant_sink = int(pck_trace_line_sink[DEF_TIME])
        flow_id_sink = int(pck_trace_line_sink[DEF_FLOW_ID])
        # protocol_number_sink = int(pck_trace_line_sink[7])
        pck_value = int(pck_trace_line_sink[DEF_PCK_VALUE])
        cord_x_sink = int(pck_trace_line_sink[DEF_PCK_CORD_X])
        cord_y_sink = int(pck_trace_line_sink[DEF_PCK_CORD_Y])


        # in this loop, it will process all the data that have occurred until the current time instant t
        # for all of the data logs. Input data will change, also all the packets transmitted/received until that
        while t_instant < t:

            if flow_id not in self.flows_known:
                self.flows_known.append(flow_id)
                #flow start time, end time, x_cord_origin, y_cord_origin, t_queing_delay
                self.flows_transmission_time.append([flow_id, t_instant, t_instant, cord_x, cord_y, 0,1E9])
                self.data2[cord_y, cord_x] += 1  # this creates the connection traces on the plots

            else:  #here I log the start and end of a flow.
                for m, f in enumerate(self.flows_transmission_time):  # f gets the line element, and m the iterator counter to it (i)
                    if flow_id == f[0]:
                        if t_instant > f[2]:

                            if t_instant - f[2] > f[5]:  # get the maximum queueing delay
                                self.flows_transmission_time[m][5] = t_instant - f[2]

                            if t_instant - f[2] < (f[6]):  # get the minimum queueing delay
                                self.flows_transmission_time[m][6] = t_instant - f[2]

                            self.flows_transmission_time[m][2] = t_instant
                            self.flows_transmission_time_updated = 1

            self.data4[cord_y, cord_x] += 1

            #get data from the next time instant to check if it has occurred already or not

            if i < len(self.pck_trace) - 2:
                i += 1
            else:
                self.timer_ta.stop()
                self.t_plot = 0
                self.img1.set_array(self.data1)
                self.img2.set_array(self.data2) #rotate the images..
                self.img3.set_array(self.data3)
                self.img4.set_array(self.data4)

                self.canvas_ta.draw()
                return

            pck_trace_line = self.pck_trace[i]
            t_instant = int(pck_trace_line[DEF_TIME])
            flow_id = int(pck_trace_line[DEF_FLOW_ID])
            cord_x = int(pck_trace_line[DEF_CORD_X])
            cord_y = int(pck_trace_line[DEF_CORD_Y])

            # plot the input data moving along the time
            if t_instant > self.input_data_time[j] + self.t_simulation_start:
                if j > 0:
                    self.data1 = self.input_data[j-1]
                else:
                    self.data1 = self.input_data[j]
                if j < len(self.input_data) - 1:
                    j += 1

            # plot the data reconstruction along the time
            if t_instant_sink < t_instant:
                k += 1
                x = self.network_size_x - np.floor(cord_x_sink + self.network_size_x / 2) - 1
                y = self.network_size_y - np.floor(cord_y_sink + self.network_size_y / 2) - 1
                self.data3[y, x] = pck_value

                if k > 1:
                    pck_trace_line_sink_previous = self.pck_trace_sink[k - 2]
                    t_instant_sink_previous = int(pck_trace_line_sink_previous[1])
                else:
                    t_instant_sink_previous = t_instant_sink

                if t_instant_sink - t_instant_sink_previous > self.t_time_window:
                    self.data3 = np.ones((self.network_size_x,
                                          self.network_size_y)) * self.mean  # reset the values know by the sink if they are too old

                pck_trace_line_sink = self.pck_trace_sink[k]
                t_instant_sink = int(pck_trace_line_sink[DEF_TIME])
                flow_id_sink = int(pck_trace_line_sink[DEF_FLOW_ID])
                pck_value = int(pck_trace_line_sink[DEF_PCK_VALUE])
                cord_x_sink = int(pck_trace_line_sink[DEF_PCK_CORD_X])
                cord_y_sink = int(pck_trace_line_sink[DEF_PCK_CORD_Y])


        # transfer the new data to its respective subplot
        # self.img1.set_array(self.data1)
        # self.img2.set_array(zip(*self.data2[::-1])) #rotate the images..
        # self.img3.set_array(zip(*self.data3[::-1]))
        # self.img4.set_array(zip(*self.data4[::-1]))

        # transfer the new data to its respective subplot
        self.img1.set_array(self.data1)
        self.img2.set_array(self.data2) #rotate the images..
        self.img3.set_array(self.data3)
        self.img4.set_array(self.data4)

        self.canvas_ta.draw()
        # self.draw()


        return self.img1, self.img2, self.img3, self.img4

    def pbExportAsImage_clicked(self):
        self.viewed = 1

        # fn = "/home/joao/Documentos/Artigos/RTNS'14/figures/overview-input-data" + self.output_figures_context
        # self.image_show(self.data1)
        # fn = "/home/joao/Documentos/Artigos/RTNS'14/figures/overview-events-detected" + self.output_figures_context
        # self.image_show(self.data2)

        self.compare_contours(self.data2, self.countour_image, show=1)
        # self.compare_contours()

        # self.convert_to_line(im = [])

    def image_show(self, data, filename='', label_x="", label_y="", colormap='hot_r', axis='on', view=False):
        pylab.figure(figsize=(3, 3), dpi=120, facecolor='w', edgecolor='w')
        pylab.xlabel(label_x, fontsize=30)
        pylab.ylabel(label_y, fontsize=30)
        pylab.tight_layout(pad=0, w_pad=0, h_pad=0)
        # pylab.labe
        # rect=[0.1, 0.1, 0.1, 0.1]


        if axis == 'off':
            pylab.gca().xaxis.set_major_locator(plt.NullLocator())
            pylab.gca().yaxis.set_major_locator(plt.NullLocator())

        im = pylab.imshow(data, cmap=pylab.get_cmap(colormap), interpolation='nearest')

        # cax = pylab.divider.append_axes("right", size="5%", pad=0.05)
        # plt.colorbar(im)
        # plt.colorbar(im, cax=cax)


        if filename!='':
            pylab.savefig(filename)

        if view==True:
            pylab.show()

        pylab.close()

    def animate_gd(self):
        self.imgg.set_array(self.input_data_list[self.ig])
        self.canvas_gd.draw()
        self.ig += 1
        if self.ig == len(self.input_data_list):
            # self.timer_gd.stop()
            self.ig = 0


    ############### Packet tracing #################



    def ReadPacketTraceNetDevice(self):
        working_dir = self.tbWorkingDir.text()
        file_extension = ".csv"
        file_name = working_dir + "/out/packets-trace-net-device" + file_extension
        #1st - Packets Trace
        return np.array(list(csv.reader(open(file_name, "rb"), delimiter=',')))

    def PlotPacketTraceFrame(self):
        global counter_plot_packet_trace, packets_traced, t_initial, t_previous, t_packet

        #todo: this parameters should be passed as arguments of the function instead of global vars

        line = self.pck_trace_net_device[counter_plot_packet_trace]

        t = 0

        # if this is an input packet, and is one of the packets I am tracking, plot it
        #2 is the packet unique id, and the flow direction = input
        if int(line[2]) in (packets_traced) and int(line[3]) == 1:
            # global DEF_CORD_X, DEF_CORD_Y, DEF_TIME

            t = int(line[DEF_TIME])  #time it happen
            x = int(line[DEF_CORD_X])  # x
            y = int(line[DEF_CORD_Y])  #y of the node that received the pck
            p = int(line[7])  #input port



            if t_initial == -1:
                t_initial = t - t_packet

            temp = float(t - t_initial) / float(t_packet)
            t_text = int(np.round(temp))

            r = 0.1
            dx = 0
            dy = 0

            if p == 4:  #came from the right. x -= 1
                xs = x + 1; ys = y; xl = -1 + r; yl = 0; dy = 0.1; tx = 'W'; va = 'top'
            if p == 3:  #came from the bottom. y -= 1
                xs = x; ys = y + 1; xl = 0; yl = -1 + r; dx = 0.06; tx = 'N'; va = 'top'
            if p == 1:  #came from the left. x += 1
                xs = x - 1; ys = y; xl = 1 - r; yl = 0; dy = -0.1; tx = 'E'; va = 'top'
            if p == 2:  #came from the top. y += 1
                xs = x; ys = y - 1; xl = 0; yl = 1 - r; dx = -0.09; tx = 'S'; va = 'bottom'

            self.ax_pt.axes.arrow(xs, ys, xl, yl, head_width=0.15, head_length=0.15, fc='r', ec='r')
            # self.ax_pt.plot(xs,ys,'x')
            self.ax_pt.annotate(str(t_text), xy=((xs + xl / 2) + dx, (ys + yl / 2) + dy),
                                # xycoords='axes points',
                                horizontalalignment='center', verticalalignment='left',
                                fontsize=13)

        #else, if the traced packet is not found in the actual line, we sould look for it
        #in the lines that follows
        elif counter_plot_packet_trace < len(self.pck_trace_net_device):

            #iterate over all the following elements until it finds a packet it is looking for
            for i in range(counter_plot_packet_trace + 1, len(self.pck_trace_net_device), 1):
                line = self.pck_trace_net_device[i]
                #checks if it is or not a packet it is looking for,

                if int(line[2]) in packets_traced and int(line[3]) == 1:
                    counter_plot_packet_trace = i
                    #iteractively calls this function to be executed once with the
                    # new iterator value, and stop executing it. Will only be executed again
                    #if the button is pressed or the timer slaps in the animation...
                    self.PlotPacketTraceFrame()
                    return

        # self.ax_pt.axes.relim()
        # self.ax_pt.axes.autoscale_view(True, True, True)

        self.canvas_pt.draw()
        counter_plot_packet_trace += 1

        # next_line = self.pck_trace_net_device[counter_plot_packet_trace]
        # if t == int(next_line[1]): #time it happen
        #     self.PlotPacketTraceFrame()


    def pbStepPacketTrace_clicked(self):
        self.PlotPacketTraceFrame()

    def pbPlotPacketTrace_clicked(self):
        global counter_plot_packet_trace, packets_traced, t_initial, t_previous, t_packet

        #check which is the packet to be traced
        # packets_traced = \

        try:
            packets_traced_tx = self.tbPacketToTrace.text()
            packets_traced = map(int, packets_traced_tx.split(","))
            # packets_traced
        except:
            QtGui.QMessageBox.warning(self, 'Error',
                                      "Make sure you have inserted comma separated integers, without blank spaces.",
                                      QtGui.QMessageBox.Ok)
            #
            return
        #read files first seeking for a list of flows in specific.
        self.pck_trace_net_device = self.ReadPacketTraceNetDevice()

        counter_plot_packet_trace = 0
        t_initial = -1
        t_packet = 14400  #0,144 ms = 144000 ns
        # t_packet = 104000 #for the network discovery packet

        self.fig_pt.clear()

        self.ax_pt = self.fig_pt.add_subplot(111, autoscale_on=False, xlim=(0, self.network_size_x),
                                             ylim=(self.network_size_y, 0))
        self.canvas_pt.draw()
        # self.ax_pt = self.fig_pt.add_subplot(111, autoscale_on=True)
        ################################################


    def load_data_from_image(self, file):
        #example: http://code.activestate.com/recipes/577591-conversion-of-pil-image-and-numpy-array/

        # file = '/home/joao/usn-data/images/flow-bw.png'
        im = Image.open(file)
        # im.show()
        # im = im.convert('I') #reads the image and put it in a int32bits map
        # im.show()
        im = im.resize((self.network_size_x, self.network_size_y), Image.ANTIALIAS)
        # im.show()

        # luminance standard calulation
        # 0.2126 * R + 0.7152 * G + 0.0722 * B

        if im.mode == 'RGBA':
            npimg = np.array(im)
            npimg = npimg[..., 0]*0.299 + npimg[..., 1]*0.587 + npimg[..., 2]*0.114
        elif im.mode == 'LA':
            npimg = np.array(im.convert('RGBA'))
            npimg = npimg[..., 0]*0.299 + npimg[..., 1]*0.587 + npimg[..., 2]*0.114

        # npim = np.asarray(im)
        # npim = np.array(im.getdata(), np.uint8).reshape(im.size[1], im.size[0], 3)
        # npim = np.array(im)

        npimg = self.normalize_data(npimg)


        # pylab.imshow(npimg, cmap=plt.cm.gray, interpolation='nearest')
        # pylab.show()


        # First test to switch this function to open CV
        # im = cv2.imread(file)
        #
        # # im_grey = im[..., 0]*0.299 + im[..., 1]*0.587 + im[..., 2]*0.114
        #
        # plt.imshow(im)
        #
        # im_resized = cv2.resize(im, (self.network_size_x, self.network_size_y))
        #
        # plt.imshow(im_resized)
        #
        # plt.show()
        #
        # npimg = self.normalize_data(im_resized)

        return npimg

    def find_contour(self, file, show=0, from_file=1):
        if from_file == 1:
            im_orig = cv2.imread(file,0) #0 to read black and white
        else:
            im_orig = file
        # ret,thresh = cv2.threshold(im_orig,127,255,0)
        # contours,hierarchy = cv2.findContours(thresh, 1, 2)
        # cnt = contours[0]
        # M = cv2.moments(cnt)
        # print M

        im_orig_shape = (im_orig.shape[1], im_orig.shape[0]) #from col and row, to x and y

        ret, im_bin = cv2.threshold(im_orig, 75, 255, cv2.THRESH_BINARY)

        # edges = cv2.Canny(bin, 100, 200)
        contours, hierarchy = cv2.findContours(im_bin.copy(), cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

        areas = [cv2.contourArea(c) for c in contours]
        max_index = np.argmax(areas)

        #get outern most countour only



        im_contours = np.ones(im_orig_shape) * 255
        cv2.drawContours(im_contours, contours, -1, (0,255,0), 1)

        im_contours_final = np.ones(im_orig_shape) * 255 #All white
        cv2.drawContours(im_contours_final, contours, max_index, (0, 255, 0), 1)

        # test= []
        # test.append(contours_outern)
        #

        halfs = np.vsplit(im_contours_final,2)
        contour_outermost1 = self.find_outermost_point(halfs[0], direction=1)
        contour_outermost2 = self.find_outermost_point(halfs[1], direction=0)[::-1]
        contour_outermost2[...,1] += halfs[0].shape[0]
        contour_outermost = np.vstack([contour_outermost1, contour_outermost2])
        temp = []
        temp.append(contour_outermost)

        im_contours_outermost = np.ones(im_orig_shape) * 255 #All white
        cv2.drawContours(im_contours_outermost, temp, -1, (0, 255, 0), 10)

        # if show == 1:
            # plt.subplot(221)
            # plt.imshow(im_orig, cmap='gray')
            # plt.subplot(222)
            # plt.imshow(im_bin)
            # plt.subplot(223)
            # plt.imshow(im_contours, cmap='gray')
            # plt.subplot(224)
            # plt.imshow(im_contours_outermost, cmap='gray')
            # plt.show()

        dir = "/home/joao/usn-data/plots/"

        self.image_show(im_orig, dir + 'input-orig.eps', axis='off', view=0)
        self.image_show(im_bin, dir + 'input-bin.eps', axis='off', view=0)
        self.image_show(im_contours_outermost, dir + 'input-contour.eps', colormap='hot', axis='off', view=0)

        return np.uint8(im_contours_outermost.copy())

    def compare_contours(self, im1, im2, show=0):
    # def compare_contours(self):

        # im2 = self.countour_image

        ret, im1_bin = cv2.threshold(np.uint8(im1), 0, 255, cv2.THRESH_BINARY_INV)
        im1_r = cv2.resize(im1_bin, (im2.shape[1], im2.shape[0]), interpolation = cv2.INTER_AREA)

        im3 = self.convert_to_line(im1_r)

        # cv2.imwrite(self.working_base_dir + 'contour-ref.png', im2)
        # cv2.imwrite(self.working_base_dir + 'contour-detected.png', im3)

        # im2 = cv2.imread(self.working_base_dir + 'contour-ref.png',0)
        # im3 = cv2.imread(self.working_base_dir + 'contour-detected.png',0)

        im2_halfs = np.vsplit(im2, 2)
        im3_halfs = np.vsplit(im3, 2)

        r1 = self.find_vertical_distances(im2_halfs[0], im3_halfs[0])
        r2 = self.find_vertical_distances(im2_halfs[1], im3_halfs[1])*-1

        # t1 = thread.start_new(self.find_vertical_distances, args = [im2_halfs[0], im3_halfs[0]])
        # t2 = thread.start_new(self.find_vertical_distances, args = [im2_halfs[1], im3_halfs[1]])

        r = np.hstack([r1, r2])

        file_name = self.working_dir + "/out/contour-diff.csv"
        self.print_to_file(file_name,r)
        # r = r1

        if show == 1:
            # plt.hist(r, 20, normed=1, facecolor='green', alpha=0.5, cumulative=True)
            # plt.show()

            # plt.imshow(cv2.add(im2/2, im1_r/2), cmap='gray')
            # plt.show()
            # plt.imshow(cv2.add(im1_r/2, im3/2), cmap='gray')
            # plt.show()
            # plt.imshow(cv2.add(im2/2, im3/2), cmap='gray')
            # plt.show()
            dir = self.working_base_dir + '/plots/'
            ext = self.working_context + ".eps"
            self.image_show(self.data1, dir + 'output-orig' + ext, axis='off', view=False)
            self.image_show(im1, dir + 'output-bin' + ext, axis='off', view=False)
            self.image_show(im3, dir + 'output-contour' + ext, colormap='hot', axis='off', view=False)
            self.image_show(cv2.add(im2/4, im3/2), dir + 'output-contour-ref' + ext, colormap='gray', axis='off', view=True)


        #return also the difference in inside area


        # cv2.imshow('Debug', im1_r)

    def convert_to_line(self, im):

        # im = cv2.imread('/home/joao/usn-data/images/detected-101x101.png',0)
        # cv2.imwrite('/home/joao/usn-data/images/detected-101x101.png', im)

        halfs = np.vsplit(im, 2) #array to aplit, and the indice

        im_part_line = np.uint8(np.ones(im.shape) * 255) #all white

        countour = []

        # countour1 = self.find_mean_point(halfs[0])
        # countour2 = self.find_mean_point(halfs[1])[::-1] #invert the second, so they can be joined together
        # countour2[...,1] += halfs[0].shape[0] #displace all the elements to the right position on the
        # countour = np.vstack([countour1, countour2])

        countour1 = self.find_outermost_point(halfs[0], 1) #from top to bottom
        countour2 = self.find_outermost_point(halfs[1], 0)[::-1] #from bottom to top
        countour2[...,1] += halfs[0].shape[0] #displace all the elements to the right position on the
        countour = np.vstack([countour1, countour2])

        # countour = cv2.approxPolyDP(countour.copy(), 10, closed=True)

        to_remove = []
        for i in range(2,len(countour)-3):
            fx_aa = countour[i-2][1]
            fx_a = countour[i-1][1]
            fx = countour[i][1]
            fx_n = countour[i+1][1]
            fx_nn = countour[i+2][1]

            if self.delta(fx, fx_a) > 100 or self.delta(fx, fx_n) > 100 or \
               self.delta(fx, fx_aa) > 100 or self.delta(fx, fx_nn) > 100:
                to_remove.append(i)

        countourf = np.array(np.delete(countour, to_remove, axis=0))

        # countourcv, hierarchy = cv2.findContours(im.copy(), cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
        #
        # areas = [cv2.contourArea(c) for c in countourcv]
        # max_index = np.argmax(areas)

        test = []
        test.append(countourf)
        cv2.drawContours(im_part_line, test, -1, (0, 200, 0), 10)
        # cv2.drawContours(im_part_line, countourcv, max_index, (0, 200, 0), 10)

        # cv2.blur(im_part_line, test, 10)

        # plt.imshow(im_part_line, cmap='gray')
        # plt.show()
        return im_part_line

    def delta(self, v1, v2):
        if v1 > v2: return np.abs(v1-v2)
        if v2 > v1: return np.abs(v2-v1)

    def find_mean_point(self, im):
        # do it for the first half first
        im_part_line = np.uint8(np.ones((im.shape[0], im.shape[1])) * 255) #all white
        contour_points = []

        for j in range(0, im.shape[1]-1): # loop over the different columns
            first = 0;
            last = 0;
            for i in range(0, im.shape[0]-1): # look for a black pixel in a vertical line, from top to bottom
                if im[i][j] == 0: # if black, for the first time
                    if first == 0: # save the highest j and lowes till now
                        first = i
                        last = i
                    if i > last: #if it is black, and below the previous last j, overwrite it
                        last = i

            #after percurring an entire row, calculate the central point of it
            if (first != 0 and last != 0):
                mean = (last - first) / 2 + first -1
                # im_part_line[mean][j] = 0 # paint the itermediate pixel in black
                # if len(contour_points) > 0:
                    # if contour_points[len(contour_points)-1][1] != mean:
                contour_points.append([j, mean]) #array containing the contour points
                # else:
                #     contour_points.append([j, mean]) #array containing the contour points
        # for p in contour_points:

        contour_points_np = np.array(contour_points)
        r = contour_points_np
        # r = cv2.approxPolyDP(contour_points_np, 1.5, closed=False)

        # for p in r:
        #     im_part_line[p[0]][p[1]] = 0 # paints the corresponding


        return r

    def find_outermost_point(self, im, direction=1):
        points = []
        for j in range(0, im.shape[1]-1): # loop over the different columns
            first = 0;
            last = 0;
            for i in range(0, im.shape[0]-1): # look for a black pixel in a vertical line, from top to bottom
                if im[i][j] == 0: # if black, for the first time
                    if first == 0: # save the highest j and lowes till now
                        first = i
                        last = i
                    if i > last: #if it is black, and below the previous last j, overwrite it
                        last = i

            #after percurring an entire row, calculate the central point of it
            if (first != 0 and last != 0):# and len(points) == 0:
                if direction == 1:
                    points.append([j, first]) #array containing the contour points
                elif direction == 0:
                    points.append([j, last]) #array containing the contour points

            # elif (first != 0 and last != 0) and len(points) > 0:
            #     if direction == 1:
            #         if last < points[len(points) -1][1] * 1.8 and last >  points[len(points) -1][1] * 0.2:
            #             points.append([j, first]) #array containing the contour points
            #     elif direction == 0:
            #         if last < points[len(points) -1][1] * 1.8 and last >  points[len(points) -1][1] * 0.2:
            #             points.append([j, last]) #array containing the contour points


        r = np.array(points)

        return r

    def find_vertical_distances(self, im1, im2):
        if im1.shape == im2.shape:
            r = []
            for j in range(0, im1.shape[1]-1): # loop over the different columns
                y_im1 = 0;
                y_im2 = 0;
                for i in range(0, im1.shape[0]-1): # look for a black pixel in a vertical line, from top to bottom
                    if im1[i][j] == 0 and y_im1 == 0: # if black, for the first time
                        y_im1 = i
                    if im2[i][j] == 0 and y_im2 == 0:
                        y_im2 = i

                if y_im1 != 0 and y_im2 != 0:
                    # if y_im1 > y_im2: d=y_im1 - y_im2
                    # else: d = y_im2 - y_im1
                    d = y_im1-y_im2
                    # if d < 100 and d > -100:
                    r.append(d)

            return r

        else:
            print 'ERROR - images do not have the same shape'
            return -1




app = QtGui.QApplication(sys.argv)
myWindow = MyWindowClass(None)
myWindow.show()
app.exec_()
# print "end"