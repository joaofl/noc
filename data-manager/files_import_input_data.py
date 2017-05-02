#Author: João Loureiro
from math import nan

from PIL import Image
import numpy as np
from numpy.lib.npyio import savetxt
import files_io
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import math
import csv
from operator import itemgetter, attrgetter, methodcaller
from scipy.interpolate import griddata

INDEX_I = 0
INDEX_X = 1
INDEX_Y = 2
INDEX_Z = 3
INDEX_P = 4

def ImportFromSU2(input_file):
    #Convert list in dictionary for post processing
    # Indexes from su2 csv files are:
    # "Global_Index", "x_coord", "y_coord", "z_coord", "Pressure", "Pressure_Coefficient", "Mach_Number"

    csv_reader = csv.reader(open(input_file, "rt"), delimiter=',')
    header = next(csv_reader) #grab the header and skip the first line



    arr_x,arr_y,arr_z,arr_p,arr_xyz, arr_xy = [],[],[],[],[],[]

    for _, e in enumerate(csv_reader):
        i, x, y, z, p = float(e[INDEX_I]), float(e[INDEX_X]), float(e[INDEX_Y]), float(e[INDEX_Z]), float(e[INDEX_P])
        arr_xyz.append( (i, x, y, z, p) )
        arr_xy.append((x,y))
        arr_x.append(x)
        arr_y.append(y)
        arr_z.append(z)
        arr_p.append(p)

    arr_xyz_sorted_y = sorted(arr_xyz, key=itemgetter(INDEX_Y)) #from smaller to greater

    #Get the bottom and the tips of the wing
    y_min = arr_xyz_sorted_y[0][INDEX_Y]
    y_max = arr_xyz_sorted_y[-1][INDEX_Y]
    y_range = y_max - y_min

    #Walk a little inside (2%) on both extremes, towards inside the wing
    k=0
    while (arr_xyz_sorted_y[k][INDEX_Y] < y_min + y_range*0.005):
        k+=1
    y_min_corrected = arr_xyz_sorted_y[k][INDEX_Y]

    j=len(arr_xyz_sorted_y)-1
    while (arr_xyz_sorted_y[j][INDEX_Y] > y_max - y_range*0.03):
        j-=1
    y_max_corrected = arr_xyz_sorted_y[j][INDEX_Y]

    #From this two points, walk to the outmost points on x at the given ymax and ymin
    arr_xyz_sorted_x = sorted(arr_xyz, key=itemgetter(INDEX_X))

    #Get the right and left extremes of the wing
    x_min = arr_xyz_sorted_x[0][INDEX_X]
    x_max = arr_xyz_sorted_x[-1][INDEX_X]
    x_range = x_max - x_min


    extent = (arr_xyz_sorted_x[0][INDEX_X], arr_xyz_sorted_x[-1][INDEX_X],
              arr_xyz_sorted_y[0][INDEX_Y], arr_xyz_sorted_y[-1][INDEX_Y])

    #The points are
    point_rb = [0,0,0]
    point_rt = [0,0,0]
    point_lb = [x_max,0,0]
    point_lt = [x_max,0,0]
    round_digits = 3

    # Approach from left to right to find the lefmost points
    for e in arr_xyz_sorted_x:
        current_round_y = round(e[INDEX_Y],round_digits)
        if current_round_y == round(y_min_corrected, round_digits): #here it should give some flexibility
            if e[INDEX_X] > point_rb[0]:
                point_rb = [e[INDEX_X], e[INDEX_Y], e[INDEX_Z]]
        if current_round_y == round(y_max_corrected, round_digits): #here it should give some flexibility
            if e[INDEX_X] > point_rt[0]:
                point_rt = [e[INDEX_X], e[INDEX_Y], e[INDEX_Z]]

    # Approach from right to left and find the rightmost points
    for e in reversed(arr_xyz_sorted_x):
        current_round_y = round(e[INDEX_Y], round_digits)
        if current_round_y == round(y_min_corrected, round_digits):  # here it should give some flexibility
            if e[INDEX_X] < point_lb[0]:
                point_lb = [e[INDEX_X], e[INDEX_Y], e[INDEX_Z]]
        if current_round_y == round(y_max_corrected, round_digits):  # here it should give some flexibility
            if e[INDEX_X] < point_lt[0]:
                point_lt = [e[INDEX_X], e[INDEX_Y], e[INDEX_Z]]


    #Found the points, now its time to separate the wing in the upper and bottom halfs
    #by cutting everything below/above the plane defined by the point_xx points...
    arr_xy_wing_top = []
    arr_p_wing_top = []

    arr_xy_wing_bottom = []
    arr_p_wing_bottom = []

    #Find the plane equation to define the separation plane
    v1 = np.array(point_rb) - np.array(point_rt)
    v2 = np.array(point_lb) - np.array(point_rt)

    # the cross product is a vector normal to the plane
    cp = np.cross(v1, v2)
    a, b, c = cp

    # This evaluates a * x3 + b * y3 + c * z3 which equals d
    d = np.dot(cp, point_rb)
    # print('The equation is {0}x + {1}y + {2}z = {3}'.format(a, b, c, d))

    for e in arr_xyz:
        x = e[INDEX_X]
        y = e[INDEX_Y]
        z = e[INDEX_Z]

        z_plane = (d - a*x - b*y) / c

        if z > z_plane:
            arr_xy_wing_top.append([e[INDEX_X], e[INDEX_Y]])
            arr_p_wing_top.append(e[INDEX_P])
        else:
            arr_xy_wing_bottom.append([e[INDEX_X], e[INDEX_Y]])
            arr_p_wing_bottom.append(e[INDEX_P])

    lsx = np.linspace(x_min, x_max, res)
    lsy = np.linspace(y_min, y_max, res)
    grid_x, grid_y = np.meshgrid(lsx, lsy)

    data_top = griddata(arr_xy_wing_top, arr_p_wing_top, (grid_x, grid_y), method='linear')
    data_bottom = griddata(arr_xy_wing_bottom, arr_p_wing_bottom, (grid_x, grid_y), method='linear')
    
    
    # x_const = x_res / x_range
    # y_const = y_res / y_range
    #
    # point_rb = [point_rb[0]*x_const, point_rb[1]*y_const, point_rb[2]]
    # point_rt = [point_rt[0]*x_const, point_rt[1]*y_const, point_rt[2]]
    # point_lb = [point_lb[0]*x_const, point_lb[1]*y_const, point_lb[2]]
    # point_lt = [point_lt[0]*x_const, point_lt[1]*y_const, point_lt[2]]

    return [data_top, data_bottom, point_lb, point_lt, point_rb, point_rt, extent]

