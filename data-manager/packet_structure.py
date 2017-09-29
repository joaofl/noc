__author__ = 'joao'

# These are the indexes of each of this field in the packet trace.

# contextual data
# time_slot = 2
time = 0
unique_address = 1
x_absolute = 2
y_absolute = 3
direction = 4
operation = 5  # r (received), t (transmitted), c (consumed)
queue_size = 6
id = 7
sensor_value = 7

# packet data
control_bits = 8
protocol = 9

x_source = 10
y_source = 11
x_dest = 12
y_dest = 13

app_protocol = 14
app_data = 15

# Definition of the NetDevices direction by their index

DIRECTION_E = 0  # , //east
DIRECTION_S = 1  # , //south
DIRECTION_W = 2  # , //west
DIRECTION_N = 3  # , //north
DIRECTION_L = 4  # //Internal, local processor
