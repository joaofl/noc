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

from PyQt5.QtWidgets import *
from os.path import expanduser
import files_io
import os
import _thread
from time import sleep


class NOCLauncher(QWidget):

    selected_log = ''
    selected_script = ''

    def __init__(self):
        QWidget.__init__(self)
        # setGeometry(x_pos, y_pos, width, height)
        self.setGeometry(300, 200, 300, 630)

        self.setWindowTitle('NoC4ns3 Launcher')

        self.label_dir = QLabel("Base directory:")
        dir = expanduser("~") + '/noc-data/'
        # self.edit_dir = QLineEdit(os.getcwd()) #current dir
        self.base_dir = QLineEdit(dir)

        self.button_load = QPushButton("Load log files")
        self.button_load.clicked.connect(self.on_click_load)

        self.listbox = QListWidget()
        self.listbox.setSelectionMode(QAbstractItemView.ExtendedSelection)
        self.listbox.clicked.connect(self.on_select)

        self.listbox_scripts = QListWidget()
        self.listbox_scripts.clicked.connect(self.on_select_scripts)

        self.label_result = QLabel()
        self.label_scripts = QLabel()

        self.label_title1 = QLabel()
        self.label_title1.setText('Log files found:')
        self.label_title2 = QLabel()
        self.label_title2.setText('Script files found:')

        self.textbox_args = QLineEdit()
        self.textbox_args_additional = QLineEdit()

        # self.button_load_anim = QPushButton("Load animation")
        # self.button_load_anim.clicked.connect(self.on_click_load_anim)

        self.button_run = QPushButton("Run selected script")
        self.button_run.clicked.connect(self.on_click_run)

        # self.button_plot_service = QPushButton("Plot service curves")
        # self.button_plot_service.clicked.connect(self.on_click_plot_service)

        # layout the widgets (vertical)
        vbox = QVBoxLayout()
        vbox.addWidget(self.label_dir)
        vbox.addWidget(self.base_dir)
        vbox.addWidget(self.button_load)
        vbox.addWidget(self.label_title1)
        vbox.addWidget(self.listbox)
        vbox.addWidget(self.label_result)
        # vbox.addWidget(self.button_load_anim)
        vbox.addWidget(self.label_title2)
        vbox.addWidget(self.listbox_scripts)
        vbox.addWidget(self.label_scripts)
        vbox.addWidget(self.textbox_args)
        vbox.addWidget(self.textbox_args_additional)
        vbox.addWidget(self.button_run)
        # vbox.addWidget(self.button_plot_service)
        self.setLayout(vbox)

        # noc_anim.QApplication.exec()

        self.on_click_load()

    def on_select(self):
        """
        an item in the listbox has been clicked/selected
        """

        # self.selected = self.listbox.currentItem().text()
        i = self.listbox.currentRow()
        self.selected_log = self.list_fnames[i]
        self.selected_dir = self.list_dir_names[i]

        self.label_result.setText(self.list_vnames[i] + ' selected')

        args = files_io.load_line(self.list_dir_names[i] + 'simulation-info.txt')

        j = self.list_vnames[i].rfind('s')
        args += ' --shaper=' + self.list_vnames[i][j+1:j+3]

        self.textbox_args.setText(args)

    def on_select_scripts(self):

        i = self.listbox_scripts.currentRow()
        self.selected_script = self.list_scripts_fnames[i]

        self.label_scripts.setText(self.selected_script.split('/')[-1] + ' selected')

    def on_click_load(self):

        ############ Load the logs

        directory = self.base_dir.text()

        self.selected_log = ''
        self.selected_dir = ''

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
            n = item.split('/')[-3]
            self.list_vnames.append(n)
            d = item.rsplit('/',1)[0]
            self.list_dir_names.append(d + '/')

        self.listbox.addItems(self.list_vnames)
        sf = "{} log files found".format(len(self.list_fnames))
        self.label_result.setText(sf)

        # l = files_io.load_line(directory + 'post_simulation_config.csv')
        self.textbox_args_additional.setText('--pos_x=5 --pos_y=6 --port=2 --showplots=True')


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

        #Check the ones selected
        # items = self.listbox.selectedItems()
        indexes = self.listbox.selectedIndexes()

        cmd_list = []

        for item in indexes:
            i = item.row()

            print('Running command {} out of {} ({:0.2f}%)'.format(i, len(indexes), (i * 100) / len(indexes) ))

            args = files_io.load_line(self.list_dir_names[i] + 'simulation-info.txt')

            j = self.list_vnames[i].rfind('s')
            args += ' --shaper=' + self.list_vnames[i][j+1:j+3]

            cmd = 'python3.5 ' + self.selected_script + \
                    ' --inputdir=' + self.list_dir_names[i] + ' --outputdir=' + self.list_dir_names[i] +  'post/ ' + \
                    ' --basedir=' + self.base_dir.text() + ' ' + \
                    args + ' ' + self.textbox_args_additional.text()

            try:

                launch_thread = True

                if (launch_thread == True):
                    _thread.start_new_thread(os.system,(cmd,))
                    print('Thread launched with command: ' + cmd)
                    sleep(1)

                    while _thread._count() >= 2: #how many threads in parallel (usually the number of cores, unless the
                                                #thread called is multi-thread already
                        sleep(1)
                else:
                    print('Wait for command execution: ' + cmd)
                    os.system(cmd)
            except:
                print('Error running command')


    def on_click_load_anim(self):
        self.anim = noc_anim.NOCAnim()
        self.anim.show()

        self.anim.inputfile = self.selected_log

        self.anim.initData()



app =  QApplication([])
mfl = NOCLauncher()
mfl.show()

app.exec_()
