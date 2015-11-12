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
import noc_io
import noc_anim
import os
import glob

class NOCLauncher(QWidget):
    def __init__(self):
        QWidget.__init__(self)
        # setGeometry(x_pos, y_pos, width, height)
        self.setGeometry(300, 200, 300, 500)

        self.label_dir = QLabel("Base directory:")
        dir = expanduser("~") + '/noc-data'
        # self.edit_dir = QLineEdit(os.getcwd()) #current dir
        self.edit_dir = QLineEdit(dir)

        self.button_load = QPushButton("Load")
        self.button_load.clicked.connect(self.on_click)

        self.listbox = QListWidget()
        self.listbox.clicked.connect(self.on_select)

        self.label_result = QLabel()

        # layout the widgets (vertical)
        vbox = QVBoxLayout()
        vbox.addWidget(self.label_dir)
        vbox.addWidget(self.edit_dir)
        vbox.addWidget(self.button_load)
        vbox.addWidget(self.listbox)
        vbox.addWidget(self.label_result)
        self.setLayout(vbox)

        # noc_anim.QApplication.exec()

    def on_select(self):
        """
        an item in the listbox has been clicked/selected
        """
        selected = self.listbox.currentItem().text()
        self.label_result.setText(selected)

        anim.inputfile = selected

    def on_click(self):
        directory = self.edit_dir.text()
        try:
            # make it the working directory
            os.chdir(directory)
        except FileNotFoundError:
            self.label_result.setText("No such directory")
            self.listbox.clear()
            return
        # create a list of all files in a given directory
        self.list_fnames = []

        self.list_fnames = noc_io.find_multiple_files(directory, "packets-trace")

        self.listbox.addItems(self.list_fnames)
        sf = "{} items loaded".format(len(self.list_fnames))
        self.setWindowTitle(sf)
        pass

        # print ()




app =  QApplication([])
mfl = NOCLauncher()
mfl.show()

anim = noc_anim.NOCAnim()
anim.show()

app.exec_()