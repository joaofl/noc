import math
import random
import matplotlib.pyplot as plt

def calculate_flow(fs):

    m = 0
    for f in fs:
        m += f['msgsize']

    t = 0
    n = 0
    step = 0.01

    t_axis = [t]
    n_axis = [n]

    while (n < m):
        n = 0
        for f in fs:
            n_l = math.ceil((t - f['offset']) * f['burstness'])
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

def get_t(F):
    n_a = []
    t_a = []

    timeline = []
    bursts_calc = []

    ms_total = 0
    for f in F:
        b = f['burstness']
        o = f['offset']
        ms = f['msgsize']
        ms_total += ms
        tf = (ms / b) + o
        timeline.append([o, +b])
        timeline.append([tf, -b])

    timeline = sorted(timeline)

    local_b = 0
    total_t = timeline[0][0]
    total_n = 0

    t_a.append(timeline[0][0])
    n_a.append(0)

    for i in range(len(timeline) - 1):
        t0 = timeline[i][0]
        t1 = timeline[i + 1][0]
        local_b += timeline[i][1]
        local_n = (t1 - t0) * local_b
        total_n += local_n
        total_t += t1 - t0

        t_a.append(total_t)
        n_a.append(total_n)

    return [t_a, n_a]

# def get_n(fs, t):
#     for f in fs:
#         n = math.ceil((t - f['offset']) * f['burstness'])
#         if n < 0:
#             n = 0
#         if n > f['msgsize']:
#             n = f['msgsize']
#     return n

b_in1 = random.random()
b_in2 = random.random()
b_in3 = random.random()

bb = (b_in1 + b_in2) / 2
if bb > 1:
    bb = 1

b_out = bb

f_in1 = {'burstness':b_in1, 'offset':3.3 * b_in1, 'msgsize':6}
f_in2 = {'burstness':b_in2, 'offset':10 * b_in2, 'msgsize':15}
f_in3 = {'burstness':b_in3, 'offset':5 * b_in3, 'msgsize':7}
F = [f_in1, f_in2, f_in3]

f_out = {'burstness':b_out, 'offset':20, 'msgsize':21}


t_in, n_in = calculate_flow(F)
t_out, n_out = calculate_flow([f_out])

t_queue, n_queue = calculate_queue(t_in, n_in, t_out, n_out)
t_delay, n_delay = calculate_delay(t_in, n_in, t_out, n_out)

t_math_in, n_math_in = get_t(F)
t_math_out, n_math_out = get_t([f_out])

######################## Plot #########################
fig, ax_main = plt.subplots(figsize=(4.2, 3.5), dpi=120, facecolor='w', edgecolor='w')

# ax_main.step(t_queue, n_queue, '-', where='post', label='Queue')
# ax_main.step(t_delay, n_delay, '-', where='post', label='Delay')

ax_main.step(t_in, n_in, '-', where='post', label='Arrival')
ax_main.step(t_out, n_out, '-', where='post', label='Departure')

ax_main.plot(t_math_in, n_math_in, label='Arr Math')
ax_main.plot(t_math_out, n_math_out, label='Dep Math')

# for i, n in enumerate(n_delay):
#     xs = [t_in[n], t_out[n]]
#     ys = [n, n]
#     ax_main.plot(xs, ys, color='grey', alpha=0.7)


ax_main.set_xlabel("Transmission time slot (TTS)")
ax_main.set_ylabel("Cumulative packet count")

plt.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
plt.legend(loc=0, fontsize=11)
plt.grid(True)

# fig, ax_main = plt.subplots(figsize=(4.2, 3.5), dpi=120, facecolor='w', edgecolor='w')
#
#
# ax_main.step(t_in, n_in, '-', where='post', label='Arrival')
# ax_main.step(t_out, n_out, '-', where='post', label='Departure')
#
# for j, t in enumerate(t_queue):
#     s = get_fx(t, t_out, n_out)
#     xs = [t, t]
#     ys = [0 + s, n_queue[j] + s]
#     ax_main.plot(xs, ys, color='grey', alpha=0.7)
#
# ax_main.set_xlabel("Transmission time slot (TTS)")
# ax_main.set_ylabel("Cumulative packet count")
#
# plt.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
# plt.legend(loc=0, fontsize=11)
# plt.grid(True)

plt.show()



