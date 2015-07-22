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
#include "ns3/network-module.h"
//#include "ns3/internet-module.h"
#include "ns3/noc-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "src/core/model/object-base.h"

#include "src/noc/model/xdense-application.h"
#include "src/noc/model/sensor-data-io.h"
//#include "src/noc/model/noc-net-device.h"
//#include "src/noc/model/noc-application.h"
//#include "ns3/simple-net-device.h"


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
log_packet(string context, Ptr<const Packet> pck) {
    NOCHeader hd;
    pck->PeekHeader(hd);
    if (hd.GetNOCProtocol() == P_EVENT_ANNOUNCEMENT) {
    //    if (pck->GetUid() == 104 || pck->GetUid() == 110 ) {
            file_packets_trace_net_device
            << "c,"
            << Simulator::Now().GetNanoSeconds() << ","
            << pck->GetUid() << ","
//            << (int) direction << ","
            << context << "," //here «, inside context there is: node_number, i/o, x, y, port_number
            << "p,"
            << hd.GetNOCProtocol() << ","
            << (int) hd.SerialNumber << ","
            << (int) hd.EventType << ","
            << hd.EventData[0] << ","
            << (int) hd.CurrentX << ","
            << (int) hd.CurrentY << endl;
    }        
    
}

void
log_queue_size(string context, Ptr<const Packet> pck){
    //Extract data from the context string
    uint32_t nd_n = std::atoi(context.substr(context.find_last_of(',') + 1).c_str());
    context.erase(context.find_last_of(','));
    uint32_t y = std::atoi(context.substr(context.find_last_of(',') + 1).c_str());
    context.erase(context.find_last_of(','));
    uint32_t x = std::atoi(context.substr(context.find_last_of(',') + 1).c_str());
    context.erase(context.find_last_of(','));
    uint32_t node_n = std::atoi(context.substr(context.find_last_of(',') + 1).c_str());
      
    x=x;
    y=y;
    
    uint64_t time = Simulator::Now().GetNanoSeconds();
    uint32_t qs,qsp;
    
    Ptr<NOCNetDevice> nd = my_node_container.Get(node_n)->GetDevice(nd_n)->GetObject<NOCNetDevice>();

    qs = nd->queue_size;
    qsp = nd->queue_size_prioritized;

   
    if (time > time_a){
        
        file_queue_size << time_a << ',' << qs_a << ',' << qsp_a << endl;  
        
        qs_a = 0;
        qsp_a = 0;
        time_a = time;
    } 
    else {
        if (qs > qs_a) {
            qs_a = qs;
        }
        if (qsp > qsp_a) {
            qsp_a = qsp;
        }
    }
//    }     
}


