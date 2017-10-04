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

import sys
from PyQt5.QtWidgets import * #QWidget, QProgressBar,QPushButton, QApplication, QLabel, QCheckBox
from PyQt5.QtGui import *
from PyQt5.QtCore import QTimer, Qt, QRectF

from node_graphics import *

class SensorAnim(QWidget):
    def __init__(self, networkSize, parent=None):
        super().__init__()
        # QWidget.__init__()



        # self.setGeometry(800, 100, self.networkSize[0] * 150 * self.s + 5, self.networkSize[1] * 150 * self.s + 50)
        self.scene = QGraphicsScene()
        self.graphics = QGraphicsView()
        self.graphics.setScene(self.scene)

        self.status_bar = QStatusBar(self)
        self.cbSensors = QCheckBox('Sensor', self)
        self.cbSensors.setChecked(True)
        self.cbPorts = QCheckBox('Ports', self)
        self.cbPorts.setChecked(True)
        self.cbLeds = QCheckBox('Leds', self)
        self.cbLeds.setChecked(True)

        self.cbSensors.clicked.connect(self.cbChanged)
        self.cbPorts.clicked.connect(self.cbChanged)
        self.cbLeds.clicked.connect(self.cbChanged)

        self.tb_zoom = QLabel('1.0x', self)
        self.tb_zoom.setFont(QFont("Monospace", 10, QFont.Bold))

        self.zoom_slider = QSlider(Qt.Horizontal)
        self.zoom_slider.setValue(0) #Initial value
        self.zoom_slider.setTickInterval(1)

        hbox_l3 = QHBoxLayout()
        hbox_l3.addWidget(self.status_bar)
        hbox_l3.addStretch(1)
        hbox_l3.addWidget(self.cbPorts)
        hbox_l3.addWidget(self.cbSensors)
        hbox_l3.addWidget(self.cbLeds)
        hbox_l3.addWidget(self.tb_zoom)
        hbox_l3.addWidget(self.zoom_slider)

        vbox = QVBoxLayout()
        vbox.addWidget(self.graphics)
        vbox.addLayout(hbox_l3)

        self.setLayout(vbox)
        self.setGeometry(200, 100, 800, 600)
        self.setWindowTitle('NoC Anim')

        self.scene.clear()

        self.networkSize = networkSize
        # Calculate node size based on canvas size
        g = self.graphics.geometry().getRect()[2:4]
        sx = (g[0]+50) / self.networkSize[0]
        sy = (g[1]+50) / self.networkSize[1]
        if sx < sy: nodes_size = round(sx)
        else:       nodes_size = round(sy)

        self.network = [[Node] * self.networkSize[0] for i in range(self.networkSize[1])]
        for x in range(self.networkSize[0]):
            for y in range(self.networkSize[1]):
                n = Node(x, y, node_side=nodes_size) #set on the existing one instead of creating another
                self.network[-y-1][x] = n # flips the y so that the (0,0) shows on the bottom left instead of top-left
                self.scene.addItem(n)

        # if self.zoom_slider.value() == 0:
        self.zoom_slider.setRange(nodes_size*0.1, nodes_size*10)
        self.zoom_slider.setValue(nodes_size)
        self.tb_zoom.setText(str(nodes_size))
        self.tb_zoom.setText(str(nodes_size))
        self.zoom_slider.valueChanged.connect(self.doZoom)

        self.show()
        self.scene.update()

        return

    def cbChanged(self):
        for x in range(self.networkSize[0]):
            for y in range(self.networkSize[1]):
                self.network[y][x].show_sensors = self.cbSensors.isChecked()
                self.network[y][x].show_ports = self.cbPorts.isChecked()
                self.network[y][x].show_leds = self.cbLeds.isChecked()

    def doZoom(self):
        self.nodes_size = self.zoom_slider.value()
        self.tb_zoom.setText(str(self.nodes_size))

        for x in range(self.networkSize[0]):
            for y in range(self.networkSize[1]):
                self.network[y][x].rescale(self.nodes_size)

        self.scene.update()
        return

    def setNode(self, x, y, port, value):

        n = self.network[y][x]
        n.elements[port].set_value(value)

    def resetPorts(self):
        for x in range(self.networkSize[0]):
            for y in range(self.networkSize[1]):
                self.network[y][x].resetPorts()

    def resetNetwork(self):
        for x in range(self.networkSize[0]):
            for y in range(self.networkSize[1]):
                self.network[y][x].reset()

    # def wheelEvent(self, event):
        # """
        # Zoom in or out of the view.
        # """
        # zoomInFactor = 1.25
        # zoomOutFactor = 1.1 / zoomInFactor
        #
        # # Save the scene pos
        # oldPos = self.graphics.mapToScene(event.pos())
        #
        # # Zoom
        # if event.angleDelta().y() > 0:
        #     zoomFactor = zoomInFactor
        # else:
        #     zoomFactor = zoomOutFactor
        #
        # self.graphics.scale(zoomFactor, zoomFactor)
        #
        # # Get the new position
        # newPos = self.graphics.mapToScene(event.pos())
        #
        # # Move scene to old position
        # delta = newPos - oldPos
        # self.graphics.translate(delta.x(), delta.y())
        # return


if __name__ == '__main__':
    app = QApplication(sys.argv)
    app.setWindowIcon(QIcon('others/grid2.png'))
    ex = SensorAnim([200,20])


    sys.exit(app.exec_())