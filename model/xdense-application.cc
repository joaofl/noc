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
#include "noc-header.h"
#include "noc-net-device.h"
#include "noc-router.h"
#include "calc.h"
#include "sensor-data-io.h"
#include "noc-types.h"
#include "ns3/noc-routing-protocols.h"
#include "ns3/xdense-application.h"



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
        
        
        
        //This is a threshold, to decide the min neghborhood a node have to be 
        //to be able to do event annoucements.
        //These are all the possible neighbors times a factor, to include nodes in the edges,
        //which have less neighbors.
//        MinNeighborhood = MaxHops * 2 * (MaxHops + 1) * 0.7; //add some tolerance.Ex 20%

        m_router->SetReceiveCallback(MakeCallback(&XDenseApp::DataReceived, this));

        if (IsSink == true) {
//            Simulator::Schedule(TimeStartOffset, &XDenseApp::NetworkDiscovery, this);
            Simulator::Schedule(Time::FromInteger(0, Time::NS), &XDenseApp::DataAnnouncementRequest, this);
//            Simulator::Schedule(Time::FromInteger(0, Time::NS), &XDenseApp::NetworkSetup, this);
            SinkReceivedData = CreateObject<NOCOutputData> ();
        }
        

        
    }
    
    void
    XDenseApp::StopApplication(void) {
        m_running = false;
    }

    void
    XDenseApp::DataSharingSchedule(uint8_t n_times, Time period) {

        for (uint8_t i = 0; i < n_times; i++) {
            Time t = MilliSeconds(period.GetMilliSeconds() * i + TimeStartOffset.GetMilliSeconds() + period.GetMilliSeconds());
            Simulator::Schedule(t, &XDenseApp::DataSharing, this);
        }

    }

    void
    XDenseApp::AddRouter(Ptr<NOCRouter> r) {
        m_router = r;
    }
    
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
                DataAnnouncementRequestReceived(pck, origin_x, origin_y);                
                break;
            case XDenseHeader::NETWORK_SETUP:
                NetworkSetupReceived(pck, origin_x, origin_y);                
                break;
            
        }
    }
    
    void
    XDenseApp::NetworkSetup() {

//        if (IsSink == true) {

        NodeRef me;
        me.x = 0;
        me.y = 0;
        m_sinksList.push_back(me);

        XDenseHeader hd;
        hd.SetXdenseProtocol(XDenseHeader::NETWORK_SETUP);


        Ptr<Packet> pck = Create<Packet>();
        pck->AddHeader(hd);
            
            
        m_router->PacketBroadcast(pck, 0);
    }

    bool
    XDenseApp::NetworkSetupReceived(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y) {
        
        if ((abs(origin_x) % 11 == 0) && (abs(origin_y) % 11 == 0)){
            Simulator::Schedule(Time::FromInteger(1000000, Time::NS), &XDenseApp::DataAnnouncementRequest, this);
        }
            
        return false;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void
    XDenseApp::DataSharing(void) {
        
    }

    bool
    XDenseApp::DataSharingReceived(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y) {
        return false;
    }

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
    XDenseApp::DataAnnouncementRequest() {
        Ptr<Packet> pck = Create<Packet>();
        
        XDenseHeader hd;
        hd.SetXdenseProtocol(XDenseHeader::DATA_ANNOUCEMENT_REQUEST);
        pck->AddHeader(hd);
        
        m_router->PacketMulticast(pck, NETWORK_ID_0, 5, 5);
//        m_router->PacketBroadcast(pck, 0);
    }
    
    void
    XDenseApp::DataAnnouncementRequestReceived(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y) {
      
        //This info can actually be obteined from the system.
        //size is actually the radius or square defined by the multicast size
        //pck duration is contained at the packet, but the total lenght is only at
        //the bottom layer. 
        
        //TODO: is the schedule needs pck size, it should get it from below layers, since it is static
        // and pre-defined
        uint16_t pck_duration = 11200;
        int8_t size_x = 10;
        int8_t size_y = 10;
//        
        
        for (uint8_t j = 0 ; j < 1 ; j++)
        {
            int32_t time_slot = 0;
            time_slot = NOCRoutingProtocols::ScheduleTransmission(origin_x, origin_y, size_x, size_y);

            if (time_slot >= 0){
                Time t_ns = Time::FromInteger(time_slot * pck_duration, Time::NS);
                Simulator::Schedule(t_ns, &XDenseApp::DataAnnouncement, this, origin_x * -1, origin_y * -1);
            }
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



