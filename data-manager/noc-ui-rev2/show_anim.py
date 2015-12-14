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

import sys, os, traceback, optparse, time
import files_io
import packet_structure as trace
from os.path import expanduser

from PyQt5.QtWidgets import * #QWidget, QProgressBar,QPushButton, QApplication, QLabel, QCheckBox
from PyQt5.QtGui import QPainter, QColor, QBrush, QFont, QPen
from PyQt5.QtCore import QTimer, Qt
from collections import namedtuple


class NOCAnim(QWidget):

    # nodesData = []
    # networkSize = [0,0]
    nodeStructure = namedtuple("NodeStructure", "core_rx, core_tx, n_rx, n_tx, s_rx, s_tx, e_rx, e_tx, w_rx, w_tx")
    nodeStructure.__new__.__defaults__ = (0,0,0,0,0,0,0,0,0,0)

    index_core_rx, index_core_tx, index_n_rx, index_n_tx, \
    index_s_rx, index_s_tx, index_e_rx, index_e_tx, \
    index_w_rx, index_w_tx, index_len\
        = 0,1,2,3,4,5,6,7,8,9,10

    networkSize = [0,0]

	#this data should come from the packet trace
    baudrate = 1500000 #bps
    packet_size = 16 * 8 # 16 bytes

    s = 0.22

    def __init__(self):
        super().__init__()

        self.initData()
        self.initUI()

    def initData(self):

        global options, args
        # load the log
        # home = expanduser("~")
        if (options.inputfile == None):
            options.inputfile = '/home/joao/noc-data/nw21x21s1r03/out/packets-trace-netdevice.csv'

        self.nextT = 0
        self.t_next = 0
        self.previous_index = 0


        self.packetTrace = files_io.load_list(options.inputfile)
        if (len(self.packetTrace) == 0):
            print ('No input file defined')
            return -1

        max_x = max( self.packetTrace[:,trace.x_absolute].astype(int) )
        max_y = max( self.packetTrace[:,trace.y_absolute].astype(int) )

        self.networkSize = [max_x + 1, max_y + 1]

        #initialize data
        self.nodesData = [[None] * self.networkSize[0] for i in range(self.networkSize[1])]

        self.resetNodes()

        self.step_enable = False

        self.step = 0

    def resetNodes(self):
        for x in range(self.networkSize[0]):
            for y in range(self.networkSize[1]):
                self.nodesData[y][x] = self.nodeStructure()

    def initUI(self):

        self.timer = QTimer(self)
        self.timer.timeout.connect(self.timerEvent)

        self.btn = QPushButton('Start', self)
        # self.btn.move(10, 10)
        self.btn.clicked.connect(self.doAction)

        # self.btn3 = QPushButton('Step', self)
        # self.btn3.move(100, 10)
        # self.btn3.clicked.connect(self.doActionStep)

        self.cb = QCheckBox('Step', self)
        # self.cb.move(100, 12)
        # cb.toggle()
        # cb.stateChanged.connect(self.changeTitle)

        self.btn2 = QPushButton('Reload', self)
        # self.btn2.move(170, 10)
        self.btn2.clicked.connect(self.doActionRestart)

        self.pbar = QProgressBar(self)
        # self.pbar.setGeometry(260, 10, 300, 24)

        self.text = QLabel('0 tts   ', self)
        # self.text.move(590, 10)
        self.text.setFont( QFont( "Monospace", 13, QFont.Bold) )

        self.text2 = QLabel('0 ns    ', self)
        # self.text.move(590, 10)
        self.text2.setFont( QFont( "Monospace", 13, QFont.Bold) )

        hbox = QHBoxLayout()
        hbox.addWidget(self.btn)
        hbox.addWidget(self.cb)
        hbox.addWidget(self.btn2)
        hbox.addWidget(self.pbar)
        hbox.addWidget(self.text)
        hbox.addWidget(self.text2)

        vbox = QVBoxLayout()
        vbox.addLayout(hbox)
        vbox.addStretch(1)

        self.setLayout(vbox)


        self.setGeometry(800, 100, 20 * 150 * self.s + 5, 20 * 150 * self.s + 50)
                # self.setGeometry(800, 100, self.networkSize[0] * 150 * self.s + 5, self.networkSize[1] * 150 * self.s + 50)
        self.setWindowTitle('NoC Anim')
        self.show()

    def doAction(self):

        if self.timer.isActive():
            self.timer.stop()
            self.btn.setText('Start')
        else:
            self.timer.start(100)
            self.btn.setText('Stop')


    def doActionRestart(self):
        self.timer.stop()
        self.btn.setText('Start')
        self.nextT = 0
        self.t_next = 0
        self.previous_index = 0
        self.pbar.setValue(0)
        # self.resetNodes()
        # self.packetTrace = noc_io.load_list(self.inputfile)
        self.initData()
        self.update()

    def timerEvent(self):
        lastT = int (self.packetTrace[-1][trace.time])

        # if self.step > lastT:
        #     self.doActionRestart()
        #     return

        self.resetNodes()

        k = 0

        previous_index = self.previous_index

        for i in range(previous_index, len(self.packetTrace)):
            self.previous_index = i
            current_trans = self.packetTrace[i]



            if int(current_trans[trace.time]) <= self.t_next:
                x = int(current_trans[trace.x_absolute])
                y = int(current_trans[trace.y_absolute])

                node = list(self.nodesData[y][x])

                if current_trans[trace.operation] == 'c':
                    node[self.index_core_rx] = 1
                elif current_trans[trace.operation] == 'g':
                    node[self.index_core_tx] = 1

                elif current_trans[trace.operation] == 'r':
                    if int(current_trans[trace.direction]) == trace.DIRECTION_N:
                        node[self.index_n_rx] = 1
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_S:
                        node[self.index_s_rx] = 1
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_E:
                        node[self.index_e_rx] = 1
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_W:
                        node[self.index_w_rx] = 1

                elif current_trans[trace.operation] == 't':
                    if int(current_trans[trace.direction]) == trace.DIRECTION_N:
                        node[self.index_n_tx] = 1
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_S:
                        node[self.index_s_tx] = 1
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_E:
                        node[self.index_e_tx] = 1
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_W:
                        node[self.index_w_tx] = 1

                self.nodesData[y][x] = self.nodeStructure(*node) #* is the unpacking operator

                # temp = [0] * self.index_len
                #
                # temp[self.index_w_tx] = 1
                # self.nodesData[4][4] = self.nodeStructure(*temp) #* is the unpacking operator

            else:
                self.t_next = int(current_trans[trace.time]) + ((self.packet_size / self.baudrate) * 10e9)
                break


        t = int (self.packetTrace[self.previous_index][trace.time])
        # self.step = self.step + 1
        self.pbar.setValue( (t / lastT) * 100)
        # self.text.setText(str(self.step) + ' tts')
        self.text.setText(str(t / 10000) + ' tts')
        self.text2.setText(str(t / 1000) + ' us')

        if t >= lastT:
            self.doAction()
            return

        if self.cb.isChecked():
            self.doAction()

        self.update()




    ######################################################
    ########### DRAWING FUNCTIONS ########################

    def resizeS(self):
        s1 = (self.geometry().width() - 20) / (self.networkSize[0] * 150)
        s2 = (self.geometry().height() - 60) / (self.networkSize[1] * 150)

        self.s = min(s1,s2)

    def paintEvent(self, e):

        # QPainter.Antialiasing 	0x01 	Indicates that the engine should antialias edges of primitives if possible.
        # QPainter.TextAntialiasing 	0x02 	Indicates that the engine should antialias text if possible. To forcibly disable antialiasing for text, do not use this hint. Instead, set QFont.NoAntialias on your font's style strategy.
        # QPainter.SmoothPixmapTransform 	0x04 	Indicates that the engine should use a smooth pixmap transformation algorithm (such as bilinear) rather than nearest neighbor.
        # QPainter.HighQualityAntialiasing 	0x08 	An OpenGL-specific rendering hint indicating that the engine should use fragment programs and offscreen rendering for antialiasing.
        # QPainter.NonCosmeticDefaultPen 	0x10 	The engine should interpret pens with a width of 0 (which otherwise enables QPen.isCosmetic()) as being a non-cosmetic pen with a width of 1.

        qp = QPainter()
        qp.begin(self)
        qp.setRenderHint(QPainter.Antialiasing)
        # qp.setRenderHint(QPainter.SmoothPixmapTransform)




        if self.networkSize != [0,0]:
            self.resizeS()
            self.drawNetwork(qp, self.networkSize, self.s)

        qp.end()

    def drawNetwork(self,qp, shape, s):
        for x in range(shape[0]):
            for y in range(shape[1]):
                self.drawNode(qp, x, y, s, self.nodesData[y][x])


    def drawNode(self, qp, x_i, y_i, s, node):
        qp.setBrush(QColor("white"))

        x,y = self.translateXY(x_i, y_i, s)


        qp.setPen(QColor("black"))


        if (node.core_rx == 1):
            qp.setBrush( self.valueToColor(node.core_rx, 1))
        if (node.core_tx == 1):
            qp.setBrush( self.valueToColor(node.core_tx, 0))

        qp.drawRect(x, y, 90*s, 90*s) #draw the node

        # qpen = QPen()
        # qpen.setColor(QColor("white"))
        qp.setPen(QColor("lightgrey"))

        # qb = QBrush()
        # qb.setStyle(Qt.SolidPattern)

        # qb.setColor()

        qp.setBrush( self.valueToColor(node.n_tx, 0) )
        qp.drawRect(x + 15*s, y - 30*s, 30*s, 30*s) #draw netdev north rx e tx
        qp.setBrush( self.valueToColor(node.n_rx, 1) )
        qp.drawRect(x + 45*s, y - 30*s, 30*s, 30*s) #draw netdev north rx e tx

        qp.setBrush( self.valueToColor(node.s_rx, 1) )
        qp.drawRect(x + 15*s, y + 90*s, 30*s, 30*s) #draw netdev south rx e tx
        qp.setBrush( self.valueToColor(node.s_tx, 0) )
        qp.drawRect(x + 45*s, y + 90*s, 30*s, 30*s)

        qp.setBrush( self.valueToColor(node.w_tx, 0) )
        qp.drawRect(x - 30*s, y + 15*s, 30*s, 30*s) #draw netdev east rx e tx
        qp.setBrush( self.valueToColor(node.w_rx, 1) )
        qp.drawRect(x - 30*s, y + 45*s, 30*s, 30*s)

        qp.setBrush( self.valueToColor(node.e_rx, 1) )
        qp.drawRect(x + 90*s, y + 15*s, 30*s, 30*s) #draw netdev west rx e tx
        qp.setBrush( self.valueToColor(node.e_tx, 0) )
        qp.drawRect(x + 90*s, y + 45*s, 30*s, 30*s)


    def translateXY(self, x,y,s):
        offset_x = 30 + 60
        offset_y = 150 - 30 + 60

        window_y_size = self.geometry().height()
        # window_y_size = 600

        x_t = x*150*s + offset_x*s
        y_t = window_y_size - y*150*s - offset_y*s

        return x_t, y_t

    def valueToColor(self, v, direction = 1):
        if v == 1:
            if direction == 1:
                return QColor("red")
            else:
                return QColor("blue")
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

    try:
        start_time = time.time()
        parser = optparse.OptionParser(formatter=optparse.TitledHelpFormatter(), usage=globals()['__doc__'], version='$Id$')

        parser.add_option ('-v', '--verbose', action='store_false', default=False, help='verbose output')
        parser.add_option ('-i', '--inputfile', help='input file containing the packet trace')
        parser.add_option ('-o', '--outputdir', help='dir to save the plots')
        parser.add_option ('-t', '--timeslotsize', help='time between two refreshes of the animator')

        (options, args) = parser.parse_args()
        #if len(args) < 1:
        #    parser.error ('missing argument')
        if options.verbose: print (time.asctime())

        app = QApplication(sys.argv)
        ex = NOCAnim()


        if options.verbose: print (time.asctime())
        if options.verbose: print ('Total execution time (s):')
        if options.verbose: print (time.time() - start_time)
        # sys.exit(0)

        sys.exit(app.exec_())
    # except (KeyboardInterrupt, e): # Ctrl-C
    #     raise e
    # except SystemExit, e: # sys.exit()
    #     raise e

    except (Exception):
        print ('ERROR, UNEXPECTED EXCEPTION')
        print (str(Exception))
        traceback.print_exc()
        os._exit(1)

