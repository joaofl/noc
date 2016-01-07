import pickle
import matplotlib.pyplot as plt

dir = '/home/joao/Repositorios/xdense-fpga/other scripts/'
file_in = 'relay-delay-fpga-100.0ks@1.5Mbps.data'
file_out = file_in + '.eps'

d = pickle.load(open(dir + file_in, 'rb'))

print ('Total of ' + str(len(d)) + ' samples.')

# Filter
# r = 1e09
# for i in range(len(d)):
#     d[i] = d[i]*r
#     d[i] = round(d[i], 0)/r

plt.figure('', figsize=(9, 5), dpi=120, facecolor='w', edgecolor='w')
plt.hist(d, 20, facecolor='green', alpha=0.9, normed=0)
plt.tight_layout()
plt.savefig(dir + file_out)
plt.show()

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
# # pylab.xlim([x_up, x_down])
# # pylab.ylim([0,400])
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