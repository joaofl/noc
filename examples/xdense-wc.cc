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
 * 
 * Repo:
 * https://bitbucket.org/joaofl/noc
 *
 * To run using Netbeans:
 * Project Properties->Run->Run Directory: ./build
 *
 * Using Eclipse
 * Run -> Run Configurations
 *
 * Under ns-3 project, Tab environment variables: Add
 *
 * name: LD_LIBRARY_PATH
 * value: ${workspace_loc:ns-3-dev}/build
 * Mark: "Append..."
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
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "src/core/model/object-base.h"

#include "ns3/noc-module.h"
#include "src/noc/model/data-io.h"
#include "src/noc/model/noc-header.h"
#include "src/noc/model/xdense-header.h"
#include "src/network/model/packet.h"
#include "src/noc/model/xdense-application.h"



NS_LOG_COMPONENT_DEFINE("XDenseWCAnalysis");

using namespace std;
using namespace ns3;

//ofstream file_packets_trace_router;
ofstream file_packets_trace_netdevice;
ofstream file_simulation_info;


void
log_netdevice_packets(string context, Ptr<const Packet> pck, uint32_t queue_size) 
{
    uint64_t now = Simulator::Now().GetNanoSeconds();
    
    NOCHeader hdnoc;
    pck->PeekHeader(hdnoc);
    
    XDenseHeader hdxd;
    pck->PeekHeader(hdxd);
    
    file_packets_trace_netdevice
//  Context info
    << now << ","
    << context << "," //<< i << "," << x.Get() << "," << y.Get() << "," << (int) NOCRouter::DIRECTION_L << ",c";
    << queue_size << ","
//  Packet info
    << pck->GetUid() << ",";
    hdnoc.Print(file_packets_trace_netdevice);
    file_packets_trace_netdevice << ",";
    hdxd.Print(file_packets_trace_netdevice);
    file_packets_trace_netdevice << "\n";
}



