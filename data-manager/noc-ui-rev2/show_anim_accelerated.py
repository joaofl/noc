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
from PyQt5.QtCore import QTimer, Qt, QRectF
from collections import namedtuple


class Node(QGraphicsItem):

    core_rx, core_tx, n_rx, n_tx, s_rx, s_tx, e_rx, e_tx, w_rx, w_tx = 0,0,0,0,0,0,0,0,0,0
    x, y = 0, 0
    x_size, y_size = 0, 0

    m = 3

    def __init__(self, position, size, parent = None):
        QGraphicsItem.__init__(self,parent)
        # self.setFlag(QGraphicsItem.ItemIsMovable)

        self.x_size, self.y_size = size
        self.x, self.y = self.translateXY(position)




    def boundingRect(self):
        r = QRectF(5 * self.m, 5 * self.m, 5 * self.m, 5 * self.m)
        # r.adjust(self, 1,1,1,1)
        return r

    def reset(self):
        self.core_rx, self.core_tx, self.n_rx, self.n_tx, self.s_rx, self.s_tx, self.e_rx, self.e_tx, self.w_rx, self.w_tx\
            = 0,0,0,0,0,0,0,0,0,0

    def translateXY(self, c):
        offset_x = 10 * self.m * (self.x_size / 2) * -1
        offset_y = 10 * self.m * (self.y_size / 2)

        x = c[0]
        y = c[1]

        # window_y_size = self.geometry().height()
        window_y_size = 0

        x_t = x * self.m * 10 + offset_x
        y_t = window_y_size - y * self.m * 10 + offset_y

        return x_t, y_t

    def paint(self, qp, QStyleOptionGraphicsItem, QWidget_widget=None):
    # def drawNode(self, qp, x_i, y_i, s, node):

        qp.setBrush(QColor("white"))

        m = self.m

        qp.setPen(QColor("lightgrey"))


        if (self.core_rx == 1):
            qp.setBrush( self.valueToColor(self.core_rx, 1))
        if (self.core_tx == 1):
            qp.setBrush( self.valueToColor(self.core_tx, 0))

        qp.drawRect(self.x, self.y, 6*m, 6*m) #draw the node

        # qpen = QPen()
        # qpen.setColor(QColor("white"))
        qp.setPen(QColor("lightgrey"))

        # qb = QBrush()
        # qb.setStyle(Qt.SolidPattern)

        # qb.setColor()

        qp.setBrush( self.valueToColor(self.n_tx, 0) )
        qp.drawRect(self.x + m, self.y - 2*m, 2*m, 2*m) #draw netdev north rx e tx
        qp.setBrush( self.valueToColor(self.n_rx, 1) )
        qp.drawRect(self.x + 3*m, self.y - 2*m, 2*m, 2*m) #draw netdev north rx e tx

        qp.setBrush( self.valueToColor(self.s_rx, 1) )
        qp.drawRect(self.x + m, self.y + 6*m, 2*m, 2*m) #draw netdev south rx e tx
        qp.setBrush( self.valueToColor(self.s_tx, 0) )
        qp.drawRect(self.x + 3*m, self.y + 6*m, 2*m, 2*m)

        qp.setBrush( self.valueToColor(self.w_tx, 0) )
        qp.drawRect(self.x - 2*m, self.y + m, 2*m, 2*m) #draw netdev east rx e tx
        qp.setBrush( self.valueToColor(self.w_rx, 1) )
        qp.drawRect(self.x - 2*m, self.y + 3*m, 2*m, 2*m)

        qp.setBrush( self.valueToColor(self.e_rx, 1) )
        qp.drawRect(self.x + 6*m, self.y + m, 2*m, 2*m) #draw netdev west rx e tx
        qp.setBrush( self.valueToColor(self.e_tx, 0) )
        qp.drawRect(self.x + 6*m, self.y + 3*m, 2*m, 2*m)


    def valueToColor(self, v, direction = 1):
        if v == 1:
            if direction == 1:
                return QColor("red")
            else:
                return QColor("blue")
        elif v == 0:
            return QColor("white")




