import files_io


fn_in = '/home/joao/noc-data/input-data/delays/test_multihop_2_hops_2017-02-09_12_30_19_749827.csv'
fn_out = '/home/joao/noc-data/input-data/delays/forward-delay-uc-high-uart-irq-by-node-10ks@3.0Mbps.data.csv'

delays = files_io.load_numerical_list(fn_in, 2)



delays_f = []


for v in delays:
    if v < 600000 and v > 500000:
        nv = int((v-2*53333)/2)
        delays_f.append(nv)


file = open(fn_out, 'w')


for v in delays_f:
    file.write('{}\n'.format(v))

file.close()