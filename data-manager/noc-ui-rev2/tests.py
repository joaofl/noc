# import time
# import numpy as np
# import matplotlib.pyplot as plt
#
# plt.axis([0, 1000, 0, 1])
# plt.ion()
# plt.show()
#
# for i in range(1000):
#     y = np.random.random()
#     plt.scatter(i, y)
#     plt.draw()
#     time.sleep(0.05)

"""
A simple example of an animated plot
"""
# import numpy as np
# import matplotlib.pyplot as plt
# import matplotlib.animation as animation
#
# fig, ax = plt.subplots()
#
# x = np.arange(0, 2*np.pi, 0.01)
# line, = ax.plot(x, np.sin(x))
#
#
# def animate(i):
#     line.set_ydata(np.sin(x + i/10.0))  # update the data
#     return line,
#
#
# # Init only required for blitting to give a clean slate.
# def init():
#     line.set_ydata(np.ma.array(x, mask=True))
#     return line,
#
# ani = animation.FuncAnimation(fig, animate, np.arange(1, 200), init_func=init,
#                               interval=25, blit=True)
# plt.show()

# import numpy as np
# import matplotlib.pyplot as plt
# import matplotlib.animation as animation
#
# fig = plt.figure()
#
#
# def f(x, y):
#     return np.sin(x) + np.cos(y)
#
# x = np.linspace(0, 2 * np.pi, 120)
# y = np.linspace(0, 2 * np.pi, 100).reshape(-1, 1)
#
# im = plt.imshow(f(x, y), cmap=plt.get_cmap('viridis'), animated=True)
#
#
# def updatefig(*args):
#     global x, y
#     x += np.pi / 15.
#     y += np.pi / 20.
#     im.set_array(f(x, y))
#     return im,
#
# ani = animation.FuncAnimation(fig, updatefig, interval=50, blit=True)
# plt.show()

#!/usr/bin/env python
# """
# An animated image
# """
# import numpy as np
# import matplotlib.pyplot as plt
# import matplotlib.animation as animation
#
# fig = plt.figure()
#
#
# def f(x, y):
#     return np.sin(x) + np.cos(y)
#
# x = np.linspace(0, 2 * np.pi, 120)
# y = np.linspace(0, 2 * np.pi, 100).reshape(-1, 1)
# # ims is a list of lists, each row is a list of artists to draw in the
# # current frame; here we are just animating one artist, the image, in
# # each frame
# ims = []
# for i in range(60):
#     x += np.pi / 15.
#     y += np.pi / 20.
#     im = plt.imshow(f(x, y), cmap='viridis', animated=True)
#     ims.append([im])
#
# ani = animation.ArtistAnimation(fig, ims, interval=50, blit=True,
#                                 repeat_delay=1000)
#
# #ani.save('dynamic_images.mp4')
#
#
# plt.show()

# import sys
# from PyQt5 import QtGui, QtCore
# from PyQt5.QtWidgets import QWidget, QApplication
# from PyQt5.QtGui import QPainter, QColor, QBrush
# import time
# import random
#
#
# class MyThread(QtCore.QThread):
#     trigger = QtCore.pyqtSignal(int)
#
#     def __init__(self, parent=None):
#         super(MyThread, self).__init__(parent)
#
#     def setup(self, thread_no):
#         self.thread_no = thread_no
#
#     def run(self):
#         time.sleep(random.random()*5)  # random sleep to imitate working
#         self.trigger.emit(self.thread_no)
#
#
# class Main(QtWidget.QMainWindow):
#     def __init__(self, parent=None):
#         super(Main, self).__init__(parent)
#         self.text_area = QtGui.QTextBrowser()
#         self.thread_button = QtGui.QPushButton('Start threads')
#         self.thread_button.clicked.connect(self.start_threads)
#
#         central_widget = QtGui.QWidget()
#         central_layout = QtGui.QHBoxLayout()
#         central_layout.addWidget(self.text_area)
#         central_layout.addWidget(self.thread_button)
#         central_widget.setLayout(central_layout)
#         self.setCentralWidget(central_widget)
#
#     def start_threads(self):
#         self.threads = []              # this will keep a reference to threads
#         for i in range(10):
#             thread = MyThread(self)    # create a thread
#             thread.trigger.connect(self.update_text)  # connect to it's signal
#             thread.setup(i)            # just setting up a parameter
#             thread.start()             # start the thread
#             self.threads.append(thread) # keep a reference
#
#     def update_text(self, thread_no):
#         self.text_area.append('thread # %d finished' % thread_no)
#
# if __name__ == '__main__':
#     app = QtGui.QApplication(sys.argv)
#
#     mainwindow = Main()
#     mainwindow.show()
#
#     sys.exit(app.exec_())

#!/usr/bin/python3
# -*- coding: utf-8 -*-

"""
ZetCode PyQt5 tutorial

This example shows a QProgressBar widget.

author: Jan Bodnar
website: zetcode.com
last edited: January 2015
"""

import sys
from PyQt5.QtWidgets import (QWidget, QProgressBar,
    QPushButton, QApplication)
from PyQt5.QtCore import QBasicTimer


class Example(QWidget):

    def __init__(self):
        super().__init__()

        self.initUI()


    def initUI(self):

        self.pbar = QProgressBar(self)
        self.pbar.setGeometry(30, 40, 200, 25)

        self.btn = QPushButton('Start', self)
        self.btn.move(40, 80)
        self.btn.clicked.connect(self.doAction)

        self.timer = QBasicTimer()
        self.step = 0

        self.setGeometry(300, 300, 280, 170)
        self.setWindowTitle('QProgressBar')
        self.show()


    def timerEvent(self, e):

        if self.step >= 100:
            self.timer.stop()
            self.btn.setText('Finished')
            return

        self.step = self.step + 1
        self.pbar.setValue(self.step)


    def doAction(self):

        if self.timer.isActive():
            self.timer.stop()
            self.btn.setText('Start')
        else:
            self.timer.start(100, self)
            self.btn.setText('Stop')


if __name__ == '__main__':

    app = QApplication(sys.argv)
    ex = Example()
    sys.exit(app.exec_())