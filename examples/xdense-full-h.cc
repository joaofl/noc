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
#include <cstdlib>
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
ofstream file_queue_size_trace;
ofstream file_flow_trace;
ofstream file_simulation_info;
ofstream file_flows_source;

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
log_flows(string context, Ptr<const Packet> pck_r){
    uint64_t now = (Simulator::Now() - start_offset).GetNanoSeconds();
    
    Ptr<Packet> pck = pck_r->Copy();
    
    NOCHeader hdnoc;
    pck->RemoveHeader(hdnoc);
    
    XDenseHeader hdxd;
    pck->RemoveHeader(hdxd);
    
    file_flow_trace
//  Context info
    << now << ","
    << context << "," //[ i, x, y, port, event ];
    << 0 << ","
//  Packet info
    << pck->GetUid() << ",";
    hdnoc.Print(file_flow_trace);
    file_flow_trace << ",";
    hdxd.Print(file_flow_trace);
    file_flow_trace << "\n";    
}

void
log_flows_source(string context, int32_t ox, int32_t oy, int32_t dx, int32_t dy, 
        double offset, double burstness, uint8_t ms, uint8_t protocol){
    string route;
    
    if (protocol == NOCHeader::PROTOCOL_UNICAST){
      route = NOCRouting::EndToEndRoute(ox, oy,dx, dy, NOCHeader::PROTOCOL_UNICAST);  
    }
    else if (protocol == NOCHeader::PROTOCOL_UNICAST_OFFSET){
        route = NOCRouting::EndToEndRoute(ox, oy,dx, dy, NOCHeader::PROTOCOL_UNICAST_OFFSET); 
    }
    
    file_flows_source << context << "," << burstness << "," << offset << "," << (int) ms << "," << route << endl;
}

uint32_t 
GetN(uint32_t size_x, uint32_t size_y, uint32_t x, uint32_t y){
    return (size_x * size_y) - ((y + 1) * size_x - x);
}

double irand(int min, int max) {
    return ((double)rand() / ((double)RAND_MAX + 1.0)) * (max - min + 1) + min;
}

