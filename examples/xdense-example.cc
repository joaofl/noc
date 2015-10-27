/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 João Loureiro
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: João Loureiro <joflo@isep.ipp.pt>
 *         Michele Albano <mialb@isep.ipp.pt>
 *         Tiago Cerqueira <1090678@isep.ipp.pt>
 * 
 */

#include <errno.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include "ns3/core-module.h"
//#include "ns3/config-store-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "src/core/model/object-base.h"

#include "ns3/noc-module.h"
#include "src/noc/model/xdense-application.h"
#include "src/noc/model/sensor-data-io.h"
#include "src/noc/model/noc-header.h"


NS_LOG_COMPONENT_DEFINE("NOCExample");


//myrepo
//ssh://aruntu@joaofl.ddns.net//media/hd/Joao/Repositorios/ns-3-dev
//https://bitbucket.org/joaofl/noc

//run command Netbeans
//"${OUTPUT_PATH}" $(cat ~/usn-data/config/input-config.c.csv)

using namespace std;
using namespace ns3;



ofstream file_packets_trace_router;
ofstream file_packets_trace_netdevice;

uint32_t t_slot(){
    //set this to 1 if one wants the start to 
    //be offset to the first transmission, instead of t=0
    static uint64_t t0 = 0; 
    static uint32_t t_transmission = 1;
    static uint64_t t_slot = 0; 
    
    uint64_t now = Simulator::Now().GetNanoSeconds();
    
    if (t0 == 1){ //first iteration
        t0 = now;
    }
    else if (t_transmission == 1 && now > t0){
        t_transmission = now - t0;
        t_slot = 1;
    }
    else if (now > t0){
        t_slot = (now - t0) / t_transmission;
    }
    
    return t_slot;
}

//context << i << "," << x.Get() << "," << y.Get() << "," << (int) direction << ",r";
void
log_router_packets(string context, Ptr<const Packet> pck) 
{
    static uint64_t i = 1;
    uint64_t now = Simulator::Now().GetNanoSeconds();

    NOCHeader hd;
    pck->PeekHeader(hd);
    
    file_packets_trace_router
    << "c,"
    << i << ","
    << t_slot() << ","
    << now << ","
    << pck->GetUid() << ","
    << context << "," 
    << "p,";
    hd.Print(file_packets_trace_router);
    
    i++;
}

void
log_netdevice_packets(string context, Ptr<const Packet> pck) 
{
    static uint64_t i = 1;
    uint64_t now = Simulator::Now().GetNanoSeconds();
    
    NOCHeader hd;
    pck->PeekHeader(hd);
    
    file_packets_trace_netdevice
    << "c,"
    << i << ","
    << t_slot() << ","
    << now << ","
    << pck->GetUid() << ","
    << context << "," 
    << "p,";
    hd.Print(file_packets_trace_netdevice);
    
    i++;
}

