import math
import numpy
# Testing the results (Time in TTS)

def b(cfi, bs):
    if (cfi >= bs):
        return bs - cfi
    else:
        return 0

def received(t, sw):
    received = []
    for f in sw: # do it fow all the flows getting into that switch
        period = f[0]
        jitter = f[1]
        bsize = f[2]

        tw = (t-jitter)
        if tw < 0:
            tw = 0

        cf = math.floor((1 / period)) * tw #how many packets a flow have produced

        bf = b(cf, bsize) #due to the burst size
        # bf = 0
        prod = cf + bf
        received.append(prod)

    treceived = numpy.sum(received)

    return treceived


def sent(t, sw):
    sent = []
    for f in sw: # do it fow all the flows getting into that switch
        period = f[0]
        jitter = f[1]
        bsize = f[2]

        tw = (t-jitter)
        if tw < 0:
            tw = 0

        cf = math.ceil((1 / period)) * tw #how many packets a flow have produced

        bf = b(cf, bsize) #due to the burst size
        bf = 0
        prod = cf + bf
        sent.append(prod)

    tproduced = numpy.sum(sent)

    return tproduced


def calculate_node(t_range):

    fa = [2, 1, 10]  # whereas the flows comming from neighbors take one time cycle more
    fb = [2, 1, 10]
    fc = [2, 1, 10]

    sw0 = [fa, fb, fc]

    ############# Exponential component ##############
    t_profile = []

    for t in range(1, t_range + 1):
        tnext = received(t, sw0)

        t_profile.append(tnext)


    return t_profile


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
        tnext = received(tactual, sw)
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