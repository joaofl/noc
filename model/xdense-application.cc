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
 */

#include "xdense-application.h"
#include "xdense-header.h"
#include "noc-routing.h"
#include "src/core/model/simulator.h"
#include "src/network/model/packet.h"
#include "ns3/noc-types.h"
#include "ns3/calc.h"




using namespace std;
namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("XDenseApplication");
    
    TypeId
    XDenseApp::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::XDenseApplication")
                .SetParent<Application> ()
                .AddConstructor<XDenseApp> ()
        
                .AddTraceSource("FlowSourceTrace", 
                "The packets received by the router of each node",
                MakeTraceSourceAccessor(&XDenseApp::m_flows_source),
                "ns3::XDenseApp::FlowSourceTrace")

                .AddTraceSource("PingDelayTrace", 
                "Ping delay",
                MakeTraceSourceAccessor(&XDenseApp::m_ping_delay),
                "ns3::XDenseApp::PingDelayTrace")

                .AddTraceSource("SensedDataTrace", 
                "Sensed data",
                MakeTraceSourceAccessor(&XDenseApp::m_sensed_data),
                "ns3::XDenseApp::SensedDataTrace")

                .AddTraceSource("SensedDataReceivedTrace", 
                "Sensed data received",
                MakeTraceSourceAccessor(&XDenseApp::m_sensed_data_received),
                "ns3::XDenseApp::SensedDataReceivedTrace")
                ;
        return tid;
    }

    XDenseApp::XDenseApp()
    :
    m_running(false)
    {
    }

    XDenseApp::~XDenseApp() {
        //  m_socket = 0;
    }

    void
    XDenseApp::StartApplication(void) {

        //This data should not be used in the protocols itself, but for logging
        //proposes only. This to keep protocol's scalable (without the need 
        //of fixed and absolute pre-defined x,y)

        IntegerValue x, y;
        AggregationRate = 0.8;
        m_router->GetAttribute("AddressX", x);
        m_router->GetAttribute("AddressY", y);
        m_router->SetReceiveCallback(MakeCallback(&XDenseApp::PacketReceived, this));
    }
    
    void
    XDenseApp::StopApplication(void) {
        m_running = false;
    }
    
    void
    XDenseApp::AddRouter(Ptr<NOCRouter> r) {
        m_router = r;
    }
    
    void 
    XDenseApp::SetShaper(double_t b, uint8_t rd, uint8_t port) {
                
        uint8_t ms = 0; //message size
        m_router->SetShaper(b, rd, ms, port);
    }
    
    void 
    XDenseApp::SetFlowGenerator(uint8_t initial_delay, double_t burstiness, double_t t_offset, uint32_t msg_size, 
            Ptr<const Packet> pck, int32_t dest_x, int32_t dest_y, bool addressing, uint8_t protocol) {
        
        if (burstiness == 0 || msg_size == 0 || IsActive == false)
            return;
                
        Time t_step_t = Time::FromInteger(PacketDuration.GetNanoSeconds() / burstiness, Time::NS);
        Time t_offset_t = Time::FromInteger(PacketDuration.GetNanoSeconds() * (initial_delay + t_offset), Time::NS);
        
        for (uint16_t i = 0 ; i < msg_size ; i++ ){
            Ptr<Packet> pck_c = pck->Copy();
            XDenseHeader hd;
            pck_c->RemoveHeader(hd);
            hd.SetData64(i,0);
            pck_c->AddHeader(hd);
            
            if (protocol == NOCHeader::PROTOCOL_UNICAST) {
                Simulator::Schedule(t_offset_t + (i * t_step_t), &NOCRouter::PacketUnicast, this->m_router, pck_c, NETWORK_ID_0, dest_x, dest_y, addressing);
            }
            else if(protocol == NOCHeader::PROTOCOL_UNICAST_OFFSET){
                Simulator::Schedule(t_offset_t + (i * t_step_t), &NOCRouter::PacketUnicastOffset, this->m_router, pck_c, NETWORK_ID_0, dest_x, dest_y, addressing);
            }
        }
    }
    
   
    void
    XDenseApp::RunApplicationWCA(bool trace, bool is_sink) {
        Time t_step = Time::FromInteger(PacketDuration.GetNanoSeconds(), Time::NS);
        
        if (is_sink){
            Simulator::Schedule(t_step, &XDenseApp::ClusterDataRequest, this);
//            ClusterDataRequest();
        }
    }
    void
    XDenseApp::RunApplicationCluster(void) {
        Time t_step = Time::FromInteger(PacketDuration.GetNanoSeconds(), Time::NS);
        Simulator::Schedule(t_step, &XDenseApp::DataSharingRequest, this);
    }


    
    void
    XDenseApp::Tests() {
        Ptr<Packet> pck = Create<Packet>();
        
        XDenseHeader hd;
        hd.SetXDenseProtocol(XDenseHeader::DATA_SHARING);
        pck->AddHeader(hd);
//        
        Time t_ns = Time::FromInteger(0,Time::NS);

//        Simulator::Schedule(t_ns, &NOCRouter::PacketUnicastHighway, this->m_router, pck, NETWORK_ID_0, 15, 15, ClusterSize_x, ClusterSize_y); //Quadrant A
//        t_ns += 10 * PacketDuration;
//        Simulator::Schedule(t_ns, &NOCRouter::PacketUnicast, this->m_router, pck, NETWORK_ID_0, -15, 15, 0); //Quadrant B
//        t_ns += 10 * PacketDuration;
//        Simulator::Schedule(t_ns, &NOCRouter::PacketUnicast, this->m_router, pck, NETWORK_ID_0, -15, -15, 0); //Quadrant C
//        t_ns += 10 * PacketDuration;
//        Simulator::Schedule(t_ns, &NOCRouter::PacketUnicast, this->m_router, pck, NETWORK_ID_0, -15,-15, 0); //Quadrant D
//        t_ns += 10 * PacketDuration;
        
        m_router->PacketUnicast(pck,NETWORK_ID_0,-15,-15, ADDRESSING_RELATIVE);
        m_router->PacketUnicast(pck,NETWORK_ID_0,-15, 15, ADDRESSING_RELATIVE);
        m_router->PacketUnicast(pck,NETWORK_ID_0, 7,-7, ADDRESSING_RELATIVE);
        m_router->PacketUnicast(pck,NETWORK_ID_0, 7, 7, ADDRESSING_RELATIVE);

        m_router->PacketUnicastOffset(pck,NETWORK_ID_0,-15,-15, ADDRESSING_RELATIVE);
        m_router->PacketUnicastOffset(pck,NETWORK_ID_0,-15, 15, ADDRESSING_RELATIVE);
        m_router->PacketUnicastOffset(pck,NETWORK_ID_0, 7,-7, ADDRESSING_RELATIVE);
        m_router->PacketUnicastOffset(pck,NETWORK_ID_0, 7, 7, ADDRESSING_RELATIVE);
//        t_ns += 60 * PacketDuration;        
//        Simulator::Schedule(t_ns, &NOCRouter::PacketMulticastRadius, this->m_router, pck, NETWORK_ID_0, 7);
//        t_ns += 60 * PacketDuration;
//        Simulator::Schedule(t_ns, &NOCRouter::PacketMulticastArea, this->m_router, pck, NETWORK_ID_0, 7, 7);
//        t_ns += 60 * PacketDuration;
//        Simulator::Schedule(t_ns, &NOCRouter::PacketMulticastIndividuals, this->m_router, pck, NETWORK_ID_0, ClusterSize, ClusterSize);
//        t_ns += 60 * PacketDuration;
//        Simulator::Schedule(t_ns, &NOCRouter::PacketBroadcast, this->m_router, pck, NETWORK_ID_0);
//        t_ns += 60 * PacketDuration;
//        m_router->PacketBroadcast(pck, 0);
    }


    void
    XDenseApp::UpdateList(NodeRef n, std::vector<NodeRef> &nodes_list) {
        //Update if present, add if absent
        bool present = false;
        uint8_t i;
        for (i = 0  ; i < nodes_list.size() ; i++){
            if (nodes_list.at(i).x == n.x && nodes_list.at(i).y == n.y){
                present = true;
                break;
            }
        }
        if (present){
            nodes_list.at(i) = n;
        }
        else{
            nodes_list.push_back(n);            
        }
    }    
    
    uint16_t
    XDenseApp::CalcTimeout(uint8_t cluster_size_x, uint8_t cluster_size_y) {
        return 2 * (ceil(cluster_size_x/2) + ceil(cluster_size_y/2)) + (cluster_size_x * cluster_size_y - 1); 
    }


    void 
    XDenseApp::PacketReceived(Ptr<const Packet> pck, uint8_t protocol, int32_t origin_x, int32_t origin_y, int32_t dest_x,int32_t dest_y) {
        //TODO: here, at this layer, we remove the XDense header and see what to do with the packet.
        //the NOC Header here is no longer in the packet
        
        Ptr<Packet> pck_c = pck->Copy();
        
        XDenseHeader h;
        pck_c->RemoveHeader(h);
        
        uint64_t tts_pck;
//        int64x64_t tts_now, tts_total;
        
        switch (h.GetXDenseProtocol()){
            case XDenseHeader::PING_RESPONSE:
                PingResponseReceived(pck, origin_x, origin_y);
                break;
            case XDenseHeader::PING:
                PingReceived(pck, origin_x, origin_y);
                break;
            case XDenseHeader::DATA_SHARING_REQUEST:
                DataSharingRequestReceived(pck, origin_x, origin_y);                
                break;
            case XDenseHeader::DATA_SHARING:
                DataSharingReceived(pck, origin_x, origin_y);                
                break;
            case XDenseHeader::CLUSTER_DATA_REQUEST:
                ClusterDataRequestReceived(pck, origin_x, origin_y, dest_x, dest_y);                
                break;
            case XDenseHeader::CLUSTER_DATA:
                ClusterDataReceived(pck, origin_x, origin_y, dest_x, dest_y);                
                break;
            case XDenseHeader::ACTION_NODES_DATA_TO_CLUSTER_HEADS:
                NodesDataToClusterDataRequestReceived(pck, origin_x, origin_y, dest_x, dest_y);                
                break;
            case XDenseHeader::ACTION_NETWORK_SETUP:
//                NetworkSetupReceived(pck, origin_x, origin_y);                
            case XDenseHeader::TRACE:
                tts_pck = h.GetData64(0);
                cout << "Traced packet received: id=" << pck->GetUid() << "\tdata=" << tts_pck << endl;            
                break;
        }
    }
    
    void 
    XDenseApp::ClusterDataRequest() {
        Ptr<Packet> pck = Create<Packet>();

        XDenseHeader hd;
        hd.SetXDenseProtocol(XDenseHeader::CLUSTER_DATA_REQUEST);
        pck->AddHeader(hd);

//        Time t_ns = Time::FromInteger(0,Time::NS);
//        Simulator::Schedule(t_ns, &NOCRouter::PacketMulticastIndividuals, this->m_router, pck, NETWORK_ID_0, ClusterSize_x, ClusterSize_y);
            this->m_router->PacketMulticastIndividuals(pck, NETWORK_ID_0, ClusterSize_x, ClusterSize_y);
//        this->ClusterDataRequestReceived(pck, 0, 0, ClusterSize_x, ClusterSize_y);
    }


    
    bool 
    XDenseApp::ClusterDataRequestReceived(Ptr<const Packet> pck_r, int32_t origin_x, int32_t origin_y, int32_t size_x, int32_t size_y) {
        
        //Cluster heads receive a request for it to share its cluster data
        //then it schedule few local requests, and replies to the global sink,
        //so it can send it back its pre-processed cluster data
        
        Time tbase = PacketDuration * (ClusterSize_x + 1) * (ClusterSize_x + 1) * 4;
        Time t;
        
        t = 0 * tbase;
//        Simulator::Schedule(t, &XDenseApp::DataSharingRequest, this); // schedule the reques
        t = tbase; //set the waiting time
        Simulator::Schedule(t, &XDenseApp::ClusterDataResponse, this, -origin_x, -origin_y); // and response
        
        return true;
    }
    
        void 
    XDenseApp::ClusterDataResponse(int32_t dest_x, int32_t dest_y) {
        
//        DataFit curve = NOCCalc::FindCurve(&m_neighborsList);
//        curve = curve;
        DataFit plane = NOCCalc::FindPlane(&m_neighborsList);
        //TODO check the size of the coeficients, to see in how many packets to send them
        // Construct the packet to be transmitted
        XDenseHeader hd_out;
        
        hd_out.SetData24(plane.a,0); //data, index, number of bits
        hd_out.SetData24(plane.b,1); //data, index, number of bits
        hd_out.SetData24(plane.c,2); //data, index, number of bits
        
        hd_out.SetXDenseProtocol(XDenseHeader::CLUSTER_DATA);
        Ptr<Packet> pck_out = Create<Packet>();
        pck_out->AddHeader(hd_out);
        
        if (IsSink){
            ClusterDataReceived(pck_out, 0, 0, 0, 0);
        }
        else{
            m_router->PacketUnicast(pck_out, NETWORK_ID_0, m_sink_x, m_sink_y, ADDRESSING_RELATIVE); 
        }
            
//        double offset_log = (Simulator::Now().GetNanoSeconds() / PacketDuration.GetNanoSeconds()) + o - 1; //get the absolute offset

//        m_flows_source(orig_x_log, orig_y_log, dest_x_log, dest_y_log, offset_log, b, ms, NOCHeader::PROTOCOL_UNICAST_OFFSET);
//        this->SetFlowGenerator(0, b, o, ms, pck_out, dest_x, dest_y, ADDRESSING_RELATIVE, NOCHeader::PROTOCOL_UNICAST_OFFSET);        
    }
        


    bool 
    XDenseApp::ClusterDataReceived(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y, int32_t size_x, int32_t size_y) {
        
        XDenseHeader hd;
        pck->PeekHeader(hd);
        
        DataFit plane;
        
        plane.a = hd.GetData24(0);
        plane.b = hd.GetData24(1);
        plane.c = hd.GetData24(2);
        
//        cout << plane.a;

        int32_t lim_y = -floor(ClusterSize_y)/2;
        int32_t lim_x = -floor(ClusterSize_x)/2;
        
        for (int32_t y = lim_y ; y < lim_y + ClusterSize_y ; y++){
            for (int32_t x = lim_x ; x < lim_x + ClusterSize_x ; x++){
               
                int64_t v = x * plane.a + y * plane.b + plane.c;
                int32_t translated_x = -origin_x - x + m_router->AddressX;
                int32_t translated_y = -origin_y - y + m_router->AddressY;
                
                m_sensed_data_received(v, translated_x, translated_y);
                
//                NodeRef n;
//                n.value = v;
//                n.x = translated_x;
//                n.y = translated_y;
//                UpdateList(n, m_sinkNetworkData);
            }        
        }
        
        //Here, provide the data (data fits) already received to be visualized
        
        return true;
    }


    //////////////////////////////////////////////////////////////////////////
    //The sink asks all nodes to send their sensed data to their closest cluster head
    void 
    XDenseApp::NodesDataToClusterDataRequest() {
        Ptr<Packet> pck = Create<Packet>();

        //Only the sink runs this code
        ClusterSize_x = 1; //Only the sink has it set here
        ClusterSize_y = 1;
        
        XDenseHeader hd;
        hd.SetXDenseProtocol(XDenseHeader::ACTION_NODES_DATA_TO_CLUSTER_HEADS);
        hd.SetData8(ClusterSize_x, 0);
        hd.SetData8(ClusterSize_y, 1);
        pck->AddHeader(hd);
        
        
        NodeRef n;
        int32_t data = Sensor->ReadSensor();
        n.value = data;
        n.x = 0;
        n.y = 0;
        //Check if it already there, then push it to the back
        UpdateList(n, m_neighborsList);

        this->m_router->PacketBroadcast(pck, NETWORK_ID_0);
        uint16_t timeout = CalcTimeout(ClusterSize_x, ClusterSize_y);
        Simulator::Schedule(PacketDuration * timeout , &XDenseApp::ClusterDataResponse, this, 0,0); 
    }

    bool 
    XDenseApp::NodesDataToClusterDataRequestReceived(Ptr<const Packet> pck, int32_t current_x, int32_t current_y, int32_t size_x, int32_t size_y) {
        XDenseHeader hd;
        pck->PeekHeader(hd);        
        //Following equations return the nearest cluster head from the node that who received this req
        
        //Grab the 8bit data from the 64bit variablez
        uint8_t cluster_size_x = hd.GetData8(0);
        uint8_t cluster_size_y = hd.GetData8(1);
        
        int16_t my_x = current_x; 
        int16_t my_y = current_y; 
        
        //Find in which cluster I am
        int8_t my_qx = NOCCalc::FindNodeCluster(my_x, cluster_size_x);
        int8_t my_qy = NOCCalc::FindNodeCluster(my_y, cluster_size_y);
        
        //Find the coordinades of the nearest cluster head.
        int16_t chead_x = my_qx * cluster_size_x;
        int16_t chead_y = my_qy * cluster_size_y;
        
        //Calculate the relative position from one to another
        int16_t dest_x = chead_x - my_x;
        int16_t dest_y = chead_y - my_y;
        
//        m_sensed_data_received( abs(my_qx *9000 + my_qy*3000), m_router->AddressX, m_router->AddressY);

        //Save the location of the sink just in case
        m_sink_x = -current_x;
        m_sink_y = -current_y;
        
        //Check if I am a cluster head. In this case sample the sensor and do not send any packet.
        //Also schedule after some 'timeout' to send the data received after processing it
        if (dest_x == 0 && dest_y == 0){
            int64_t data = Sensor->ReadSensor();
            m_sensed_data(data);
//            m_sensed_data_received(data, m_router->AddressX, m_router->AddressY);
            
            NodeRef n;
            n.value = data;
            n.x = 0;
            n.y = 0;
            //TODO: Check if it already there, before push it to the back
            UpdateList(n, m_neighborsList);
            
            //2x the distance to the farther node on the cluster + the time to receive all packets
            //this is pessimistic, but upper bounds the time required (check further)
            uint16_t timeout = CalcTimeout(cluster_size_x, cluster_size_y); 
            //Send processed data it to the sink
            Simulator::Schedule(PacketDuration * timeout , &XDenseApp::ClusterDataResponse, this, 0,0); 
            
            return 1;
        }
        
        //Debug string
//      std::cout << my_x << ",\t" << my_y << ",\t" << int(my_qx) << ",\t" << int(my_qy) << ",\t" << dest_x << ",\t" << dest_y << "\n";
        
//        //Simulator::Schedule(PacketDuration * 30 , &XDenseApp::DataSharing, this, dest_x,dest_y);
        DataSharing(dest_x,dest_y);
        return 1;
    }
    void
    XDenseApp::DataSharingRequest() {
        Ptr<Packet> pck = Create<Packet>();
        
        XDenseHeader hd;
        hd.SetXDenseProtocol(XDenseHeader::DATA_SHARING_REQUEST);
        hd.SetData64(ClusterSize_x,0);
        pck->AddHeader(hd);
        
        m_router->PacketMulticastArea(pck, NETWORK_ID_0, ClusterSize_x, ClusterSize_y);
    }
    
    void
    XDenseApp::DataSharingRequestReceived(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y) {
//        XDenseHeader hd;
//        pck->PeekHeader(hd);
//        uint8_t size_x = hd.GetData();
//        uint8_t size_y = hd.GetData();
        
        double_t b = 1;
        uint32_t ms = 1;
        uint8_t o = 0;
        
        int32_t dest_x = origin_x * -1;
        int32_t dest_y = origin_y * -1;
        
        int32_t orig_x_log = m_router->AddressX;
        int32_t orig_y_log = m_router->AddressY;
        int32_t dest_x_log = m_router->AddressX + dest_x;
        int32_t dest_y_log = m_router->AddressY + dest_y;
        
        // Construct the packet to be transmitted
        XDenseHeader hd_out;
        hd_out.SetXDenseProtocol(XDenseHeader::DATA_SHARING);
        Ptr<Packet> pck_out = Create<Packet>();
        pck_out->AddHeader(hd_out);
        
//        uint8_t dist = NOCRouting::Distance(orig_x_log, orig_y_log, dest_x_log, dest_y_log);
//        double offset_log = dist + o; //get the absolute offset
        double offset_log = (Simulator::Now().GetNanoSeconds() / PacketDuration.GetNanoSeconds()) + o - 1; //get the absolute offset

        m_flows_source(orig_x_log, orig_y_log, dest_x_log, dest_y_log, offset_log, b, ms, NOCHeader::PROTOCOL_UNICAST);
        this->SetFlowGenerator(0, b, o, ms, pck_out, dest_x, dest_y, ADDRESSING_RELATIVE, NOCHeader::PROTOCOL_UNICAST);
    }

    
    void
    XDenseApp::DataSharing(int32_t x_dest, int32_t y_dest) {

        int64_t data = Sensor->ReadSensor();
        m_sensed_data(data);
        
        Ptr<Packet> pck = Create<Packet>();
        XDenseHeader hd;
        hd.SetXDenseProtocol(XDenseHeader::DATA_SHARING);
        hd.SetData64(data,0);
        pck->AddHeader(hd);

        m_router->PacketUnicast(pck, NETWORK_ID_0, x_dest, y_dest, ADDRESSING_RELATIVE); 
    }
    
    bool
    XDenseApp::DataSharingReceived(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y) {
//        Here, the node should build an matrix with the received data
        Ptr<Packet> pck_c = pck->Copy();
        XDenseHeader h;
        pck_c->RemoveHeader(h);
        int64_t data = h.GetData64(0); 
        
//        if (IsSink){
//            m_sensed_data_received(data, origin_x*-1  + m_router->AddressX, origin_y*-1  + m_router->AddressY);            
//            return true;
//        }
            
        NodeRef n;
        n.value = data;
        n.x = origin_x;
        n.y = origin_y;
        //Check if it already there, then push it to the back
        UpdateList(n, m_neighborsList);
                
        //TODO this should use relative addresses so we can see what each node receives.
//        m_sensed_data_received(data, origin_x *-1 + m_router->AddressX, origin_y * -1 + m_router->AddressY);
        
        return false;
    }
    
    void XDenseApp::CalculateShaper(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y) {
        
        XDenseHeader hd;
        pck->PeekHeader(hd);
        
        uint8_t size_x = hd.GetData64(0);
        uint8_t size_y = hd.GetData64(0);
        
        int32_t dest_x = origin_x * -1;
        int32_t dest_y = origin_y * -1;
        
        int8_t last_long;
        int8_t last_alt;
        int8_t delta_long;
        int8_t delta_alt;
        int8_t pos_long = 0;
        int8_t pos_alt = 0;        
        
        //Do the axe translation to get all quadrants computed as the same
       
        char quad = NOCRouting::FindQuadrant(origin_x, origin_y);
         
        last_long = size_x / 2;
        last_alt = size_y / 2;
        
        switch (quad){
            case NOCRouting::QUADRANT_PXPY://Positive X and positive Y
                pos_long = origin_x;
                pos_alt = origin_y;
                break;
            case NOCRouting::QUADRANT_NXPY:
                pos_long = origin_y;
                pos_alt = origin_x * -1;
                break;
            case NOCRouting::QUADRANT_NXNY:
                pos_long = origin_x * -1;
                pos_alt = origin_y * -1;
                break;
            case NOCRouting::QUADRANT_PXNY:
                pos_long = origin_y * -1;
                pos_alt = origin_x;
                break;
                        
        }
        
        //Define the flow characteristics
        uint8_t dist = NOCRouting::Distance(pos_long, pos_alt, last_long, last_alt);
        
        double_t b      = double(1) / double(abs(last_long) * (abs(last_alt) + 1));
        uint32_t ms     = 5;
        uint8_t rd      = dist + 1; // all nodes send together. First wait for the last
        rd = 0; //send the response as soon as the request is received
        
        //At each node rd = 0, but relatively to origin, at the time the request was made,
        // rd = do + 1, where do is distance to the origin
        rd = rd;
        
        // Calculate the traffic shaping parameters        
        delta_long = last_long - pos_long;
        delta_alt = last_alt - pos_alt;

        uint32_t shaper_rd = delta_long + 1;
        uint32_t total_ms = (delta_long + 1) * ms;
        double_t max_ms_over_b = (total_ms - 1) / (b * (delta_long));

        if (pos_long == 0 && pos_alt > 0){
            shaper_rd = shaper_rd + (delta_alt - 1);
            total_ms = total_ms + ((total_ms) * (delta_alt));
            max_ms_over_b = (total_ms) / (b * (delta_alt) * (delta_long));
        }
        
        double_t shaper_b = total_ms / max_ms_over_b;
        if (shaper_b > 1) shaper_b = 1;

//        if (pos_long == 0 && pos_alt == 1){
//            shaper_rd = 0;
//            shaper_b = 0.5;
            
            uint8_t dir = NOCRouting::UnicastClockwiseXY(dest_x, dest_y);
            dir = dir;
//            this->SetShaper(shaper_b, shaper_rd, dir);
//        }
    }

    
    void
    XDenseApp::Ping(int32_t x_dest, int32_t y_dest) {
        Ptr<Packet> pck = Create<Packet>();

        XDenseHeader hd;
        hd.SetXDenseProtocol(XDenseHeader::PING);
        
        int64_t tts_now = Simulator::Now().GetNanoSeconds();
        
        hd.SetData64(tts_now, 0);
        pck->AddHeader(hd);

        m_router->PacketUnicast(pck, NETWORK_ID_0, x_dest, y_dest, ADDRESSING_RELATIVE); 
    }

    void 
    XDenseApp::PingReceived(Ptr<const Packet> pck_rcv, int32_t origin_x, int32_t origin_y) {
        //get the time stamp, and put it back on the response and send it 
        XDenseHeader hd_in;
        pck_rcv->PeekHeader(hd_in);
        
        Ptr<Packet> pck = Create<Packet>();

        XDenseHeader hd_out;
        hd_out.SetData64(hd_in.GetData64(0),0);
        hd_out.SetXDenseProtocol(XDenseHeader::PING_RESPONSE);
        pck->AddHeader(hd_out);

        m_router->PacketUnicast(pck, NETWORK_ID_0, -origin_x, origin_y, ADDRESSING_RELATIVE);
    }

    void 
    XDenseApp::PingResponseReceived(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y) {
        Ptr<Packet> pck_c = pck->Copy(); 
        XDenseHeader h;
        pck_c->RemoveHeader(h);
        
        int64_t tts_pck;
        int64_t tts_now;
        
        tts_now = Simulator::Now().GetNanoSeconds();
        tts_pck = h.GetData64(0);
                
        m_ping_delay(tts_now - tts_pck);

    }

}



