__author__ = 'joao'


from PyQt4.uic.properties import QtCore
import sys
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import uic, QtGui



form_class = uic.loadUiType("noc.ui")[0]  # Load the UI

class noc_ui(QtGui.QMainWindow, form_class):
    def __init__(self):
        super(noc_ui, self).__init__()
        self.setupUi(self)

        # self.pbGenerateConfig.clicked.connect(self.pbGenerateConfig_clicked)

        self.show()

    def pbGenerateConfig_clicked(self):
        print "ok"


app = QtGui.QApplication(sys.argv)
ui = noc_ui()

r = app.exec_()
sys.exit(r)