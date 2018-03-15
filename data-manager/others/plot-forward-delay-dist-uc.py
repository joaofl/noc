import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import burr, pareto, invgamma, uniform, norm, dgamma, gaussian_kde

import files_io


file_in = '/home/joao/noc-data/input-data/delays/forward-delay-uc-high-uart-irq-by-node-10ks@3.0Mbps.data.csv'

d = files_io.load_numerical_list(file_in)

pck_dur = 53333

d[:] = [ x/pck_dur for x in d] #in us

d_max = max(d)
d_min = min(d)
d_mean = np.mean(d)
d_count = len(d)  # d = d_count

fig, ax = plt.subplots(1, 1, figsize=(6.5, 3.1), dpi=120, facecolor='w', edgecolor='w')
ax.set_xlabel('Internal Delay ($\mu s$)')
# ax.set_ylabel('Normalized Density')
# ax.grid(True)

ax.hist(d, bins='fd', facecolor='lightblue', alpha=1, normed=1, cumulative=0, log=0, align='mid')#, label='Histogram')
# ax.hist(d, bins=100, facecolor='lightblue', alpha=1, normed=1, cumulative=1, log=0, align='mid', label='Measured data')

x = np.linspace(d_min, d_max, d_count)


param_norm = norm.fit(d)
y_norm = norm.pdf(x, param_norm[0], param_norm[1])
ax.plot(x, y_norm,'r--', color='darkgreen', label='Normal distribution fit')

print(param_norm)

ax.legend(loc='best', frameon=True, prop={'size':12})
fig.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
# file_out = file_in + '-pdf.eps'
# plt.savefig(dir + file_out)


plt.show()


