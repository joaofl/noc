import math
import random
from math import ceil
import matplotlib.pyplot as plt
import analysis_wc


def simulate_flow(fs):

    m = 0
    for f in fs:
        m += f['msgsize']

    t = 0
    n = 0
    step = 0.0001

    t_axis = [t]
    n_axis = [n]

    while (n < m):
        n = 0
        for f in fs:
            n_l = math.floor((t - f['offset']) * f['burstness'])
            if n_l < 0:
                n_l = 0
            if n_l > f['msgsize']:
                n_l = f['msgsize']

            n += n_l

        if n != n_axis[-1]: #only add new values, since it is a step
            n_axis.append(n)
            t_axis.append(t)
        t += step

    return t_axis, n_axis

def get_fx(value, x, fx):
    fx_out = fx[-1] #start with the maximum produced
    for i, v in enumerate(x):
        if v > value:
            fx_out = fx[i-1]
            break

    return fx_out

def calculate_queue(t_in, n_in, t_out, n_out):

    n_queue = []
    t_queue = sorted(t_in + t_out)

    for t in t_queue:
        nin = get_fx(t, t_in, n_in)
        nout = get_fx(t, t_out, n_out)

        n_queue.append(nin - nout)

    return  t_queue, n_queue

def calculate_delay(t_in, n_in, t_out, n_out):

    n_delay = n_in
    t_delay = []

    for n in n_delay:
        tin = get_fx(n, n_in, t_in)
        tout = get_fx(n, n_out, t_out)

        t_delay.append(tout - tin)

    return t_delay, n_delay


def calculate_delay_model(t_math_in, n_math_in, t_math_out, n_math_out):
    delays = []
    for i,n in enumerate(n_math_in):
        #Horizontal distances
        t = get_t(t_math_out[0], t_math_out[1], n_math_out[0], n_math_out[1], n)
        # xs = [t_math_in[i], t]
        # ys = [n, n]
        delays.append(t_math_in[i] - t)

    return delays


def calculate_queue_model(t_math_in, n_math_in, t_math_out, n_math_out):
    queues = []
    for i, n in enumerate(n_math_in):
        #Vertical distances
        n_calc = get_n(t_math_out[0], t_math_out[1], n_math_out[0], n_math_out[1], t_math_in[i])
        # xs = [t_math_in[i], t_math_in[i]]
        # ys = [n, n_calc]
        queues.append(n - n_calc)

    return queues




def get_points(F):
    timeline = []

    for f in F:
        b = f['burstness']
        ms = f['msgsize']
        ti = f['offset'] + 1/b
        tf = (ms / b) + ti
        timeline.append([ti, +b])
        timeline.append([tf, -b])

    timeline = sorted(timeline)

    ###################################

    local_b = 0
    total_n = 0
    total_t = timeline[0][0]
    n_a = [0]
    t_a = [timeline[0][0]]

    for i in range(len(timeline) - 1):
        t0 = timeline[i][0]
        t1 = timeline[i + 1][0]
        local_b += timeline[i][1]
        local_n = ((t1 - t0) * local_b)
        total_n += local_n
        total_t += (t1 - t0)

        t_a.append(total_t)
        n_a.append(total_n)

    ###################################

    return [t_a, n_a]

def get_t(ti,tf,ni,nf,n):
    s = (tf-ti)/(nf-ni)
    return n*s + ti

def get_n(ti,tf,ni,nf,t):
    s = 1
    if (nf != ni):
        s = (tf-ti)/(nf-ni)
        n = ((t-ti)/s)+ni
    else: #whenever there is a flat part on the arrival curve
        n=ni

    if n < 0: # to desconsider the part of the line that goes below the x-axis
        n = 0
    return n

