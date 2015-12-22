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
from PyQt5.QtOpenGL import *
from PyQt5.QtGui import QPainter, QColor, QBrush, QFont, QPen
from PyQt5.QtCore import QTimer, Qt, QRectF
from collections import namedtuple


class Node(QGraphicsItem):

    #This should be private
    __upToDate = False

    __core_rx, __core_tx = 0,0
    __north_rx, __north_tx = 0,0
    __south_rx, __south_tx = 0,0
    __east_rx, __east_tx = 0,0
    __west_rx, __west_tx = 0,0

    __core_rx_brush, __core_tx_brush = QColor("white"),QColor("white")
    __north_rx_brush, __north_tx_brush = QColor("white"),QColor("white")
    __south_rx_brush, __south_tx_brush = QColor("white"),QColor("white")
    __east_rx_brush, __east_tx_brush = QColor("white"),QColor("white")
    __west_rx_brush, __west_tx_brush = QColor("white"),QColor("white")

    def __init__(self, position, nw_size, node_size, parent = None):
        QGraphicsItem.__init__(self,parent)
        # self.setFlag(QGraphicsItem.ItemIsMovable)

        self.__m = node_size / 10

        self.__x_size, self.__y_size = nw_size
        self.__x, self.__y = self.translateXY(position)

    def updateSize(self, size):
        self.__m = size / 10

    def boundingRect(self):
        r = QRectF(5 * self.__m, 5 * self.__m, 5 * self.__m, 5 * self.__m)
        # r.adjust(self, 1,1,1,1)
        return r

    def reset(self):
        self.setProperty(0,0,0,0,0,0,0,0,0,0)


    def setProperty(self, core_rx=None, core_tx=None, north_rx=None, north_tx=None,
                    south_tx=None, south_rx=None, east_tx=None, east_rx=None, west_rx=None, west_tx=None):
        if core_rx is not None:
            if core_rx != self.__core_rx:
                self.__core_rx = core_rx
                self.__core_rx_brush = self.valueToColor(core_rx, 'rx')
                self.__upToDate = False
        if core_tx is not None:
            if core_tx != self.__core_tx:
                self.__core_tx = core_tx
                self.__core_tx_brush = self.valueToColor(core_tx, 'tx')
                self.__upToDate = False

        if north_rx is not None:
            if north_rx != self.__north_rx:
                self.__north_rx = north_rx
                self.__north_rx_brush = self.valueToColor(north_rx, 'rx')
                self.__upToDate = False
        if north_tx is not None:
            if north_tx != self.__north_tx:
                self.__north_tx = north_tx
                self.__north_tx_brush = self.valueToColor(north_tx, 'tx')
                self.__upToDate = False

        if south_rx is not None:
            if south_rx != self.__south_rx:
                self.__south_rx = south_rx
                self.__south_rx_brush = self.valueToColor(south_rx, 'rx')
                self.__upToDate = False
        if south_tx is not None:
            if south_tx != self.__south_tx:
                self.__south_tx = south_tx
                self.__south_tx_brush = self.valueToColor(south_tx, 'tx')
                self.__upToDate = False

        if east_rx is not None:
            if east_rx != self.__east_rx:
                self.__east_rx = east_rx
                self.__east_rx_brush = self.valueToColor(east_rx, 'rx')
                self.__upToDate = False
        if east_tx is not None:
            if east_tx != self.__east_tx:
                self.__east_tx = east_tx
                self.__east_tx_brush = self.valueToColor(east_tx, 'tx')
                self.__upToDate = False

        if west_rx is not None:
            if west_rx != self.__west_rx:
                self.__west_rx = west_rx
                self.__west_rx_brush = self.valueToColor(west_rx, 'rx')
                self.__upToDate = False
        if west_tx is not None:
            if west_tx != self.__west_tx:
                self.__west_tx = west_tx
                self.__west_tx_brush = self.valueToColor(west_tx, 'tx')
                self.__upToDate = False


    def valueToColor(self, v, direction):
        if v == 1:
            if direction == 'rx':
                return QColor("red")
            elif direction == 'tx':
                return QColor("blue")
        elif v == 0:
            return QColor("white")

    def translateXY(self, c):
        # offset_x = (0 * self.m * ((self.x_size - 1) / 1) ) * -1
        # offset_y = (10 * self.m * ((self.y_size - 1) / 1) )
        offset_x = 0
        offset_y = 0

        x = c[0]
        y = c[1]

        # window_y_size = self.geometry().height()
        window_y_size = 0

        x_t = x * self.__m * 10 + offset_x
        y_t = y * self.__m * 10 + offset_y

        return x_t, y_t

    def paint(self, qp, QStyleOptionGraphicsItem, QWidget_widget=None):
    # def drawNode(self, qp, x_i, y_i, s, node):

        # if self.__upToDate == False:
            # self.__upToDate = True

        qp.setBrush(QColor("white"))

        # qpen = QPen()
        # qpen.setColor(QColor("white"))
        qp.setPen(QColor("lightgrey"))

        # qb = QBrush()
        # qb.setStyle(Qt.SolidPattern)

        # qb.setColor()

        qp.setBrush(self.__north_tx_brush)
        qp.drawRect(self.__x + self.__m, self.__y - 2 * self.__m, 2 * self.__m, 2 * self.__m) #draw netdev north rx e tx
        qp.setBrush(self.__north_rx_brush)
        qp.drawRect(self.__x + 3 * self.__m, self.__y - 2 * self.__m, 2 * self.__m, 2 * self.__m) #draw netdev north rx e tx

        qp.setBrush(self.__south_rx_brush)
        qp.drawRect(self.__x + self.__m, self.__y + 6 * self.__m, 2 * self.__m, 2 * self.__m) #draw netdev south rx e tx
        qp.setBrush(self.__south_tx_brush)
        qp.drawRect(self.__x + 3 * self.__m, self.__y + 6 * self.__m, 2 * self.__m, 2 * self.__m)

        qp.setBrush(self.__west_tx_brush)
        qp.drawRect(self.__x - 2 * self.__m, self.__y + self.__m, 2 * self.__m, 2 * self.__m) #draw netdev east rx e tx
        qp.setBrush(self.__west_rx_brush)
        qp.drawRect(self.__x - 2 * self.__m, self.__y + 3 * self.__m, 2 * self.__m, 2 * self.__m)

        qp.setBrush(self.__east_rx_brush)
        qp.drawRect(self.__x + 6 * self.__m, self.__y + self.__m, 2 * self.__m, 2 * self.__m) #draw netdev west rx e tx
        qp.setBrush(self.__east_tx_brush)
        qp.drawRect(self.__x + 6 * self.__m, self.__y + 3 * self.__m, 2 * self.__m, 2 * self.__m)


        qp.setPen(QColor("darkgrey"))

        if (self.__core_rx == 1):
            qp.setBrush( self.__core_rx_brush)
        if (self.__core_tx == 1):
            qp.setBrush( self.__core_tx_brush)

        qp.drawRect(self.__x, self.__y, 6 * self.__m, 6 * self.__m) #draw the node



