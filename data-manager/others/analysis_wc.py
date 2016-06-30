import math
import numpy
# Testing the results (Time in TTS)

def b(cfi, bs):
    if (cfi >= bs):
        return bs - cfi
    else:
        return 0

def received(t, sw, ceil=1):
    received = []
    for f in sw: # do it fow all the flows getting into that switch
        freq = f[0]
        jitter = f[1]
        bsize = f[2]

        tw = (t-jitter)
        if tw < 0:
            tw = 0

        if ceil == 0:
            cf = math.floor((1 / freq) * tw) #how many packets a flow have produced
        elif ceil == 1:
            cf = math.ceil((1 / freq) * tw) #how many packets a flow have produced

        bf = b(cf, bsize) #due to the burst size
        received.append(cf + bf)

    treceived = numpy.sum(received)

    return treceived


def sent(sw, tf):
    p = []
    j = []
    b = []
    for f in sw: # do it fow all the flows getting into that switch
        freq = f[0]
        jitter = f[1]
        bursts = f[2]

        p.append(1/freq)
        j.append(jitter)
        b.append(bursts)

    # if numpy.sum(p) > 1:
    #     pp = 1
    # else:
    #     pp = numpy.sum(p)
    #
    jj = numpy.min(j) + 1
    #
    bb = numpy.sum(b)

    #y = ax + b
    # 0 =  a * 0 + b  => b = 0
    # bb = a * tf + b

    # bb = a * tf

    a = bb / tf

    pp = a

    if pp > 1:
        pp = 1

    return [1/pp,jj,bb]


def calculate_node(t_range, freq_a = 2, freq_b = 2, freq_c = 2 ):

    fa = [freq_a, 3, 10]  # whereas the flows comming from neighbors take one time cycle more
    fb = [freq_b, 3, 10]
    fc = [freq_c, 3, 10]

    sw_in = [fa, fb, fc]

    ############# Input ##############
    received_profile_c = []
    received_profile_f = []
    for t in range(1, t_range + 1):
        received_profile_c.append(received(t, sw_in, ceil=1))
        received_profile_f.append(received(t, sw_in, ceil=0))


    ############# Output ##############
    transmited_profile = []

    tf=0

    for tf in range(0, len(received_profile_c)):
        if received_profile_f[tf] == 30:
            break

    fo = sent(sw_in, tf)
    sw_out = [fo]

    for t in range(1, t_range + 1):
        transmited_profile.append(received(t,sw_out))

    return [received_profile_c, transmited_profile]


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