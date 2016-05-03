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
    def __init__(destination=0, r=0, j=0):
        self.rate = r
        self.jitter = j
        self.port_destination = destination

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

        self.port_north      = port(port.DIR_N)
        self.port_south      = port(port.DIR_S)
        self.port_east       = port(port.DIR_E)
        self.port_west       = port(port.DIR_W)
        self.port_internal   = port(port.DIR_I)
        self.ports = [self.port_north, self.port_south, self.port_east, self.port_west, self.port_internal]

        self.id = id
        self.queue = 0 #Queue initial size

        #Packets totals
        self.stats_transmitted = 0
        self.stats_received = 0

    def iterate(self):
        balance_east = 0

        for p in self.ports:
            if p.input.port_dest == port.DIR_E:
                balance_east += p.input.rate
            if p.output.port_dest == port.DIR_E:
                balance_east += 1




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
f = flow()
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