import math
import numpy
# Testing the results (Time in TTS)

# def b(cfi, bs):
#     if (cfi >= bs):
#         return bs - cfi
#     else:
#         return 0

def produced(t, sw):
    received = []
    for f in sw: # do it fow all the flows getting into that switch
        utilization = f[0]
        jitter = f[1]
        bsize = f[2]

        tw = (t-jitter)
        if tw < 0:
            tw = 0

        produced = math.ceil((utilization) * tw) #how many packets a flow have produced

        if (produced >= bsize):
            produced = bsize

        received.append(produced)

    total_produced = numpy.sum(received)

    return total_produced

def time_taken(sw, direction='out'):
    utilization_list = []
    jitter_list = []
    burst_list = []
    t = 0

    for f in sw:  # do it fow all the flows getting into that switch
        utilization = f[0]
        jitter = f[1]
        burst = f[2]

        jitter_list.append(jitter)
        utilization_list.append(utilization)
        burst_list.append(burst)

    jitter_min = numpy.min(jitter_list)

    utilization_equivalent = 0
    for b in burst_list:
        utilization_equivalent += b / max(burst_list)

    n = numpy.sum(burst_list)

    if (direction == 0 or direction == 'out'):
        t = ((n-1) / utilization_equivalent) + jitter_min + 1
    if (direction == 1 or direction == 'in'):
        t = ((n) / utilization_equivalent) + jitter_min

    return t


def resulting_flow(sw):
    u = []
    j = []
    b = []
    for f in sw: # do it fow all the flows getting into that switch
        utilization = f[0]
        jitter = f[1]
        bsize = f[2]

        u.append(utilization)
        j.append(jitter)
        b.append(bsize)

    j_out = numpy.min(j) + 1
    b_out = numpy.sum(b)
    # u_out = b_out / (t - 1)

    u_out = 0
    for v in b:
        u_out += v / max(b)

    if u_out > 1:
        u_out = 1

    return [u_out, j_out, b_out]


def calculate_node(sw_in):

    step = 0.01

    print(sw_in)

    ############# Input #############################################
    received_profile = []
    received_equivalent = []
    transmitted_equivalent = []

    burst = 0

    for f in sw_in:
        burst += f[2]

    count = 0
    t = 0

    while(count < burst):
        count = produced(t, sw_in)
        received_profile.append([t, count])
        t += step
    t -= step #removed from last iteration not done
    print(t)



    t_taken = time_taken(sw_in, direction='in')
    print('t_in = ' + str(t_taken))

    received_equivalent.append([t_taken, burst])

    ############# Output ##########################################
    transmited_profile = []

    fo = resulting_flow(sw_in)
    sw_out = [fo]

    print(sw_out)

    count = 0
    t = 0

    while(count < burst):
        count = produced(t, sw_out)
        transmited_profile.append([t, count])
        t += step
    t -= step #removed from last iteration not done

    print(t)

    t_taken = time_taken(sw_out, direction='out')
    print('t_out = ' + str(t_taken))

    transmitted_equivalent.append([t_taken, burst])


    return [received_profile, transmited_profile, received_equivalent, transmitted_equivalent]
    # return [received_profile, transmited_profile, fo, received_equivalent]


def calculate():

    # {p, j, bs, D} period, jitter, burst size, deadline
    # The internal flow has jitter 0, since it is at the node already
    fa = [1, 0, 200]  # whereas the flows comming from neighbors take one time cycle more
    fb = [1, 0, 200]
    fc = [1, 0, 200]
    fd = [1, 0, 200]

    sw2 = [fa, fb, fc]
    sw1 = [fb, fb, fa]
    sw0 = [fb, fb, fb]

    # at each hop, the flows that compete for the same output
    route = [sw1, sw1, sw1, sw1, sw1]

    ############# Linear component ###################
    tt_basic = 0
    dist = len(route) - 1
    tt_basic = dist

    tt_basic = 0

    ############# Linear component ###################
    tt_int = 0
    for i in range(0, dist):
        tt_int += len(route[i]) - 1

    tt_int = 0 #not sure if it makes sense to separate it

    ############# Exponential component ##############
    # q_n+1 = (floor(Sum(1/Pi...Pn)) + 1 - 2) * q_n
    tt_queue = 0
    q_profile = []

    tactual = 1
    for sw in route:
        tnext = produced(tactual, sw)
        tactual = tnext

        tt_queue += tnext
        q_profile.append(tnext)

    # for t in range(0,30):
    #     print(str(t) + " " + str(queue(t, sw0)))


        # q_next = (math.ceil(p) +1 -2) * q_this # -2 since the this waiting was acconted separetely
        # q_this = q_next
        # q_profile.append(q_this)
        # tt_queue += q_this

    tt_tot = tt_basic + tt_int + tt_queue
    return tt_tot

    print(q_profile)
    print(tt_basic)
    print(tt_int)
    print(tt_queue)
    print(tt_tot)