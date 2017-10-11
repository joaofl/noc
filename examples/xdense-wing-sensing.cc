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
ofstream file_packet_trace;
ofstream file_simulation_info;
ofstream file_flows_source;
ofstream file_sensed_data;

Time start_offset, packet_duration;

void
log_netdevice_packets(string context, Ptr<const Packet> pck_r) 
{
//    uint64_t now = Simulator::Now().GetNanoSeconds();
    uint64_t now = (Simulator::Now() - start_offset).GetNanoSeconds();
    
    Ptr<Packet> pck = pck_r->Copy();
    
    NOCHeader hdnoc;
    pck->RemoveHeader(hdnoc);
    
    XDenseHeader hdxd;
    pck->RemoveHeader(hdxd);
    
    file_packet_trace
//  Context info
    << now << ","
    << context << "," //[ i, x, y, port, event ];
    << 0 << ","
//  Packet info
    << pck->GetUid() << ",";
    hdnoc.Print(file_packet_trace);
    file_packet_trace << ",";
    hdxd.Print(file_packet_trace);
    file_packet_trace << "\n";
}

void
log_sensed_data(string context, int64_t data){
    uint64_t now = (Simulator::Now() - start_offset).GetNanoSeconds();
    
    file_packet_trace //put in the same file
//  Context info
    << now << ","
    << context << "," //[ i, x, y, port, event ];
    << data << '\n';
} 

void
log_sensed_data_received(string context, int64_t data, int32_t cord_x, int32_t cord_y){
    uint64_t now = (Simulator::Now() - start_offset).GetNanoSeconds();
    
    file_packet_trace //put in the same file
//  Context info
    << now << ","
    << context << "," //[ i, x, y, port, event ];
    << data << ","
    << cord_x << "," 
    << cord_y << '\n';
    
//    cout << "Data received\n";
} 

//void
//log_flows_source(string context, int32_t ox, int32_t oy, int32_t dx, int32_t dy, 
//        double offset, double burstness, uint8_t ms, uint8_t protocol){
//    string route;
//    
//    if (protocol == NOCHeader::PROTOCOL_UNICAST){
//      route = NOCRouting::EndToEndRoute(ox, oy,dx, dy, NOCHeader::PROTOCOL_UNICAST);  
//    }
//    else if (protocol == NOCHeader::PROTOCOL_UNICAST_OFFSET){
//        route = NOCRouting::EndToEndRoute(ox, oy,dx, dy, NOCHeader::PROTOCOL_UNICAST_OFFSET); 
//    }
//    
//    file_flows_source << context << "," << burstness << "," << offset << "," << (int) ms << "," << route << endl;
//}

uint32_t 
GetN(uint32_t size_x, uint32_t size_y, uint32_t x, uint32_t y){
    return (size_x * size_y) - ((y + 1) * size_x - x);
}

