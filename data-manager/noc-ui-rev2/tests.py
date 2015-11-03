# import time
# import numpy as np
# import matplotlib.pyplot as plt
#
# plt.axis([0, 1000, 0, 1])
# plt.ion()
# plt.show()
#
# for i in range(1000):
#     y = np.random.random()
#     plt.scatter(i, y)
#     plt.draw()
#     time.sleep(0.05)

"""
A simple example of an animated plot
"""
# import numpy as np
# import matplotlib.pyplot as plt
# import matplotlib.animation as animation
#
# fig, ax = plt.subplots()
#
# x = np.arange(0, 2*np.pi, 0.01)
# line, = ax.plot(x, np.sin(x))
#
#
# def animate(i):
#     line.set_ydata(np.sin(x + i/10.0))  # update the data
#     return line,
#
#
# # Init only required for blitting to give a clean slate.
# def init():
#     line.set_ydata(np.ma.array(x, mask=True))
#     return line,
#
# ani = animation.FuncAnimation(fig, animate, np.arange(1, 200), init_func=init,
#                               interval=25, blit=True)
# plt.show()

# import numpy as np
# import matplotlib.pyplot as plt
# import matplotlib.animation as animation
#
# fig = plt.figure()
#
#
# def f(x, y):
#     return np.sin(x) + np.cos(y)
#
# x = np.linspace(0, 2 * np.pi, 120)
# y = np.linspace(0, 2 * np.pi, 100).reshape(-1, 1)
#
# im = plt.imshow(f(x, y), cmap=plt.get_cmap('viridis'), animated=True)
#
#
# def updatefig(*args):
#     global x, y
#     x += np.pi / 15.
#     y += np.pi / 20.
#     im.set_array(f(x, y))
#     return im,
#
# ani = animation.FuncAnimation(fig, updatefig, interval=50, blit=True)
# plt.show()

#!/usr/bin/env python
"""
An animated image
"""
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

fig = plt.figure()


def f(x, y):
    return np.sin(x) + np.cos(y)

x = np.linspace(0, 2 * np.pi, 120)
y = np.linspace(0, 2 * np.pi, 100).reshape(-1, 1)
# ims is a list of lists, each row is a list of artists to draw in the
# current frame; here we are just animating one artist, the image, in
# each frame
ims = []
for i in range(60):
    x += np.pi / 15.
    y += np.pi / 20.
    im = plt.imshow(f(x, y), cmap='viridis', animated=True)
    ims.append([im])

ani = animation.ArtistAnimation(fig, ims, interval=50, blit=True,
                                repeat_delay=1000)

#ani.save('dynamic_images.mp4')


plt.show()