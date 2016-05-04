# XDense flow model
# Joao Loureiro


####### NODE MODEL #######
#
#
#           ______
# ____in___|      |
# ____in___|      |___out____
# ____in___|      |
#          |______|
#
##########################
#
class flow:

    def __init__(self, period=1, hperiod = 1, jitter=0):
        self.hperiod = hperiod
        self.period = period
        self.jitter = jitter
        self.released = False
        self.t = 0

    #This function will release or not packets according to flows characteristics
    def generate(self):
        out = 0

        if self.released == False:
            if self.t == self.jitter:
                self.t = 0
                self.released = True
                out = 1

        else:
            if self.t == self.hperiod:
                self.t = 0;

            if self.t % self.period == 0:
                out = 1

        self.t += 1

        return out

class node:
    def __init__(self, id, input_flows=[], output_flow=flow()):

        self.input_flows = input_flows
        self.output_flow_capacity = output_flow

        self.id = id
        self.queue = 0 #Queue initial size

        #Packets totals
        self.stats_tot_transmitted = 0
        self.stats_tot_received = 0
        self.stats_transmitted = 0
        self.stats_received = 0
        self.t = -1

    def iterate(self):
        self.t += 1

        input_balance = 0
        output_balance = 0

        for f in self.input_flows:
            input_balance += f.generate()

        self.stats_tot_received += input_balance
        self.stats_received = input_balance


        self.queue += input_balance

        output_capacity = self.output_flow_capacity.generate()

        if output_capacity > 0 and self.queue > 0:
            output_balance = output_capacity
            self.queue -= output_capacity #queue decreases if no packet comes in and there is outpuc capacity
            self.stats_tot_transmitted += output_balance
            self.stats_transmitted = output_balance

        return self.stats_transmitted

    def print(self):

        print(
            't=' + str(self.t) + '\t\t' +
            'id=' + str(self.id) + '\t' +
            'rt:\t' + str(self.stats_tot_received) + '\t' +
            'tt:\t' + str(self.stats_tot_transmitted) + '\t' +
            'r:\t' + str(self.stats_received) + '\t' +
            't:\t' + str(self.stats_transmitted) + '\t' +
            'q:\t' + str(self.queue)
        )


############# Declaring ###################################



#tf = 10 #Transmission time slots
# out = ''
# for i in range(0,50):
#     out += str(f.generate())
# print(out)
# exit(0)



# ia, ib, ic = [flow(period=1, hperiod=1, jitter=1) for i in range(3)]
ia = flow(period=1, hperiod=1, jitter=0)
ib = flow(period=1, hperiod=1, jitter=0)
ic = flow(period=1, hperiod=1, jitter=0)
id = flow(period=1, hperiod=1, jitter=0) #The flow generated inside has jitter 0 because it is buffered 1 cycle before
                                        # the neighbors packets arrive
o = flow(period=1, hperiod=1, jitter=0)

n_nodes = 4
nodes = []

for i in range(n_nodes):
    n = node(i, input_flows=[ia,ib,ic,id], output_flow=o)
    nodes.append(n)

# n = nodes[0]
# for t in range(10):
#     n.iterate()
#     n.print()

############## Running ####################################
t = 0
for n in nodes:
    for i in range(t+1):
        n.iterate()
    t += n.queue + 1

wc = nodes[-1].t + 1 #+1 is the the last transmission delay

############# Showing results ############################

#print('Duration: ' + str(tf) + '\n')

# wc = 0
for n in nodes:
    n.print()

print('\nWC=' + str(wc))