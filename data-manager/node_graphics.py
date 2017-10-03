from PyQt5.QtWidgets import * #QWidget, QProgressBar,QPushButton, QApplication, QLabel, QCheckBox
from PyQt5.QtOpenGL import *
from PyQt5.QtGui import *
from PyQt5.QtCore import QTimer, Qt, QRectF


class Port():
    __alpha = 150
    __white = QColor(255, 255, 255, __alpha)  # r,g,b,alpha
    __green = QColor(0, 255, 0, __alpha)
    __red = QColor(255, 0, 0, __alpha)

    def __init__(self, rect, scale, io):
        self.__rect_base = rect

        self.set_scale(scale)

        ###
        self.__io = io
        # self.__value = 0
        self.__color = self.__white

        ###
        self.color_off = self.__white
        if io == 1:
            self.color_on = self.__red
        else:
            self.color_on = self.__green

    def get_rect(self):
        return QRectF(self.__rect_scaled[0],self.__rect_scaled[1],
                      self.__rect_scaled[2],self.__rect_scaled[3])

    def get_color(self):
        return self.__color

    def set_scale(self, scale):
        self.__scale = scale
        self.__rect_scaled = [ self.__rect_base[0] * scale,
                               self.__rect_base[1] * scale,
                               self.__rect_base[2] * scale,
                               self.__rect_base[3] * scale ]
    # def get_scale(self):
    #     return self.__scale

    def set_value(self, v):
        if v == 0:
            self.__color = self.color_off
        if v == 1:
            self.__color = self.color_on

    def reset(self):
        self.__color = self.color_off


class Sensor():
    __white = QColor(255, 255, 255, 0)  # r,g,b,alpha

    def __init__(self, rect, scale):
        self.__rect_base = rect

        self.set_scale(scale)

        ###
        self.reset()

    def get_rect(self):
        return QRectF(self.__rect_scaled[0],self.__rect_scaled[1],
                      self.__rect_scaled[2],self.__rect_scaled[3])

    def set_value(self, v):
        self.__value = v
        self.value_changed = True

    def get_value(self):
        return self.__value

    def set_scale(self, scale):
        self.__scale = scale
        self.__rect_scaled = [ self.__rect_base[0] * scale,
                               self.__rect_base[1] * scale,
                               self.__rect_base[2] * scale,
                               self.__rect_base[3] * scale ]
    def reset(self):
        self.__value = -1
        # self.__color = self.__white
        self.value_changed = True

    def get_color(self):
        if self.value_changed == False:
            return self.color
        else:
            ######################### Convert to RGB ########################
            def interpolate(val, y0, x0, y1, x1):
                return (val - x0) * (y1 - y0) / (x1 - x0) + y0

            def base(val):
                if val <= -0.75:
                    return 0
                elif val <= -0.25:
                    return interpolate(val, 0.0, -0.75, 1.0, -0.25)
                elif val <= 0.25:
                    return 1.0
                elif val <= 0.75:
                    return interpolate(val, 1.0, 0.25, 0.0, 0.75)
                else:
                    return 0.0

            def red(gray):
                gray = (gray * 2) - 1
                return base(gray - 0.5)

            def green(gray):
                gray = (gray * 2) - 1
                return base(gray)

            def blue(gray):
                gray = (gray * 2) - 1
                return base(gray + 0.5)

            # if sensor_choice == packet_defines.APP_RESOURCE_SENSOR_LIGHT:
            #     if value > 1500: value = 1500
            #     value = float(value) / 1500
            #
            # if sensor_choice == packet_defines.APP_RESOURCE_SENSOR_TEMPERATURE:
            #     if value - 10 > 30: value = 40
            #     value = float(value - 10) / 40
            #
            # if sensor_choice == packet_defines.APP_RESOURCE_SENSOR_PRESSURE:
            #     if value - 980 > 50: value = 1030
            #     value = float(value - 980) / 50

            if self.__value == -1:
                return QColor("white")

            value = int(self.__value) / pow(2, 16)

            r = red(value)
            g = green(value)
            b = blue(value)
            self.color = QColor.fromRgbF(r, g, b)

            self.value_changed = False
            return self.color



