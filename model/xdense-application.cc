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

#include "src/core/model/object-base.h"

#include "xdense-application.h"
#include "xdense-header.h"

//#include "noc-header.h"
#include "noc-net-device.h"
#include "noc-router.h"
#include "noc-routing-protocols.h"

#include "calc.h"
#include "sensor-data-io.h"
#include "ns3/noc-net-device.h"
#include "ns3/xdense-application.h"
//#include "noc-types.h"




//using namespace ns3NOCCalc;
using namespace std;
namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("XDenseApplication");

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
        
        //This is a threshold, to decide the min neghborhood a node have to be 
        //to be able to do event annoucements.
        //These are all the possible neighbors times a factor, to include nodes in the edges,
        //which have less neighbors.
//        MinNeighborhood = MaxHops * 2 * (MaxHops + 1) * 0.7; //add some tolerance.Ex 20%

        

        if (IsSink == true) {
            Tests();
//            ClusterDataRequest();
        }
        

        
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
        
//        m_router->PacketUnicast(pck,NETWORK_ID_0,-15,-15,USE_RELATIVE_ADDRESS);
//        m_router->PacketUnicast(pck,NETWORK_ID_0,-15, 15,USE_RELATIVE_ADDRESS);
//        m_router->PacketUnicast(pck,NETWORK_ID_0, 15,-15,USE_RELATIVE_ADDRESS);
        m_router->PacketUnicast(pck,NETWORK_ID_0, 15, 15,USE_RELATIVE_ADDRESS);

//        m_router->PacketUnicastOffset(pck,NETWORK_ID_0,-15,-15);
//        m_router->PacketUnicastOffset(pck,NETWORK_ID_0,-15, 15);
//        m_router->PacketUnicastOffset(pck,NETWORK_ID_0, 15,-15);
        m_router->PacketUnicastOffset(pck,NETWORK_ID_0, 15, 15);
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

    

//    void
//    XDenseApp::DataSharingSchedule(uint8_t n_times, Time period) {
//
//        for (uint8_t i = 0; i < n_times; i++) {
//            Time t = MilliSeconds(period.GetMilliSeconds() * i + period.GetMilliSeconds());
//            Simulator::Schedule(t, &XDenseApp::DataSharing, this);
//        }
//
//    }

 
    void 
    XDenseApp::DataReceived(Ptr<const Packet> pck, uint8_t protocol, int32_t origin_x, int32_t origin_y, int32_t dest_x,int32_t dest_y) {
        //TODO: here, at this layer, we remove the XDense header and see what to do with the packet.
        //the NOC Header here is no longer in the packet
        Ptr<Packet> pck_c = pck->Copy();
        
        XDenseHeader h;
        pck_c->RemoveHeader(h);
        
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

        Simulator::Schedule(t_ns, &NOCRouter::PacketMulticastIndividuals, this->m_router, pck, NETWORK_ID_0, ClusterSize_x, ClusterSize_y);
    }
    
    bool 
    XDenseApp::ClusterDataRequestReceived(Ptr<const Packet> pck_r, int32_t origin_x, int32_t origin_y, int32_t size_x, int32_t size_y) {
        
//        ClusterSize = size_x * size_y;
        
        Time tbase = PacketDuration * ClusterSize_x;
        Time t;
        
        for (int i = 1; i < 2; i++) {
            t = i * tbase;
            Simulator::Schedule(t, &XDenseApp::DataSharingRequest, this);
        }
        
        return true;
    }



    
//    void
//    XDenseApp::NetworkSetup() {
//
////        if (IsSink == true) {
//
////        NodeRef me;
////        me.x = 0;
////        me.y = 0;
////        m_sinksList.push_back(me);
//
//        XDenseHeader hd;
//        hd.SetXdenseProtocol(XDenseHeader::NETWORK_SETUP);
//
//
//        Ptr<Packet> pck = Create<Packet>();
//        pck->AddHeader(hd);
//            
//            
//        m_router->PacketBroadcast(pck, 0);
//    }
//
//    bool
//    XDenseApp::NetworkSetupReceived(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y) {
//        
//        if ((abs(origin_x) % 11 == 0) && (abs(origin_y) % 11 == 0)){
////            Simulator::Schedule(Time::FromInteger(1000000, Time::NS), &XDenseApp::DataAnnouncementRequest, this);
//               DataSharingRequest();
//        }
//        
//        
//        return false;
//    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