def DeploySensors(p_lb, p_lt, p_rb, p_rt):

    xs_bottom = np.linspace(p_lb[0], p_rb[0], nw_size_x)
    xs_top = np.array([e + p_lt[0] - p_lb[0] for e in xs_bottom])
    ys_left = np.linspace(p_lb[1], p_lt[1], nw_size_y)

    x_interspace = xs_bottom[1] - xs_bottom[0]
    y_interspace = ys_left[1] - ys_left[0]

    placement = []

    a_r, b_r = findline(p_rb[0], p_rb[1], p_rt[0], p_rt[1])
    for i,_ in enumerate(xs_bottom):
        a, b = findline(xs_bottom[i], ys_left[0], xs_top[i], ys_left[-1])

        for j,ye in enumerate(ys_left):
            # plus half interspace, so it folds down to the bottom surface at the front edge of the wing
            x_pos = ((ye - b) / a) + (x_interspace/2)
            y_pos = ye

            #Check if it is inside the wing
            x_lim = (ye - b_r) / a_r
            if x_pos < x_lim*0.98:
                placement.append([i, j, x_pos, y_pos])

    return x_interspace, y_interspace, placement


def ImportFromCFD(input_file ='/home/joao/noc-data/input-data/sensors/sources/surface_flow_00015.csv',
                  output_dir ='/home/joao/noc-data/input-data'):


    ns_x, ns_y = 15, 35 #number of sensors in each direction

    res_x, res_y = 500, 800 #mesh size
    crop_y_top = 0.02
    crop_y_bottom = 0.01

    output = np.empty([ns_y, ns_x])


    temp = files_io.load_list(input_file)

    if temp == []:
        print('No input file found: {}'.format(input_file))
        exit(0)

    points = []
    x, y, p = [], [], []


    for i in range(1, len(temp) - 1):
        l = temp[i]
        if float(l[2]) >= 0: #get only the upper part of the wing, without overlaping with the data from the bottom surface
            points.append([float(l[0]), float(l[1])]) #, float(l[3])]) #x , y, pressure
            x.append(float(l[0]))
            y.append(float(l[1]))
            p.append(float(l[3]))

    xmax = max(x)
    xmin = min(x)
    ymax = max(y)
    ymin = min(y)

    lsx = np.linspace(xmin, xmax, res_x)
    lsy = np.linspace(ymin, ymax, res_y)
    grid_x, grid_y = np.meshgrid(lsx, lsy)

    data = griddata(points, p, (grid_x, grid_y), method='linear')


    # Find the edges of the wing
    ytop = int(data.shape[0] * ( 1 - crop_y_top)) #y size
    ybottom = int(data.shape[0] * crop_y_bottom) #y size

    xtop = [np.NaN, np.NaN]
    xbottom = [np.NaN, np.NaN]

    for x in range(0, data.shape[1], 1):
        if math.isnan(data[ytop][x]) == False and math.isnan(xtop[0]) == True:
            xtop[0] = x
        if math.isnan(data[ybottom][x]) == False and math.isnan(xbottom[0]) == True:
            xbottom[0] = x

    for x in range(data.shape[1] - 1, -1, -1):
        if math.isnan(data[ytop][x]) == False and math.isnan(xtop[1]) == True:
            xtop[1] = x
        if math.isnan(data[ybottom][x]) == False and math.isnan(xbottom[1]) == True:
            xbottom[1] = x

    dx = xbottom[1] - xbottom[0]
    # Comment the line below if nodes are supposed to fit, or be left
    xtop[1] = xtop[0] + dx

    #Now, find each line equation between bottom and top
    xlinetop = np.linspace(xtop[0] + 5, xtop[1] - 5, ns_x)
    xlinebottom = np.linspace(xbottom[0] + 5, xbottom[1] - 5, ns_x)
    # xlinetop = xlinebottom

    ycords = np.linspace(ybottom, ytop, ns_y)

    xcordsall = []

    ix, iy = 0,0

    for i in range(len(xlinebottom)):
        xcords = []
        a, b = findline(xlinebottom[i], ybottom, xlinetop[i], ytop) #find the line coeffs
        iy = 0
        for v in ycords:
            vy = int(v)
            vx = int((v - b) / a)
            # vx = xlinebottom[i]
            xcords.append(vx)
            try:
                output[iy][ix] = data[vy][vx]
            except:
                output[iy][ix] = np.NaN
                # print('Error')

            iy += 1

        xcordsall.append(xcords.copy())
        ix += 1

    # plt.subplot(121)
    plt.figure('1', figsize=(2.8, 4.2), dpi=120, facecolor='w', edgecolor='w')
    plt.imshow(data, cmap=plt.get_cmap('jet'), origin='lower', interpolation='nearest') #bicubi,
    # plt.scatter(xtop, [ytop, ytop], s=20, color='red')
    # plt.scatter(xbottom, [ybottom, ybottom], s=20, color='red')

    for i in range(len(xlinebottom)):
        for j,v in enumerate(ycords):
            tmp_x = xcordsall[i][j]
            tmp_y = ycords[j]
            try:
                if not np.isnan(data[int(tmp_y)][int(tmp_x)]):
                    plt.scatter(tmp_x, tmp_y, s=2.4, color='grey')
                    # print(data[int(tmp_y)][int(tmp_x)])
            except:
                None
                # print('Opa')
    plt.tight_layout(pad=0.5, w_pad=0.8, h_pad=0.9)

    # plt.subplot(122)
    plt.figure('2', figsize=(2.8, 4.2), dpi=120, facecolor='w', edgecolor='w')
    plt.imshow(output, cmap=plt.get_cmap('jet'), origin='lower', interpolation='nearest')
    plt.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.9)

    plt.show()