class Node(QGraphicsItem):
    def __init__(self, x, y, node_side, parent=None):
        QGraphicsItem.__init__(self,parent)
        # self.setFlag(QGraphicsItem.ItemIsMovable)

        self.show_sensors = True
        self.show_ports = True
        self.show_leds = True

        self.node_side = node_side

        scale = node_side / 6

        self.x_orig = x * node_side
        self.y_orig = y * node_side

        self.x_cord = x
        self.y_cord = y

        # self.__x, self.__y = self.translateXY(cordinate, scale)
        self.setPos(self.x_orig, self.y_orig) #this sets the origin of the object relative to the scenario

        self.port_n_rx = Port([2, 0, 1, 2], scale, 1)
        self.port_n_tx = Port([3, 0, 1, 2], scale, 0) #(xorig,yorig, w,l)

        self.port_s_tx = Port([2, 4, 1, 2], scale, 0)
        self.port_s_rx = Port([3, 4, 1, 2], scale, 1)

        self.port_e_rx = Port([4, 2, 2, 1], scale, 1)
        self.port_e_tx = Port([4, 3, 2, 1], scale, 0)

        self.port_w_tx = Port([0, 2, 2, 1], scale, 0)
        self.port_w_rx = Port([0, 3, 2, 1], scale, 1)

        self.core = Port([2, 2, 2, 2], scale, 2)
        self.sensor = Sensor([0, 0, 6, 6], scale)

        # self.rescale(node_size)

        self.elements = {'north_rx': self.port_n_rx,
                        'north_tx': self.port_n_tx,
                        'south_rx': self.port_s_rx,
                        'south_tx': self.port_s_tx,
                        'east_rx':  self.port_e_rx,
                        'east_tx':  self.port_e_tx,
                        'west_rx':  self.port_w_rx,
                        'west_tx':  self.port_w_tx,
                        'core':     self.core,
                        'sensor':   self.sensor}

    def rescale(self, node_side):

        self.node_side = node_side
        scale = self.node_side / 6

        self.x_orig = self.x_cord * self.node_side
        self.y_orig = self.y_cord * self.node_side

        for key, e in self.elements.items():
            e.set_scale(scale)

        self.setPos(self.x_orig, self.y_orig) #this sets the origin of the object relative to the scenario



    # def mouseDoubleClickEvent(self, QMouseEvent):
    #
    #     pos = QMouseEvent.pos()
    #
    #     port = self.get_who(pos)
    #
    #     msg = "Node analysis run for port {} on node [{},{}]".format(port, self.__x_origin, self.__y_origin)
    #     ex.status_bar.showMessage(msg)
    #
    #     lauch_external(self.__x_origin, self.__y_origin, port)
    #
    # def mousePressEvent(self, QGraphicsSceneMouseEvent):
    #     pos = QGraphicsSceneMouseEvent.pos()
    #
    #     who = self.get_who(pos)
    #
    #     msg = "Selected port {} on node [{},{}]".format(who, self.__x_origin, self.__y_origin)
    #     # ex.status_bar.showMessage(msg)
    #     print(msg)
    #
    def boundingRect(self):
        r = QRectF(0, 0, self.node_side, self.node_side)
        return r

    # def get_who(self, pos):
    #     who = None
    #     if self.__core_rect.contains(pos):
    #         who = self.port['core']
    #
    #     elif self.__east_rx_rect.contains(pos):
    #         who = self.port['east']
    #     elif self.__east_tx_rect.contains(pos):
    #         who = self.port['east']
    #
    #     elif self.__west_rx_rect.contains(pos):
    #         who = self.port['west']
    #     elif self.__west_tx_rect.contains(pos):
    #         who = self.port['west']
    #
    #     elif self.__north_rx_rect.contains(pos):
    #         who = self.port['north']
    #     elif self.__north_tx_rect.contains(pos):
    #         who = self.port['north']
    #
    #     elif self.__south_rx_rect.contains(pos):
    #         who = self.port['south']
    #     elif self.__south_tx_rect.contains(pos):
    #         who = self.port['south']
    #
    #     return who

    def resetPorts(self):
        for key, e in self.elements.items():
            if type(e) == Port:
                e.reset()


    def paint(self, qp, QStyleOptionGraphicsItem, QWidget_widget=None):
        if self.show_sensors == True:
            if self.sensor.get_value() != -1:
                qp.setPen(QColor('white'))
                qp.setBrush(self.sensor.get_color())
                qp.drawRect(self.sensor.get_rect())

        if self.show_ports == True:
            qp.setPen(QColor("lightgrey"))

            for key, e in self.elements.items():
                if type(e) == Port:
                    qp.setBrush(e.get_color())
                    qp.drawRect(e.get_rect())


            # qp.setBrush(QColor('red'))
            # qp.drawEllipse(0, self.y_orig, 10,10)
            # qp.drawEllipse(self.node_side, self.y_orig+self.node_side, 10,10)
            # qp.drawRect(self.boundingRect())



        #     qp.setPen(QColor("darkgrey"))
        #     qp.setBrush(self.__white)
        #
        #     # rect = MyRect(x + c, self.__y + c, 4 * c, 4 * c)
        #     # rect.setActive(True)
        #     # rect.acceptHoverEvents()
        #
        #     if (self.__core_rx == 1):
        #         qp.setBrush( self.__core_rx_brush)
        #     if (self.__core_tx == 1):
        #         qp.setBrush( self.__core_tx_brush)
        #     qp.drawRect(self.__core_rect)
        #
        # if self.show_leds == True:
        #     qp.setBrush(self.__white)
        #     qp.setBrush( self.__led_off_brush)
        #     # qp.setPen(white)
        #     if (self.__led == 1):
        #         qp.setBrush( self.__led_on_brush)
        #         # qp.setPen(self.__green)
        #     qp.drawEllipse(3.3 * self.__m, 3.3 * self.__m, 1.3 * self.__m, 1.3 * self.__m)  # draw the node
            # if (self.__led == 0):
            #     qp.setBrush( self.__led_off_brush)

            # qp.setPen(QColor("darkgrey"))
            # qp.drawText(1.3 * m, 3 * m, self.__text)

            # self.updated = False
