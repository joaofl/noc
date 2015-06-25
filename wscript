# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

def configure(conf):
    conf.env['armadillo'] = conf.check(mandatory=True, lib='armadillo', uselib_store='ARMADILLO')
    conf.env['opencv'] = conf.check(mandatory=False, lib='opencv', libpath="/usr/local/lib", uselib_store='OPENCV')

#    conf.env.append_value("CXXFLAGS", ["-O3", "-larmadillo"])
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('usn', ['core', 'mobility', 'network', 'mpi'])
    module.use.append("ARMADILLO")
    module.use.append("OPENCV")
    module.source = [
        'helper/usn-helper.cc',
        'helper/grid-helper.cc',
        'model/usn-net-device.cc',
        'model/usn-channel.cc',
        'model/usn-remote-channel.cc',
        'model/usn-header.cc',
        'model/usn-application.cc',
        'model/usn-calc.cc',
        'model/usn-sensor.cc',
        'model/usn-io-data.cc',
        'model/usn-switch.cc',
        'model/fast-edge.cc',
        'model/imageio.cc'
        ]
        

    module_test = bld.create_ns3_module_test_library('usn')
    module_test.source = [
        'test/usn-test-suite.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'usn'
    headers.source = [
        'helper/usn-helper.h',
        'helper/grid-helper.h',
        'model/usn-net-device.h',
        'model/usn-channel.h',
        'model/usn-remote-channel.h',
        'model/usn-header.h',
        'model/usn-application.h',
        'model/usn-calc.h',
        'model/usn-sensor.h',
        'model/usn-types.h',
        'model/usn-switch.h',
        'model/usn-io-data.h',
        'model/fast-edge.h',
        'model/imageio.h'
        ]


    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    if bld.env['ENABLE_GSL']:
        module.use.extend(['GSL', 'GSLCBLAS', 'M'])
        #obj_test.use.extend(['GSL', 'GSLCBLAS', 'M'])

    #bld.ns3_python_bindings()

