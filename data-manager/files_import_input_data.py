#Author: João Loureiro

from PIL import Image
import numpy as np
from numpy.lib.npyio import savetxt
import av
import matplotlib.pyplot as plt


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



ImportFromVideo()

exit(0)