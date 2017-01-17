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
import os
from subprocess import Popen, PIPE, call
from joblib import Parallel, delayed
import numpy as np
from time import sleep

class NOCPreLauncher(QWidget):

    selected_log = ''
    selected_script = ''

    def __init__(self):
        QWidget.__init__(self)
        # setGeometry(x_pos, y_pos, width, height)
        self.setGeometry(300, 200, 600, 600)

        font_title = QFont()
        font_title.setBold(True)

        self.setWindowTitle('NoC4ns3 Pre Simulation Launcher')

        ##########################################################
        dir_ns3 = expanduser("~") + '/Repositorios/ns-3-dev'
        dir_output = expanduser("~") + '/noc-data'

        self.label_ns3_dir = QLabel("NS-3 and Output dir")
        self.label_ns3_dir.setFont(font_title)
        # self.edit_dir = QLineEdit(os.getcwd()) #current dir
        self.textbox_ns3_dir = QLineEdit(dir_ns3)
        self.textbox_ns3_dir.editingFinished.connect(self.param_text_changed)

        self.textbox_dir_output = QLineEdit(dir_output)
        self.textbox_dir_output.editingFinished.connect(self.param_text_changed)

        ##########################################################
        self.label_input_param = QLabel("Input parameters")
        self.label_input_param.setFont(font_title)

        self.label_param_network_size = QLabel("Network size [x,y] ")
        self.textbox_param_xsize = QLineEdit()
        self.textbox_param_xsize.editingFinished.connect(self.param_text_changed)
        self.textbox_param_ysize = QLineEdit()
        self.textbox_param_ysize.editingFinished.connect(self.param_text_changed)

        self.label_param_sinks = QLabel("Sinks ")
        self.textbox_param_sinks = QLineEdit()
        self.textbox_param_sinks.editingFinished.connect(self.param_text_changed)

        self.label_param_nsize = QLabel("Neighb. size ")
        self.textbox_param_nsize = QLineEdit()
        self.textbox_param_nsize.editingFinished.connect(self.param_text_changed)

        self.label_param_baudrate = QLabel("Baudrate [kbps] ")
        self.textbox_param_baudrate = QLineEdit()
        self.textbox_param_baudrate.editingFinished.connect(self.param_text_changed)

        self.label_param_context = QLabel("Context ")
        self.textbox_param_context = QLineEdit()
        self.textbox_param_context.editingFinished.connect(self.param_text_changed)

        ##########################################################
        self.label_dir = QLabel("Input data:")
        self.label_dir.setFont(font_title)

        dir_ns3 = expanduser("~") + '/noc-data/input-data'
        # self.edit_dir = QLineEdit(os.getcwd()) #current dir
        self.edit_dir = QLineEdit(dir_ns3)

        self.button_load = QPushButton("Load input data files")
        self.button_load.clicked.connect(self.on_click_load)

        self.button_import = QPushButton("Import...")
        self.button_import.clicked.connect(self.on_click_import)

        self.listbox_data_files = QListWidget()
        self.listbox_data_files.clicked.connect(self.on_select)
        # self.label_data_files = QLabel('Click on load to find files...')

        ##########################################################

        # self.listbox_scripts = QListWidget()
        # self.listbox_scripts.clicked.connect(self.on_select_scripts)


        # self.label_scripts = QLabel()

        # self.label_title1 = QLabel()
        # self.label_title1.setText('Data files found:')
        self.label_simulation_command = QLabel('Run command:')
        self.label_simulation_command.setFont(font_title)

        self.textbox_command = QLineEdit()

        self.button_add_to_batch = QPushButton("Add to list")
        self.button_add_to_batch.clicked.connect(self.on_click_add)

        self.label_batch = QLabel('Simulations to be run:')
        self.label_batch.setFont(font_title)
        self.listbox_batch = QListWidget()
        self.listbox_batch.clicked.connect(self.on_select)
        # self.label_batch_selected = QLabel('Select one or more simuation to run...')
        self.button_run_batch = QPushButton("Run selected")
        self.button_run_batch.clicked.connect(self.on_click_run)


        ###################### Displacing items ##################
        vbox = QVBoxLayout()
        ##########################################################
        vbox.addWidget(self.label_ns3_dir)
        vbox.addWidget(self.textbox_ns3_dir)
        vbox.addWidget(self.textbox_dir_output)
        ##########################################################
        # vbox.addWidget(self.label_input_param)
        hbox = QHBoxLayout()
        hbox.addWidget(self.label_param_network_size)
        hbox.addWidget(self.textbox_param_xsize)
        hbox.addWidget(self.textbox_param_ysize)
        # vbox.addItem(hbox)

        hbox = QHBoxLayout()
        hbox.addSpacing(72)
        hbox.addWidget(self.label_param_sinks)
        hbox.addWidget(self.textbox_param_sinks)
        # vbox.addItem(hbox)

        hbox = QHBoxLayout()
        hbox.addSpacing(30)
        hbox.addWidget(self.label_param_nsize)
        hbox.addWidget(self.textbox_param_nsize)
        # vbox.addItem(hbox)

        hbox = QHBoxLayout()
        hbox.addSpacing(30)
        hbox.addWidget(self.label_param_baudrate)
        hbox.addWidget(self.textbox_param_baudrate)
        # vbox.addItem(hbox)

        hbox = QHBoxLayout()
        hbox.addSpacing(56)
        hbox.addWidget(self.label_param_context)
        hbox.addWidget(self.textbox_param_context)
        # vbox.addItem(hbox)
        ##########################################################
        vbox.addWidget(self.label_dir)
        vbox.addWidget(self.edit_dir)
        hbox = QHBoxLayout()
        hbox.addWidget(self.button_load)
        hbox.addWidget(self.button_import)
        vbox.addItem(hbox)
        vbox.addWidget(self.listbox_data_files)
        # vbox.addWidget(self.label_data_files)
        ##########################################################
        vbox.addWidget(self.label_simulation_command)
        vbox.addWidget(self.textbox_command)
        vbox.addWidget(self.button_add_to_batch)
        ##########################################################
        vbox.addWidget(self.label_batch)
        vbox.addWidget(self.listbox_batch)
        # vbox.addWidget(self.label_batch_selected)
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
        self.param_text_changed()

        # self.label_data_files.setText(self.list_vnames[i] + ' selected')

        # args = files_io.load_line(self.list_dir_names[i] + 'simulation-info.txt')

        # self.textbox_args.setText(args)

    def param_text_changed(self):
        # print('Text changed')
        params = ''
        try:
            v = int(self.textbox_param_xsize.text())
            params += '--size_x=' + str(v) + ' '
        except:
            self.textbox_param_xsize.setText('')

        try:
            v = int(self.textbox_param_ysize.text())
            params += '--size_y=' + str(v) + ' '
        except:
            self.textbox_param_ysize.setText('')

        try:
            v = int(self.textbox_param_sinks.text())
            params += '--sinks=' + str(v) + ' '
        except:
            self.textbox_param_sinks.setText('')

        try:
            v = int(self.textbox_param_nsize.text())
            params += '--size_n=' + str(v) + ' '
        except:
            self.textbox_param_nsize.setText('')

        try:
            v = int(self.textbox_param_baudrate.text())
            params += '--baudrate=' + str(v) + ' '
        except:
            self.textbox_param_baudrate.setText('')

        if self.textbox_param_context.text() != '':
            params += '--context=' + self.textbox_param_context.text() + ' '

        if self.selected_log != '':
            params += '--input_data=' + self.selected_log + ' '

        if self.textbox_dir_output.text() != '':
            params += '--output_data=' + self.textbox_dir_output.text() + ' '

        run_command = './waf --run="src/noc/examples/xdense-example '
        self.textbox_command.setText(run_command + params + '"')

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
        # sf = "{} log files found".format(len(self.list_fnames))
        # self.label_data_files.setText(sf)


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
        self.listbox_batch.clear()
        # print('Add to list')
        # t = self.textbox_command.text()
        # self.listbox_batch.addItem(t)
        # self.textbox_command.setText('')
        self.generate_scenarios()



    def run(self, cmd, dir):

        p = Popen(cmd, cwd=dir, shell=True, stdout=PIPE, universal_newlines=True)
        return p, p.communicate()

    def on_click_run(self):

        dir = self.textbox_ns3_dir.text()
        cmd_list = []
        process_list = []

        for i in range(0, self.listbox_batch.count()):
            cmd_list.append(str(self.listbox_batch.item(i).text()))

        # output = Parallel(n_jobs=8)(delayed(self.run)(cmd, dir) for cmd in cmd_list)

        p = Popen(cmd_list[0], cwd=dir, shell=True)  # , stdout=PIPE, universal_newlines=True)
        p.wait()


        for i, cmd in enumerate(cmd_list):

            print('Simulation scenario {} out of {} ({:0.2f}%)'.format(i+1, len(cmd_list), i*100/len(cmd_list)))

            # for cmd in cmds:
            p = Popen(cmd, cwd=dir, shell=True) #, stdout=PIPE, universal_newlines=True)
            # sleep(0.2)
            process_list.append(p)

            for p in process_list:
                p.wait()

            process_list = []
            # print(out)

            # for i, p in enumerate(process_list):
            #     p.wait()
            #     print('------------' + 'P' + str(i) + '------------')
            #     print(output_list[i])
            #

                # output_list = []


    def generate_scenarios(self):

        # dir = '/home/joao/noc-data/input-data'
        # file_to_run = 'src/noc/examples/xdense-full'
        # file_to_run = 'src/noc/examples/xdense-cluster'
        files_to_run = ['src/noc/examples/xdense-cluster', 'src/noc/examples/xdense-full']
        # context = 'WCA_ALL_'
        n_size_mask = [1, 2, 3, 4, 5]
        # n_size_mask = [3]
        # beta_mask = ['0.01', '0.02', '0.04', '0.05', '0.06', '0.08', '0.10', '0.50', '1.00']
        # beta_mask = ['0.10', '0.20', '0.30', '0.40', '0.50', '0.60', '0.70', '0.80', '0.90', '1.00']
        # beta_mask = ['{:0.2f}'.format(b/100) for b in range(1,101, 2)]
        beta_mask = ['{:0.4f}'.format(b/100) for b in np.logspace(0, 2, 30)]

        # beta_num = sorted([(101-b)/100 for b in np.logspace(0, 2, 30)])
        # beta_mask = ['{:0.04f}'.format(b) for b in beta_num]
        shapers_mask = ['BU', 'RL', 'TD', 'TL']
        # beta_mask = ['0.90', '1.00']
        # shapers_mask = ['BU', 'RL']



        for b in beta_mask:
            for n in n_size_mask:
                for s in shapers_mask:
                    for file in files_to_run:
                        self.listbox_batch.addItem('./waf --run="{} --beta={} --size_n={} --extra={}" '.format(file, b,n,s))


        print('Total number of sim scenarios: {}'.format(self.listbox_batch.count()))


app =  QApplication([])
mfl = NOCPreLauncher()
mfl.show()

app.exec_()