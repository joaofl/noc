import math
import numpy
# Testing the results (Time in TTS)

# def b(cfi, bs):
#     if (cfi >= bs):
#         return bs - cfi
#     else:
#         return 0

def produced_until(t, sw):
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

def time_taken(fl, n='all'):
    b = fl[0]
    rd = fl[1]
    ms = fl[2]

    if n=='all':
        n = ms

    t = (float(n-1) / b) + rd

    return t

def burst_size(sw):
    b_out = 0
    for f in sw:  # do it fow all the flows getting into that switch
        b_out += f[2]

    return b_out

def resulting_flow(sw, model='B'):
    burstiness = []
    offset = []
    msg_size = []
    ms_over_b = []
    ms_over_b_o = []
    for f in sw: # do it fow all the flows getting into that switch
        b = f[0]
        o = f[1]
        ms = f[2]

        if ms > 0:
            burstiness.append(b)
            offset.append(o)
            msg_size.append(ms)
            ms_over_b.append((ms)/b)
            ms_over_b_o.append(((ms - 1) / b) + o)

    msg_size_out = numpy.sum(msg_size)

    if model == 'A': # or len(sw) == 1:
        burstiness_out = msg_size_out / numpy.max(ms_over_b)
        offset_out = numpy.max(offset) + 1
    elif model == 'B':
        bursts_calc = []
        tfixed = numpy.max(ms_over_b_o)
        pfixed = msg_size_out

        for i in range(len(sw)):
            ti = offset[i]
            tf = ms_over_b_o[i]
            pi = produced_until(ti, sw) + 1
            pf = produced_until(tf, sw) + 1

            dp1 = pfixed - pi
            dp2 = pfixed - pf
            dt1 = tfixed - ti
            dt2 = tfixed - tf

            b1 = dp1 / dt1
            b2 = dp2 / dt2

            bursts_calc.append(b1)
            bursts_calc.append(b2)

        burstiness_out = max(bursts_calc)
        offset_out = numpy.max(ms_over_b_o) - ((msg_size_out - 1) / burstiness_out) + 1

    elif model == 'C':
        bursts_calc = []
        tfixed = min(offset)
        pfixed = 1

        for i in range(len(sw)):
            ti = offset[i]
            tf = ms_over_b_o[i]
            pi = produced_until(ti, sw) + 1
            pf = produced_until(tf, sw) + 1

            dp1 = pi - pfixed
            dp2 = pf - pfixed
            dt1 = ti - tfixed
            dt2 = tf - tfixed

            b1 = dp1 / dt1
            b2 = dp2 / dt2

            bursts_calc.append(b1)
            bursts_calc.append(b2)

        bursts_calc = [e for e in bursts_calc if not math.isnan(e)] #remove Nan's otherwise they get detected as min
        burstiness_out = min(bursts_calc)
        offset_out = tfixed + 1

    if burstiness_out > 1:
        burstiness_out = 1

    return [burstiness_out, offset_out, msg_size_out]


def calculate_node(sw_in):

    step = 0.01
    ############# Input #############################################
    x = []
    y = []

    msg_size_total = 0

    for f in sw_in:
        msg_size_total += f[2]

    count = -1
    t = 0

    while(count < msg_size_total):
        count_n = produced_until(t, sw_in)
        if count_n != count:
            count = count_n
            x.append(t)
            y.append(count)

        t += step

    return [x, y]


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
        tnext = produced_until(tactual, sw)
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