//    void
//    XDenseApp::DataSharing(void) {
//        
//    }
//
//    bool
//    XDenseApp::DataSharingReceived(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y) {
//        return false;
//    }

//    For testing proposes
//    void
//    XDenseApp::DataAnnouncementTT(void) {
//            Ptr<Packet> pck = Create<Packet>();
//            m_router->PacketUnicast(pck, NETWORK_ID_0, 0, 0, USE_ABSOLUTE_ADDRESS);  
            
//            m_router->PacketUnicast(pck, NETWORK_ID_0, 2, 2, USE_RELATIVE_ADDRESS);        
//            m_router->PacketUnicast(pck, NETWORK_ID_0, 2, -2, USE_RELATIVE_ADDRESS);        
//            m_router->PacketUnicast(pck, NETWORK_ID_0, -2, -2, USE_RELATIVE_ADDRESS);       
//            m_router->PacketUnicast(pck, NETWORK_ID_0, -2, 2, USE_RELATIVE_ADDRESS);        
            
//            m_router->PacketUnicast(pck, NETWORK_ID_0, 4, 4, USE_ABSOLUTE_ADDRESS);        
//            m_router->PacketUnicast(pck, NETWORK_ID_0, 4, 0, USE_ABSOLUTE_ADDRESS);        
//            m_router->PacketUnicast(pck, NETWORK_ID_0, 10, 10, USE_ABSOLUTE_ADDRESS);        
//            m_router->PacketUnicast(pck, NETWORK_ID_0, 0, 4, USE_ABSOLUTE_ADDRESS);                
//    }
    
    void
    XDenseApp::DataSharingRequest() {
        Ptr<Packet> pck = Create<Packet>();
        
        XDenseHeader hd;
        hd.SetXdenseProtocol(XDenseHeader::DATA_ANNOUCEMENT_REQUEST);
        pck->AddHeader(hd);
        
        m_router->PacketMulticastArea(pck, NETWORK_ID_0, ClusterSize_x/2, ClusterSize_y/2);
    }
    
    void
    XDenseApp::DataSharingRequestReceived(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y) {
      
        Time t_ns;
        int32_t time_slot = 0;
        
        for (uint8_t j = 0 ; j < 1 ; j++)
        {
            time_slot = NOCRoutingProtocols::ScheduleTransmission(origin_x, origin_y, ClusterSize_x, ClusterSize_y);
            
            if (time_slot >= 0){
                t_ns = time_slot * PacketDuration;
            }
            else{
                t_ns = Time::FromInteger(0, Time::US);
            }
            Simulator::Schedule(t_ns, &XDenseApp::DataAnnouncement, this, origin_x * -1, origin_y * -1);
        }
    }
    
    void
    XDenseApp::DataAnnouncement(int32_t x_dest, int32_t y_dest) {

        Ptr<Packet> pck = Create<Packet>();

        XDenseHeader hd;
        hd.SetXdenseProtocol(XDenseHeader::DATA_ANNOUCEMENT);
        pck->AddHeader(hd);

        //            m_router->PacketUnicast(pck, NETWORK_ID_0, x_dest, y_dest, USE_ABSOLUTE_ADDRESS); 
        m_router->PacketUnicast(pck, NETWORK_ID_0, x_dest, y_dest, USE_RELATIVE_ADDRESS); 
    }
    
    bool
    XDenseApp::DataAnnoucementReceived(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y) {
//        Here, the node should build an matrix with the received data
//        cout << "Data annoucement received" << endl;
        return false;
    }

}



