import pickle
import matplotlib.pyplot as plt
import numpy as np

dir = '/home/joao/noc-data/hw-measurements/'
# file_in = 'relay-delay-uc-light-sensor-noled10.0ks@3.0Mbps.data'
# file_in = 'relay-delay-fpga-100.0ks@1.5Mbps.data'
file_in = 'relay-delay-uc-delay-until-1ms-10.0ks@3.0Mbps.data'
# file_in = 'relay-delay-fpga-10.0ks@3.0Mbps.data'
file_out = file_in + '.eps'

d = pickle.load(open(dir + file_in, 'rb'))

print ('Total of ' + str(len(d)) + ' samples.')

# Filter
# r = 1e09
# for i in range(len(d)):
#     d[i] = d[i]*r
#     d[i] = round(d[i], 0)/r


d[:] = [x*1000000 for x in d] #in us
d_max = max(d)
d_min = min(d)
d_count = len(d)  # d = d_count

weights = np.ones_like(d)/len(d)

plt.figure('', figsize=(6.5, 3.1), dpi=120, facecolor='w', edgecolor='w')
plt.hist(d, 20, facecolor='green',weights=weights, alpha=0.9, normed=0)

plt.grid(True)

# plt.gca().get_yaxis().get_major_formatter().set_powerlimits((0, 0))
# plt.gca().get_xaxis().get_major_formatter().set_powerlimits((0, 0))

plt.xlabel('Forward Delay ($\mu$s)')
plt.ylabel('Normalized Density')

# plt.xlim([d_min - abs(2*d_min), d_max + abs(2*d_min)])
plt.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.5)
plt.savefig(dir + file_out)
plt.show()




from scipy.stats import expon
import matplotlib.pyplot as plt
fig, ax = plt.subplots(1, 1)

# Calculate a few first moments:
# >>>

mean, var, skew, kurt = expon.stats(moments='mvsk')

# Display the probability density function (pdf):
# >>>

# x = np.linspace(expon.ppf(0.01), expon.ppf(0.99), 100)
x = d
ax.plot(x, expon.pdf(x), 'r-', lw=5, alpha=0.6, label='expon pdf')

# Alternatively, freeze the distribution and display the frozen pdf:
# >>>

rv = expon()
ax.plot(x, rv.pdf(x), 'k-', lw=2, label='frozen pdf')

# Check accuracy of cdf and ppf:
# >>>

vals = expon.ppf([0.001, 0.5, 0.999])
np.allclose([0.001, 0.5, 0.999], expon.cdf(vals))
True

# Generate random numbers:
# >>>

# r = expon.rvs(size=1000)
r=d

# And compare the histogram:
# >>>

ax.hist(r, normed=False, histtype='stepfilled', alpha=0.2)
ax.legend(loc='best', frameon=False)
plt.show()


# vs(loc=0, scale=1, size=1) 	Random variates.
# pdf(x, loc=0, scale=1) 	Probability density function.
# logpdf(x, loc=0, scale=1) 	Log of the probability density function.
# cdf(x, loc=0, scale=1) 	Cumulative density function.
# logcdf(x, loc=0, scale=1) 	Log of the cumulative density function.
# sf(x, loc=0, scale=1) 	Survival function (1-cdf — sometimes more accurate).
# logsf(x, loc=0, scale=1) 	Log of the survival function.
# ppf(q, loc=0, scale=1) 	Percent point function (inverse of cdf — percentiles).
# isf(q, loc=0, scale=1) 	Inverse survival function (inverse of sf).
# moment(n, loc=0, scale=1) 	Non-central moment of order n
# stats(loc=0, scale=1, moments='mv') 	Mean(‘m’), variance(‘v’), skew(‘s’), and/or kurtosis(‘k’).
# entropy(loc=0, scale=1) 	(Differential) entropy of the RV.
# fit(data, loc=0, scale=1) 	Parameter estimates for generic data.
# expect(func, loc=0, scale=1, lb=None, ub=None, conditional=False, **kwds) 	Expected value of a function (of one argument) with respect to the distribution.
# median(loc=0, scale=1) 	Median of the distribution.
# mean(loc=0, scale=1) 	Mean of the distribution.
# var(loc=0, scale=1) 	Variance of the distribution.
# std(loc=0, scale=1) 	Standard deviation of the distribution.
# interval(alpha, loc=0, scale=1) 	Endpoints of the range that contains alpha percent of the distribution

# filename='0'
# title = ""
# lable_x = ""
# lable_y = ""
# x_size = 9
# y_size = 5
# bins = 30
# show=False
# bin_width=0
#
# values = []
# offset = data[0][1] #take the initial time
# r = 1e6
# x_down = data[0][1]/r
# x_up = data[-1][1]/r
#
# # offset = 0
# for line in data:
#     values.append(int(np.round((line[1] - offset) / r)))
#
# bin_width = int(bin_width/r)
#
# bins=range(min(values), max(values) + 2*bin_width, bin_width)
#
#
# # example data
# mu = np.mean(values) # mean of distribution
# sigma = np.std(values) # standard deviation of distribution
#
#
# # the histogram of the data
# pylab.figure(title, figsize=(x_size, y_size), dpi=120, facecolor='w', edgecolor='w')
# n, bins, patches = plt.hist(values, bins, facecolor='blue', alpha=0.9) #, histtype='step')
# #normed=1, facecolor='blue', alpha=0.9, cumulative=True, histtype='step'
# # add a 'best fit' line
# # y = mlab.normpdf(bins, mu, sigma)
# # plt.plot(bins, y, 'r--')
# plt.xlabel(lable_x, fontsize=20)
# plt.ylabel(lable_y, fontsize=20)
# pylab.xlim([x_up, x_down])
# pylab.ylim([0,400])
# plt.title(title)
#
# # Tweak spacing to prevent clipping of ylabel
# plt.subplots_adjust(left=0.15)
#
# ax = pylab.gca()
# # ax.set_xticklabels([0,0,2,4,6,8,10])
# for label in ax.get_xticklabels() + ax.get_yticklabels():
#   label.set_fontsize(18)
#
# pylab.tight_layout()
#
# if filename!='0':
#     dir = filename[:filename.rfind("/")]
#     if not os.path.exists(dir):
#         os.makedirs(dir)
#     pylab.savefig(filename)
# if show == True:
#     plt.show()