int
main(int argc, char *argv[]) {

    //LogComponentEnable("NOCNetDevice",LOG_LEVEL_ALL);
    //LogComponentEnable("NOCChannel",LOG_LEVEL_ALL);
    //LogComponentEnable("NOCHeader",LOG_LEVEL_ALL);

    
    
    // Default values
    uint32_t size_x = 4;
    uint32_t size_y = 4;
    
    //1000000 kHz =  1 GHz, which is the processor 
    //frequency and the time it takes to transmit a packet
    uint32_t baudrate = 1000000; 

    struct passwd *pw = getpwuid(getuid());
    string homedir = pw->pw_dir;
        
    string dir_base = homedir + "/ns3-epiphany-example";
    
    stringstream dir_context;
    dir_context << "/nw" << size_x << "x" <<size_y << "/";
    
    dir_output = dir_base + dir_context.str() + "out/";
    dir_input = dir_base + dir_context.str() + "in/";    

    CommandLine cmd;
    cmd.AddValue("size_x", "Network size in the X axe", size_x);
    cmd.AddValue("size_y", "Network size in the Y axe", size_y);
    cmd.AddValue("baudrate", "Communication bauldrate in kHz", baudrate);
//    cmd.AddValue("channel_count", "Communication bauldrate in kHz", baudrate);
//    cmd.AddValue("channel_width", "Communication bauldrate in kHz", baudrate);
//    cmd.AddValue("parallel_comm", "Communication bauldrate in kHz", baudrate);
    cmd.AddValue("dir_base", "Directory with the I/O simulation data", dir_base);

    cmd.Parse(argc, argv);
    ///////////////////////////////////////////////////////////////

    dir_output = dir_base + "/out/";
    dir_input = dir_base + "/in/";
    
    int status;
    status = mkpath(dir_output.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (status != 0) {
        cout << "Error creating the directory " << dir_output << "\n. The program will terminate now.";
        exit(0);
    }
   
    //Using the new helper
    GridHelper my_grid_network_helper;
    my_grid_network_helper.SetNetworkAttribute("SizeX", size_x);
    my_grid_network_helper.SetNetworkAttribute("SizeY", size_y);
    
    //three net devices per direction (c-mesh, r-mesh and )
    //total 3 x 4 = 12
    my_grid_network_helper.SetNetworkAttribute("ChannelCount", 3);
    
    // 1GHz
    my_grid_network_helper.SetDeviceAttribute("DataRate", DataRateValue(DataRate(baudrate * 1000)));
    my_grid_network_helper.SetDeviceAttribute("InterframeGap", TimeValue(MilliSeconds(0)));
    
    //If the connection is not serial, then the packet size defines the link width,
    //and one packet is transmitted in one tick of a given baudrate    
    my_grid_network_helper.SetDeviceAttribute("SerialComm", BooleanValue(false));
    
    //in bytes 104 bits epiphany e-link between nodes
    my_grid_network_helper.SetDeviceAttribute("PacketSize", IntegerValue(13)); 
    //my_grid_network_helper.SetChannelAttribute("Delay", TimeValue(MilliSeconds(0)));
    
    my_node_container = my_grid_network_helper.InitializeNetwork();
    uint32_t n_nodes = my_node_container.GetN();
           
        
    //Install applications;
    
   
    ApplicationContainer my_noc_sink_app_container;
    ApplicationContainer my_noc_app_container;
    ApplicationContainer my_noc_router_container;
   
    for (uint32_t i = 0; i < n_nodes; i++) {


        uint32_t x = i % size_x;
        uint32_t y = floor(i / size_y);

        Ptr<EpiphanyApp> my_ep_app = CreateObject<EpiphanyApp> ();
        my_ep_app->SetStartTime(Seconds(0));

        //Setup router
        Ptr<NOCRouter> my_noc_router = my_node_container.Get(i)->GetApplication(INSTALLED_NOC_SWITCH)->GetObject<NOCRouter>();
        
        ostringstream ss;
        ss << i  << "i" << "," << x << "," << y;
        my_noc_router->TraceConnect("SwitchRxTrace", ss.str(), MakeCallback(&log_packet));
        
        ss << i  << "o" << "," << x << "," << y;
        my_noc_router->TraceConnect("SwitchTxTrace", ss.str(), MakeCallback(&log_packet));

        //Should be installed in this order!!!
        my_node_container.Get(i)->AddApplication(my_ep_app);
        my_noc_app_container.Add(my_ep_app);
    }

    
    

    //**************** Simulation Setup **************************

    //PacketMetadata::Enable ();
    //AnimationInterface anim("animation.xml");

    filename = dir_output + "packets-trace.csv";
    cout << "Log file created at: '" << filename << "'" << endl;
    file_packets_trace.open(filename.c_str(), ios::out);

    Simulator::Stop(Seconds(20));
    Simulator::Run();

    //************************************************************


    //**************** Output Printing ***************************
    file_packets_trace.close();
    
    cout << Simulator::Now().GetSeconds();
    cout << "Done!" << endl << endl;
    
    Simulator::Destroy();
    return 0;
}