def ImportFromImage(input_file ='/home/joao/noc-data/input-data/sources/images/flow.png', output_dir ='/home/joao/noc-data/input-data'):
    # im_file = str(QFileDialog.getOpenFileName(self, ("Select an image file"), tbWorkingDir.text() + '\..\images'))

    input_data_list = []

    imarray = load_data_from_image(input_file) #todo: use open CV to load instead
    # countour_image = find_contour(im_file, show=0)

    input_data_list.append(imarray)
    # input_data_list.append(imarray)
    write_input_data_to_disk(output_dir, input_data_list)

    # imgg.set_array(input_data_list[0])
    # canvas_gd.draw()
    # ig = 0
    # timer_gd.start(200)

def ImportFromVideo(output_dir = '/home/joao/noc-data/input-data/mixing_layer.csv'):
    # video_file = str(QFileDialog.getOpenFileName(self, ("Select a video file"), tbWorkingDir.text() + '\..\videos'))
    input_data_list = []
    video_file = '/home/joao/noc-data/input-data/sources/Large Eddy Simulation of a Plane Turbulent Mixing Layer - Laminar Inflow.mp4'

    # ti = 10
    xi = 160
    yi = 38
    xf = 420
    yf = 255
    frames = 10

    container = av.open(video_file)
    # video = next(s for s in container.streams if s.type == b'video')
    video = container.streams[0]

    i = 0
    for packet in container.demux(video):
        if i == frames:
            break
        for frame in packet.decode():
            # frame.to_image().save('/path/to/frame-%04d.jpg' % frame.index)
            image = frame.to_image()
            image = convert_image(image, xi=xi, xf=xf, yi=yi, yf=yf)
            input_data_list.append(image)
            i += 1


    input_data_list = normalize_data_array(input_data_list, 16) #16 bits sensors

    image_show(input_data_list[0])

    write_input_data_to_disk(output_dir, input_data_list)

