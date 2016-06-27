import math
import numpy
# Testing the results (Time in TTS)

def b(cfi, bs):
    if (cfi >= bs):
        return bs - cfi
    else:
        return 0

def queue(t, sw):
    capacity = 1
    produced = []
    for f in sw: # do it fow all the flows getting into that switch
        period = f[0]
        jitter = f[1]
        bsize = f[2]

        tw = (t-jitter)
        if tw < 0:
            tw = 0

        cf = math.floor((1 / period) * tw)
        bf = b(cf, bsize)
        prod = cf + bf
        produced.append(prod)

    consumed = capacity * t - 1
    if consumed < 0:
        consumed = 0

    tqueue = numpy.sum(produced) - consumed

    if tqueue < 0:
        tqueue = 0

    return tqueue


xo, yo = 1, 10
xd, yd = 0, 0

# {p, j, bs, D} period, jitter, burst size, deadline
swc = 1 #Switch capacity
                 #The internal flow has jitter 0, since it is at the node already
fa = [1, 0, 200] #whereas the flows comming from neighbors take one time cycle more
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
    tnext = queue(tactual, sw)
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


print(q_profile)
print(tt_basic)
print(tt_int)
print(tt_queue)
print(tt_tot)