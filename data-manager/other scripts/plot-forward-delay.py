import pickle

import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import burr, pareto, invgamma, uniform



dir = '/home/joao/noc-data/hw-measurements/'
# file_in = 'relay-delay-uc-light-sensor-noled10.0ks@3.0Mbps.data'
# file_in = 'relay-delay-fpga-100.0ks@1.5Mbps.data'
file_in = 'relay-delay-uc-delay-until-1ms-10.0ks@3.0Mbps.data'
# file_in = 'relay-delay-fpga-10.0ks@3.0Mbps.data'
file_out = file_in + '.eps'

d_in = pickle.load(open(dir + file_in, 'rb'))

print ('Total of ' + str(len(d_in)) + ' samples.')

# Filter
# r = 1e09
# for i in range(len(d)):
#     d[i] = d[i]*r
#     d[i] = round(d[i], 0)/r



d = [x*1000000 for x in d_in] #in us
d_max = max(d)
d_min = min(d)
d_count = len(d)  # d = d_count

np.savetxt(dir + file_in + '.csv', d, delimiter=',')

fig, ax = plt.subplots(1, 1, figsize=(6.5, 3.1), dpi=120, facecolor='w', edgecolor='w')
ax.set_xlabel('Forward Delay ($\mu s$)')
ax.set_ylabel('Normalized Density')
# ax.grid(True)


# ax.hist(d, 30, facecolor='green', alpha=0.5, normed=0, weights=weights)
# bins = 29

# hist = np.histogram(d, bins='auto')

ax.hist(d, bins='fd', facecolor='lightblue', alpha=1, normed=1, cumulative=1, log=0, align='mid', label='Measured data')
#
# d.sort(reverse=True)
x = np.linspace(d_min,d_max, d_count)

# ax.plot(x, d)

# c = 37.3155
# k = 0.0214833


param = burr.fit(d)
y = burr.cdf(x, param[0], param[1])
ax.plot(x,y,'r--', color='darkred', label='Burr distribution fit')



# f = fitter(d)
# f.fit()
#
# print(f.summary())
# plt.gca().get_yaxis().get_major_formatter().set_powerlimits((0, 0))
# plt.gca().get_xaxis().get_major_formatter().set_powerlimits((0, 0))

# plt.xlim([d_min - abs(2*d_min), d_max + abs(2*d_min)])

ax.legend(loc='lower right', frameon=True, prop={'size':12})
fig.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
plt.savefig(dir + file_out)
plt.show()


