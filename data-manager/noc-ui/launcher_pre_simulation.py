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
# from PyQt5.QtGui import *
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
        self.setGeometry(300, 200, 300, 400)

        self.setWindowTitle('NoC4ns3 Pre Simulation Launcher')

        self.label_dir = QLabel("NS-3 base dir:")
        dir = expanduser("~") + '/ns-3'
        # self.edit_dir = QLineEdit(os.getcwd()) #current dir
        self.edit_dir = QLineEdit(dir)

        self.label_dir = QLabel("Input data base dir:")
        dir = expanduser("~") + '/noc-data/input-data/'
        # self.edit_dir = QLineEdit(os.getcwd()) #current dir
        self.edit_dir = QLineEdit(dir)

        self.button_load = QPushButton("Load input data files")
        self.button_load.clicked.connect(self.on_click_load)

        self.listbox = QListWidget()
        self.listbox.clicked.connect(self.on_select)

        self.listbox_scripts = QListWidget()
        self.listbox_scripts.clicked.connect(self.on_select_scripts)

        self.label_result = QLabel()
        self.label_scripts = QLabel()

        self.label_title1 = QLabel()
        self.label_title1.setText('Data files found:')
        self.label_title2 = QLabel()
        self.label_title2.setText('Simulations to run:')

        self.textbox_args = QLineEdit()

        # self.button_load_anim = QPushButton("Load animation")
        # self.button_load_anim.clicked.connect(self.on_click_load_anim)

        self.button_run = QPushButton("Run selected script")
        self.button_run.clicked.connect(self.on_click_run)

        # self.button_plot_service = QPushButton("Plot service curves")
        # self.button_plot_service.clicked.connect(self.on_click_plot_service)

        # layout the widgets (vertical)
        vbox = QVBoxLayout()
        vbox.addWidget(self.label_dir)
        vbox.addWidget(self.edit_dir)
        vbox.addWidget(self.button_load)
        vbox.addWidget(self.label_title1)
        vbox.addWidget(self.listbox)
        vbox.addWidget(self.label_result)
        # vbox.addWidget(self.button_load_anim)
        vbox.addWidget(self.label_title2)
        vbox.addWidget(self.listbox_scripts)
        vbox.addWidget(self.label_scripts)
        vbox.addWidget(self.textbox_args)
        vbox.addWidget(self.button_run)
        # vbox.addWidget(self.button_plot_service)
        self.setLayout(vbox)

        # noc_anim.QApplication.exec()

    def on_select(self):
        """
        an item in the listbox has been clicked/selected
        """
        # self.selected = self.listbox.currentItem().text()
        i = self.listbox.currentRow()
        self.selected_log = self.list_fnames[i]

        self.label_result.setText(self.list_vnames[i] + ' selected')

        args = files_io.load_line(self.list_dir_names[i] + 'simulation-info.txt')
        self.textbox_args.setText(args)

    def on_select_scripts(self):

        i = self.listbox_scripts.currentRow()
        self.selected_script = self.list_scripts_fnames[i]

        self.label_scripts.setText(self.selected_script.split('/')[-1] + ' selected')

    def on_click_load(self):

        ############ Load the logs

        directory = self.edit_dir.text()

        if (os.path.isdir(directory)):
            self.listbox.clear()
        else:
            self.listbox.clear()
            self.label_result.setText("No such directory")
            return

        # create a list of all files in a given directory
        self.list_fnames = []
        self.list_vnames = []
        self.list_dir_names = []


        self.list_fnames = files_io.find_multiple_files(directory, "packets-trace")

        for item in self.list_fnames:
            n = item.split('/')[4]
            self.list_vnames.append(n)
            d = item.rsplit('/',1)[0]
            self.list_dir_names.append(d + '/')

        self.listbox.addItems(self.list_vnames)
        sf = "{} log files found".format(len(self.list_fnames))
        self.label_result.setText(sf)


        ############## Load the scripts

        directory = os.getcwd()

        self.list_scripts_fnames = files_io.find_multiple_files(directory, "show")
        self.list_scripts_vnames = []

        for item in self.list_scripts_fnames:
            self.list_scripts_vnames.append(item.split('/')[-1].split('.')[0].replace('show_', ''))

        self.listbox_scripts.clear()
        self.listbox_scripts.addItems(self.list_scripts_vnames)


        pass




    def on_click_run(self):
        # scriptname = 'noc_flow_analysis'

        args = ' --inputfile=' + self.selected_log + ' ' + self.textbox_args.text()

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