def pil2array(img):
    return np.array(img.getdata(),
                    np.uint8).reshape(img.size[1], img.size[0], 3)

def convert_image(image, xi=0, yi=0, xf=500, yf=500):
    image = pil2array(image)

    image_out = image[..., 0]*0.299 + image[..., 1]*0.587 + image[..., 2]*0.114

    image_out = image_out[yi:yf,xi:xf] #100:880,320:1100

    return image_out

def load_data_from_image(file):
    #example: http://code.activestate.com/recipes/577591-conversion-of-pil-image-and-numpy-array/

    # file = '/home/joao/usn-data/images/flow-bw.png'
    im = Image.open(file)
    # im.show()
    # im = im.convert('I') #reads the image and put it in a int32bits map
    # im.show()
    im = im.resize((self.network_size_x, self.network_size_y), Image.ANTIALIAS)
    # im.show()

    # luminance standard calulation
    # 0.2126 * R + 0.7152 * G + 0.0722 * B

    if im.mode == 'RGBA':
        npimg = np.array(im)
        npimg = npimg[..., 0]*0.299 + npimg[..., 1]*0.587 + npimg[..., 2]*0.114
    elif im.mode == 'LA':
        npimg = np.array(im.convert('RGBA'))
        npimg = npimg[..., 0]*0.299 + npimg[..., 1]*0.587 + npimg[..., 2]*0.114

    # npim = np.asarray(im)
    # npim = np.array(im.getdata(), np.uint8).reshape(im.size[1], im.size[0], 3)
    # npim = np.array(im)

    npimg = normalize_data(npimg)


    # pylab.imshow(npimg, cmap=plt.cm.gray, interpolation='nearest')
    # pylab.show()


    # First test to switch this function to open CV
    # im = cv2.imread(file)
    #
    # # im_grey = im[..., 0]*0.299 + im[..., 1]*0.587 + im[..., 2]*0.114
    #
    # plt.imshow(im)
    #
    # im_resized = cv2.resize(im, (self.network_size_x, self.network_size_y))
    #
    # plt.imshow(im_resized)
    #
    # plt.show()
    #
    # npimg = self.normalize_data(im_resized)

    return npimg

def write_input_data_to_disk(full_path_id, data):
    with open(full_path_id, 'wb') as output_data:
        input_data_snapshot_number = 0
        for frame in data:

            #TODO: check if this functions generates errors
            separator = '@' + str(input_data_snapshot_number)
            savetxt(output_data, frame, delimiter=',', fmt='%u')
            savetxt(output_data, [separator], fmt='%s')

            input_data_snapshot_number += 1

def normalize_data(data):
    #normalize the plot to the sensor resolution and range

    max_value = np.max(data)
    min_value = np.min(data)
    sens_scale = pow(2, self.sbSensorResolution.value())

    try:
        data = data.astype('float') # this conversion does not work when importing from CFD
    except:
        data = data

    data = (data + min_value * -1) / (np.abs(max_value) + np.abs(min_value))
    data *= sens_scale

    # for i in range(0, len(data)):
    #     data[i] = (data[i] + min_value * -1) / (np.abs(max_value) + np.abs(min_value))
    #     data[i] *= sens_scale
    #     # data[i] = np.uint16(data[i])

    return np.uint16(data)

def normalize_data_array(data_array, sensor_resolution):
    #normalize the plot to the sensor resolution and range

    max_value = np.max(data_array)
    min_value = np.min(data_array)
    sens_scale = pow(2, sensor_resolution) - 1

    for i in range(len(data_array)):
        try:
            data_array[i] = data_array[i].astype('float') # this conversion does not work when importing from CFD
        except:
            data_array[i] = data_array[i]

        data_array[i] = (data_array[i] + min_value * -1) / (np.abs(max_value) + np.abs(min_value))
        data_array[i] *= sens_scale

    return np.uint16(data_array)

def translate(value, mino, maxo, mind, maxd):

    do = maxo - mino
    dd = maxd - mind

    v = (dd * value) / do

    return int(v)