int
main(int argc, char *argv[]) {

    //LogComponentEnable("NOCNetDevice",LOG_LEVEL_ALL);
    //LogComponentEnable("NOCChannel",LOG_LEVEL_ALL);
    //LogComponentEnable("XDenseHeader",LOG_LEVEL_ALL);
    
    // Default values
    
    uint32_t size_x = 1 + (5*2+1) * 2; //This represents a single quadrant
    uint32_t size_y = 1 + (5*2+1) * 2;
    uint32_t size_neighborhood = 1; //radius. includes all nodes up to 2 hops away (5x5 square area)
    uint32_t sinks_n = 1;
    uint32_t baudrate = 3000000; //30000 kbps =  3 Mbps
    uint32_t pck_size = 16 * 10; //16 bytes... But this is not a setting, since it 2 stop bits

    struct passwd *pw = getpwuid(getuid());
    string homedir = pw->pw_dir;
    string context = "WCA_FULL_";


//    ['0.10', '0.20', '0.30', '0.40', '0.50', '0.60', '0.70', '0.80', '0.90', '1.00']      
//    double_t beta = 0.1;
    string beta_str = "0.01";
//    float_t c_rate = 0.05; //compression rate

    string output_data_dir = homedir + "/noc-data";
    
    string input_sensors_data_path = "";
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
    
    stringstream context_dir;
    context_dir << "/";
    context_dir << context;
    context_dir << "b" << beta_str;
    context_dir << "nw" << size_x << "x" << size_y;
//    context_dir << "s" << sinks_n;
    context_dir << "n" << size_neighborhood;
    context_dir << "s" << extra;
    context_dir << "/";
    
    packet_duration = Time::FromInteger((pck_size * 1e9) / baudrate, Time::NS);
    start_offset = packet_duration;
    
    string dir_output = output_data_dir + context_dir.str() + "out/";
    string dir_input = output_data_dir + context_dir.str() + "in/";     
    
//  input_sensors_data_path = "/home/joao/noc-data/input-data/mixing_layer.csv";
//  input_delay_data_path = output_data_dir + "/input-data/delays/forward-delay-fpga-10.0ks@3.0Mbps.data.csv";
    input_shaping_data_path = dir_output + "post/shaping_config.csv.---DONT_LOAD---";
    
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

    NOCRouterShapingConf my_shaping_data;
    if ( my_shaping_data.LoadData(input_shaping_data_path)  == 0){
        cout << "Error loading traffic shaping information at " << input_shaping_data_path << "\n";
    }
    else{
        cout << "Traffic shaping information loaded from " << input_shaping_data_path << "\n";
    }
    
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
            << " --packet_size=" << pck_size;
    cout << "Log file created at " << filename << endl;
    

    filename = dir_output + "packets-trace-netdevice.csv";
    file_packet_trace.open(filename.c_str(), ios::out);
    cout << "Log file created at " << filename << endl;

//    filename = dir_output + "queue-size-trace.csv";
//    file_queue_size_trace.open(filename.c_str(), ios::out);
//    cout << "Log file created at " << filename << endl;

//    filename = dir_output + "flows-trace.csv";
//    file_flow_trace.open(filename.c_str(), ios::out);
//    cout << "Log file created at " << filename << endl;
    

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
        
        //Setup router
        Ptr<NOCRouter> my_noc_router = my_node_container.Get(i)->GetApplication(INSTALLED_NOC_ROUTER)->GetObject<NOCRouter>();
        IntegerValue x, y;
        my_noc_router->GetAttribute("AddressX", x);
        my_noc_router->GetAttribute("AddressY", y);
        my_noc_router->SetDataRate(DataRate(baudrate));
        my_noc_router->RoutingDelays = my_router_delay_model; //TODO: use a method set instead, otherwise there will be no default value, and it wont work.
//        my_noc_router->ServerPolicy = NOCRouter::ROUND_ROBIN; //TODO: use a method set instead
        my_noc_router->ServerPolicy = NOCRouter::FIFO;
        my_noc_router->SetRoutingProtocolUnicast(NOCRouting::ROUTING_PROTOCOL_XY_CLOCKWISE);
//        my_noc_router->SetRoutingProtocolUnicast(NOCRouting::ROUTING_PROTOCOL_YFIRST);
	my_router_delay_model->InputData = &my_input_data;

        //Setup app
        my_xdense_app->IsSink = false;
        my_xdense_app->IsActive = true;
        //TODO: Believe it should be get from the packet header itself, or ask the router
        my_xdense_app->PacketDuration = packet_duration;  //nano seconds
        my_xdense_app->ClusterSize_x = size_neighborhood;
        my_xdense_app->ClusterSize_y = size_neighborhood;      
        
        
        ostringstream context[4];
        
        context[0] << i << "," << x.Get() << "," << y.Get();
        my_xdense_app->TraceConnect("FlowSourceTrace", context[0].str(), MakeCallback(&log_flows_source));  

        
        context[1] << i << "," << x.Get() << "," << y.Get() << "," << (int) NOCRouting::DIRECTION_L << ",c";
        my_noc_router->TraceConnect("RouterCxTrace", context[1].str(), MakeCallback(&log_netdevice_packets));
        my_noc_router->TraceConnect("RouterCxTrace", context[1].str(), MakeCallback(&log_flows));
        
        context[2] << i << "," << x.Get() << "," << y.Get() << "," << (int) NOCRouting::DIRECTION_L << ",g";
        my_noc_router->TraceConnect("RouterGxTrace", context[2].str(), MakeCallback(&log_netdevice_packets));
        my_noc_router->TraceConnect("RouterGxTrace", context[2].str(), MakeCallback(&log_flows));
        
        
        //Setup NetDevice's Callback
        Ptr<NOCNetDevice> my_net_device;
        int8_t direction;
        for (uint8_t j = 0 ; j < my_noc_router->GetNDevices() ; j++)
        {
            my_net_device = my_noc_router->GetNetDevice(j);
            direction = my_noc_router->GetNetDeviceInfo(my_net_device).direction; 
            
            ostringstream context_nd[4];
               
            context_nd[0] << i << "," << x.Get() << "," << y.Get() << "," << (int) direction << ",r";
            my_net_device->TraceConnect("MacRx", context_nd[0].str(), MakeCallback(&log_netdevice_packets));
            context_nd[1] << i << "," << x.Get() << "," << y.Get() << "," << (int) direction << ",t";
            my_net_device->TraceConnect("MacTx", context_nd[1].str(), MakeCallback(&log_netdevice_packets));            

//            context_nd[2] << i << "," << x.Get() << "," << y.Get() << "," << (int) direction << ",r";
//            my_net_device->TraceConnect("MacRxQueue", context_nd[2].str(), MakeCallback(&log_queues)); //Since there is a FIFO at the output port only
//            context_nd[3] << i << "," << x.Get() << "," << y.Get() << "," << (int) direction << ",t";
//            my_net_device->TraceConnect("MacTxQueue", context_nd[3].str(), MakeCallback(&log_queues)); 
            
            
            if (my_shaping_data.IsShaped(x.Get(), y.Get(), direction)){
                float b = my_shaping_data.GetBurstiness(x.Get(), y.Get(), direction);
                float o = my_shaping_data.GetOffset(x.Get(), y.Get(), direction);
                uint8_t ms = my_shaping_data.GetMsgSize(x.Get(), y.Get(), direction);; //message size, in case shaping have to be applied cyclic
                my_net_device->SetShaper(b,o,ms);
            }
        }

        //Should be installed in this order!!!
        my_node_container.Get(i)->AddApplication(my_noc_router);
        my_node_container.Get(i)->AddApplication(my_router_delay_model);
        my_node_container.Get(i)->AddApplication(my_xdense_app);

        my_xdense_app_container.Add(my_xdense_app);
        my_xdense_data_io_container.Add(my_router_delay_model);
        my_xdense_app->AddRouter(my_noc_router);
    }

    
    //////////////////// From here, initialize the application at the nodes ///////////////// 
    uint8_t  initial_delay = 1;
    double_t offset;   
    uint32_t ms;
    