class NOCAnim(QWidget):



    def __init__(self):
        super().__init__()

        self.initUI()
        self.initData()


    def initData(self):

        global options, args
        # load the log
        # home = expanduser("~")
        if (options.inputfile == None):
            options.inputfile = '/home/joao/noc-data/nw51x51s1n5cTESTS/out/packets-trace-netdevice.csv'

        self.networkSize = [int(options.size_x), int(options.size_y)]

        self.packetDuration = (int(options.packet_size) / int(options.baudrate)) * 1e9

        self.nextT = 0
        self.t_next = 0
        self.previous_index = 0


        self.packetTrace = files_io.load_list(options.inputfile)
        if (len(self.packetTrace) == 0):
            print ('No input file defined')
            return -1

        if self.networkSize == [0,0]:
            max_x = max( self.packetTrace[:,trace.x_absolute].astype(int) )
            max_y = max( self.packetTrace[:,trace.y_absolute].astype(int) )

            self.networkSize = [max_x + 1, max_y + 1]


        #initialize data
        self.network = [[Node] * self.networkSize[0] for i in range(self.networkSize[1])]

        for x in range(self.networkSize[0]):
            for y in range(self.networkSize[1]):
                n = Node([x,y], self.networkSize)
                self.network[y][x] = n
                self.scene.addItem(n)

        self.step_enable = False

        self.step = 0

    def initUI(self):

        self.timer = QTimer(self)
        self.timer.timeout.connect(self.timerEvent)

        self.btn = QPushButton('Start', self)
        # self.btn.move(10, 10)
        self.btn.clicked.connect(self.doAction)

        self.cb = QCheckBox('Step', self)

        self.btn2 = QPushButton('Reload', self)
        self.btn2.clicked.connect(self.doActionRestart)

        self.pbar = QProgressBar(self)
        # self.pbar.setGeometry(260, 10, 300, 24)

        self.text = QLabel('0 tts   ', self)
        # self.text.move(590, 10)
        self.text.setFont( QFont( "Monospace", 12, QFont.Bold) )

        self.text2 = QLabel('0 ns    ', self)
        # self.text.move(590, 10)
        self.text2.setFont( QFont( "Monospace", 12, QFont.Bold) )

        self.scene = QGraphicsScene()
        self.graphics = QGraphicsView()
        self.graphics.setScene(self.scene)


        hbox = QHBoxLayout()
        hbox.addWidget(self.btn)
        hbox.addWidget(self.cb)
        hbox.addWidget(self.btn2)
        hbox.addWidget(self.pbar)
        hbox.addWidget(self.text)
        hbox.addWidget(self.text2)

        vbox = QVBoxLayout()
        vbox.addLayout(hbox)
        # vbox.addStretch(1)
        vbox.addWidget(self.graphics)

        self.setLayout(vbox)


        self.setGeometry(800, 100, 800, 600)
                # self.setGeometry(800, 100, self.networkSize[0] * 150 * self.s + 5, self.networkSize[1] * 150 * self.s + 50)
        self.setWindowTitle('NoC Anim')
        self.show()

    def doAction(self):

        if self.timer.isActive():
            self.timer.stop()
            self.btn.setText('Start')
        else:
            self.timer.start(200)
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

    def resetNetwork(self):
        for x in range(self.networkSize[0]):
            for y in range(self.networkSize[1]):
                self.network[y][x].reset()

    def timerEvent(self):
        lastT = int (self.packetTrace[-1][trace.time])

        self.resetNetwork()

        previous_index = self.previous_index


        for i in range(previous_index, len(self.packetTrace)):
            self.previous_index = i
            current_trans = self.packetTrace[i]

            if int(current_trans[trace.time]) <= self.t_next:
                x = int(current_trans[trace.x_absolute])
                y = int(current_trans[trace.y_absolute])

                node = self.network[y][x]

                if current_trans[trace.operation] == 'c':
                    node.core_rx = 1
                elif current_trans[trace.operation] == 'g':
                    node.core_tx = 1

                elif current_trans[trace.operation] == 'r':
                    if int(current_trans[trace.direction]) == trace.DIRECTION_N:
                        node.n_rx = 1
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_S:
                        node.s_rx = 1
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_E:
                        node.e_rx = 1
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_W:
                        node.w_rx = 1

                elif current_trans[trace.operation] == 't':
                    if int(current_trans[trace.direction]) == trace.DIRECTION_N:
                        node.n_tx = 1
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_S:
                        node.s_tx = 1
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_E:
                        node.e_tx = 1
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_W:
                        node.w_tx = 1

            else:
                self.t_next = int(current_trans[trace.time]) + self.packetDuration * 0.6
                break


        t = int (self.packetTrace[self.previous_index][trace.time])
        # self.step = self.step + 1
        self.pbar.setValue( (t / lastT) * 100)
        # self.text.setText(str(self.step) + ' tts')
        self.text.setText(str(round(t / 10000, 2)) + ' tts')
        self.text2.setText(str(round(t / 1000, 2)) + ' us')

        if t >= lastT:
            self.doAction()
            return

        if self.cb.isChecked():
            self.doAction()

        # self.graphics.update()
        self.update()
        self.scene.update()





if __name__ == '__main__':

    try:
        start_time = time.time()
        parser = optparse.OptionParser(formatter=optparse.TitledHelpFormatter(), usage=globals()['__doc__'], version='$Id$')

        parser.add_option ('-v', '--verbose', action='store_false', default=False, help='verbose output')
        parser.add_option ('-i', '--inputfile', help='input file containing the packet trace')
        parser.add_option ('-o', '--outputdir', help='dir to save the plots')
        parser.add_option ('-t', '--timeslotsize', help='time between two refreshes of the animator')
        parser.add_option ('-x', '--size_x', help='network size', default=0)
        parser.add_option ('-y', '--size_y', help='network size', default=0)
        parser.add_option ('-s', '--sinks_n', help='number of sinks', default=1)
        parser.add_option ('-b', '--baudrate', help='baudrate utilized', default=1500000)
        parser.add_option ('-p', '--packet_size', help='packet_size in bits', default=16*8)
        parser.add_option ('-n', '--size_neighborhood', help='neighborhood size', default=0)


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

