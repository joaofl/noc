import math
import numpy
from scipy import stats
# Testing the results (Time in TTS)

# def b(cfi, bs):
#     if (cfi >= bs):
#         return bs - cfi
#     else:
#         return 0

def produced_until(t, sw):

    if not isinstance(sw[0], list): #if it is a single flow, make it switch-like
        sw = [sw]

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

def time_taken_new(sw, p_in='all'):

    if not isinstance(sw[0], list): #if it is a single flow, make it switch-like
        sw = [sw]

    timeline = []

    for fl in sw:
        b = fl[0]
        rd = fl[1]
        ms = fl[2]

        ti = rd
        tf = (ms / b) + ti

        timeline.append([ti, +b])
        timeline.append([tf, -b])

        if p_in== 'all':
            p_in += ms

    timeline = sorted(timeline)

    msg_size_cum = 0
    msg_size_total = []
    msg_size_total.append(msg_size_cum)
    # msg_size_total.append()  # start with zero packets transmited

    local_b = 0
    for i in range(1, len(timeline)):
        t0 = timeline[i - 1][0]
        t1 = timeline[i][0]
        dt = t1 - t0
        local_b += timeline[i - 1][1]

        msg_size_cum += dt * local_b
        msg_size_total.append(msg_size_cum)


    local_b = 0
    t_total = 0
    for i in range(len(timeline)-1):
        local_b += timeline[i][1]
        t = timeline[i][0]
        p = msg_size_total[i]
        p_n = msg_size_total[i+1]

        if p_in >= p and p_in <= p_n:
            dif = p_in - p
            t_total = t + (dif / local_b)
            break


    return t_total

def time_taken_old(fl, n='all'):
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

def resulting_flow(sw, model='TL'):

    if model not in ['BU','TD','RL','TL']:
        # model = 'TL'
        print('Model not specified. Exiting...')
        exit(-1)

    if len(sw) == 1:
        return [sw[0][0], sw[0][1] + 1, sw[0][2]]


    # for f in sw: # do it fow all the flows getting into that switch
    #     if type(f) == dict:
    #         f = [f['burstness'], f['offset'], f['msgsize']]


    if model == 'BU': #from befining to the end of the flow
        bursts_calc = []
        timeline = []
        ms_out = 0

        for f in sw:
            b = f[0]
            o = f[1]
            ms = f[2]
            ms_out += ms

            if ms > 0 and b > 0:
                ti = o
                tf = (ms / b) + ti
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

            try:
                bursts_calc.append( (total_tb) / total_t )
            except:
                print("Exception division by zero")
                exit(-1)

        burstiness_out = min(bursts_calc)
        offset_out = timeline[0][0] + 1

        if burstiness_out > 1:
            burstiness_out = 1

        return [burstiness_out, offset_out, ms_out]
########################################################################################################################
    if model == 'TL': # from end to begining of the flow
        bursts_calc = []
        timeline = []
        ms_out = 0

        if len(sw) == 1:
            return  [ sw[0][0], sw[0][1]+1, sw[0][2] ]

        for f in sw:
            b = f[0]
            o = f[1]
            ms = f[2]
            ms_out += ms

            if ms > 0 and b > 0:
                tf = o
                ti = (ms / b) + tf
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

            try:
                bursts_calc.append( (total_tb) / total_t )
            except:
                print("Division by zero at TL calculation")

        burstiness_out = max(bursts_calc)

        offset_out = timeline[0][0] - ms_out/burstiness_out + 1

        if burstiness_out >= 1:
            burstiness_out = 1
            if model == 'TL':
                model = 'RL' #in this case it fails, so redirect to model C
        else:
            return [burstiness_out, offset_out, ms_out]


    ###################################################################################################################
    if model == 'RL': #find the min offset for the nearest beta=1 departure curve
        timeline = []
        ms_out = 0

        if len(sw) == 1:
            return  [ sw[0][0], sw[0][1]+1, sw[0][2] ]

        for f in sw:
            b = f[0]
            o = f[1]
            ms = f[2]
            ms_out += ms

            if ms > 0 and b > 0:
                ti = o
                tf = (ms / b) + ti
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

        ts = [timeline[i][0] for i in range(len(timeline))]

        try:
            slope, intercept, r_value, p_value, std_err = stats.linregress(ts, msg_size_total)
        except:
            print('Pau')

        burstiness_out = slope
        if burstiness_out > 1:
            burstiness_out = 1

        offsets = []
        for i in range(len(timeline)):
            t = timeline[i][0]
            n = msg_size_total[i]

            offsets.append(-(n/burstiness_out) + t)

        offset_out = max(offsets) + 1

        return [burstiness_out, offset_out, ms_out]


def arrival_departure(sw):

    if not isinstance(sw[0], list): #if it is a single flow, make it switch-like
        sw = [sw]

    step = 0.02
    ############# Input #############################################
    x = []
    y = []

    msg_size_total = 0

    for f in sw:
        msg_size_total += f[2]

    count = -1
    t = 0

    while(count < msg_size_total):
        count_n = produced_until(t, sw)
        if count_n != count:
            count = count_n
            x.append(t)
            y.append(count)

        t += step

    return [x, y]
