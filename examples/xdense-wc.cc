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
#include "src/noc/model/noc-routing.h"



NS_LOG_COMPONENT_DEFINE("XDenseWCAnalysis");

using namespace std;
using namespace ns3;

//ofstream file_packets_trace_router;
ofstream file_packets_trace_netdevice;
ofstream file_simulation_info;


void
log_netdevice_packets(string context, Ptr<const Packet> pck_r) 
{
    uint64_t now = Simulator::Now().GetNanoSeconds();
    
    Ptr<Packet> pck = pck_r->Copy();
    
    NOCHeader hdnoc;
    pck->RemoveHeader(hdnoc);
    
    XDenseHeader hdxd;
    pck->RemoveHeader(hdxd);
    
    file_packets_trace_netdevice
//  Context info
    << now << ","
    << context << "," //[ i, x, y, port, event ];
    << 0 << ","
//  Packet info
    << pck->GetUid() << ",";
    hdnoc.Print(file_packets_trace_netdevice);
    file_packets_trace_netdevice << ",";
    hdxd.Print(file_packets_trace_netdevice);
    file_packets_trace_netdevice << "\n";
}


uint32_t GetN(uint32_t size_x, uint32_t size_y, uint32_t x, uint32_t y){
    return (size_x * size_y) - ((y + 1) * size_x - x);
}

