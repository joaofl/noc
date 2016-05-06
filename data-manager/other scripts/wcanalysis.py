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

def generate(period, duration, jitter):
    f = []
    if duration == 0:
        duration = 100

    for t in range(duration):
        if period == 0 or t < jitter:
            f.append(0)
            continue

        if t % period == 0:
            f.append(1)
            continue

        f.append(0)

    return f

class flow:

    #duration 0 means flow last forever
    def __init__(self, period=1, duration = 0, jitter=0):
        self.duration = duration
        self.period = period
        self.jitter = jitter
        self.released = False
        self.t = 0

        self.flow = generate(self.period, self.duration, self.jitter)

        #This function will release or not packets according to flows characteristics

    def get(self):
        if self.t >= len(self.flow):
            return 0
        r = self.flow[self.t]
        self.t += 1
        return r

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

        self.output = []

    def iterate(self):
        self.t += 1

        input_balance = 0
        output_balance = 0

        for f in self.input_flows:
            input_balance += f.get()

        self.stats_tot_received += input_balance
        self.stats_received = input_balance


        self.queue += input_balance

        output_capacity = self.output_flow_capacity.get()

        if output_capacity > 0 and self.queue > 0:
            output_balance = output_capacity
            self.queue -= output_capacity #queue decreases if no packet comes in and there is outpuc capacity
            self.stats_tot_transmitted += output_balance
            self.stats_transmitted = output_balance

        self.output.append(self.stats_transmitted)

        return self.stats_transmitted

    def print(self):

        print(
            'id=' + str(self.id) + '\t' +
            't=' + str(self.t) + '\t\t' +
            'rt:\t' + str(self.stats_tot_received) + '\t' +
            'tt:\t' + str(self.stats_tot_transmitted) + '\t' +
            'r:\t' + str(self.stats_received) + '\t' +
            't:\t' + str(self.stats_transmitted) + '\t' +
            'q:\t' + str(self.queue)
        )


############# Declaring ###################################

# ia, ib, ic = [flow(period=1, duration=1, jitter=1) for i in range(3)]
ia = flow(period=2, duration=0, jitter=0)
ib = flow(period=2, duration=0, jitter=0)
ic = flow(period=2, duration=0, jitter=0)
id = flow(period=1, duration=0, jitter=0) #The flow generated inside has jitter 0 because it is buffered 1 cycle before
                                        # the neighbors packets arrive
o = flow(period=1, duration=0, jitter=0)

print('Input flows:')
print(ia.flow)
print(ib.flow)
print(ic.flow)
print(id.flow)
print(o.flow)

n_nodes = 4
nodes = []

for i in range(n_nodes):
    n = node(i, input_flows=[ia, ib, ic, id], output_flow=o)
    nodes.append(n)

# nodes[0].input_flows.append(id)

############## Running ####################################
print('\nNodes output:')
t = 1   #we analyse from the next second time instant,
        # since it is when it have already received and transmitted packets
for n in nodes:
    for j in range(t):
        n.iterate()
    t += n.queue + 1
    print(n.output)
    # Next step would be to iterate once again with the worst
    # case as limit t, but then, feed the nodes with the output
    # of the previous ones, to converge to the real WC.

############# Showing results ############################
print('\nResults:')
wc = nodes[-1].t + 1 #+1 is the the last transmission delay
for n in nodes:
    n.print()

print('\nWC=' + str(wc))


nodes = []
for i in range(n_nodes + 1):
    n = node(i, input_flows=[ia, ib, ic], output_flow=o)
    nodes.append(n)

############ Re-iteration ###############################
# id = flow(period=2, duration=0, jitter=0)
# nodes[0].input_flows.append(id)
#
# for i in range(len(nodes) - 1):
#     for t in range(wc):
#         nodes[i].iterate()
#
#     f = nodes[i].output
#     nodes[i+1].input_flows.append(f)
#
