import pickle

import matplotlib.pyplot as plt
import numpy as np
# from pyqt_fit import kde
from pip._vendor import colorama
from scipy.stats import burr, pareto, invgamma, uniform, norm, dgamma, gaussian_kde




dir = '/home/joao/noc-data/others/hw-measurements/'

# file_in = 'relay-delay-fpga-10.0ks@3.0Mbps.data'
file_in = 'relay-delay-uc-high-uart-irq-fine-10ks@3.0Mbps.data'


din = pickle.load(open(dir + file_in, 'rb'))


#10 bits per byte @ 3Mbps
pck_duration_t = 0 #when it is not measured
# pck_duration_t = 53.3e-6 #theoretical
# pck_duration_m = 533e-7 #measured

din[:] = [(x - pck_duration_t)*1000000 for x in din] #in us

d=[]

filter = True
if filter: #remove outliers
    i=0
    while i < len(din) - 1:
        if din[i] > 0 and din[i] < 51:
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
ax.set_ylabel('Normalized Density')
# ax.grid(True)

ax.hist(d, bins='fd', facecolor='lightblue', alpha=1, normed=1, cumulative=0, log=0, align='mid', label='Histogram')
# ax.hist(d, bins=100, facecolor='lightblue', alpha=1, normed=1, cumulative=1, log=0, align='mid', label='Measured data')

x = np.linspace(d_min, d_max, d_count)

# est = kde.KDE1D(d)
# ax.plot(x, est(x), label='Estimate (bw={:.3g})'.format(est.bandwidth))

my_kde_pdf = gaussian_kde(d, bw_method=None)
ax.plot(x, my_kde_pdf(x), label='Gaussian kernel density', color='darkgreen')

# param_norm = uniform.fit(d)
# y_norm = uniform.cdf(x, param_norm[0], param_norm[1])
# ax.plot(x, y_norm,'r--', color='brown', label='Uniform distribution fit')


# plt.xlim([d_min - abs(2*d_min), d_max + abs(2*d_min)])

ax.legend(loc='best', frameon=True, prop={'size':12})
fig.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
file_out = file_in + '-pdf.eps'
plt.savefig(dir + file_out)



##########Do it again for the other plot


# fig, ax = plt.subplots(1, 1, figsize=(6.5, 3.1), dpi=120, facecolor='w', edgecolor='w')
# ax.set_xlabel('Forward Delay ($\mu s$)')
# ax.set_ylabel('Cumulative Density')
# # ax.grid(True)
#
# ax.hist(d, bins='fd', facecolor='lightblue', alpha=1, normed=1, cumulative=1, log=0, align='mid', label='Measured data')
# ax.hist(d, bins=20, facecolor='lightblue', alpha=1, normed=1, cumulative=0, log=0, align='mid', label='Measured data')
#
#
# param_norm = uniform.fit(d)
# y_norm = uniform.pdf(x, param_norm[0], param_norm[1])
# ax.plot(x,y_norm,'r--', color='brown', label='Uniform distribution fit')
#
# plt.xlim([d_min - abs(2*d_min), d_max + abs(2*d_min)])
#
# ax.legend(loc='upper right', frameon=True, prop={'size':12})
# fig.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
# file_out = file_in + '-cdf.eps'
# plt.savefig(dir + file_out)


plt.show()