int
main(int argc, char *argv[]) {

    //LogComponentEnable("NOCNetDevice",LOG_LEVEL_ALL);
    //LogComponentEnable("NOCChannel",LOG_LEVEL_ALL);
    //LogComponentEnable("XDenseHeader",LOG_LEVEL_ALL);
    
    // Default values
    
    uint32_t size_x = 20*2; //This represents a single quadrant
    uint32_t size_y = 30;
    uint32_t size_neighborhood = 1; //radius. includes all nodes up to 2 hops away (5x5 square area)
    uint32_t sinks_n = 1;
    uint32_t sink_x = 20;
    uint32_t sink_y = 10;
    uint32_t baudrate = 3000000; //30000 kbps =  3 Mbps
    uint32_t pck_size = 16 * 10; //16 bytes... But this is not a setting, since it 2 stop bits
    
    string beta_str;
    
    struct passwd *pw = getpwuid(getuid());
    string homedir = pw->pw_dir;
    string context = "WING_";

    string output_data_dir = homedir + "/noc-data";
    
    string input_sensors_data_path = "/home/joao/noc-data/input-data/sensors/pitching-onera.csv";
    string input_delay_data_path = "";
    string input_shaping_data_path = "";
    
    string extra = "XX"; //Contains shaping information
 
    CommandLine cmd;
    cmd.AddValue("output_data", "Directory for simulations output", output_data_dir);
    cmd.AddValue("input_data", "Directory for simulations input", input_sensors_data_path);
    cmd.AddValue("input_delay_data", "Directory with delays measurements", input_delay_data_path);
    cmd.AddValue("context", "String to identify the simulation instance", context);
    cmd.AddValue("size_x", "Network size in the X axe", size_x);
    cmd.AddValue("size_y", "Network size in the Y axe", size_y);
    cmd.AddValue("size_n", "Neighborhood size", size_neighborhood);
    cmd.AddValue("sinks", "Network size in the X axe", sinks_n);
    cmd.AddValue("baudrate", "Communication baudrate (bbps)", baudrate);
    cmd.AddValue("beta", "Burstiness of nodes individually", beta_str);
    cmd.AddValue("extra", "Extra context, to be appended to the output dir name", extra);

    cmd.Parse(argc, argv);
    
    ///////////////////////////////////////////////////////////////
    
    
    //Context, which is the string used to create the folder the files will be placed
    stringstream context_dir;
    context_dir << "/";
    context_dir << context;
//    context_dir << "b" << beta_str;
    context_dir << "nw" << size_x << "x" << size_y;
//    context_dir << "s" << sinks_n;
//    context_dir << "n" << size_neighborhood;
    context_dir << "s" << extra;
    context_dir << "/";
    
    packet_duration = Time::FromInteger((pck_size * 1e9) / baudrate, Time::NS);
    start_offset = packet_duration;
    
    string dir_output = output_data_dir + context_dir.str() + "out/";
    string dir_input = output_data_dir + context_dir.str() + "in/";     
    
//  input_sensors_data_path = "/home/joao/noc-data/input-data/mixing_layer.csv";
//  input_delay_data_path = output_data_dir + "/input-data/delays/forward-delay-fpga-10.0ks@3.0Mbps.data.csv";
//    input_shaping_data_path = dir_output + "post/shaping_config.csv.---DONT_LOAD---";
    
    int status;
    status = mkpath(dir_output.c_str());
    //TODO: threat possible errors here
    if (status != 0) {
        cout << "Error creating the directory " << dir_output << "\n";
    }
    
    DataIO my_sensors_input_data;
    
    if ( my_sensors_input_data.LoadArray3D(input_sensors_data_path)  == 0){
        cout << "Error loading the input data file at " << input_sensors_data_path << "\n";
        exit(1);
    }
    else{
        cout << "Sensors data sucessfully loaded:  " << input_sensors_data_path << "\n";
    }

//    NOCRouterShapingConf my_shaping_data;
//    if ( my_shaping_data.LoadData(input_shaping_data_path)  == 0){
//        cout << "Error loading traffic shaping information at " << input_shaping_data_path << "\n";
//    }
//    else{
//        cout << "Traffic shaping information loaded from " << input_shaping_data_path << "\n";
//    }
    
    /////////////////////// Create logfiles ///////////////////////
    string filename;

    filename = dir_output + "simulation-info.txt";    
    file_simulation_info.open(filename.c_str(), ios::out);    
    file_simulation_info
            << "--size_x=" << size_x
            << " --size_y=" << size_y
            << " --size_neighborhood=" << size_neighborhood
            << " --sinks_n=" << sinks_n
            << " --baudrate=" << baudrate
            << " --beta=" << beta_str
            << " --packet_size=" << pck_size
            << " --output_data=" << output_data_dir
            << " --input_data=" << input_sensors_data_path
            << " --input_delay_data=" << input_delay_data_path
            << " --context=" << context
            << " --extra=" << extra;
        cout << "Log file created at " << filename << endl;
        
    file_simulation_info.close(); //it is only modified here, so can be closed

    filename = dir_output + "packets-trace-netdevice.csv";
    file_packet_trace.open(filename.c_str(), ios::out);
    cout << "Log file created at " << filename << endl;

    filename = dir_output + "sensed-data-trace.csv";
    file_sensed_data.open(filename.c_str(), ios::out);
    cout << "Log file created at " << filename << endl;    

    filename = dir_output + "flows-source.csv";
    file_flows_source.open(filename.c_str(), ios::out);
    cout << "Log file created at " << filename << endl;
    
    
     ///////////// Setup and initialize the network
   
    //Using the new helper
    GridHelper my_grid_network_helper;
    my_grid_network_helper.SetNetworkAttribute("SizeX", size_x);
    my_grid_network_helper.SetNetworkAttribute("SizeY", size_y);
    my_grid_network_helper.SetNetworkAttribute("ChannelCount", 1); //net devices per direction
                                                          //total = n x 4
    
    my_grid_network_helper.SetDeviceAttribute("DataRate", DataRateValue(DataRate(baudrate)));
    my_grid_network_helper.SetDeviceAttribute("InterframeGap", TimeValue(MilliSeconds(0)));
    my_grid_network_helper.SetDeviceAttribute("SerialComm", BooleanValue(true));
    
    my_grid_network_helper.SetChannelAttribute("Delay", TimeValue(MilliSeconds(0)));
    
    NodeContainer my_node_container;
    my_node_container = my_grid_network_helper.InitializeNetwork();
    
    //Install applications;
    ApplicationContainer my_xdense_app_container;
    ApplicationContainer my_xdense_router_container;
    ApplicationContainer my_xdense_data_io_container;
    
    uint32_t n_nodes = my_node_container.GetN();
    
    Ptr<XDenseApp> my_xdense_sink_app;
        
    for (uint32_t i = 0; i < n_nodes; i++) {
        Ptr<XDenseApp> my_xdense_app = CreateObject<XDenseApp> ();
        Ptr<XDenseSensorModel> my_sensor_model = CreateObject<XDenseSensorModel> ();
        
        //Setup router
        Ptr<NOCRouter> my_noc_router = my_node_container.Get(i)->GetApplication(INSTALLED_NOC_ROUTER)->GetObject<NOCRouter>();
        IntegerValue x, y;
        my_noc_router->GetAttribute("AddressX", x);
        my_noc_router->GetAttribute("AddressY", y);
        my_noc_router->SetDataRate(DataRate(baudrate));
        my_noc_router->ServerPolicy = NOCRouter::FIFO;
        my_noc_router->SetRoutingProtocolUnicast(NOCRouting::ROUTING_PROTOCOL_XY_CLOCKWISE);
	
        //Setup app
        
        int64_t v = my_sensors_input_data.GetValue3D(0,x.Get(),y.Get());
        if (v != -1)
            my_noc_router->IsActive = true; //Disable nodes which are non existent, which are "outside" the wing
        else
            my_noc_router->IsActive = false;
            
        
        if (y.Get() != sink_y || x.Get() != sink_x){
            my_xdense_app->IsSink = false;
        }
        else {
            my_xdense_app->IsSink = true;
            my_xdense_sink_app = my_xdense_app;
        }
        
        //TODO: Believe it should be get from the packet header itself, or ask the router
        my_xdense_app->PacketDuration = packet_duration;  //nano seconds
        my_xdense_app->ClusterSize_x = size_neighborhood;
        my_xdense_app->ClusterSize_y = size_neighborhood;
        
        my_sensor_model->SensorPosition.x = x.Get();
        my_sensor_model->SensorPosition.y = y.Get();
        my_sensor_model->InputData = &my_sensors_input_data;
        my_xdense_app->Sensor = my_sensor_model;
        
        
        ostringstream context[5];
        
//        context[0] << i << "," << x.Get() << "," << y.Get();
//        my_xdense_app->TraceConnect("FlowSourceTrace", context[0].str(), MakeCallback(&log_flows_source));
        
        context[3] << i << "," << x.Get() << "," << y.Get() << ",0,s";
        my_xdense_app->TraceConnect("SensedDataTrace", context[3].str(), MakeCallback(&log_sensed_data));  

        context[4] << i << "," << x.Get() << "," << y.Get() << ",0,S";
        my_xdense_app->TraceConnect("SensedDataReceivedTrace", context[4].str(), MakeCallback(&log_sensed_data_received));  

        context[1] << i << "," << x.Get() << "," << y.Get() << "," << (int) NOCRouting::DIRECTION_L << ",c";
        my_noc_router->TraceConnect("RouterCxTrace", context[1].str(), MakeCallback(&log_netdevice_packets));
//        my_noc_router->TraceConnect("RouterCxTrace", context[1].str(), MakeCallback(&log_flows));
        
        context[2] << i << "," << x.Get() << "," << y.Get() << "," << (int) NOCRouting::DIRECTION_L << ",g";
        my_noc_router->TraceConnect("RouterGxTrace", context[2].str(), MakeCallback(&log_netdevice_packets));
//        my_noc_router->TraceConnect("RouterGxTrace", context[2].str(), MakeCallback(&log_flows));
        
        
        //Setup NetDevice's Callback
        Ptr<NOCNetDevice> my_net_device;
        int8_t direction;
        for (uint8_t j = 0 ; j < my_noc_router->GetNDevices() ; j++)
        {
            my_net_device = my_noc_router->GetNetDevice(j);
            direction = my_noc_router->GetNetDeviceInfo(my_net_device).direction; 
            
            ostringstream context_nd[2];
               
            context_nd[0] << i << "," << x.Get() << "," << y.Get() << "," << (int) direction << ",r";
            my_net_device->TraceConnect("MacRx", context_nd[0].str(), MakeCallback(&log_netdevice_packets));
            context_nd[1] << i << "," << x.Get() << "," << y.Get() << "," << (int) direction << ",t";
            my_net_device->TraceConnect("MacTx", context_nd[1].str(), MakeCallback(&log_netdevice_packets));            
        }

        //Should be installed in this order!!!
        my_node_container.Get(i)->AddApplication(my_noc_router);
        my_node_container.Get(i)->AddApplication(my_sensor_model);
        my_node_container.Get(i)->AddApplication(my_xdense_app);

        my_xdense_app_container.Add(my_xdense_app);
        my_xdense_data_io_container.Add(my_sensor_model);
        my_xdense_app->AddRouter(my_noc_router);
    }

    
    Simulator::Schedule(packet_duration , &XDenseApp::NodesDataToClusterDataRequest, my_xdense_sink_app);
    
    
    //////////////////// From here, initialize the application at the nodes ///////////////// 

//    for (uint32_t x = 0; x < size_x; x++) {
//        for (uint32_t y = 0; y < size_y; y++) {
//            
//            uint32_t n = GetN(size_x, size_y, x, y);
////            uint16_t d = NOCRouting::Distance(sink_x, sink_y, x, y);
//          
//            if (y == sink_y && x == sink_x){
//                //sink does not send to itself
//                my_xdense_app_container.Get(n)->GetObject<XDenseApp>()->IsSink = true;
//                continue;
//            } 
//            //my_xdense_app_container.Get(n)->GetObject<XDenseApp>()->m_flows_source(x, y, sink_x, sink_y, offset, beta, ms, NOCHeader::PROTOCOL_UNICAST);
//            Ptr<XDenseApp> app = my_xdense_app_container.Get(n)->GetObject<XDenseApp>();
//            
//            for (uint16_t n = 1 ; n <= 1 ; n++){
//                Simulator::Schedule(n * packet_duration * 100 , &XDenseApp::DataSharing, app, sink_x, sink_y);
//            }
//        }
//    }



    cout << endl << "Simulation started, please wait..." << endl ;
    Simulator::Stop(Seconds(1));
    Simulator::Run();
    //**************** Output Printing ***************************
    cout << "Simulation completed successfully" << endl ;
    
    file_packet_trace.close();
    file_flows_source.close();
    file_sensed_data.close(); 
    
    cout << "Log files saved" << endl;

    Simulator::Destroy();
    return 0;
}
