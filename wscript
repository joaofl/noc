# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

def configure(conf):
    conf.env['armadillo'] = conf.check(mandatory=True, lib='armadillo', uselib_store='ARMADILLO')
    conf.env['opencv'] = conf.check(mandatory=False, lib='opencv', libpath="/usr/local/lib", uselib_store='OPENCV')

#    conf.env.append_value("CXXFLAGS", ["-O3", "-larmadillo"])
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('noc', ['core', 'mobility', 'network', 'mpi'])
    module.use.append("ARMADILLO")
    module.use.append("OPENCV")
    module.source = [
        'helper/noc-grid-helper.cc',
        'model/noc-net-device.cc',
        'model/noc-channel.cc',
        'model/noc-remote-channel.cc',
        'model/noc-header.cc',
        'model/noc-application.cc',
        'model/noc-address.cc',
        'model/calc.cc',
        'model/sensor.cc',
        'model/sensor-data-io.cc',
        'model/noc-switch.cc',
        'model/fast-edge.cc',
        'model/imageio.cc'
        ]
        

    module_test = bld.create_ns3_module_test_library('noc')
    module_test.source = [
        'test/noc-test-suite.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'noc'
    headers.source = [
        'helper/noc-grid-helper.h',
        'model/noc-net-device.h',
        'model/noc-channel.h',
        'model/noc-remote-channel.h',
        'model/noc-header.h',
        'model/noc-application.h',
        'model/noc-address.h',
        'model/calc.h',
        'model/sensor.h',
        'model/noc-types.h',
        'model/noc-switch.h',
        'model/sensor-data-io.h',
        'model/fast-edge.h',
        'model/imageio.h'
        ]


    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    if bld.env['ENABLE_GSL']:
        module.use.extend(['GSL', 'GSLCBLAS', 'M'])
        #obj_test.use.extend(['GSL', 'GSLCBLAS', 'M'])

    #bld.ns3_python_bindings()