int
main(int argc, char *argv[]) {

    //LogComponentEnable("NOCNetDevice",LOG_LEVEL_ALL);
    //LogComponentEnable("NOCChannel",LOG_LEVEL_ALL);
    //LogComponentEnable("XDenseHeader",LOG_LEVEL_ALL);
    
    // Default values
    
    uint32_t size_x = 11;
    uint32_t size_y = 3;
    uint32_t size_neighborhood = 5; //odd only, so neighborhoods do not overlap eachother
    uint32_t sinks_n = 1;
    uint32_t baudrate = 3000000; //30000 kbps =  3 Mbps
    uint32_t pck_size = 16 * 10; //16 bytes... But this is not a setting, since it 2 stop bits

    struct passwd *pw = getpwuid(getuid());
    string homedir = pw->pw_dir;
    string context = "WC_ANALYSIS";
        
    string output_data_dir = homedir + "/noc-data";
    string input_sensors_data_path = "/home/joao/noc-data/input-data/mixing_layer.csv";
    
    string input_delay_data_path = "";
 
    CommandLine cmd;
    cmd.AddValue("context", "String to identify the simulation instance", context);
    cmd.AddValue("size_x", "Network size in the X axe", size_x);
    cmd.AddValue("size_y", "Network size in the Y axe", size_y);
    cmd.AddValue("size_n", "Neighborhood size", size_neighborhood);
    cmd.AddValue("sinks", "Network size in the X axe", sinks_n);
    cmd.AddValue("baudrate", "The baudrate of the node's communication ports [bbps]", baudrate);
    cmd.AddValue("output_data", "Directory for simulation's output", output_data_dir);
    cmd.AddValue("input_data", "Directory for simulation's input", input_sensors_data_path);
    cmd.AddValue("input_delay_data", "Directory with delays measurements", input_delay_data_path);

    cmd.Parse(argc, argv);
    
    ///////////////////////////////////////////////////////////////

    stringstream context_dir;
    context_dir << "/nw" << size_x << "x" <<size_y << "s" << sinks_n << "n" << size_neighborhood << "c";
    context_dir << context;
    context_dir << "/";
    
    string dir_output = output_data_dir + context_dir.str() + "out/";
    string dir_input = output_data_dir + context_dir.str() + "in/";     
    
    int status;
    status = mkpath(dir_output.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    //TODO: threat possible errors here
    if (status != 0) {
        cout << "Error creating the directory " << dir_output << "\n";
    }
    
    DataIO my_input_data;
    
    if ( my_input_data.LoadArray(input_delay_data_path)  == 0){
        cout << "Error loading the input data file at " << input_delay_data_path << "\n";
    }
    else{
        cout << "Delay's data sucessfully loaded:  " << input_delay_data_path << "\n";
    }
    
   
    //Using the new helper
    GridHelper my_grid_network_helper;
    my_grid_network_helper.SetNetworkAttribute("SizeX", size_x);
    my_grid_network_helper.SetNetworkAttribute("SizeY", size_y);
    my_grid_network_helper.SetNetworkAttribute("ChannelCount", 1); //net devices per direction
                                                          //total = n x 4
    
    my_grid_network_helper.SetDeviceAttribute("DataRate", DataRateValue(DataRate(baudrate)));
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
    ApplicationContainer my_xdense_data_io_container;
    
    uint32_t n_nodes = my_node_container.GetN();
        
    for (uint32_t i = 0; i < n_nodes; i++) {
        Ptr<XDenseApp> my_xdense_app = CreateObject<XDenseApp> ();
        Ptr<NOCRouterDelayModel> my_router_delay_model = CreateObject<NOCRouterDelayModel> ();
        

        //Setup app
        my_xdense_app->IsSink = false;
        //TODO: Believe it should be get from the packet header itself
        my_xdense_app->PacketDuration = Time::FromInteger((pck_size * 1e9) / baudrate, Time::NS);  //nano seconds
        my_xdense_app->ClusterSize_x = size_neighborhood;
        my_xdense_app->ClusterSize_y = size_neighborhood;

        //Setup router
        Ptr<NOCRouter> my_noc_router = my_node_container.Get(i)->GetApplication(INSTALLED_NOC_ROUTER)->GetObject<NOCRouter>();
        IntegerValue x, y;
        my_noc_router->GetAttribute("AddressX", x);
        my_noc_router->GetAttribute("AddressY", y);
        my_noc_router->RoutingDelays = my_router_delay_model;
        my_noc_router->SetRoutingProtocolUnicast(NOCRouter::ROUTING_PROTOCOL_YFIRST);
	my_router_delay_model->InputData = &my_input_data;
        
  
//        int8_t direction = -1;
        ostringstream context_router_rx, context_router_tx, context_router_cx, context_router_gx;
        
        context_router_cx << i << "," << x.Get() << "," << y.Get() << "," << (int) NOCRouter::DIRECTION_L << ",c";
        my_noc_router->TraceConnect("RouterCxTrace", context_router_cx.str(), MakeCallback(&log_netdevice_packets));
        
        context_router_gx << i << "," << x.Get() << "," << y.Get() << "," << (int) NOCRouter::DIRECTION_L << ",g";
        my_noc_router->TraceConnect("RouterGxTrace", context_router_gx.str(), MakeCallback(&log_netdevice_packets));
        
        //Setup NetDevice's Callback
        Ptr<NOCNetDevice> my_net_device;
        int8_t direction;
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

        //Should be installed in this order!!!
        my_node_container.Get(i)->AddApplication(my_xdense_app);
        my_node_container.Get(i)->AddApplication(my_router_delay_model);

        my_xdense_app_container.Add(my_xdense_app);
        my_xdense_data_io_container.Add(my_router_delay_model);
        my_xdense_app->AddRouter(my_noc_router);
    }


    uint32_t x = 0;
    uint32_t y = 1;
    uint32_t n = x + y * size_x;
    my_xdense_app_container.Get(n)->GetObject<XDenseApp>()->IsSink = true;
    my_xdense_sink_app_container.Add(my_xdense_app_container.Get(n)); //container with the sinks only
    
//    x=0;
//    y=2;
//    n = x + y * size_x;
//    my_xdense_app_container.Get(n)->GetObject<XDenseApp>()->IsSink = true;
//    my_xdense_sink_app_container.Add(my_xdense_app_container.Get(n)); //container with the sinks only

    //**************** Simulation Setup **************************

    string filename;

    filename = dir_output + "simulation-info.txt";    
    file_simulation_info.open(filename.c_str(), ios::out);    
    file_simulation_info
            << "--size_x=" << size_x
            << " --size_y=" << size_y
            << " --size_neighborhood=" << size_neighborhood
            << " --sinks_n=" << sinks_n
            << " --baudrate=" << baudrate
            << " --packet_size=" << pck_size;
    file_simulation_info.close();

    filename = dir_output + "packets-trace-netdevice.csv";
    file_packets_trace_netdevice.open(filename.c_str(), ios::out);

    cout << endl << "Simulation started. Please wait..." << endl ;
    

    Simulator::Stop(Seconds(1));
    Simulator::Run();

    //************************************************************


    //**************** Output Printing ***************************

    cout << "Simulation complete." << endl ;
    
    
//    file_packets_trace_router.close();
    file_packets_trace_netdevice.close();
    
    
    cout << "Log files created at: '" << dir_output << "'" << endl;

//    cout << Simulator::Now().GetSeconds();

    Simulator::Destroy();
    return 0;
}
