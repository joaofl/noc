# Copyright (c) 2015 João Loureiro
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
# Author: João Loureiro <joflo@isep.ipp.pt>

__author__ = 'João Loureiro <joflo@isep.ipp.pt>'


import csv
import numpy as np
import usn_calc
import os
from numpy.lib.npyio import savetxt

#definition of the protocols used

DEF_DATA_SHARING = 1
DEF_DATA_ANNOUCEMENT = 2

#Definitions for the packet trace
DEF_TIME = 1
DEF_FLOW_ID = 2
DEF_CORD_X = 5
DEF_CORD_Y = 6
DEF_PROTOCOL = 8
DEF_DIRECTION = 3
DEF_PCK_VALUE = 11
DEF_PCK_CORD_X = 12
DEF_PCK_CORD_Y = 13
DEF_BAUDRATE = 10000000

DEF_PCK_SIZE = 56 #7 bytes = 7x8 bits
DEF_PCK_TRANSMISSION_TIME = 8799 #(DEF_PCK_SIZE / DEF_BAUDRATE) * 10E9 #in ns

#Definitions of the flow trace
DEF_ID = 0
DEF_TI = 1
DEF_TF = 2
DEF_TD = 3
DEF_THOP_MIN = 4
DEF_THOP_MAX = 5
DEF_THOP_MEAN = 6
DEF_XI = 7
DEF_XF = 8
DEF_XD = 9
DEF_YI = 10
DEF_YF = 11
DEF_YD = 12
DEF_D = 13
DEF_SIZE = 14

def flow_trace_parallel(pck_trace, i):

    flow = []
    id = int(pck_trace[i][DEF_FLOW_ID])
    found = 0

    #current values
    t_c = int(pck_trace[i][DEF_TIME])
    x_c = int(pck_trace[i][DEF_CORD_X])
    y_c = int(pck_trace[i][DEF_CORD_Y])

    for j in range(len(flow)):
        if id == int(flow[j][DEF_ID]):
            found += 1

            #take the last added flow and update its information based on the pck found
            t_delta = t_c - flow[j][DEF_TF]

            flow[j][DEF_TF] = t_c
            flow[j][DEF_TD] = t_c - flow[j][DEF_TI]

            #check the time between the last two transmission
            if t_delta > flow[j][DEF_THOP_MAX]:
                flow[j][DEF_THOP_MAX] = t_delta

            if t_delta < flow[j][DEF_THOP_MIN] or flow[j][DEF_THOP_MIN] == 0:
                flow[j][DEF_THOP_MIN] = t_delta

            flow[j][DEF_XF] = x_c
            flow[j][DEF_YF] = y_c

            d = usn_calc.dist(flow[j][DEF_XI], flow[j][DEF_XF], flow[j][DEF_YI], flow[j][DEF_YF])
            flow[j][DEF_XD] = d[0]
            flow[j][DEF_YD] = d[1]
            flow[j][DEF_D] = d[2]

            flow[j][DEF_THOP_MEAN] = flow[j][DEF_TD] / flow[j][DEF_D]

    if found == 0:
        #assing the initial values to the flow, according to the definition above
        flow.append(np.zeros(DEF_SIZE))
        index = len(flow) - 1
        flow[index][DEF_ID] = int(id)
        flow[index][DEF_TI] = t_c - DEF_PCK_TRANSMISSION_TIME #packet transmission time
        flow[index][DEF_TF] = t_c
        flow[index][DEF_XI] = x_c
        flow[index][DEF_YI] = y_c

    return flow



def flow_trace(pck_trace):

    flows = []

    #TODO: este loop deve iterar uma unica vez sobre o arquivo pck-trace, e varias sobre o arquivo flow-trace. a fim de tornar mais rapido esta funcacaoo

    for i in range(0, len(pck_trace)):
        id = int(pck_trace[i][DEF_FLOW_ID])
        found = 0

        #current values
        t_c = int(pck_trace[i][DEF_TIME])
        x_c = int(pck_trace[i][DEF_CORD_X])
        y_c = int(pck_trace[i][DEF_CORD_Y])

        for j in range(len(flows)):
            if id == int(flows[j][DEF_ID]):
                found += 1

                #take the last added flow and update its information based on the pck found
                t_delta = t_c - flows[j][DEF_TF]

                flows[j][DEF_TF] = t_c
                flows[j][DEF_TD] = t_c - flows[j][DEF_TI]

                #check the time between the last two transmission
                if t_delta > flows[j][DEF_THOP_MAX]:
                    flows[j][DEF_THOP_MAX] = t_delta

                if t_delta < flows[j][DEF_THOP_MIN] or flows[j][DEF_THOP_MIN] == 0:
                    flows[j][DEF_THOP_MIN] = t_delta

                flows[j][DEF_XF] = x_c
                flows[j][DEF_YF] = y_c

                d = usn_calc.dist(flows[j][DEF_XI], flows[j][DEF_XF], flows[j][DEF_YI], flows[j][DEF_YF])
                flows[j][DEF_XD] = d[0]
                flows[j][DEF_YD] = d[1]
                flows[j][DEF_D] = d[2]

                flows[j][DEF_THOP_MEAN] = flows[j][DEF_TD] / flows[j][DEF_D]

        if found == 0:
            #assing the initial values to the flow, according to the definition above
            flows.append(np.zeros(DEF_SIZE))
            index = len(flows) - 1
            flows[index][DEF_ID] = int(id)
            flows[index][DEF_TI] = t_c - DEF_PCK_TRANSMISSION_TIME #packet transmission time
            flows[index][DEF_TF] = t_c
            flows[index][DEF_XI] = x_c
            flows[index][DEF_YI] = y_c


    return flows


def load_list(file_name):
    try:
        return np.array(list(csv.reader(open(file_name, "rb"), delimiter=',')))
    except:
        return -1

# def dist_to_sink(x, y):
#
#     x = np.abs(x)
#     y = np.abs(y)
#
#     if x >= self.sink_x:  # calculates the distance
#         dx = x - self.sink_x
#     else:
#         dx = self.sink_x - x
#     if y >= self.sink_y:
#         dy = y - self.sink_y
#     else:
#         dy = self.sink_y - y
#
#     dist = dx + dy
#
#     return dx, dy, dist

def load_single_value(filename):
    file = open(filename)
    data = file.read()

    return float(data)

def write_single_value(value, filename):
    with file(filename, 'w') as file_out:
        file_out.write(str(value))
        file_out.close()
        # except:
        #     print("Error writing file: " + filename + '\n')
        #     #QtGui.QMessageBox.error(self, 'Error', "Error writing file: " + file_name, QtGui.QMessageBox.Ok)
        #     return


def load_sensors_data(filename, t=0):
    data_over_time = []
    data_snapshot = []
    j = 0
    for line in open(filename):
        if line.startswith('@'):
            j+=1
            data_over_time.append(np.array(data_snapshot))
            data_snapshot = []
        else:
            data_snapshot.append(np.array(line.strip().split(',')).astype('int'))


    return data_over_time[t]

def find_multiple_files(working_base_dir, filename):

    traces = []

    for (dirpath, dirnames, filenames) in os.walk(working_base_dir):
        for file in filenames:
            if file.startswith(filename):
                traces.append(os.path.join(dirpath, file))

    traces.sort()

    return  traces