def image_show(data, filename='', label_x="", label_y="", colormap='hot_r', axis='on', view=True):
    plt.figure(figsize=(3, 3), dpi=120, facecolor='w', edgecolor='w')
    plt.xlabel(label_x, fontsize=30)
    plt.ylabel(label_y, fontsize=30)
    plt.tight_layout(pad=0, w_pad=0, h_pad=0)
    # plt.labe
    # rect=[0.1, 0.1, 0.1, 0.1]


    if axis == 'off':
        plt.gca().xaxis.set_major_locator(plt.NullLocator())
        plt.gca().yaxis.set_major_locator(plt.NullLocator())

    im = plt.imshow(data, cmap=plt.get_cmap(colormap), interpolation='nearest')

    # cax = plt.divider.append_axes("right", size="5%", pad=0.05)
    # plt.colorbar(im)
    # plt.colorbar(im, cax=cax)


    if filename!='':
        plt.savefig(filename)

    if view==True:
        plt.show()

    plt.close()
#################################################################

def findline(xa, ya, xb, yb):
    a = float(ya - yb) / float(xa - xb)
    b = ya - (a * xa)
    return a,b


def plot(data_top, data_bottom, point_lb, point_lt, point_rb, point_rt, placement, extent):
    # ext = (extent[0][INDEX_X], extent[1][INDEX_X], extent[0][INDEX_Y], extent[1][INDEX_Y])

    sb[0].scatter(point_rb[0], point_rb[1], s=20, color='black', marker='x')
    sb[0].scatter(point_rt[0], point_rt[1], s=20, color='black', marker='o')
    sb[0].scatter(point_lb[0], point_lb[1], s=20, color='black', marker='+')
    sb[0].scatter(point_lt[0], point_lt[1], s=20, color='black', marker='^')

    for [_, _, x_pos, y_pos] in placement:
        sb[0].scatter(x_pos, y_pos, s=5, color='grey', alpha=0.6)

    img_top = sb[0].imshow(data_top, cmap=plt.get_cmap('jet'), origin='lower', interpolation='nearest',
                              extent=extent)


    sb[1].scatter(point_rb[0], point_rb[1], s=20, color='black', marker='x')
    sb[1].scatter(point_rt[0], point_rt[1], s=20, color='black', marker='o')
    sb[1].scatter(point_lb[0], point_lb[1], s=20, color='black', marker='+')
    sb[1].scatter(point_lt[0], point_lt[1], s=20, color='black', marker='^')

    for [_, _, x_pos, y_pos] in placement:
        sb[1].scatter(x_pos, y_pos, s=5, color='grey', alpha=0.6)

    img_bottom = sb[1].imshow(data_bottom, cmap=plt.get_cmap('jet'), origin='lower', interpolation='nearest',
                              extent=extent)


    return img_top, img_bottom

#This has to be according to the real size of the wing,
# so we can use metric interspacing between nodes
#https://home.aero.polimi.it/freecase/?OpenFOAM_%2B_Code_Aster:Aerodynamic_problems:ONERA_M6_wing
# y_res = 1196.3 #mm
#this calcs the base of the wing, till the right top tip
#0.28 = 15.8 degrees, which is the back angle of the Onera M6
# x_res = 805.9 + math.tan(0.28) * y_res
#It gives number of pixels that we want look for values. Its related to the maximum number of sensors deployed.
#but must give a smooth interpolations at least
res = 1000

#How many nodes should be deployed
nw_size_x, nw_size_y = 30, 40 #nodes

#Time step size according to the CFD simulation config file
t_step = 500 #ns

fig, sb = plt.subplots(1, 2, sharey=True)
imgs = []

# for i in range(0,1000):
for i in [100, 110, 120, 130, 140, 150]:
# for i in [100]:
    fn = '/home/joao/noc-data/input-data/sensors/sources/su2/pitching_more_angle/surface_flow_{:05d}.csv'.format(i)
    print('Importing data from SU2 file' + fn)

    [data_top, data_bottom, point_lb, point_lt, point_rb, point_rt, extent] = ImportFromSU2(fn)

    print('Deploying sensors')
    x_interspace, y_interspace, placement = DeploySensors(point_lb, point_lt, point_rb, point_rt)

    print('Ploting')
    img_top, img_bottom = plot(data_top, data_bottom, point_lb, point_lt, point_rb, point_rt, placement, extent)

    imgs.append([img_top, img_bottom])
    plt.tight_layout(pad=0.5, w_pad=0.5, h_pad=0.9)

print('Done processing. Preparing to plot...')
ani = animation.ArtistAnimation(fig, imgs, interval=100, blit=False, repeat_delay=0)
# ani.save('/home/joao/noc-data/input-data/sensors/sources/su2/pitching_oneram6.mp4')

plt.show()
exit(0)