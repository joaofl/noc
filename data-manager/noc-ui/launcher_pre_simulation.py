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

# from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import * #QWidget, QApplication
from os.path import expanduser
import files_io
# import noc_
import os
import glob
import _thread


class NOCPreLauncher(QWidget):

    selected_log = ''
    selected_script = ''

    def __init__(self):
        QWidget.__init__(self)
        # setGeometry(x_pos, y_pos, width, height)
        self.setGeometry(300, 200, 250, 600)

        font_title = QFont()
        font_title.setBold(True)

        self.setWindowTitle('NoC4ns3 Pre Simulation Launcher')

        ##########################################################
        self.label_ns3_dir = QLabel("NS-3 base dir:")
        self.label_ns3_dir.setFont(font_title)
        dir = expanduser("~") + '/Repositorios/ns-3-dev'
        # self.edit_dir = QLineEdit(os.getcwd()) #current dir
        self.edit_ns3_dir = QLineEdit(dir)

        ##########################################################
        self.label_input_param = QLabel("Simulation input parameters:")
        self.label_input_param.setFont(font_title)

        self.label_param_network_size = QLabel("Network size [x,y]:")
        self.textbox_param_xsize = QLineEdit()
        self.textbox_param_ysize = QLineEdit()

        ##########################################################
        self.label_dir = QLabel("Simulation input data:")
        self.label_dir.setFont(font_title)

        dir = expanduser("~") + '/noc-data/input-data'
        # self.edit_dir = QLineEdit(os.getcwd()) #current dir
        self.edit_dir = QLineEdit(dir)

        self.button_load = QPushButton("Load input data files")
        self.button_load.clicked.connect(self.on_click_load)

        self.button_import = QPushButton("Import...")
        self.button_import.clicked.connect(self.on_click_import)

        self.listbox_data_files = QListWidget()
        self.listbox_data_files.clicked.connect(self.on_select)
        self.label_data_files = QLabel('Click on load to find files...')

        ##########################################################

        # self.listbox_scripts = QListWidget()
        # self.listbox_scripts.clicked.connect(self.on_select_scripts)


        # self.label_scripts = QLabel()

        # self.label_title1 = QLabel()
        # self.label_title1.setText('Data files found:')
        self.label_simulation_command = QLabel('Simulation command:')
        self.label_simulation_command.setFont(font_title)

        self.textbox_command = QTextEdit()

        self.button_add_to_batch = QPushButton("Add for batch processing")
        self.button_add_to_batch.clicked.connect(self.on_click_add)

        self.label_batch = QLabel('Simulations to run:')
        self.label_batch.setFont(font_title)
        self.listbox_batch = QListWidget()
        self.listbox_batch.clicked.connect(self.on_select)
        self.label_batch_selected = QLabel('Select one or more simuation to run...')
        self.button_run_batch = QPushButton("Run selected")
        self.button_run_batch.clicked.connect(self.on_click_run)


        ###################### Displacing items ##################
        vbox = QVBoxLayout()
        ##########################################################
        vbox.addWidget(self.label_ns3_dir)
        vbox.addWidget(self.edit_ns3_dir)
        ##########################################################
        vbox.addWidget(self.label_input_param)
        hbox = QHBoxLayout()
        hbox.addWidget(self.label_param_network_size)
        hbox.addWidget(self.textbox_param_xsize)
        hbox.addWidget(self.textbox_param_ysize)
        vbox.addItem(hbox)
        ##########################################################
        vbox.addWidget(self.label_dir)
        vbox.addWidget(self.edit_dir)
        hbox = QHBoxLayout()
        hbox.addWidget(self.button_load)
        hbox.addWidget(self.button_import)
        vbox.addItem(hbox)
        vbox.addWidget(self.listbox_data_files)
        vbox.addWidget(self.label_data_files)
        ##########################################################
        vbox.addWidget(self.label_simulation_command)
        vbox.addWidget(self.textbox_command)
        vbox.addWidget(self.button_add_to_batch)
        ##########################################################
        vbox.addWidget(self.label_batch)
        vbox.addWidget(self.listbox_batch)
        vbox.addWidget(self.label_batch_selected)
        vbox.addWidget(self.button_run_batch)

        self.setLayout(vbox)

        # noc_anim.QApplication.exec()

    def on_select(self):
        """
        an item in the listbox has been clicked/selected
        """
        # self.selected = self.listbox.currentItem().text()
        i = self.listbox_data_files.currentRow()
        self.selected_log = self.list_fnames[i]

        self.label_data_files.setText(self.list_vnames[i] + ' selected')

        # args = files_io.load_line(self.list_dir_names[i] + 'simulation-info.txt')
        # self.textbox_args.setText(args)

    def on_click_load(self):

        ############ Load the logs

        directory = self.edit_dir.text()

        if (os.path.isdir(directory)):
            self.listbox_data_files.clear()
        else:
            self.listbox_data_files.clear()
            self.label_data_files.setText("No such directory")
            return

        # create a list of all files in a given directory
        self.list_fnames = []
        self.list_vnames = []
        self.list_dir_names = []


        self.list_fnames = files_io.find_multiple_files(directory, ".csv")

        for item in self.list_fnames:
            n = item.split('/')[-1]
            self.list_vnames.append(n)
            d = item.rsplit('/',1)[0]
            self.list_dir_names.append(d + '/')

        self.listbox_data_files.addItems(self.list_vnames)
        sf = "{} log files found".format(len(self.list_fnames))
        self.label_data_files.setText(sf)


        ############## Load the scripts

        directory = os.getcwd()

        self.list_scripts_fnames = files_io.find_multiple_files(directory, "show")
        self.list_scripts_vnames = []

        for item in self.list_scripts_fnames:
            self.list_scripts_vnames.append(item.split('/')[-1].split('.')[0].replace('show_', ''))

        # self.listbox_scripts.clear()
        # self.listbox_scripts.addItems(self.list_scripts_vnames)


        pass

    def on_click_import(self):
        print('Call the other script to do that')

    def on_click_add(self):
        print('Add to list')

    def on_click_run(self):
        # scriptname = 'noc_flow_analysis'

        args = ' --inputfile=' + self.selected_log + ' ' + self.textbox_command.text()

        cmd = 'python3.4 ' + self.selected_script + args
        print('Command executed: ' + cmd)

        launch_thread = True

        if (launch_thread == True):
            _thread.start_new_thread(os.system,(cmd,))
        else:
            os.system(cmd)

    # def pbRunSimulation_clicked(self):
    #     #self.pbGenerate_clicked()
    #     #TODO: replace the fixed string with the appropriated var
    #     self.teCommand.setText('Starting simulation. Please be patient')
    #     cmd = './waf --run "src/noc/examples/noc-example $(cat /home/joao/usn-data/config/input-config.c.csv)"'
    #     cmd_dir = self.tbNs3Dir.text()
    #
    #     thread.start_new_thread(self.lunchSimulationProcess, (cmd,cmd_dir))
    #     with file(self.config_file, 'r') as f:
    #         self.teCommand.setText('Simulation running with the parameters:\n' + f.read())

        # config = (
        #     " --size_x=" + str(self.network_size_x) +
        #     " --size_y=" + str(self.network_size_y) +
        #     " --size_neighborhood=" + str(self.neighborhood_size) +
        #     " --sinks_n=" + str(self.number_of_sinks) +
        #     " --sampling_cycles=" + str(self.number_of_cycles) +
        #     " --sampling_period=" + str(self.period) +
        #     " --baudrate=" + str(self.baudrate) +
        #     " --operational_mode=" + str(int(self.cbReadAll.isChecked())) +
        #     " --io_data_dir=" + str(self.working_dir)
        # )


    def on_click_load_anim(self):
        self.anim = noc_anim.NOCAnim()
        self.anim.show()

        self.anim.inputfile = self.selected_log

        self.anim.initData()



app =  QApplication([])
mfl = NOCPreLauncher()
mfl.show()

app.exec_()