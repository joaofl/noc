import pickle

import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import burr, pareto, invgamma, uniform, norm, dgamma



dir = '/home/joao/noc-data/hw-measurements/'
# file_in = 'relay-delay-uc-light-sensor-noled10.0ks@3.0Mbps.data'
# file_in = 'relay-delay-fpga-100.0ks@1.5Mbps.data'
file_in = 'relay-delay-uc-longtimeout-10.0ks@3.0Mbps.data'
# file_in = 'relay-delay-fpga-10.0ks@3.0Mbps.data'


d_in = pickle.load(open(dir + file_in, 'rb'))


din = [x*1000000 for x in d_in] #in us


filter = False
if filter:
    d=[]
    i=0
    while i < len(din) - 1:
        if din[i] < 200:
            d.append(din[i])
        i += 1
else:
    d = din

d_max = max(d)
d_min = min(d)
d_mean = np.mean(d)
d_count = len(d)  # d = d_count

np.savetxt(dir + file_in + '.csv', d, delimiter=',')

print ('Total of ' + str(d_count) + ' samples.')
print ('Min measured: ' + str(d_min))
print ('Mean measured: ' + str(d_mean))
print ('Max measured: ' + str(d_max))

np.savetxt(dir + file_in + '.csv', d, delimiter=',')

####################

fig, ax = plt.subplots(1, 1, figsize=(6.5, 3.1), dpi=120, facecolor='w', edgecolor='w')
ax.set_xlabel('Forward Delay ($\mu s$)')
ax.set_ylabel('Cumulative Density')
# ax.grid(True)

ax.hist(d, bins='fd', facecolor='lightblue', alpha=1, normed=1, cumulative=1, log=0, align='mid', label='Measured data')

x = np.linspace(d_min,d_max, d_count)

# param = burr.fit(d,1,2)
# y = burr.cdf(x, param[0], param[1])
# ax.plot(x,y,'r--', color='darkred', label='Burr distribution fit custom')
#
# param = burr.fit(d)
# y = burr.cdf(x, param[0], param[1])
# ax.plot(x,y,'r--', color='red', label='Burr distribution fit')

param = dgamma.fit(d)
y = dgamma.cdf(x, param[0])
ax.plot(x,y,'r--', color='brown', label='DGamma distribution fit')

param_norm = norm.fit(d)
y_norm = norm.cdf(x, param_norm[0])
ax.plot(x, y_norm,'r--', color='brown', label='Pareto distribution fit')


# plt.xlim([d_min - abs(2*d_min), d_max + abs(2*d_min)])

ax.legend(loc='lower right', frameon=True, prop={'size':12})
fig.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
file_out = file_in + '-cdf.eps'
plt.savefig(dir + file_out)

##########Do it again for the other plot


fig, ax = plt.subplots(1, 1, figsize=(6.5, 3.1), dpi=120, facecolor='w', edgecolor='w')
ax.set_xlabel('Forward Delay ($\mu s$)')
ax.set_ylabel('Normalized Density')
# ax.grid(True)

ax.hist(d, bins='fd', facecolor='lightblue', alpha=1, normed=1, cumulative=0, log=0, align='mid', label='Measured data')

x = np.linspace(d_min,d_max, d_count)

# param = burr.fit(d)
# y = burr.pdf(x, param[0], param[1])
# ax.plot(x,y,'r--', color='darkred', label='Burr distribution fit')
#
# param = pareto.fit(d)
# y = pareto.pdf(x, param[0])
# ax.plot(x,y,'r--', color='brown', label='Pareto distribution fit')

param = dgamma.fit(d)
y = dgamma.pdf(x, param[0])
ax.plot(x,y,'r--', color='brown', label='DGamma distribution fit')

param_norm = norm.fit(d)
y_norm = norm.pdf(x, param_norm[0])
ax.plot(x,y_norm,'r--', color='brown', label='Pareto distribution fit')

# plt.xlim([d_min - abs(2*d_min), d_max + abs(2*d_min)])

ax.legend(loc='upper right', frameon=True, prop={'size':12})
fig.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
file_out = file_in + '-pdf.eps'
plt.savefig(dir + file_out)


plt.show()


