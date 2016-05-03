# XDense flow model
# Joao Loureiro


####### NODE MODEL #######
#
#       p_north
#         ___
#        |   |
#   p_west | q |--> p_east
#        |___|
#
#       p_south
##########################

class flow:
    def __init__(p=0, j=0):
        self.period = p
        self.jitter = j

class port:
    DIR_N = 0
    DIR_S = 1
    DIR_E = 2
    DIR_W = 3
    DIR_I = 4

    def __init__(self, direction, in_flow=flow(), out_flow = flow()):
        self.input = in_flow
        self.output = out_flow
        self.direction = direction
        self.queue = 0

class node:
    def __init__(self, id):
                # n_in_flow = flow(0,0), n_out_flow = flow(0,0),
                # s_in_flow = flow(0,0), s_out_flow = flow(0,0),
                # e_in_flow = flow(0,0), e_out_flow = flow(0,0),
                # w_in_flow = flow(0,0), w_out_flow = flow(0,0),
                # i_in_flow = flow(0,0), i_out_flow = flow(0,0)):

        # self.n_in  = n_in_flow
        # self.n_out = n_out_flow
        #
        # self.s_in  = s_in_flow
        # self.s_out = s_out_flow
        #
        # self.e_in  = e_in_flow
        # self.e_out = e_out_flow
        #
        # self.w_in  = w_in_flow
        # self.w_out = w_out_flow
        #
        # self.i_in  = i_in_flow
        # self.i_out = i_out_flow

        port_north      = port(port.DIR_N)
        port_south      = port(port.DIR_S)
        port_east       = port(port.DIR_E)
        port_west       = port(port.DIR_W)
        port_internal   = port(port.DIR_I)

        self.ports = [port_north, port_south, port_east, port_west, port_internal]

        self.id = id

        #Outputs
        self.queue = 0 #Queue initial size

        #Packets totals
        self.stats_transmitted = 0
        self.stats_received = 0

    def iterate(self):
        balance = 0
        for p in self.ports:
            balance +=




    def print(self):

        print(
            'n=' + str(self.index) + '\t\t' +
            'Received:\t' + str(self.stats_received) + '\t' +
            'Transmitted:\t' + str(self.stats_transmitted) + '\t' +
            #print('Output rate: ' + str(self.p_east))
            'Queue size:\t' + str(self.queue)
        )

    def get_queue_size(self):
        return self.queue




############# Declaring ###################################



#tf = 10 #Transmission time slots
n_nodes = 4
nodes = []

for i in range(n_nodes):
    n = node(i) #known constant input rates
    nodes.append(n)

############## Running ####################################
for n in nodes: #step once inside each

    for i in range(t):
        n.iterate(p_north=p_north, p_west=p_west, p_south=p_south, p_switch=p_switch)
        t_next = n.get_queue_size()

    t += t_next



############# Showing results ############################

#print('Duration: ' + str(tf) + '\n')

wc = 0
for n in nodes:
    n.print()
    wc += n.get_queue_size() + 1

print('\nWC=' + str(wc))