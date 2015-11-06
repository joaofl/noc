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
    //    m_packetSize(0),
    m_nPackets(0),
    m_dataRate(0),
    //    m_sendEvent(),
    m_running(false)
    //IsSink(false),
    //m_packetsSent(0),
    //m_packetsReceived(0)
    {
    }

    XDenseApp::~XDenseApp() {
        //  m_socket = 0;
    }

    //    void
    //    XDenseApp::Setup(bool IsSink) {
    //        IsSink = IsSink;
    //
    //    }

    void
    XDenseApp::StartApplication(void) {

//        TimeStartOffset = Seconds(1);
        
        //This is a threshold, to decide the min neghborhood a node have to be 
        //to be able to do event annoucements.
        //These are all the possible neighbors times a factor, to include nodes in the edges,
        //which have less neighbors.
        MinNeighborhood = MaxHops * 2 * (MaxHops + 1) * 0.7; //add some tolerance.Ex 20%

        SensorValueLast = 0;

        EventRef er;
        er.detected = false;
        er.data[0] = 0;
        er.data[1] = 0;
        er.data[2] = 0;
        m_lastEvents.assign(EV_COUNT, er);
        m_SerialNumber.assign(P_COUNT, 0);
        //        PacketTrace.assign(P_COUNT, temp);


        m_router->SetReceiveCallback(MakeCallback(&XDenseApp::DataReceived, this));

        if (IsSink == true) {
//            Simulator::Schedule(TimeStartOffset, &XDenseApp::NetworkDiscovery, this);
            SinkReceivedData = CreateObject<NOCOutputData> ();
        }
        
        Time t = Time::FromInteger(0, Time::MS);
        Time t_window = Time::FromInteger(0, Time::MS);
//        if (IsSink == true)
        
        uint16_t pck_duration = 7200;
        pck_duration = pck_duration;
        
        IntegerValue x, y;
        m_router->GetAttribute("AddressX", x);
        m_router->GetAttribute("AddressY", y);
        
//        if (x.Get() % 2 == 0 && y.Get() % 2 == 0){
//        if (x.Get() != 0 ){
        
        MaxHops = 10;
//        uint8_t x_size = 9;
//        uint8_t y_size = 10;
        
//        if ( (x.Get() + y.Get() <= MaxHops) && (x.Get() != 0) ){
//        && (x.Get() + y.Get() <= MaxHops)
//        if ( (x.Get() != 0)  ){
//        if (x.Get() == 10 && y.Get() == 10){
            if (IsSink == true){
//            Simulator::Schedule(t, &XDenseApp::NetworkDiscovery, this);
//            t += Time::FromInteger(pck_duration * 50, Time::NS);
            Simulator::Schedule(t, &XDenseApp::DataAnnouncementRequest, this);

//            for (uint8_t j = 0 ; j < 10 ; j++)
//            {
//                    t = Time::FromInteger(pck_duration * (x.Get() - 1) * (y_size - 1), Time::NS);
//                    t_window = Time::FromInteger(j * 120 * pck_duration, Time::NS);
//                        
//                    Simulator::Schedule(t + t_window, &XDenseApp::DataAnnouncementTT, this);
//            }
        }
    }
    
    void
    XDenseApp::StopApplication(void) {
        m_running = false;

        //        if (m_sendEvent.IsRunning()) {
        //            Simulator::Cancel(m_sendEvent);
        //        }
    }

    void
    XDenseApp::DataSharingSchedule(uint8_t n_times, Time period) {

        for (uint8_t i = 0; i < n_times; i++) {
            Time t = MilliSeconds(period.GetMilliSeconds() * i + TimeStartOffset.GetMilliSeconds() + period.GetMilliSeconds());
            Simulator::Schedule(t, &XDenseApp::DataSharing, this);
        }

    }

    NodeRef
    XDenseApp::GetSinkAt(uint8_t i) {
        return m_sinksList.at(i);
    }

    uint8_t
    XDenseApp::GetSinkN(void) {
        return m_sinksList.size();
    }

    NodeRef
    XDenseApp::GetNeighborAt(uint8_t i) {
        return m_neighborsList.at(i);
    }

    uint8_t
    XDenseApp::GetNeighborN(void) {
        return m_neighborsList.size();
    }

    void
    XDenseApp::AddRouter(Ptr<NOCRouter> r) {
        m_router = r;
    }
    
   

 
    void 
    XDenseApp::DataReceived(Ptr<const Packet> pck, uint8_t protocol, int32_t origin_x, int32_t origin_y, int32_t dest_x,int32_t dest_y) {
        //TODO: here, at this layer, we remove the XDense header and see what to do with the packet.
        //the NOC Header here is no longer in the packet
        
        
        uint16_t pck_duration = 7200;
        
//        if ( (origin_x != 0) && protocol == 0 ){ //from broadcast
        if ( protocol == 1 ){ //from multicast
            for (uint8_t j = 0 ; j < 1 ; j++)
            {
                int32_t t = 0;
                t = NOCRoutingProtocols::ScheduleTransmission(origin_x, origin_y, dest_x, dest_y);
                
                if (t >= 0){
                    Time t_ns = Time::FromInteger(t * pck_duration, Time::NS);
                    Simulator::Schedule(t_ns, &XDenseApp::DataAnnouncement, this, origin_x*-1, origin_y*-1);
                }
            }
        }
    }
    
    
    void
    XDenseApp::NetworkDiscovery() {

//        if (IsSink == true) {

        NodeRef me;
        me.x = 0;
        me.y = 0;
        m_sinksList.push_back(me);

        XDenseHeader hd;
        hd.CurrentX = 0;
        hd.CurrentY = 0;
        hd.SetNOCProtocol(P_NETWORK_DISCOVERY);
        hd.OperationalMode = OperationalMode;
        hd.SerialNumber = m_SerialNumber.at(P_NETWORK_DISCOVERY);
        m_SerialNumber.at(P_NETWORK_DISCOVERY)++;

        Ptr<Packet> pck = Create<Packet>();
//        pck->AddHeader(hd);
            
            
        m_router->PacketBroadcast(pck, 0);
    }

    bool
    XDenseApp::NetworkDiscoveryReceived(Ptr<const Packet> pck, uint8_t origin_port) {

        return false;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void
    XDenseApp::DataSharing(void) {
        
    }


    bool
    XDenseApp::DataSharingReceived(Ptr<const Packet> pck, uint8_t origin_port) {
        return false;
    }

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
//        hd.Protocol = P_DATA_ANNOUCEMENT_REQUEST;
//        hd.SetNOCProtocol(XDenseHeader::P_DATA_ANNOUCEMENT_REQUEST);
//        pck->AddHeader(hd);
        
        m_router->PacketMulticast(pck,NETWORK_ID_0, 10, 10);
//        m_router->PacketBroadcast(pck, 0);
    }

    
    void
    XDenseApp::DataAnnouncement(int32_t x_dest, int32_t y_dest) {



            Ptr<Packet> pck = Create<Packet>();
//            pck->AddHeader(hd);
//            m_router->PacketUnicast(pck, 0, sink.x, sink.y, false);     
            
//            m_router->PacketUnicast(pck, NETWORK_ID_0, x_dest, y_dest, USE_ABSOLUTE_ADDRESS); 
            m_router->PacketUnicast(pck, NETWORK_ID_0, x_dest, y_dest, USE_RELATIVE_ADDRESS); 
        


    }
    


    bool
    XDenseApp::DataAnnoucementReceived(Ptr<const Packet> pck, uint8_t origin_port) {
        
        return false;
    }

}