int
main(int argc, char *argv[]) {

    //LogComponentEnable("NOCNetDevice",LOG_LEVEL_ALL);
    //LogComponentEnable("NOCChannel",LOG_LEVEL_ALL);
    //LogComponentEnable("XDenseHeader",LOG_LEVEL_ALL);
    
    // Default values
    
    uint32_t size_x = 6;
    uint32_t size_y = 2;
    uint32_t size_neighborhood = 0; //odd only, so neighborhoods do not overlap eachother
    uint32_t sinks_n = 1;
    uint32_t baudrate = 3000000; //30000 kbps =  3 Mbps
    uint32_t pck_size = 16 * 10; //16 bytes... But this is not a setting, since it 2 stop bits

    struct passwd *pw = getpwuid(getuid());
    string homedir = pw->pw_dir;
    string context = "WC_ANALYSIS_F1x5";
        
    string output_data_dir = homedir + "/noc-data";
    
    string input_sensors_data_path = "";
//    string input_sensors_data_path = "/home/joao/noc-data/input-data/mixing_layer.csv";
    
    string input_delay_data_path = "";
//    input_delay_data_path = "/home/joao/noc-data/input-data/delays/forward-delay-fpga-10.0ks@3.0Mbps.data.csv";
 
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
    context_dir << "/";
    context_dir << "nw" << size_x << "x" <<size_y;
//    context_dir << "s" << sinks_n;
//    context_dir << "n" << size_neighborhood;
    context_dir << "c" << context;
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
    ApplicationContainer my_xdense_app_container;
    ApplicationContainer my_xdense_router_container;
    ApplicationContainer my_xdense_data_io_container;
    
    uint32_t n_nodes = my_node_container.GetN();
        
    for (uint32_t i = 0; i < n_nodes; i++) {
        Ptr<XDenseApp> my_xdense_app = CreateObject<XDenseApp> ();
        Ptr<NOCRouterDelayModel> my_router_delay_model = CreateObject<NOCRouterDelayModel> ();
        

        //Setup app
        my_xdense_app->IsSink = false;
        my_xdense_app->IsActive = true;
        //TODO: Believe it should be get from the packet header itself, or ask the router
        my_xdense_app->PacketDuration = Time::FromInteger((pck_size * 1e9) / baudrate, Time::NS);  //nano seconds
        my_xdense_app->ClusterSize_x = size_neighborhood;
        my_xdense_app->ClusterSize_y = size_neighborhood;

        //Setup router
        Ptr<NOCRouter> my_noc_router = my_node_container.Get(i)->GetApplication(INSTALLED_NOC_ROUTER)->GetObject<NOCRouter>();
        IntegerValue x, y;
        my_noc_router->GetAttribute("AddressX", x);
        my_noc_router->GetAttribute("AddressY", y);
        my_noc_router->SetDataRate(DataRate(baudrate));
        my_noc_router->RoutingDelays = my_router_delay_model; //TODO: use a method set instead, otherwise there will be no default value, and it wont work.
        my_noc_router->ServerPolicy = NOCRouter::ROUND_ROBIN; //TODO: use a method set instead
//        my_noc_router->ServerPolicy = NOCRouter::FIFO;
//        my_noc_router->SetRoutingProtocolUnicast(NOCRouting::ROUTING_PROTOCOL_XY_CLOCKWISE);
        my_noc_router->SetRoutingProtocolUnicast(NOCRouting::ROUTING_PROTOCOL_YFIRST);
	my_router_delay_model->InputData = &my_input_data;
        
        
        ostringstream context_router_rx, context_router_tx, context_router_cx, context_router_gx;
        
        context_router_cx << i << "," << x.Get() << "," << y.Get() << "," << (int) NOCRouting::DIRECTION_L << ",c";
        my_noc_router->TraceConnect("RouterCxTrace", context_router_cx.str(), MakeCallback(&log_netdevice_packets));
        
        context_router_gx << i << "," << x.Get() << "," << y.Get() << "," << (int) NOCRouting::DIRECTION_L << ",g";
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
        my_node_container.Get(i)->AddApplication(my_noc_router);
        my_node_container.Get(i)->AddApplication(my_router_delay_model);
        my_node_container.Get(i)->AddApplication(my_xdense_app);

        my_xdense_app_container.Add(my_xdense_app);
        my_xdense_data_io_container.Add(my_router_delay_model);
        my_xdense_app->AddRouter(my_noc_router);
    }

    uint32_t s1x, s1y, s2x, s2y, s3x, s3y, s4x, s4y;
    
    s1x = 0; s1y = 0;
    s2x = 19; s2y = 0;
    s3x = 10; s3y = 0;
    s4x = 15; s4y = 0;
    
    //Tweak the compiler if not in use
    s1x = s1x; s1y = s1y; 
    s2x = s2x; s2y = s2y; 
    s3x = s3x; s3y = s3y; 
    s4x = s4x; s4y = s4y;    
    
    uint32_t jitter, burst, distance;
    
//    Time t_ns = Time::FromInteger(0, Time::NS);
    
    for (uint32_t x = 0; x < size_x; x++) {
        for (uint32_t y = 0; y < size_y; y++) {
            uint32_t n = GetN(size_x, size_y, x, y);
            //One have to make sure to not schedule 2 flows to the same node
            //neither send packets to itsel
            
            
            distance = (x - s1x) + (y - s1y);
            distance = 5;
            distance = distance;
            
//            jitter = y * size_x - 1;
            jitter = 1;
            burst = 10;
            burst = burst;
//            uint32_t start;
           
            for (uint8_t ss = 0 ; ss < 1 ; ss++){
                
//                start = ss * 1000 + 1;
//                start = 1;
            
    //            if (y != 0 && x != size_x - 1){ 
    //                my_xdense_app_container.Get(n)->GetObject<XDenseApp>()->SetFlowGenerator(period, jitter, duration, s1x, s1y, false);                                          
    //            }            
//                if (y == 0 && x > 0 && x <= distance){ 
//                    my_xdense_app_container.Get(n)->GetObject<XDenseApp>()->SetFlowGenerator(start, period, jitter, burst, s1x, s1y, false);                                          
//                }            
//                if (y == 1 && x == 2){ 
//                    my_xdense_app_container.Get(n)->GetObject<XDenseApp>()->SetFlowGenerator(0.50, jitter, 20, s1x, s1y, true);                                          
//                }            
//                if (y == 2 && x == 1){ 
//                    my_xdense_app_container.Get(n)->GetObject<XDenseApp>()->SetFlowGenerator(0.80, jitter, 15, s1x, s1y, true);                                          
//                }            
//                if (y == 0 && x == 1){ 
//                    my_xdense_app_container.Get(n)->GetObject<XDenseApp>()->SetFlowGenerator(0.30, jitter, 25, s1x, s1y, true);                                          
//                }            
                if (y == 1 && x > 0){// 0 &&x <= size_x - 6){ 
                    my_xdense_app_container.Get(n)->GetObject<XDenseApp>()->SetFlowGenerator(1, jitter, burst, s1x, s1y, false);                                          
                }            
//                if (y == 2 && x > 0 && x <= distance){ 
//                    my_xdense_app_container.Get(n)->GetObject<XDenseApp>()->SetFlowGenerator(start, period, jitter, burst, s1x, s1y, false);                                          
//                }            
    //            else if (y == 1){ 
    //                my_xdense_app_container.Get(n)->GetObject<XDenseApp>()->SetFlowGenerator(period, jitter, duration, s1x, s1y, false);               
    //            }
    //            else if(y != 0 && x == size_x - 1){
    ////                Simulator::Schedule(t_ns, &XDenseApp::SetFlowGenerator, my_xdense_app_container.Get(n)->GetObject<XDenseApp>(), period, jitter, 1, s1x, s1y, true);
    //                my_xdense_app_container.Get(n)->GetObject<XDenseApp>()->SetFlowGenerator(period, jitter, 1, s1x, s1y, true); 
    //            }
//                else if(y == 1 && x == distance + 1){
//    //                Simulator::Schedule(t_ns, &XDenseApp::SetFlowGenerator, my_xdense_app_container.Get(n)->GetObject<XDenseApp>(), period, jitter, 1, s1x, s1y, true);
//                    my_xdense_app_container.Get(n)->GetObject<XDenseApp>()->SetFlowGenerator(start, 1, jitter, 1, s1x, s1y, true); 
//                }
            }
//            }
                
                
        }
    }


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

    //**************** Output Printing ***************************

    cout << "Simulation complete." << endl ;
    file_packets_trace_netdevice.close();
    cout << "Log files created at: '" << dir_output << "'" << endl;

    Simulator::Destroy();
    return 0;
}
