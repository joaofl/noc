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

def time_taken(sw, n, direction='out'):
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

    if n=='all':
        n = numpy.sum(burst_list)

    if (direction == 0 or direction == 'out'):
        t = ((n-1) / utilization_equivalent) + jitter_min + 1
    if (direction == 1 or direction == 'in'):
        t = ((n) / utilization_equivalent) + jitter_min

    return t

def burst_size(sw):
    b_out = 0
    for f in sw:  # do it fow all the flows getting into that switch
        b_out += f[2]

    return b_out

def resulting_flow(sw, analysis):
    burstiness = []
    release_delay = []
    msg_size = []
    ms_over_b = []
    for f in sw: # do it fow all the flows getting into that switch
        b = f[0]
        rd = f[1]
        ms = f[2]

        if ms > 0:
            burstiness.append(b)
            release_delay.append(rd)
            msg_size.append(ms)
            ms_over_b.append(ms/b)


    msg_size_out = numpy.sum(msg_size)

    if analysis == 'eted':
        burstiness_out = numpy.sum(msg_size) / numpy.max(ms_over_b)
        release_delay_out = numpy.max(release_delay) + 1
    elif analysis == 'queue':
        burstiness_out = numpy.sum(burstiness)
        release_delay_out = numpy.min(release_delay) + 1

    if burstiness_out > 1:
        burstiness_out = 1

    return [burstiness_out, release_delay_out, msg_size_out]


def calculate_node(sw_in):

    step = 0.01

    print(sw_in)

    ############# Input #############################################
    arrivals = []

    msg_size_total = 0

    for f in sw_in:
        msg_size_total += f[2]

    count = 0
    t = 0

    while(count < msg_size_total):
        count = produced_until(t, sw_in)
        arrivals.append([t, count])
        t += step
    t -= step #removed from last iteration not done
    print(t)


    ############# Output ##########################################
    departures_eted = []
    departures_queue = []

    fo_eted = resulting_flow(sw_in, analysis='eted')
    fo_queue = resulting_flow(sw_in, analysis='queue')

    sw_out_eted = [fo_eted]
    sw_out_queue = [fo_queue]

    count = 0
    t = 0

    while(count < msg_size_total):
        count = produced_until(t, sw_out_eted)
        departures_eted.append([t, count])
        t += step
    t -= step #removed from last iteration not done

    count = 0
    t = 0

    while(count < msg_size_total):
        count = produced_until(t, sw_out_queue)
        departures_queue.append([t, count])
        t += step
    t -= step #removed from last iteration not done



    return [arrivals, departures_eted, departures_queue]
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