def plot(t_in, n_in, t_out, n_out, t_math_in, n_math_in, t_math_out, n_math_out, model):

    ######################## Plot #########################
    fig, ax_main = plt.subplots(figsize=(3.6, 3), dpi=110, facecolor='w', edgecolor='w')

    # ax_main.step(t_queue, n_queue, '-', where='post', label='Queue')
    # ax_main.step(t_delay, n_delay, '-', where='post', label='Delay')

    ax_main.step(t_in, n_in, '-', color='green', where='post', label='SIM')
    ax_main.plot(t_math_in, n_math_in,'--', color='blue', label=model)

    ax_main.step(t_out, n_out, '-', color='lightgreen', where='post', label='')
    ax_main.plot(t_math_out, n_math_out,'--', color='blue', label='')

    # for i, n in enumerate(n_delay):
    #     xs = [t_in[n], t_out[n]]
    #     ys = [n, n]
    #     ax_main.plot(xs, ys, color='grey', alpha=0.7)

    for i,n in enumerate(n_math_in):
        #Vertical distances
        # t = get_t(t_math_out[0], t_math_out[1], n_math_out[0], n_math_out[1], n)
        # xs = [t_math_in[i], t]
        # ys = [n, n]
        # ax_main.plot(xs, ys, '-.+', color='grey', alpha=0.8)

        #Horizontal distances
        n_calc = get_n(t_math_out[0], t_math_out[1], n_math_out[0], n_math_out[1], t_math_in[i])
        xs = [t_math_in[i], t_math_in[i]]
        ys = [n, n_calc]
        ax_main.plot(xs, ys, ':+', color='grey', alpha=0.8)


    ti_model = t_math_out[0]
    arr_i = 0
    for i,t in enumerate(t_math_in):
        if t > ti_model:
            arr_i = i-1
            break

    #Vertical distances
    n_calc = get_n(t_math_in[arr_i], t_math_in[arr_i+1], n_math_in[arr_i], n_math_in[arr_i+1], ti_model)
    xs = [ti_model, ti_model]
    ys = [0, n_calc]
    ax_main.plot(xs, ys, ':+', color='grey', alpha=1, label='Queue')

    #Horizontal distances
    # t_calc = get_t(t_math_out[0], t_math_out[1], n_math_out[0], n_math_out[1], n_calc)
    # xs = [ti_model, t_calc]
    # ys = [n_calc, n_calc]
    # ax_main.plot(xs, ys, '-.+', color='grey', alpha=1, label='Delay')

    ax_main.set_xlabel("Transmission time slot (TTS)")
    ax_main.set_ylabel("Cumulative packet count")
    # ax_main.set_xlim([3.5,31])

    plt.tight_layout(pad=0.5, w_pad=0.3, h_pad=0.3)
    plt.legend(loc=0, fontsize=11)
    # plt.legend(loc=0, prop={'size': 6})
    plt.grid(False)

    filename="/home/joao/noc-data/post/model-queue-delay-{}.pdf".format(model)
    plt.savefig(filename)
    # print("Plot saved in " + filename)

def example():
    ######################################################################################

    b_in0 = random.random()
    b_in1 = random.random()
    b_in2 = random.random()
    b_in3 = random.random()
    msg = random.random()

    bb = (b_in1 + b_in2) / 2
    if bb > 1:
        bb = 1

    f_in = []

    # f_in0 = {'burstness':0.25, 'offset':2.2, 'msgsize':6}
    # f_in1 = {'burstness':0.37, 'offset':3.5, 'msgsize':5}
    # f_in2 = {'burstness':0.62, 'offset':4.2, 'msgsize':7}

    f_in.append( {'burstness':1, 'offset':0, 'msgsize':5} )
    f_in.append( {'burstness':1, 'offset':15, 'msgsize':5} )
    # f_in.append( {'burstness':1, 'offset':0, 'msgsize':5} )


    # f_in0 = {'burstness':0.25, 'offset':0, 'msgsize':6}
    # f_in1 = {'burstness':0.37, 'offset':0, 'msgsize':5}
    # f_in2 = {'burstness':0.62, 'offset':0, 'msgsize':7}

    # f_in0 = {'burstness':b_in0, 'offset':1.5 * b_in0, 'msgsize':ceil(12*msg)}
    # f_in1 = {'burstness':b_in1, 'offset':3.3 * b_in1, 'msgsize':ceil(6*msg)}
    # f_in2 = {'burstness':b_in2, 'offset':10 * b_in2, 'msgsize':ceil(15*msg)}

    # f_in3 = {'burstness':b_in3, 'offset':5 * b_in3, 'msgsize':ceil(7*msg)}

    print('In: ', f_in)

    for model in ['Max-S', 'Min-O', "LQ"]:
    # for model in ['Max-S']:

        b,o,ms = analysis_wc.resulting_flow(f_in, model)
        f_out = {'burstness':b, 'offset':o, 'msgsize':ms}

        print('Out: ', f_out)

        # msg_size_total = sum([f['msgsize'] for f in F])

        t_in, n_in = simulate_flow(f_in)
        t_out, n_out = simulate_flow([f_out])

        t_queue, n_queue = calculate_queue(t_in, n_in, t_out, n_out)
        t_delay, n_delay = calculate_delay(t_in, n_in, t_out, n_out)

        t_math_in, n_math_in = get_points(f_in)
        t_math_out, n_math_out = get_points([f_out])


        plot(t_in, n_in, t_out, n_out, t_math_in, n_math_in, t_math_out, n_math_out, model)

        print('Max queue = {}'.format(max(n_queue)))
        print('Total time = {}'.format(o + ms/b))

    plt.show()

example()


