#!/usr/bin/python3
# -*- coding: utf-8 -*-

"""
ZetCode PyQt5 tutorial

This example draws three rectangles in three
#different colours.

author: Jan Bodnar
website: zetcode.com
last edited: January 2015
"""

import sys
import noc_io
import noc_packet_structure as trace
from os.path import expanduser

from PyQt5.QtWidgets import QWidget, QProgressBar,QPushButton, QApplication
from PyQt5.QtGui import QPainter, QColor, QBrush
from PyQt5.QtCore import QTimer
from collections import namedtuple


class NOCAnim(QWidget):

    # nodesData = []
    # networkSize = [0,0]
    nodeStructure = namedtuple("NodeStructure", "n_rx, n_tx, s_rx, s_tx, e_rx, e_tx, w_rx, w_tx")
    # packetTrace = None

    def __init__(self):
        super().__init__()

        self.initData()
        self.initUI()

    def initData(self):
        # load the log
        home = expanduser("~")
        inputfile = home + '/noc-data/tests/out/packets-trace-netdevice.csv'
        self.packetTrace = noc_io.load_list(inputfile)

        if (len(self.packetTrace)) == 0:
            print('Log file is empty')
            exit(1)

        max_x = max( self.packetTrace[:,trace.x_absolute].astype(int) )
        max_y = max( self.packetTrace[:,trace.y_absolute].astype(int) )

        self.networkSize = [max_y, max_x]

        #initialize data
        self.nodesData = [[self.nodeStructure] * self.networkSize[0] ] * self.networkSize[1]

        for x in range(self.networkSize[0]):
            for y in range(self.networkSize[1]):
                self.nodesData[y][x] = self.nodeStructure(0,0,0,0,0,0,0,0)

    def initUI(self):

        self.timer = QTimer(self)
        self.timer.timeout.connect(self.timerEvent)

        self.step = 0

        self.pbar = QProgressBar(self)
        self.pbar.setGeometry(100, 10, 300, 24)

        self.btn = QPushButton('Start', self)
        self.btn.move(10, 10)
        self.btn.clicked.connect(self.doAction)

        self.setGeometry(800, 100, 500, 600)
        self.setWindowTitle('NoC Anim')
        self.show()

    def doAction(self):

        if self.timer.isActive():
            self.timer.stop()
            self.btn.setText('Start')
        else:
            self.timer.start(100)
            self.btn.setText('Stop')

    def timerEvent(self):

        if self.step >= 100:
            self.timer.stop()
            self.btn.setText('Finished')
            return

        self.nodesData[0][self.step] = self.nodeStructure(1,0,0,0,0,0,0,0)
        # app.processEvents()

        self.step = self.step + 1
        self.pbar.setValue(self.step)

        self.update()

    def paintEvent(self, e):

        qp = QPainter()
        qp.begin(self)

        self.drawNetwork(qp, self.networkSize, 0.30)

        qp.end()

    def drawNetwork(self,qp, shape, s):

        #

        for x in range(shape[0]):
            for y in range(shape[1]):
                self.drawNode(qp, x, y, s, self.nodesData[y][x])

    def drawNode(self, qp, x_i, y_i, s, node):
        col_node = QColor("white")
        # col_node = QColor("white")

        x,y = self.translateXY(x_i, y_i, s)

        qp.setBrush(col_node)
        qp.drawRect(x, y, 90*s, 90*s) #draw the node

        qp.setBrush( self.valueToColor(node.n_rx) )
        qp.drawRect(x + 15*s, y - 30*s, 30*s, 30*s) #draw netdev north rx e tx
        qp.setBrush( self.valueToColor(node.n_tx) )
        qp.drawRect(x + 45*s, y - 30*s, 30*s, 30*s) #draw netdev north rx e tx

        qp.setBrush( self.valueToColor(node.s_rx) )
        qp.drawRect(x + 15*s, y + 90*s, 30*s, 30*s) #draw netdev south rx e tx
        qp.setBrush( self.valueToColor(node.s_tx) )
        qp.drawRect(x + 45*s, y + 90*s, 30*s, 30*s)

        qp.setBrush( self.valueToColor(node.e_rx) )
        qp.drawRect(x - 30*s, y + 15*s, 30*s, 30*s) #draw netdev east rx e tx
        qp.setBrush( self.valueToColor(node.e_tx) )
        qp.drawRect(x - 30*s, y + 45*s, 30*s, 30*s)

        qp.setBrush( self.valueToColor(node.w_rx) )
        qp.drawRect(x + 90*s, y + 15*s, 30*s, 30*s) #draw netdev west rx e tx
        qp.setBrush( self.valueToColor(node.w_tx) )
        qp.drawRect(x + 90*s, y + 45*s, 30*s, 30*s)


    def translateXY(self, x,y,s):
        offset_x = 30 + 10
        offset_y = 150 - 30 + 10

        window_y_size = self.geometry().height()
        # window_y_size = 600

        x_t = x*150*s + offset_x*s
        y_t = window_y_size - y*150*s - offset_y*s

        return x_t, y_t

    def valueToColor(self, v):
        if v == 1:
            return QColor("red")
        elif v == 0:
            return QColor("white")


    # def drawRectangles(self, qp):
    #
    #     col = QColor(0, 0, 0)
    #
    #     col.setNamedColor('#d4d4d4')
    #     qp.setPen(col)
    #
    #     qp.setBrush(QColor(200, 0, 0))
    #     qp.drawRect(10, 15, 90, 60)
    #
    #     qp.setBrush(QColor(255, 80, 0, 160))
    #     qp.drawRect(130, 15, 90, 60)
    #
    #     qp.setBrush(QColor(25, 0, 90, 200))
    #     qp.drawRect(250, 15, 90, 60)


if __name__ == '__main__':

    app = QApplication(sys.argv)
    ex = NOCAnim()
    sys.exit(app.exec_())