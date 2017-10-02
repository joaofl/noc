from PyQt5.QtWidgets import * #QWidget, QProgressBar,QPushButton, QApplication, QLabel, QCheckBox
from PyQt5.QtOpenGL import *
from PyQt5.QtGui import *
from PyQt5.QtCore import QTimer, Qt, QRectF


class Node(QGraphicsItem):

    #This should be private

    __alpha = 150
    __white = QColor(255, 255, 255, __alpha) #r,g,b,alpha
    __green = QColor(0  , 255,   0, __alpha)
    __red =   QColor(255,   0,   0, __alpha)

    __upToDate = False

    __text = ''
    __led = 0
    __led_on_brush, __led_off_brush = __green, __white

    __sensor_value = 0
    __core_rx, __core_tx = 0,0
    __north_rx, __north_tx = 0,0
    __south_rx, __south_tx = 0,0
    __east_rx, __east_tx = 0,0
    __west_rx, __west_tx = 0,0

    __sensor_value_brush = __white
    __core_rx_brush, __core_tx_brush = __white, __white
    __north_rx_brush, __north_tx_brush = __white, __white
    __south_rx_brush, __south_tx_brush = __white, __white
    __east_rx_brush, __east_tx_brush = __white, __white
    __west_rx_brush, __west_tx_brush = __white, __white

    port = {
        'east_rx'  :   0b0000000000000001,
        'east_tx'  :   0b0000000000000010,
        'south_rx' :   0b0000000000000100,
        'south_tx' :   0b0000000000001000,
        'west_rx'  :   0b0000000000010000,
        'west_tx'  :   0b0000000000100000,
        'north_rx' :   0b0000000001000000,
        'north_tx' :   0b0000000010000000,
        'core_rx'  :   0b0000000100000000,
        'core_tx'  :   0b0000001000000000,
        'all'      :   0b0000001111111111,
        'none'     :   0b0000000000000000,
    }

    # direction = {
    #     0 : 'out',
    #     1 : 'in'
    # }

    def __init__(self, position, nw_size, node_size, parent=None):
        QGraphicsItem.__init__(self,parent)
        # self.setFlag(QGraphicsItem.ItemIsMovable)

        self.show_sensors = True
        self.show_ports = True
        self.show_leds = True

        # self.__m = node_size / 10
        self.nw_size = nw_size
        self.position = position

        self.updated = True

        self.rescale(node_size)

    def rescale(self, node_size):
        self.__x_pos = self.position[0]
        self.__y_pos = self.position[1]
        self.__x_size, self.__y_size = self.nw_size
        self.__m = node_size / 10

        self.__x, self.__y = self.translateXY(self.position, self.nw_size, self.__m)

        self.setPos(self.__x, self.__y)
        m = self.__m

        # QRectF(qreal x, qreal y, qreal width, qreal height)
        self.__north_tx_rect = QRectF(    m, -2 * m, 2 * m, 3 * m)
        self.__north_rx_rect = QRectF(3 * m, -2 * m, 2 * m, 3 * m)

        self.__south_tx_rect = QRectF(3 * m, 5 * m, 2 * m, 3 * m)
        self.__south_rx_rect = QRectF(    m, 5 * m, 2 * m, 3 * m)

        self.__east_tx_rect = QRectF(5 * m, 3 * m, 3 * m, 2 * m)
        self.__east_rx_rect = QRectF(5 * m,     m, 3 * m, 2 * m)

        self.__west_tx_rect = QRectF(-2 * m,     m, 3 * m, 2 * m)
        self.__west_rx_rect = QRectF(-2 * m, 3 * m, 3 * m, 2 * m)

        self.__core_rect = QRectF(m, m, 4 * m, 4 * m)

        self.__sensor_rect = QRectF(-2 * m, -2 * m, 10 * m, 10 * m)


    def translateXY(self, position, size, ratio):
        side = ratio * 10

        x = position[0]
        y = position[1]

        x_t = x * side
        y_t = (y * -1) * side

        return x_t, y_t

    def getPos(self):
        return self.__x, self.__y


    def mouseDoubleClickEvent(self, QMouseEvent):

        pos = QMouseEvent.pos()

        port = self.get_who(pos)

        msg = "Node analysis run for port {} on node [{},{}]".format(port, self.__x_pos, self.__y_pos)
        ex.status_bar.showMessage(msg)

        lauch_external(self.__x_pos, self.__y_pos, port)

    def mousePressEvent(self, QGraphicsSceneMouseEvent):
        pos = QGraphicsSceneMouseEvent.pos()

        who = self.get_who(pos)

        msg = "Selected port {} on node [{},{}]".format(who, self.__x_pos, self.__y_pos)
        # ex.status_bar.showMessage(msg)
        print(msg)

    def boundingRect(self):
        r = QRectF(-2 * self.__m , -2 * self.__m, 10 * self.__m, 10 * self.__m)
        # r.adjust(self, 1,1,1,1)
        return r

    def get_who(self, pos):
        who = None
        if self.__core_rect.contains(pos):
            who = self.port['core']

        elif self.__east_rx_rect.contains(pos):
            who = self.port['east']
        elif self.__east_tx_rect.contains(pos):
            who = self.port['east']

        elif self.__west_rx_rect.contains(pos):
            who = self.port['west']
        elif self.__west_tx_rect.contains(pos):
            who = self.port['west']

        elif self.__north_rx_rect.contains(pos):
            who = self.port['north']
        elif self.__north_tx_rect.contains(pos):
            who = self.port['north']

        elif self.__south_rx_rect.contains(pos):
            who = self.port['south']
        elif self.__south_tx_rect.contains(pos):
            who = self.port['south']

        return who

    # def reset(self):

    # def setPropertyPorts():
    #
    # # def setPropertyCore():
    #
    # def setPropertyPorts(self, direction, rxtx, value):
    #     if rxtx == 'rx':
    #         return
    #     else:
    #         if direction == 'n':
    #             self.__north_rx_brush = self.directionToColor(north_rx, 'rx')
    #         elif direction == 's':
    #             self.__north_rx_brush = self.directionToColor(north_rx, 'rx')


    def setPropertyBitwise(self, bitmask):

        # self.property_ports |=

        return



    def setProperty(self, core_rx=None, core_tx=None, north_rx=None, north_tx=None,
                    south_tx=None, south_rx=None, east_tx=None, east_rx=None, west_rx=None, west_tx=None,
                    text = None, led = None, sensor_value=None):

        self.updated = True

        if text is not None:
            self.__text = text
        if led is not None:
            self.__led = led
        if core_rx is not None:
            self.__core_rx_brush = self.directionToColor(core_rx, 'rx')
        if core_tx is not None:
            self.__core_tx_brush = self.directionToColor(core_tx, 'tx')
        if north_rx is not None:
            self.__north_rx_brush = self.directionToColor(north_rx, 'rx')
        if north_tx is not None:
            self.__north_tx_brush = self.directionToColor(north_tx, 'tx')
        if south_rx is not None:
            self.__south_rx_brush = self.directionToColor(south_rx, 'rx')
        if south_tx is not None:
            self.__south_tx_brush = self.directionToColor(south_tx, 'tx')
        if east_rx is not None:
            self.__east_rx_brush = self.directionToColor(east_rx, 'rx')
        if east_tx is not None:
            self.__east_tx_brush = self.directionToColor(east_tx, 'tx')
        if west_rx is not None:
            self.__west_rx_brush = self.directionToColor(west_rx, 'rx')
        if west_tx is not None:
            self.__west_tx_brush = self.directionToColor(west_tx, 'tx')
        if sensor_value is not None:
            self.__sensor_value_brush = self.sensorValueToColor(sensor_value)


    def directionToColor(self, v, direction):
        if v == 0:
            return self.__white
        if v == 1:
            if direction == 'rx':
                return self.__green
            elif direction == 'tx':
                return self.__red

    def sensorValueToColor(self, v):
        # TODO: do the conversion depending on the colormap
        # random.seed()

        # r = random.randint(0, 255)
        # g = random.randint(0, 255)
        # b = random.randint(0, 255)
        # c = QColor(r, g, b, 255)
        # return c

        if v == -1:
            return QColor("white")

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

        value = int(v) / pow(2,16)

        r = red(value)
        g = green(value)
        b = blue(value)

        c = QColor.fromRgbF(r, g, b)

        return c




    def paint(self, qp, QStyleOptionGraphicsItem, QWidget_widget=None):
        # qp.setBrush(white)

        # if self.updated == True:

        if self.show_sensors == True:
            qp.setPen(QColor("white"))
            qp.setBrush(self.__sensor_value_brush)
            qp.drawRect(self.__sensor_rect)

        if self.show_ports == True:
            qp.setPen(QColor("lightgrey"))
            qp.setBrush(self.__north_tx_brush)
            qp.drawRect(self.__north_tx_rect) #draw netdev north rx e tx
            qp.setBrush(self.__north_rx_brush)
            qp.drawRect(self.__north_rx_rect) #draw netdev north rx e tx

            qp.setBrush(self.__south_rx_brush)
            qp.drawRect(self.__south_rx_rect) #draw netdev south rx e tx
            qp.setBrush(self.__south_tx_brush)
            qp.drawRect(self.__south_tx_rect)

            qp.setBrush(self.__west_tx_brush)
            qp.drawRect(self.__west_tx_rect) #draw netdev east rx e tx
            qp.setBrush(self.__west_rx_brush)
            qp.drawRect(self.__west_rx_rect)

            qp.setBrush(self.__east_rx_brush)
            qp.drawRect(self.__east_rx_rect) #draw netdev west rx e tx
            qp.setBrush(self.__east_tx_brush)
            qp.drawRect(self.__east_tx_rect)


            qp.setPen(QColor("darkgrey"))
            qp.setBrush(self.__white)

            # rect = MyRect(x + c, self.__y + c, 4 * c, 4 * c)
            # rect.setActive(True)
            # rect.acceptHoverEvents()

            if (self.__core_rx == 1):
                qp.setBrush( self.__core_rx_brush)
            if (self.__core_tx == 1):
                qp.setBrush( self.__core_tx_brush)
            qp.drawRect(self.__core_rect)

        if self.show_leds == True:
            qp.setBrush(self.__white)
            qp.setBrush( self.__led_off_brush)
            # qp.setPen(white)
            if (self.__led == 1):
                qp.setBrush( self.__led_on_brush)
                # qp.setPen(self.__green)
            qp.drawEllipse(3.3 * self.__m, 3.3 * self.__m, 1.3 * self.__m, 1.3 * self.__m)  # draw the node
            # if (self.__led == 0):
            #     qp.setBrush( self.__led_off_brush)

            # qp.setPen(QColor("darkgrey"))
            # qp.drawText(1.3 * m, 3 * m, self.__text)

            # self.updated = False
