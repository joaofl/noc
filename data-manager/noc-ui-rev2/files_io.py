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
import numpy
import os



def load_list(file_name):
    a = []
    try:
        a = numpy.array(list(csv.reader(open(file_name, "rt"), delimiter=',')))
        if (len(a)) == 0:
            print('Log file is empty\n')
            # exit(1)
        # else:
        #     return a
    except:
        print("Log file does not exist\n")
        # return -1

    return a

def load_line(filename):
    file = open(filename)
    data = file.read()

    return data


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