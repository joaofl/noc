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

from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import * #QWidget, QApplication
from os.path import expanduser
import os
import glob

class MyFileList(QWidget):
    def __init__(self):
        QWidget.__init__(self)
        # setGeometry(x_pos, y_pos, width, height)
        self.setGeometry(300, 200, 300, 500)

        self.label_dir = QLabel("Base directory:")
        # pick a directory you have as default
        #self.edit_dir = QLineEdit("C:/Temp")  # Windows
        #self.edit_dir = QLineEdit(os.path.abspath("Extra"))  # OS X
        dir = expanduser("~") + '/noc-data'
        # self.edit_dir = QLineEdit(os.getcwd())
        self.edit_dir = QLineEdit(dir)

        self.button_load = QPushButton("Load")
        # bind the button click to a function reference
        # newer connect style used with PyQT 4.5 and higher
        self.button_load.clicked.connect(self.on_click)

        self.listbox = QListWidget()
        # new connect style, needs PyQt 4.5+
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

    def on_select(self):
        """
        an item in the listbox has been clicked/selected
        """
        selected = self.listbox.currentItem().text()
        self.label_result.setText(selected)

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
        #mask = "*.jpg"  # .jpg files only
        mask = "*.*"    # all files
        for path in glob.glob(mask):
            dirname, filename = os.path.split(path)
            #print(filename)  # test
            self.list_fnames.append(filename)
        self.listbox.addItems(self.list_fnames)
        sf = "{} items loaded".format(len(self.list_fnames))
        self.setWindowTitle(sf)
        pass


app =  QApplication([])
mfl = MyFileList()
mfl.show()
app.exec_()