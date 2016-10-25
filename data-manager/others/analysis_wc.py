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
        burstiness = f[0]
        jitter = f[1]
        msg_size = f[2]

        tw = (t-jitter)
        if tw < 0:
            tw = 0

        produced = math.ceil((burstiness) * tw) #how many packets a flow have produced

        if (produced >= msg_size):
            produced = msg_size

        received.append(produced)

    total_produced = numpy.sum(received)

    return total_produced

def time_taken_new(sw, n_in='all'):

    if isinstance(sw, int): #if it is a single flow, make it switch-like
        sw = [sw]

    timeline = []

    for fl in sw:
        b = fl[0]
        rd = fl[1]
        ms = fl[2]

        ti = fl[1]
        tf = (ms / b) + ti

        timeline.append([ti, +b, 0])
        timeline.append([tf, -b, ms])

        if n_in== 'all':
            n_in += ms

    timeline = sorted(timeline)

    b_local = 0
    t_total = 0
    n_total = 0

    for i in range(len(timeline) - 1): #find in which segment it is
        b_local += timeline[i][1]
        ti = (timeline[i][0])
        tf = (timeline[i+1][0])
        dt = (tf-ti)

        n = dt * b_local

        if (n <= n_in):
            t_total += dt
            n_total += n
        else:
            tf = (n_in - n_total) / b_local
            t_total += tf
            break



    return t_total

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
            ms_over_b_o.append(((ms) / b) + o)

    msg_size_out = numpy.sum(msg_size)


    if model == 'A': # or len(sw) == 1:
        burstiness_out = msg_size_out / numpy.max(ms_over_b)
        offset_out = numpy.max(offset) + 1

    elif model == 'B': # from end to begining of the flow
        bursts_calc = []
        timeline = []
        timeline_norm = []

        for i in range(len(sw)):
            b = burstiness[i]
            tf = offset[i]
            ti = (msg_size[i] / b) + tf
            timeline.append([ti, +b])
            timeline.append([tf, -b])

        timeline = sorted(timeline, reverse=True)

        local_b = 0
        total_t = 0
        total_tb = 0
        for i in range(len(timeline) - 1):
            t0 = timeline[i][0]
            t1 = timeline[i+1][0]
            dt = t0 - t1
            local_b += timeline[i][1]
            local_tb = dt * local_b
            total_tb += local_tb
            total_t += dt

            bursts_calc.append( (total_tb) / total_t )

        burstiness_out = max(bursts_calc)

        offset_out = timeline[0][0] - msg_size_out/burstiness_out + 1

        if burstiness_out >= 1: #in this case it fails, so redirect to model D
            model = 'D'


    if model == 'C': #from befining to the end of the flow
        bursts_calc = []
        timeline = []

        for i in range(len(sw)):
            b = burstiness[i]
            ti = offset[i]
            tf = (msg_size[i] / b) + ti
            timeline.append([ti, +b])
            timeline.append([tf, -b])

        timeline = sorted(timeline)

        local_b = 0
        total_t = 0
        total_tb = 0
        for i in range(len(timeline) - 1):
            t0 = timeline[i][0]
            t1 = timeline[i+1][0]
            local_b += timeline[i][1]
            local_tb = (t1 - t0) * local_b
            total_tb += local_tb
            total_t += t1 - t0

            bursts_calc.append( (total_tb) / total_t )

        burstiness_out = min(bursts_calc)
        offset_out = timeline[0][0] + 1

        if burstiness_out > 1:
            burstiness_out = 1

    if model == 'D': #find the min offset for the nearest beta=1 departure curve
        timeline = []

        for i in range(len(sw)):
            b = burstiness[i]
            ti = offset[i]
            tf = (msg_size[i] / b) + ti
            timeline.append([ti, +b])
            timeline.append([tf, -b])

        timeline = sorted(timeline)

        msg_size_total = []
        msg_size_cum = 0
        msg_size_total.append(msg_size_cum) #start with zero packets transmited

        local_b = 0

        for i in range(1, len(timeline)):
            t0 = timeline[i-1][0]
            t1 = timeline[i][0]
            dt = t1 - t0
            local_b += timeline[i-1][1]

            msg_size_cum += dt * local_b

            msg_size_total.append(msg_size_cum)


        offsets = []
        for i in range(len(timeline)):
            t = timeline[i][0]
            p = msg_size_total[i]

            offsets.append((p - t) * -1)

        burstiness_out = 1
        offset_out = max(offsets) + 1

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