//    ms = ceil(pow(size_neighborhood + 1, 2) * (1 - c_rate));
    
    ms = ceil(pow(float_t(size_neighborhood) * 2 + 1, 2) * 4 * 0.04); 
    //consider 1 packet per node reduced by the comrpession rate
    
    
    uint32_t sink_x = 0;
    uint32_t sink_y = 0;
    
    double_t beta = stod(beta_str);
    uint16_t ch = 0;

    // First count for the number of CH
    for (uint32_t x = 0; x < size_x; x++) {
        for (uint32_t y = 0; y < size_y; y++) {
            
            uint8_t dx = NOCRouting::DistanceLinear(sink_x , x);
            uint8_t dy = NOCRouting::DistanceLinear(sink_y , y);
            
            if (y == sink_y && x == sink_x){ //The one to trace
                //sink does not send to itself
            }  
            else if((dx + size_neighborhood) % (size_neighborhood * 2 + 1) == 0 && 
                    (dy + size_neighborhood) % (size_neighborhood * 2 + 1) == 0){
                ch++;
            } 
        }
    }
    
    
    int COUNT = ch,    // X
        MAX_VAL = 20,  // Y
        MAX_SUM = ch * ms; // Z

    std::vector<int> bucket_ms(COUNT, 0);
    srand(time(0));
    int remaining = MAX_SUM;
    while (remaining > 0)
    {
        int rndBucketIdx = irand(0, COUNT-1);
        if (bucket_ms[rndBucketIdx] == MAX_VAL)
            continue;                       // this bucket is already full
        bucket_ms[rndBucketIdx]++;
        remaining--;
    }
    
    uint32_t m_factor = 10 * ch;
    COUNT = ch;    // X
    MAX_VAL = 1*m_factor;  // Y
    MAX_SUM = (beta * ch) * m_factor; // Z

    std::vector<double> bucket_beta(COUNT, 0);
    srand(time(0));
    remaining = MAX_SUM;
    while (remaining > 0)
    {
        int rndBucketIdx = irand(0, COUNT-1);
        if (bucket_beta[rndBucketIdx] == MAX_VAL)
            continue;                       // this bucket is already full
        bucket_beta[rndBucketIdx] += 1;
        remaining--;
    }

    std::cout << "Printing sequence: \n"; 
    int sum_ms = 0;
    double_t sum_beta = 0;
    for (int i = 0; i < COUNT; ++i)
    {
//        std::cout << bucket_ms[i] << ' ';
        sum_ms+= bucket_ms[i];
        sum_beta+= bucket_beta[i] / m_factor;
    }
    std::cout << sum_ms << " = " << (ch * ms) << "\n";
    std::cout << sum_beta << " = " << (ch * beta);
//    

    int i = 0;
    for (uint32_t x = 0; x < size_x; x++) {
        for (uint32_t y = 0; y < size_y; y++) {
            
            uint32_t n = GetN(size_x, size_y, x, y);
            XDenseHeader hd_out;
            hd_out.SetXdenseProtocol(XDenseHeader::DATA_SHARING);
            Ptr<Packet> pck_out = Create<Packet>();
            pck_out->AddHeader(hd_out);
            
            uint8_t d = NOCRouting::Distance(sink_x, sink_y, x, y);
            uint8_t dx = NOCRouting::DistanceLinear(sink_x , x);
            uint8_t dy = NOCRouting::DistanceLinear(sink_y , y);
            
            offset = d;
            
            if (y == sink_y && x == sink_x){ //The one to trace
                //sink does not send to itself
            }  
            else if((dx + size_neighborhood) % (size_neighborhood * 2 + 1) == 0 && 
                    (dy + size_neighborhood) % (size_neighborhood * 2 + 1) == 0){
                
                beta = (double_t) bucket_beta[i] / (double_t) m_factor;
                ms = bucket_ms[i];
                i++;
                
                my_xdense_app_container.Get(n)->GetObject<XDenseApp>()->m_flows_source(x, y, sink_x, sink_y, offset, beta, ms, NOCHeader::PROTOCOL_UNICAST);
                my_xdense_app_container.Get(n)->GetObject<XDenseApp>()->SetFlowGenerator(initial_delay, beta, offset, ms, pck_out, sink_x, sink_y, XDenseApp::ADDRESSING_ABSOLUTE, NOCHeader::PROTOCOL_UNICAST_OFFSET);                                          
            } 
        }
    }


    //**************** Simulation Setup **************************

    
    cout << endl << "Simulation started, please wait..." << endl ;
    
    Simulator::Stop(Seconds(1));
    Simulator::Run();

    //**************** Output Printing ***************************

    cout << "Simulation completed successfully" << endl ;
    
    file_packet_trace.close();
//    file_queue_size_trace.close();
//    file_flow_trace.close();
    file_flows_source.close();
    file_simulation_info.close(); //it is only modified here, so can be closed
    
    cout << "Log files saved" << endl;

    Simulator::Destroy();
    return 0;
}
