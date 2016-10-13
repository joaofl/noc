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
                
        m_router->SetShaper(b, rd, port);
    }
    
    void 
    XDenseApp::SetFlowGenerator(uint8_t start_delay, double_t burstiness, double_t offset, uint32_t msg_size, Ptr<const Packet> pck, int32_t dest_x, int32_t dest_y) {
        if (burstiness == 0 || IsActive == false)
            return;        

//        XDenseHeader hd;
//        pck->PeekHeader(hd);
//        hd.SetXdenseProtocol(XDenseHeader::DATA_ANNOUCEMENT);
//        if (trace){
//            hd.SetXdenseProtocol(XDenseHeader::TRACE);
//            hd.SetData(0 + release_delay);
//        }
        int32_t orig_x, orig_y;
        
        orig_x = m_router->AddressX;
        orig_y = m_router->AddressY;
        
        m_flows_source(orig_x, orig_y, dest_x, dest_y, offset, burstiness, msg_size);
        
        Time t_step = Time::FromInteger(PacketDuration.GetNanoSeconds() / burstiness, Time::NS);
        Time t_offset = Time::FromInteger(PacketDuration.GetNanoSeconds() * (offset + start_delay), Time::NS);
        
        for (uint16_t i = 0 ; i < msg_size ; i++ ){
            Ptr<Packet> pck_c = pck->Copy();
            
            Simulator::Schedule(t_offset + (i * t_step), &NOCRouter::PacketUnicast, this->m_router, pck_c, NETWORK_ID_0, dest_x, dest_y, USE_ABSOLUTE_ADDRESS);
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
        
        m_router->PacketUnicast(pck,NETWORK_ID_0,-15,-15,USE_RELATIVE_ADDRESS);
        m_router->PacketUnicast(pck,NETWORK_ID_0,-15, 15,USE_RELATIVE_ADDRESS);
        m_router->PacketUnicast(pck,NETWORK_ID_0, 7,-7,USE_RELATIVE_ADDRESS);
        m_router->PacketUnicast(pck,NETWORK_ID_0, 7, 7,USE_RELATIVE_ADDRESS);

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
                tts_now = Simulator::Now().GetNanoSeconds() / PacketDuration;
                tts_pck = h.GetData();
                tts_total = tts_now - tts_pck;
                cout << "Received at the app layer:" << Simulator::Now().GetNanoSeconds() << " tts:" << tts_total << "\n";            
                break;
            
        }
    }
    
    void 
    XDenseApp::ClusterDataRequest() {
        Ptr<Packet> pck = Create<Packet>();

        XDenseHeader hd;
        hd.SetXdenseProtocol(XDenseHeader::CLUSTER_DATA_REQUEST);
        pck->AddHeader(hd);

        Time t_ns = Time::FromInteger(0,Time::NS);

//        Simulator::Schedule(t_ns, &NOCRouter::PacketMulticastIndividuals, this->m_router, pck, NETWORK_ID_0, ClusterSize_x, ClusterSize_y);
            this->m_router->PacketMulticastIndividuals(pck, NETWORK_ID_0, ClusterSize_x, ClusterSize_y);
//        this->ClusterDataRequestReceived(pck, 0, 0, ClusterSize_x, ClusterSize_y);
    }
    
    bool 
    XDenseApp::ClusterDataRequestReceived(Ptr<const Packet> pck_r, int32_t origin_x, int32_t origin_y, int32_t size_x, int32_t size_y) {
        
        //Cluster heads receive a request for it to share its cluster data
        //then it schedule few local requests, and replies to the global sink,
        //so it can send it back its pre-processed cluster data
        
        Time tbase = PacketDuration * (ClusterSize_x * ClusterSize_x - 1);
        Time t;
        
        for (int i = 1; i <= 1; i++) {
            t = (i + 0) * tbase;
            Simulator::Schedule(t, &XDenseApp::DataSharingRequest, this); // schedule the request
            t = (i + 1) * tbase + PacketDuration; //set the waiting time
            Simulator::Schedule(t, &XDenseApp::ClusterDataResponse, this, origin_x*-1, origin_y*-1); // and response
        }
        
        return true;
    }
    
        void 
    XDenseApp::ClusterDataResponse(int32_t x_dest, int32_t y_dest) {
        Ptr<Packet> pck = Create<Packet>();

        XDenseHeader hd;
//        hd.SetXdenseProtocol(XDenseHeader::CLUSTER_DATA_RESPONSE);
        hd.SetXdenseProtocol(XDenseHeader::TRACE);
        hd.SetData(0 ); // + release_delay);
        
        pck->AddHeader(hd);

        Time t_ns = Time::FromInteger(0,Time::NS);
        
        for (uint8_t j = 0 ; j < 10 ; j++)
        {
            t_ns = j * PacketDuration;
            Simulator::Schedule(t_ns, &NOCRouter::PacketUnicastOffset, this->m_router, pck, NETWORK_ID_0, x_dest, y_dest);
        
        }
    }

    
    void
    XDenseApp::DataSharingRequest() {
        Ptr<Packet> pck = Create<Packet>();
        
        XDenseHeader hd;
        hd.SetXdenseProtocol(XDenseHeader::DATA_ANNOUCEMENT_REQUEST);
        hd.SetData(ClusterSize_x);
        pck->AddHeader(hd);
        
        m_router->PacketMulticastArea(pck, NETWORK_ID_0, ClusterSize_x/2, ClusterSize_y/2);
    }
    
    void
    XDenseApp::DataSharingRequestReceived(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y) {
        
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
            case NOCRouting::QUADRANT_PXPY:
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
        uint32_t ms     = 1;
        uint8_t rd      = dist + 1; // all nodes send together. First wait for the last
        rd = 0; //send the response as soon as the request is received
        //At each node rd = 0, but relatively to origin, at the time the request was made,
        // rd = do + 1, where do is distance to the origin
        
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
            this->SetShaper(shaper_b, shaper_rd, dir);
//        }
        
        // Construct the packet to be transmitted
        XDenseHeader hd_out;
        hd_out.SetXdenseProtocol(XDenseHeader::DATA_ANNOUCEMENT);
        Ptr<Packet> pck_out = Create<Packet>();
        pck_out->AddHeader(hd_out);
        
        this->SetFlowGenerator(0, b, rd, ms, pck_out, dest_x, dest_y);
    }
    
    void
    XDenseApp::DataAnnouncement(int32_t x_dest, int32_t y_dest) {

        Ptr<Packet> pck = Create<Packet>();

        XDenseHeader hd;
        hd.SetXdenseProtocol(XDenseHeader::DATA_ANNOUCEMENT);
        pck->AddHeader(hd);

        m_router->PacketUnicast(pck, NETWORK_ID_0, x_dest, y_dest, USE_RELATIVE_ADDRESS); 
    }
    
    bool
    XDenseApp::DataAnnoucementReceived(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y) {
//        Here, the node should build an matrix with the received data
//        cout << "Data annoucement received" << endl;
        return false;
    }

}



