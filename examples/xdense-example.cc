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



ofstream file, 
        file_packets_trace,
        file_packets_trace_sink,
        file_packets_trace_net_device, 
        file_queue_size, 
        file_queue_size_prioritized, 
        file_value_annoucement_total_time;

string dir_output;
string dir_input;

uint32_t global_sampling_period;

string filename;

uint16_t *received_data;

NodeContainer my_node_container;

uint32_t qs_a, qsp_a; //queue sizes from the previous cycle, to create the log of queue sizes.
uint64_t time_a;

uint32_t value_annoucement_start_time,
        value_annoucement_end_time, 
        value_annoucement_count;


uint32_t sampling_cycles;
uint32_t sampling_period; //at every 100.000.000 ns = 100ms default
uint8_t log_start_at_period;
uint32_t start_offset;

void
log_packet(string context, Ptr<const Packet> pck) 
{
    NOCHeader hd;
    pck->PeekHeader(hd);
    
    std::cout
//    file_packets_trace
    << "c,"
    << Simulator::Now().GetNanoSeconds() << ","
    << pck->GetUid() << ","
    << context << "," 
    << "p,";
//    hd.Print(file_packets_trace);
    hd.Print(std::cout);
}

int
main(int argc, char *argv[]) {

    //LogComponentEnable("NOCNetDevice",LOG_LEVEL_ALL);
    //LogComponentEnable("NOCChannel",LOG_LEVEL_ALL);
    //LogComponentEnable("XDenseHeader",LOG_LEVEL_ALL);

    
    
    // Default values
    uint32_t size_x = 101;
    uint32_t size_y = 101;
    uint32_t size_neighborhood = 4;
    uint32_t sinks_n = 1;
    uint32_t baudrate = 1000; //1000 kbps =  1 Mbps
    uint32_t operational_mode = 0; //sample the entire network
    
    start_offset = 1000000; //1e9ns = 1s
    log_start_at_period = 2;
    sampling_cycles = 10;
    sampling_period = 100000; //at every 100.000.000 ns = 100ms default
    
    global_sampling_period = sampling_period;
    
    // For logging the queue size over time
    qs_a = 0;
    qsp_a = 0;

    
    value_annoucement_start_time = 0;
    value_annoucement_end_time = 0;
    value_annoucement_count = 0;
    
    struct passwd *pw = getpwuid(getuid());
    string homedir = pw->pw_dir;
        
    string io_data_dir = homedir + "/usn-data";
    
    stringstream context_dir;
    context_dir << "/nw" << size_x << "x" <<size_y << "s" << sinks_n << "n" << size_neighborhood << "/";
    
    dir_output = io_data_dir + context_dir.str() + "out/";
    dir_input = io_data_dir + context_dir.str() + "in/";    

    CommandLine cmd;
    cmd.AddValue("size_x", "Network size in the X axe", size_x);
    cmd.AddValue("size_y", "Network size in the Y axe", size_y);
    cmd.AddValue("operational_mode", "Operate by detecting events (=1) or for sampling all the network (=0)", operational_mode);
    cmd.AddValue("size_neighborhood", "Neighborhood size", size_neighborhood);
    cmd.AddValue("sinks_n", "Network size in the X axe", sinks_n);
    cmd.AddValue("sampling_cycles", "The number of times each node should sample its sensor, and perform data exchange", sampling_cycles);
    cmd.AddValue("sampling_period", "The period between each sensor sampling [us]", sampling_period);
    cmd.AddValue("log_start_at_period", "The period from witch logging starts", log_start_at_period);
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
//    my_grid_network.SetDeviceAttribute("PacketSize", IntegerValue(4)); //in bytes
    
    my_grid_network_helper.SetChannelAttribute("Delay", TimeValue(MilliSeconds(0)));
    
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


        uint32_t x = i % size_x;
        uint32_t y = floor(i / size_y);

        Ptr<XDenseApp> my_xdense_app = CreateObject<XDenseApp> ();
        Ptr<SENSOR> my_sensor = CreateObject<SENSOR> ();

        //Setup app
        my_xdense_app->IsSink = false;
        my_xdense_app->MaxHops = size_neighborhood;
        my_xdense_app->SamplingCycles = sampling_cycles; // at least 2, since the first is sacrificed to
        my_xdense_app->SamplingPeriod = sampling_period; // at least 2, since the first is sacrificed to
        my_xdense_app->OperationalMode = 255; //Not defined, since it is defined by the sink ND packet
        my_xdense_app->SetStartTime(Seconds(0));


        //Setup Net Device's Callback
//        uint8_t n_devices = my_node_container.Get(i)->GetNDevices();
        
//        Ptr<NOCNetDevice> my_net_device;

//        for (uint32_t j = 0; j < n_devices; j++) 
//        {
//            my_net_device = my_node_container.Get(i)->GetDevice(j)->GetObject<NOCNetDevice>();
////            uint8_t n = my_net_device->GetNOCAddress();
//            ostringstream ss; //Used as the context. From which node (x,y) the callback was generated.
//            ss << i << "," << x << "," << y << "," << (int) j;
//            my_net_device->TraceConnect("MacRx", ss.str(), MakeCallback(&packet_received_netdevice_mac));
//            my_net_device->TraceConnect("MacTx", ss.str(), MakeCallback(&packet_sent_netdevice_mac));
//            
//            my_net_device->TraceConnect("PhyTxEnd", ss.str(), MakeCallback(&packet_exchanged_phy));
//            my_net_device->TraceConnect("PhyRxEnd", ss.str(), MakeCallback(&packet_exchanged_phy));
//        }

        //Setup router
        Ptr<NOCRouter> my_noc_router = my_node_container.Get(i)->GetApplication(INSTALLED_NOC_ROUTER)->GetObject<NOCRouter>();

        ostringstream s1;
        s1 << i << "," << x << "," << y << ",i";
        my_noc_router->TraceConnect("RouterRxTrace", s1.str(), MakeCallback(&log_packet));
        
        ostringstream s2;
        s2 << i << "," << x << "," << y << ",o";
        my_noc_router->TraceConnect("RouterTxTrace", s2.str(), MakeCallback(&log_packet));

        //Setup sensor
        my_sensor->SensorPosition.x = x;
        my_sensor->SensorPosition.y = y;
        my_sensor->InputData = &my_input_data;


        //Should be installed in this order!!!
        my_node_container.Get(i)->AddApplication(my_xdense_app);
        my_node_container.Get(i)->AddApplication(my_sensor);

        my_xdense_app_container.Add(my_xdense_app);
        my_xdense_sensor_container.Add(my_sensor);
        my_xdense_app->AddRouter(my_noc_router);
    }

    
    
    // Setting the sinks
    //uint32_t nodes_n = m_sizeX * m_sizeY;

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
        ostringstream ss;
        ss << n << "," << x << "," << y;
        //        my_noc_router_container.Get(n)->GetObject<NOCRouter>()->TraceConnect("SwitchRxTrace", "18,32", MakeCallback(&packets_received_sink));
        //        my_noc_router_container.Get(n)->GetObject<NOCRouter>()->TraceConnect("SwitchTxTrace", "58,33", MakeCallback(&packets_received_sink));
        
        Ptr<NOCRouter> my_noc_router = my_node_container.Get(n)->GetApplication(INSTALLED_NOC_ROUTER)->GetObject<NOCRouter>();

        
