import math
import random
import matplotlib.pyplot as plt




def get_t(fs, n):
    for f in fs:
        t = ...
    return t
def get_n(fs, t):
    for f in fs:
        n = math.ceil((t - f['offset']) * f['burstness'])
        if n < 0:
            n = 0
        if n > f['msgsize']:
            n = f['msgsize']
    return n

b_in1 = random.random()
b_in2 = random.random()

bb = (b_in1 + b_in2)
if bb > 1:
    bb = 1

b_out = random.random() * bb

f_in1 = {'burstness':b_in1, 'offset':0, 'msgsize':20}
f_in2 = {'burstness':b_in2, 'offset':10, 'msgsize':20}
f_out = {'burstness':b_out, 'offset':20, 'msgsize':40}







######################## Plot #########################
fig, ax_main = plt.subplots(figsize=(6.5, 3.1), dpi=120, facecolor='w', edgecolor='w')

ax_main.step(t_in, n_in, '-', where='post', label='Arrival')

ax_main.step(t_out, n_out, '-', where='post', label='Departure')

ax_main.step(t_queue, n_queue, '-', where='post', label='Queue')

ax_main.step(t_delay, n_delay, '-', where='post', label='Delay')

ax_main.set_xlabel("Transmission time slot (TTS)")
ax_main.set_ylabel("Cumulative packet count")

plt.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
plt.legend(loc=0, fontsize=11)
plt.grid(True)

plt.show()