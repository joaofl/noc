__author__ = 'joao'

# These are the indexes of each of this field in the packet trace.

# contextual data
time_slot = 2
time = 3
id = 4
unique_address = 5
x_absolute = 6
y_absolute = 7
direction = 8
operation = 9  # r (received), t (transmitted), c (consumed)

# packet data
control_bits = 11
protocol = 12

x_source = 13
y_source = 14
x_dest = 15
y_dest = 16

# Definition of the NetDevices direction by their index

DIRECTION_E = 0  # , //east
DIRECTION_S = 1  # , //south
DIRECTION_W = 2  # , //west
DIRECTION_N = 3  # , //north
DIRECTION_L = 4  # //Internal, local processor
