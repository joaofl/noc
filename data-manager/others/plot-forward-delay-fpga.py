import pickle

import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import burr, pareto, invgamma, uniform



dir = '/home/joao/noc-data/others/hw-measurements/'
# file_in = 'relay-delay-uc-light-sensor-noled10.0ks@3.0Mbps.data'
# file_in = 'relay-delay-fpga-100.0ks@1.5Mbps.data'
# file_in = 'relay-delay-uc-delay-until-1ms-10.0ks@3.0Mbps.data'
file_in = 'relay-delay-fpga-10.0ks@3.0Mbps.data'


d_in = pickle.load(open(dir + file_in, 'rb'))


d = [x*1000000 for x in d_in] #in us
d_max = max(d)
d_min = min(d)
d_mean = np.mean(d)
d_count = len(d)  # d = d_count

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

# ax.hist(d, bins='fd', facecolor='lightblue', alpha=1, normed=1, cumulative=1, log=0, align='mid', label='Measured data')

x = np.linspace(d_min,d_max, d_count)

param = uniform.fit(d)
# y = uniform.pdf(x, param[0], param[1])
# y = uniform.cdf(x, param[0], param[1])
# ax.plot(x,y,'r--', color='darkred', label='Uniform distribution fit')

# plt.xlim([d_min - abs(2*d_min), d_max + abs(2*d_min)])

ax.legend(loc='upper left', frameon=True, prop={'size':12})
fig.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
file_out = file_in + '-cdf.eps'
plt.savefig(dir + file_out)

##########Do it again for the other plot


fig, ax = plt.subplots(1, 1, figsize=(6.5, 3.1), dpi=120, facecolor='w', edgecolor='w')
ax.set_xlabel('Internal Delay ($\mu s$)')
ax.set_ylabel('Normalized Density')
# ax.grid(True)

ax.hist(d, bins='fd', facecolor='lightblue', alpha=1, normed=1, cumulative=0, log=0, align='mid') #, label='Measured data')

x = np.linspace(d_min,d_max, d_count)

param = uniform.fit(d)
y = uniform.pdf(x, param[0], param[1])
# y = uniform.cdf(x, param[0], param[1])
ax.plot(x,y,'r--', color='green', label='Uniform distribution fit')

print(param)

# plt.xlim([d_min - abs(2*d_min), d_max + abs(2*d_min)])

ax.legend(loc='lower center', frameon=True, prop={'size':12})
fig.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
file_out = file_in + '-pdf.eps'
plt.savefig(dir + file_out)


plt.show()