int
main(int argc, char *argv[]) {

    //LogComponentEnable("NOCNetDevice",LOG_LEVEL_ALL);
    //LogComponentEnable("NOCChannel",LOG_LEVEL_ALL);
    //LogComponentEnable("XDenseHeader",LOG_LEVEL_ALL);
    
    
    // Default values
    
    uint32_t sampling_cycles = 10;
    uint32_t sampling_period = 100000; //at every 100.000.000 ns = 100ms default
    uint32_t size_x = 101;
    uint32_t size_y = 101;
    uint32_t size_neighborhood = 4;
    uint32_t sinks_n = 1;
    uint32_t baudrate = 1000; //1000 kbps =  1 Mbps
    uint32_t operational_mode = 0; //sample the entire network
    
    struct passwd *pw = getpwuid(getuid());
    string homedir = pw->pw_dir;
        
    string io_data_dir = homedir + "/usn-data";
    
    stringstream context_dir;
    context_dir << "/nw" << size_x << "x" <<size_y << "s" << sinks_n << "n" << size_neighborhood << "/";
    
    string dir_output = io_data_dir + context_dir.str() + "out/";
    string dir_input = io_data_dir + context_dir.str() + "in/";    

    CommandLine cmd;
    cmd.AddValue("size_x", "Network size in the X axe", size_x);
    cmd.AddValue("size_y", "Network size in the Y axe", size_y);
    cmd.AddValue("operational_mode", "Operate by detecting events (=1) or for sampling all the network (=0)", operational_mode);
    cmd.AddValue("size_neighborhood", "Neighborhood size", size_neighborhood);
    cmd.AddValue("sinks_n", "Network size in the X axe", sinks_n);
    cmd.AddValue("sampling_cycles", "The number of times each node should sample its sensor, and perform data exchange", sampling_cycles);
    cmd.AddValue("sampling_period", "The period between each sensor sampling [us]", sampling_period);
//    cmd.AddValue("log_start_at_period", "The period from witch logging starts", log_start_at_period);
    cmd.AddValue("baudrate", "The baudrate of the node's communication ports [kbps]", baudrate);
    cmd.AddValue("io_data_dir", "Directory with the I/O simulation data", io_data_dir);

    cmd.Parse(argc, argv);
    ///////////////////////////////////////////////////////////////

    dir_output = io_data_dir + "/out/";
    dir_input = io_data_dir + "/in/";
    
    int status;
    status = mkpath(dir_output.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    //TODO: threat possible errors here
    if (status != 0) {
        cout << "Error creating the directory " << dir_output << "\n";
    }
   
    //Using the new helper
    GridHelper my_grid_network_helper;
    my_grid_network_helper.SetNetworkAttribute("SizeX", size_x);
    my_grid_network_helper.SetNetworkAttribute("SizeY", size_y);
    my_grid_network_helper.SetNetworkAttribute("ChannelCount", 1); //net devices per direction
                                                          //total = n x 4
    
    my_grid_network_helper.SetDeviceAttribute("DataRate", DataRateValue(DataRate(baudrate * 1000)));
    my_grid_network_helper.SetDeviceAttribute("InterframeGap", TimeValue(MilliSeconds(0)));
    my_grid_network_helper.SetDeviceAttribute("SerialComm", BooleanValue(true));
    //If the connection is not serial, then the packet size defines the link width,
    //and one packet is transmitted in one tick of a given baudrate
    //my_grid_network.SetDeviceAttribute("PacketSize", IntegerValue(4)); //in bytes
    
    my_grid_network_helper.SetChannelAttribute("Delay", TimeValue(MilliSeconds(0)));
    
    NodeContainer my_node_container;
    my_node_container = my_grid_network_helper.InitializeNetwork();
    
    //Install applications;
    ApplicationContainer my_xdense_sink_app_container;
    ApplicationContainer my_xdense_app_container;
    ApplicationContainer my_xdense_router_container;
    ApplicationContainer my_xdense_sensor_container;
    
    uint32_t n_nodes = my_node_container.GetN();
    
        //TODO: this should be done inside the sensor module
    NOCInputData my_input_data;
    if ( my_input_data.LoadFromFile(dir_input + "input-data.s.csv")  == 0){
        cout << "Error loading the input data file at " << dir_input << "input-data.s.csv";
        return -1;
    }
    
    
    for (uint32_t i = 0; i < n_nodes; i++) {
//        uint32_t x = i % size_x;
//        uint32_t y = floor(i / size_y);

        Ptr<XDenseApp> my_xdense_app = CreateObject<XDenseApp> ();
        Ptr<Sensor> my_sensor = CreateObject<Sensor> ();

        //Setup app
        my_xdense_app->IsSink = false;
        my_xdense_app->MaxHops = size_neighborhood;
        my_xdense_app->SamplingCycles = sampling_cycles; // at least 2, since the first is sacrificed to
        my_xdense_app->SamplingPeriod = sampling_period; // at least 2, since the first is sacrificed to
        my_xdense_app->OperationalMode = 255; //Not defined, since it is defined by the sink ND packet
        my_xdense_app->SetStartTime(Seconds(0));


        //Setup router
        Ptr<NOCRouter> my_noc_router = my_node_container.Get(i)->GetApplication(INSTALLED_NOC_ROUTER)->GetObject<NOCRouter>();
        IntegerValue x, y;
        my_noc_router->GetAttribute("AddressX", x);
        my_noc_router->GetAttribute("AddressY", y);
//        my_noc_router->GetAttribute("UniqueID", i);
        
        int8_t direction = -1;
        ostringstream context_router_rx, context_router_tx, context_router_cx, context_router_gx;
        context_router_rx << i << "," << x.Get() << "," << y.Get() << "," << (int) direction << ",r";
        my_noc_router->TraceConnect("RouterRxTrace", context_router_rx.str(), MakeCallback(&log_router_packets));
        context_router_tx << i << "," << x.Get() << "," << y.Get() << "," << (int) direction << ",t";
        my_noc_router->TraceConnect("RouterTxTrace", context_router_tx.str(), MakeCallback(&log_router_packets));
        
        context_router_cx << i << "," << x.Get() << "," << y.Get() << "," << (int) NOCRouter::DIRECTION_L << ",c";
        my_noc_router->TraceConnect("RouterCxTrace", context_router_cx.str(), MakeCallback(&log_router_packets));
        
        
        //Setup NetDevice's Callback
        Ptr<NOCNetDevice> my_net_device;
        for (uint8_t j = 0 ; j < my_noc_router->GetNDevices() ; j++)
        {
            my_net_device = my_noc_router->GetNetDevice(j);
            direction = my_noc_router->GetNetDeviceInfo(my_net_device).direction; 
            
            ostringstream context_nd_rx, context_nd_tx;
            context_nd_rx << i << "," << x.Get() << "," << y.Get() << "," << (int) direction << ",r";
            my_net_device->TraceConnect("MacRx", context_nd_rx.str(), MakeCallback(&log_netdevice_packets));
            context_nd_tx << i << "," << x.Get() << "," << y.Get() << "," << (int) direction << ",t";
            my_net_device->TraceConnect("MacTx", context_nd_tx.str(), MakeCallback(&log_netdevice_packets));            
        }
        
        context_router_gx << i << "," << x.Get() << "," << y.Get() << "," << (int) NOCRouter::DIRECTION_L << ",g";
        my_noc_router->TraceConnect("RouterGxTrace", context_router_gx.str(), MakeCallback(&log_netdevice_packets));

        //Setup sensor
        my_sensor->SensorPosition.x = x.Get();
        my_sensor->SensorPosition.y = y.Get();
        my_sensor->InputData = &my_input_data;


        //Should be installed in this order!!!
        my_node_container.Get(i)->AddApplication(my_xdense_app);
        my_node_container.Get(i)->AddApplication(my_sensor);

        my_xdense_app_container.Add(my_xdense_app);
        my_xdense_sensor_container.Add(my_sensor);
        my_xdense_app->AddRouter(my_noc_router);
    }


    double delta_x = (double) size_x / (2 * (double) sinks_n); 
    // takes the individuals in the center of the network
    //TODO: a way of manually setting it
    
    for (uint32_t i = 0; i < sinks_n; i++) {
        uint32_t x = floor((i + 1) * 2 * delta_x - delta_x);
        uint32_t y = floor((double) size_y / 2);
        uint32_t n = x + y * size_x;
        my_xdense_app_container.Get(n)->GetObject<XDenseApp>()->IsSink = true;
        my_xdense_app_container.Get(n)->GetObject<XDenseApp>()->OperationalMode = operational_mode; //the sink should spread the operational mode to others
        my_xdense_sink_app_container.Add(my_xdense_app_container.Get(n)); //container with the sinks only
    }



    //**************** Simulation Setup **************************

    //PacketMetadata::Enable ();
    //AnimationInterface anim("animation.xml");
    string filename;
    filename = dir_output + "packets-trace-router.csv";
    
    file_packets_trace_router.open(filename.c_str(), ios::out);

    filename = dir_output + "packets-trace-netdevice.csv";
    
    file_packets_trace_netdevice.open(filename.c_str(), ios::out);


    Simulator::Stop(Seconds(50));
    Simulator::Run();

    //************************************************************


    //**************** Output Printing ***************************

    cout << "Done!" << endl << endl;
    
    
    file_packets_trace_router.close();
    file_packets_trace_netdevice.close();
    
    cout << "Log file created at: '" << filename << "'" << endl;
    cout << "Log file created at: '" << filename << "'" << endl;

    cout << Simulator::Now().GetSeconds();

    Simulator::Destroy();
    return 0;
}