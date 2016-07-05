import math
import numpy
# Testing the results (Time in TTS)

def b(cfi, bs):
    if (cfi >= bs):
        return bs - cfi
    else:
        return 0

def produced(t, sw):
    received = []
    for f in sw: # do it fow all the flows getting into that switch
        utilization = f[0]
        jitter = f[1]
        bsize = f[2]

        tw = (t-jitter)
        if tw < 0:
            tw = 0

        cf = math.ceil((utilization) * tw) #how many packets a flow have produced
        # cf = (1 / freq) * tw

        bf = b(cf, bsize) #due to the burst size
        received.append(cf + bf)

    treceived = numpy.sum(received)

    return treceived


def resulting_flow(sw, tf):
    u = []
    j = []
    b = []
    for f in sw: # do it fow all the flows getting into that switch
        utilization = f[0]
        jitter = f[1]
        bursts = f[2]

        u.append(utilization)
        j.append(jitter)
        b.append(bursts)

    jj = numpy.min(j) + 1
    bb = numpy.sum(b)
    pp = bb / tf

    if pp > 1:
        pp = 1

    return [pp, jj, bb]


def calculate_node():

    fa = [0.5, 2, 20]  # whereas the flows comming from neighbors take one time cycle more
    fb = [0.8, 2, 15]
    fc = [0.3, 2, 25]

    step = 0.01

    sw_in = [fa, fb, fc]
    # sw_in = [fa]
    ############# Input ##############
    received_profile = []
    # for t in range(0, t_range * 10):

    burst = 0

    for f in sw_in:
        burst += f[2]

    count = 0
    t = 0

    while(count < burst):
        count = produced(t, sw_in)
        received_profile.append([t, count])
        t += step


    ############# Output ##############
    transmited_profile = []

    fo = resulting_flow(sw_in, t)
    sw_out = [fo]

    # for t in range(0, t_range):
    #     transmited_profile.append(produced(t, sw_out))

    count = 0
    t = 0

    while(count < burst):
        count = produced(t, sw_out)
        transmited_profile.append([t, count])
        t += step

    return [received_profile, transmited_profile]


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