//        my_noc_router->GetObject<NOCRouter>()->TraceConnect("SwitchRxTrace", ss.str(), MakeCallback(&log_packet));
//        my_noc_router->GetObject<NOCRouter>()->TraceConnect("SwitchTxTrace", ss.str(), MakeCallback(&log_packet));
    }

    

//     
//   NOCHelper my_noc;
//    //    my_noc.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
//    my_noc.SetDeviceAttribute("DataRate", DataRateValue(DataRate(baudrate * 1000)));
//    //    my_noc.SetChannelAttribute("Delay", StringValue("0us"));
//    my_noc.SetChannelAttribute("Delay", TimeValue(MilliSeconds(0)));

    





    //**************** Simulation Setup **************************

    //PacketMetadata::Enable ();
    //AnimationInterface anim("animation.xml");

    filename = dir_output + "packets-trace.csv";
    cout << "Log file created at: '" << filename << "'" << endl;
    file_packets_trace.open(filename.c_str(), ios::out);

//    filename = dir_output + "packets-trace-net-device.csv";
//    cout << "Log file created at: '" << filename << "'" << endl;
//    file_packets_trace_net_device.open(filename.c_str(), ios::out);
//
//    filename = dir_output + "packets-sink-trace.csv";
//    cout << "Log file created at: '" << filename << "'" << endl;
//    file_packets_trace_sink.open(filename.c_str(), ios::out);
//
//    filename = dir_output + "queue-size-over-time.csv";
//    cout << "Log file created at: '" << filename << "'" << endl;
//    file_queue_size.open(filename.c_str(), ios::out);
//    
//    filename = dir_output + "queue-size-prioritized-over-time.csv";
//    cout << "Log file created at: '" << filename << "'" << endl;
//    file_queue_size_prioritized.open(filename.c_str(), ios::out);    
//    
//    filename = dir_output + "value-announcement-delay.csv";
//    cout << "Log file created at: '" << filename << "'" << endl;
//    file_value_annoucement_total_time.open(filename.c_str(), ios::out);

    Simulator::Stop(Seconds(20));
    Simulator::Run();

    //************************************************************


    //**************** Output Printing ***************************

    cout << "Done!" << endl << endl;
    
    
    //Write to file the information the sinks have received
//    ApplicationContainer::Iterator i;
//    uint8_t s = 0;
//    for (i = my_noc_sink_app_container.Begin(); i != my_noc_sink_app_container.End(); ++i) {
//        stringstream ss;
//        ss << "output-data-s-" << (int)s << ".csv";
//        filename = dir_output + ss.str();
////        if (operational_mode == 0)
////            (*i)->GetObject<XDenseApp>()->SinkReceivedData->WriteToFile(filename, size_neighborhood); // some Application method
////        else if (operational_mode == 1)
////            (*i)->GetObject<XDenseApp>()->SinkReceivedData->WriteToFile(filename, size_neighborhood);
////       //     (*i)->GetObject<XDenseApp>()->SinkReceivedData->WritePointsToFile(filename, size_x, size_y);
//        cout << "Log file created at: '" << filename << "'" << endl;
//    }
    
    file_value_annoucement_total_time << value_annoucement_start_time << "," << value_annoucement_end_time << "," << value_annoucement_count;

//    print_data(my_node_container, size_x, size_y);
    
    file_value_annoucement_total_time.close();
    file_packets_trace.close();
    file_packets_trace_net_device.close();
    file_packets_trace_sink.close();

    cout << Simulator::Now().GetSeconds();

    //    NOCPlotData my_plots;
    //    
    //    my_plots.PlotExample(dir);


    Simulator::Destroy();
    return 0;
}