# class Network(QOpenGLWidget):
#     def __init__(self, parent=None):
#         super(GLWidget, self).__init__(parent)

class NOCAnim(QWidget):



    def __init__(self):
        super().__init__()

        self.initUI()
        self.initData()



    def initUI(self):

        self.timer = QTimer(self)
        self.timer.timeout.connect(self.timerEvent)

        self.btn = QPushButton('Start', self)
        self.btn.clicked.connect(self.doAction)

        self.cb = QCheckBox('Step', self)

        self.btn2 = QPushButton('Reload', self)
        self.btn2.clicked.connect(self.doActionRestart)

        self.pbar = QProgressBar(self)

        self.tb_time_tts = QLabel('0 tts   ', self)
        self.tb_time_tts.setFont(QFont("Monospace", 12, QFont.Bold))

        self.tb_time_ns = QLabel('0 ns    ', self)
        self.tb_time_ns.setFont(QFont("Monospace", 12, QFont.Bold))

        self.tb_zoom = QLabel('1.0x', self)
        self.tb_zoom.setFont(QFont("Monospace", 10, QFont.Bold))

        self.zoom_slider = QSlider(Qt.Horizontal)
        self.zoom_slider.setRange(0,100)
        self.zoom_slider.setTickInterval(1)
        self.zoom_slider.valueChanged.connect(self.doZoom)

        self.tb_refresh = QLabel('50ms', self)
        self.tb_refresh.setFont(QFont("Monospace", 10, QFont.Bold))

        self.refresh_slider = QSlider(Qt.Horizontal)
        self.refresh_slider.setRange(0,100)
        self.refresh_slider.setTickInterval(1)
        self.refresh_slider.valueChanged.connect(self.doChangeRefreshRate)


        self.graphics = QGraphicsView()
        # self.graphics.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        # self.graphics.verticalScrollBar().setSingleStep(1)
        # self.graphics.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        # self.graphics.horizontalScrollBar().setSingleStep(1)
        self.graphics.setViewport(QGLWidget())
        self.graphics.resize(100,100)

        self.scene = QGraphicsScene()
        self.graphics.setScene(self.scene)


        hbox_l1 = QHBoxLayout()
        hbox_l1.addWidget(self.btn)
        hbox_l1.addWidget(self.cb)
        hbox_l1.addWidget(self.btn2)
        hbox_l1.addWidget(self.pbar)
        hbox_l1.addWidget(self.tb_time_tts)
        hbox_l1.addWidget(self.tb_time_ns)

        hbox_l2 = QHBoxLayout()
        hbox_l2.addStretch(1)
        hbox_l2.addWidget(self.tb_zoom)
        hbox_l2.addWidget(self.zoom_slider)
        hbox_l2.addSpacing(1)
        hbox_l2.addWidget(self.tb_refresh)
        hbox_l2.addWidget(self.refresh_slider)


        vbox = QVBoxLayout()
        vbox.addLayout(hbox_l1)
        vbox.addLayout(hbox_l2)
        vbox.addWidget(self.graphics)




        self.setLayout(vbox)


        self.setGeometry(800, 100, 800, 600)
                # self.setGeometry(800, 100, self.networkSize[0] * 150 * self.s + 5, self.networkSize[1] * 150 * self.s + 50)
        self.setWindowTitle('NoC Anim')
        self.show()



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

        self.nodes_size = 12

        self.node_refresh_rate = 50
        self.zoom_rate = 0.5

        for x in range(self.networkSize[0]):
            for y in range(self.networkSize[1]):
                n = Node([x,y], self.networkSize, node_size=self.nodes_size)
                self.network[y][x] = n
                self.scene.addItem(n)

        self.scene.setSceneRect(QRectF(0, 0, (self.networkSize[0]-1) * self.nodes_size, (self.networkSize[1] - 1) * self.nodes_size))

        self.step_enable = False
        self.step = 0

    def doZoom(self):
        range = [5, 20]
        v = self.zoom_slider.value()
        self.zoom_rate = round((v * range[1] / 100) + range[0], 1)
        self.tb_zoom.setText(str(self.zoom_rate)+'x')

        self.nodes_size = self.zoom_rate
        # self.update()
        # self.graphics.scale(self.zoom_rate,self.zoom_rate)
        # self.graphics.resize(100,100)
        # self.graphics.setma
        #
    def doChangeRefreshRate(self):
        range = [50, 2000]
        v = self.refresh_slider.value()
        self.node_refresh_rate = round((v * range[1] / 100) + range[0], 1)
        self.tb_refresh.setText(str(self.node_refresh_rate) + 'ms')
        self.timer.setInterval(self.node_refresh_rate)
        # self.graphics.setma


    def doAction(self):

        if self.timer.isActive():
            self.timer.stop()
            self.btn.setText('Start')
        else:
            self.timer.start(self.node_refresh_rate)
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
                self.network[y][x].updateSize(self.nodes_size)

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
                    node.setProperty(core_rx = 1)
                elif current_trans[trace.operation] == 'g':
                    node.setProperty(core_tx = 1)

                elif current_trans[trace.operation] == 'r':
                    if int(current_trans[trace.direction]) == trace.DIRECTION_N:
                        # node.north_rx = 1
                        node.setProperty(north_rx=1)
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_S:
                        node.setProperty(south_rx = 1)
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_E:
                        node.setProperty(east_rx = 1)
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_W:
                        node.setProperty(west_rx = 1)

                elif current_trans[trace.operation] == 't':
                    if int(current_trans[trace.direction]) == trace.DIRECTION_N:
                        node.setProperty(north_tx = 1)
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_S:
                        node.setProperty(south_tx = 1)
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_E:
                        node.setProperty(east_tx = 1)
                    elif int(current_trans[trace.direction]) == trace.DIRECTION_W:
                        node.setProperty(west_tx = 1)

            else:
                self.t_next = int(current_trans[trace.time]) + self.packetDuration * 0.6
                break


        t = int (self.packetTrace[self.previous_index][trace.time])
        self.pbar.setValue( (t / lastT) * 100)
        self.tb_time_tts.setText(str(round(t / 10000, 2)) + ' tts')
        self.tb_time_ns.setText(str(round(t / 1000, 2)) + ' us')

        if t >= lastT:
            self.doAction()
            return

        if self.cb.isChecked():
            self.doAction()

        # self.graphics.update()
        self.scene.update()
        self.update()





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

