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


import csv
import numpy as np
import usn_io
from copy import copy, deepcopy


def trhoughput(pck_trace, t_interval = 8799, offset=True):

    #8800 ns = 8,8 microseconds to send each packet with 8bytes each

    throughput = []
    t0 = int(pck_trace[0][usn_io.DEF_TI])
    i=0;
    count = 0
    count_a = 0
    t_next = t_interval * i + t0

    # throughput.append([float(t_next - t_interval -t0)/1000000, 1]) #in ms
    throughput.append([i, 1]) #in ms

    i += 1
    t_next = t_interval * i + t0

    if offset==False:
        t0 = 0

    for p in pck_trace:

        if int(p[usn_io.DEF_DIRECTION]) == 1:

            if int(p[usn_io.DEF_TI]) < t_next:
                count += 1

            else:

                if count > 0:
                    # throughput.append([float(t_next - t_interval -t0)/1000000, count]) #in ms
                    throughput.append([i, count]) #in ms

                    count_a = count
                    count = 0

                i += 1
                t_next = t_interval * i + t0




    # throughput.append([float(t_next - t_interval -t0)/1000000, 1]) #just to put it down to 0
    throughput.append([i, 1]) #just to put it down to 0

    return throughput

def dist(xi, xf, yi, yf):
    xi = np.abs(xi)
    yi = np.abs(yi)

    if xi >= xf:  # calculates the distance
        dx = xi - xf
    else:
        dx = xf - xi
    if yi >= yf:
        dy = yi - yf
    else:
        dy = yf - yi

    return dx, dy, dx+dy

def round(pck_time):
    return int(np.round(float(pck_time) / float(10), 0) * 10)

def trim(data, new_x, new_y):
    x_out = data.shape[1] - new_x
    y_out = data.shape[0] - new_y
    grow = False
    if x_out >= 0:
        for i in range(int(np.floor(x_out / 2) + 1)):
            if data.shape[1] != new_x:
                data = np.delete(data, 0, 1)
            if data.shape[1] != new_x:
                data = np.delete(data, -1, 1)
    else:
        for i in range(int(np.floor(abs(x_out) / 2) + 1)):
            if data.shape[1] != new_x:
                data = np.insert(data,0 , 0, 1)
            if data.shape[1] != new_x:
                data = np.insert(data,-1 , 0, 1)

    if y_out >= 0:
        for i in range(int(np.floor(y_out / 2) + 1)):
            if data.shape[0] != new_y:
                data = np.delete(data, 0, 0)
            if data.shape[0] != new_y:
               data = np.delete(data, -1, 0)
    else:
        for i in range(int(np.floor(abs(y_out) / 2) + 1)):
            if data.shape[0] != new_y:
                data = np.insert(data, 0 , np.zeros(data.shape[1]), 0)
            if data.shape[0] != new_y:
                data = np.vstack((data,np.zeros(data.shape[1])))
        # grow = True

    # if grow == True:
    #     data = np.resize(data,[new_x, new_y])




    return data


def diff(data_in, data_out):

    data_diff = data_in

    for i in range(data_in.shape[0]):
        for j in range(data_in.shape[1]):
            if data_out[i][j] != 0:
                data_diff[i][j] = ((data_in[i][j] - data_out[i][j]) ** 2)
            else:
                data_diff[i][j] = 0

    # data_diff = ((data_in - data_out) ** 2)

    #mean square error
    mse = np.sum(data_diff / float(data_in.shape[0] * data_in.shape[1]))

    return data_diff, mse

def normalize_data_array(data_array, sensor_resolution):
    #normalize the plot to the sensor resolution and range

    max_value = np.max(data_array)
    min_value = np.min(data_array)
    sens_scale = pow(2, sensor_resolution) - 1

    for i in range(len(data_array)):
        try:
            data_array[i] = data_array[i].astype('float') # this conversion does not work when importing from CFD
        except:
            data_array[i] = data_array[i]

        data_array[i] = (data_array[i] + min_value * -1) / (np.abs(max_value) + np.abs(min_value))
        data_array[i] *= sens_scale

    # for i in range(0, len(data)):
    #     data[i] = (data[i] + min_value * -1) / (np.abs(max_value) + np.abs(min_value))
    #     data[i] *= sens_scale
    #     # data[i] = np.uint16(data[i])

    return np.uint16(data_array)

def interpolate(data_in):

    data = data_in[:]

    for x in range(data_in.shape[0]-1):
        for y in range(data_in.shape[1]-1):
            if data[x][y] == 0:
                data[x][y] = interpolate_pixel(data_in, x, y)

    return data

def interpolate_pixel(data, xp, yp):
    th = 10
    value = 0
    max = 0
    for x in range(xp-th, xp+th):
        if x < 0:
            continue
        if x > data.shape[0]-1:
            continue

        for y in range(yp-th, yp+th):
            if y < 0:
                continue
            if y > data.shape[1]-1:
                continue

            d = dist(xp,x,yp,y)[2]
            if d < th and d > 0 and data[x][y] != 0:
                value += (float(data[x][y]) / 65535) / (4 * d)
                if data[x][y] > max: max = data[x][y]

    value *= 65535
    if value > max: value = max
    return value

def rebuild_plato(data):

    th = 5000
    tol = 0.60
    y_m = int(np.floor((data.shape[0])/2))
    z_up = 0
    y_up = 0

    z_do = 0
    y_do = 0
    y_do_a = 0
    y_up_a = 0
    drawn = 0
    for x in range(data.shape[1]-1):
        for y in range(y_m-1, 0, -1): #goes up looking for the upper limit
            if data[y][x] > th:
                z_up = data[y][x]
                y_up = y
                break

        for y in range(y_m+2, (data.shape[0]-1), 1): #goes down looking for the lower limit
            if data[y][x] > th:
                z_do = data[y][x]
                y_do = y
                break

        if drawn > 0 and y_do != 0 and y_up != 0: # means that it has anterior values already
            if y_do > (y_do_a + y_do_a * tol):
                # y_do = int(y_do_a + y_do_a * tol)
                y_do = y_do_a
                z_do = z_do_a

            elif y_do < (y_do_a - y_do_a * tol):
                # y_do = int(y_do_a - y_do_a * tol)
                y_do = y_do_a
                z_do = z_do_a

            if y_up > (y_up_a + y_up_a * tol):
                # y_up = int(y_up_a + y_up_a * tol)
                y_up = y_up_a
                z_up = z_up_a

            elif y_up < (y_up_a - y_up_a * tol):
                # y_up = int(y_up_a - y_up_a * tol)
                y_up = y_up_a
                z_up = z_up_a

        if (y_do != 0 and y_up != 0 and y_do > y_up):

            z_max = np.max((z_up,z_do))
            z_min = np.min((z_up,z_do))
            d_max = y_do - y_up
            # if z_min == z_do:
            a = float(z_max-z_min) / float(d_max)
            b = float(z_min)

            for y in range(y_up, y_do, 1):

                d = float(y - y_up)
                z = int(a*d + b)
                data[y][x] = z

                if data[y][x-1] != 0:
                    data[y][x] = (z + data[y][x-1]) / 2

                drawn += 1


            z_up_a = z_up
            y_up_a = y_up
            z_do_a = z_do
            y_do_a = y_do
            z_up = 0
            y_up = 0
            z_do = 0
            y_do = 0


    return data