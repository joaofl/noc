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
#include "src/core/model/simulator.h"
#include "src/network/model/packet.h"
#include "noc-routing.h"
#include "ns3/xdense-application.h"



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
        m_router->GetAttribute("AddressX", x);
        m_router->GetAttribute("AddressY", y);
        m_router->SetReceiveCallback(MakeCallback(&XDenseApp::DataReceived, this));
        
//        XDenseHeader h1;
//        NOCHeader h2;
        
//        m_packetSize = h1.GetSerializedSize() + h2.GetSerializedSize();
//        m_baudrate = NOCNetDevice.
//        m_packetSize = 16; //bytes
//        m_packetDuration = (m_packetSize * 10) / m_baudrate; //10 bits per byte
        

//        if (IsSink == true) {
////            Tests();
////            ClusterDataRequest();
//        }
        
//        Time t_ns = Time::FromInteger(0,Time::NS);
//        Simulator::Schedule(t_ns, &XDenseApp::GenerateFlow, this);
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
            hd.SetData(i);
            pck_c->AddHeader(hd);
            
            if (protocol == NOCHeader::PROTOCOL_UNICAST) {
                Simulator::Schedule(t_offset_t + (i * t_step_t), &NOCRouter::PacketUnicast, this->m_router, pck_c, NETWORK_ID_0, dest_x, dest_y, addressing);
            }
            else if(protocol == NOCHeader::PROTOCOL_UNICAST_OFFSET){
                Simulator::Schedule(t_offset_t + (i * t_step_t), &NOCRouter::PacketUnicastOffset, this->m_router, pck_c, NETWORK_ID_0, dest_x, dest_y);
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
    XDenseApp::Tests() {
        Ptr<Packet> pck = Create<Packet>();
        
        XDenseHeader hd;
        hd.SetXdenseProtocol(XDenseHeader::DATA_ANNOUCEMENT);
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

        m_router->PacketUnicastOffset(pck,NETWORK_ID_0,-15,-15);
        m_router->PacketUnicastOffset(pck,NETWORK_ID_0,-15, 15);
        m_router->PacketUnicastOffset(pck,NETWORK_ID_0, 7,-7);
        m_router->PacketUnicastOffset(pck,NETWORK_ID_0, 7, 7);
//        t_ns += 60 * PacketDuration;        
//        
//        Simulator::Schedule(t_ns, &NOCRouter::PacketMulticastRadius, this->m_router, pck, NETWORK_ID_0, 7);
//        t_ns += 60 * PacketDuration;
//        
//        Simulator::Schedule(t_ns, &NOCRouter::PacketMulticastArea, this->m_router, pck, NETWORK_ID_0, 7, 7);
//        t_ns += 60 * PacketDuration;
//        
//        Simulator::Schedule(t_ns, &NOCRouter::PacketMulticastIndividuals, this->m_router, pck, NETWORK_ID_0, ClusterSize, ClusterSize);
//        t_ns += 60 * PacketDuration;

//        
//        Simulator::Schedule(t_ns, &NOCRouter::PacketBroadcast, this->m_router, pck, NETWORK_ID_0);
//        t_ns += 60 * PacketDuration;
//        
//        m_router->PacketBroadcast(pck, 0);
    }
    
    

    void 
    XDenseApp::DataReceived(Ptr<const Packet> pck, uint8_t protocol, int32_t origin_x, int32_t origin_y, int32_t dest_x,int32_t dest_y) {
        //TODO: here, at this layer, we remove the XDense header and see what to do with the packet.
        //the NOC Header here is no longer in the packet
        Ptr<Packet> pck_c = pck->Copy();
        
        XDenseHeader h;
        pck_c->RemoveHeader(h);
        
        uint64_t tts_pck;
        int64x64_t tts_now, tts_total;
        
        switch (h.GetXdenseProtocol()){
            case XDenseHeader::DATA_ANNOUCEMENT:
                DataAnnoucementReceived(pck, origin_x, origin_y);
                break;
            case XDenseHeader::DATA_ANNOUCEMENT_REQUEST:
                DataSharingRequestReceived(pck, origin_x, origin_y);                
                break;
            case XDenseHeader::CLUSTER_DATA_REQUEST:
                ClusterDataRequestReceived(pck, origin_x, origin_y, dest_x, dest_y);                
                break;
            case XDenseHeader::NETWORK_SETUP:
//                NetworkSetupReceived(pck, origin_x, origin_y);                
            case XDenseHeader::TRACE:
//                tts_now = Simulator::Now().GetNanoSeconds() / PacketDuration;
                tts_pck = h.GetData();
                cout << "Traced packet received: id=" << pck->GetUid() << "\tdata=" << tts_pck << endl; 
//                tts_total = tts_now - tts_pck;
//                cout << "Received at the app layer:" << Simulator::Now().GetNanoSeconds() << " tts:" << tts_total << "\n";            
                break;
            
        }
    }
    
    void 
    XDenseApp::ClusterDataRequest() {
        Ptr<Packet> pck = Create<Packet>();

        XDenseHeader hd;
        hd.SetXdenseProtocol(XDenseHeader::CLUSTER_DATA_REQUEST);
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
        
        Time tbase = PacketDuration * (ClusterSize_x + 1) * (ClusterSize_x + 1) * 5;
        Time t;
        

        t = 0 * tbase;
        Simulator::Schedule(t, &XDenseApp::DataSharingRequest, this); // schedule the request
        
        t = tbase; //set the waiting time
        Simulator::Schedule(t, &XDenseApp::ClusterDataResponse, this, -origin_x, -origin_y); // and response
        
        return true;
    }
    
        void 
    XDenseApp::ClusterDataResponse(int32_t dest_x, int32_t dest_y) {
        
//            Simulator::Schedule(t_ns, &NOCRouter::PacketUnicastOffset, this->m_router, pck, NETWORK_ID_0, x_dest, y_dest);
        
        double_t b = 1;
        uint32_t ms = (ClusterSize_x + 1) * (ClusterSize_x + 1) * (1 - AggregationRate) * 5 / 4;
        uint8_t o = 0;
        
        int32_t orig_x_log = m_router->AddressX;
        int32_t orig_y_log = m_router->AddressY;
        int32_t dest_x_log = m_router->AddressX + dest_x;
        int32_t dest_y_log = m_router->AddressY + dest_y;
        
        // Construct the packet to be transmitted
        XDenseHeader hd_out;
        hd_out.SetXdenseProtocol(XDenseHeader::CLUSTER_DATA_RESPONSE);
        Ptr<Packet> pck_out = Create<Packet>();
        pck_out->AddHeader(hd_out);
        
        double offset_log = (Simulator::Now().GetNanoSeconds() / PacketDuration.GetNanoSeconds()) + o - 1; //get the absolute offset

        m_flows_source(orig_x_log, orig_y_log, dest_x_log, dest_y_log, offset_log, b, ms, NOCHeader::PROTOCOL_UNICAST_OFFSET);
        this->SetFlowGenerator(0, b, o, ms, pck_out, dest_x, dest_y, ADDRESSING_RELATIVE, NOCHeader::PROTOCOL_UNICAST_OFFSET);        
    }

    
    void
    XDenseApp::DataSharingRequest() {
        Ptr<Packet> pck = Create<Packet>();
        
        XDenseHeader hd;
        hd.SetXdenseProtocol(XDenseHeader::DATA_ANNOUCEMENT_REQUEST);
        hd.SetData(ClusterSize_x);
        pck->AddHeader(hd);
        
        m_router->PacketMulticastArea(pck, NETWORK_ID_0, ClusterSize_x, ClusterSize_y);
    }
    
    void
    XDenseApp::DataSharingRequestReceived(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y) {
//        XDenseHeader hd;
//        pck->PeekHeader(hd);
//        uint8_t size_x = hd.GetData();
//        uint8_t size_y = hd.GetData();
        
        double_t b = 0.1;
        uint32_t ms = 5;
        uint8_t o = 0;
        
        int32_t dest_x = origin_x * -1;
        int32_t dest_y = origin_y * -1;
        
        int32_t orig_x_log = m_router->AddressX;
        int32_t orig_y_log = m_router->AddressY;
        int32_t dest_x_log = m_router->AddressX + dest_x;
        int32_t dest_y_log = m_router->AddressY + dest_y;
        
        // Construct the packet to be transmitted
        XDenseHeader hd_out;
        hd_out.SetXdenseProtocol(XDenseHeader::DATA_ANNOUCEMENT);
        Ptr<Packet> pck_out = Create<Packet>();
        pck_out->AddHeader(hd_out);
        
//        uint8_t dist = NOCRouting::Distance(orig_x_log, orig_y_log, dest_x_log, dest_y_log);
//        double offset_log = dist + o; //get the absolute offset
        double offset_log = (Simulator::Now().GetNanoSeconds() / PacketDuration.GetNanoSeconds()) + o - 1; //get the absolute offset

        m_flows_source(orig_x_log, orig_y_log, dest_x_log, dest_y_log, offset_log, b, ms, NOCHeader::PROTOCOL_UNICAST);
        this->SetFlowGenerator(0, b, o, ms, pck_out, dest_x, dest_y, ADDRESSING_RELATIVE, NOCHeader::PROTOCOL_UNICAST);
    }

    void XDenseApp::CalculateShaper(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y) {
        
        XDenseHeader hd;
        pck->PeekHeader(hd);
        
        uint8_t size_x = hd.GetData();
        uint8_t size_y = hd.GetData();
        
        int32_t dest_x = origin_x * -1;
        int32_t dest_y = origin_y * -1;
        
        int8_t last_long;
        int8_t last_alt;
        int8_t delta_long;
        int8_t delta_alt;
        int8_t pos_long;
        int8_t pos_alt;        
        
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
//        double_t b      = double(1) / double(ClusterSize_x * (ClusterSize_y -1));
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
    XDenseApp::DataAnnouncement(int32_t x_dest, int32_t y_dest) {

        Ptr<Packet> pck = Create<Packet>();

        XDenseHeader hd;
        hd.SetXdenseProtocol(XDenseHeader::DATA_ANNOUCEMENT);
        pck->AddHeader(hd);

        m_router->PacketUnicast(pck, NETWORK_ID_0, x_dest, y_dest, ADDRESSING_RELATIVE); 
    }
    
    bool
    XDenseApp::DataAnnoucementReceived(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y) {
//        Here, the node should build an matrix with the received data
//        cout << "Data annoucement received" << endl;
        return false;
    }

}



