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
 */


#include "src/core/model/object.h"
#include "src/network/model/node.h"

#include "noc-header.h"
#include "noc-net-device.h"
#include "noc-router.h"


using namespace std;
namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("NOCRouter");

    TypeId
    NOCRouter::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::NOCRouter")
                .SetParent<Application> ()
                .AddConstructor<NOCRouter> ()
        
                .AddTraceSource("SwitchRxTrace", 
                "The packets received by the router of each node",
                MakeTraceSourceAccessor(&NOCRouter::m_routerRxTrace),
                "ns3::NOCRouter::SwitchRxTrace")
        
                .AddTraceSource("SwitchTxTrace", 
                "The packets sent by the router of each node",
                MakeTraceSourceAccessor(&NOCRouter::m_routerTxTrace),
                "ns3::NOCRouter::SwitchTxTrace")
        
                .AddTraceSource("SwitchTxDropTrace", 
                "The packets sent by the router of each node",
                MakeTraceSourceAccessor(&NOCRouter::m_routerTxDropTrace),
                "ns3::NOCRouter::SwitchTxDropTrace")
        
                .AddAttribute("ChannelCount",
                "Defines the number of NOCNetDevices installed at each direction",
                UintegerValue(true),
                MakeUintegerAccessor(&NOCRouter::m_channelCount),
                MakeUintegerChecker<uint8_t>())
        
                .AddAttribute("AddressX",
                "The X coordinate of the router in the grid (required depending on the protocol)",
                IntegerValue(0),
                MakeIntegerAccessor(&NOCRouter::m_addressX),
                MakeIntegerChecker<int32_t>())

                .AddAttribute("AddressY",
                "The Y coordinate of the router in the grid (required depending on the protocol)",
                IntegerValue(0),
                MakeIntegerAccessor(&NOCRouter::m_addressY),
                MakeIntegerChecker<int32_t>())
                ;
        return tid;
    }

    NOCRouter::NOCRouter()
    : m_running(false) {
    }

    NOCRouter::~NOCRouter() {
    }

    void
    NOCRouter::StartApplication(void) {
        m_running = true;
        
        NetDeviceContainer::Iterator nd;
        for (nd = m_netDevices.Begin() ; nd != m_netDevices.End() ; nd++){    
            (*nd)->GetObject<NOCNetDevice>()->SetReceiveCallback(MakeCallback(&NOCRouter::PacketReceived, this));
//            (*nd)->GetObject<NOCNetDevice>()->SetRemoteSignalChangedCallback(MakeCallback(&NOCRouter::RemoteWaitChanged, this));
//            (*nd)->GetObject<NOCNetDevice>()->SetLocalSignalChangedCallback(MakeCallback(&NOCRouter::LocalWaitChanged, this));
        }
        
//        Simulator::Schedule(PicoSeconds(1500), &NOCRouter::ServePackets, this);
        m_port_to_serve = DIRECTION_E;
    }

    void
    NOCRouter::StopApplication(void) {
        m_running = false;

        if (m_sendEvent.IsRunning()) {
            Simulator::Cancel(m_sendEvent);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    
    void NOCRouter::SetReceiveCallback(ReceiveCallback cb){
        m_receiveCallBack = cb;
    }
    
    void
    NOCRouter::AddNetDevice(Ptr<NOCNetDevice> nd, uint8_t cluster, uint32_t network, uint8_t direction){
        NetDeviceInfo nd_info;
        
        nd_info.cluster_id = cluster;
        nd_info.network_id = network;
        nd_info.direction = direction;
        nd_info.nd_pointer = nd;
        nd_info.wait = false;
        nd_info.wait_remote = false;
//        nd_info.pck_buffered = false;
        m_netDeviceInfoArray.push_back(nd_info);
        
        nd->SetIfIndex(m_netDeviceInfoArray.size() - 1);        
        m_netDevices.Add(nd);
    }
    
    Ptr<NOCNetDevice>
    NOCRouter::GetNetDevice(uint8_t network, uint8_t direction){
        for (uint8_t i = 0 ; i < m_netDeviceInfoArray.size() ; i++){
            NetDeviceInfo nd_info = m_netDeviceInfoArray.at(i);
            if (nd_info.direction == direction && nd_info.network_id == network){
                return nd_info.nd_pointer;
            }
        }
        return NULL; //if the specified node was not found
    }
    
    NOCRouter::NetDeviceInfo
    NOCRouter::GetNetDeviceInfo(uint8_t network, uint8_t direction){
        NetDeviceInfo nd_info;
        for (uint8_t i = 0 ; i < m_netDeviceInfoArray.size() ; i++){
            nd_info = m_netDeviceInfoArray.at(i);
            if (nd_info.direction == direction && nd_info.network_id == network){
                return nd_info;
            }
        }
        nd_info.cluster_id = 99;
        return nd_info; //if the specified node was not found
    }
    int8_t
    NOCRouter::GetNetDeviceInfoIndex(uint8_t network, uint8_t direction){
        NetDeviceInfo nd_info;
        for (uint8_t i = 0 ; i < this->m_netDeviceInfoArray.size() ; i++){
            nd_info = this->m_netDeviceInfoArray.at(i);
            if (nd_info.direction == direction && nd_info.network_id == network){
                return i;
            }
        }
        return -1; //if the specified node was not found
    }
    int8_t
    NOCRouter::GetNetDeviceInfoIndex(Ptr<NOCNetDevice> nd){
//        for (uint8_t i = 0 ; i < this->m_netDeviceInfoArray.size() ; i++){
//            if (nd == this->m_netDeviceInfoArray[i].nd_pointer)
//            {
//                return i; //From the index initially set
//            }
//        }
//        return -1; //if the specified node was not found  
        
        return nd->GetIfIndex();
    }
    
    NOCRouter::NetDeviceInfo 
    NOCRouter::GetNetDeviceInfo(Ptr<NOCNetDevice> nd) {
        for (uint8_t i = 0 ; i < m_netDeviceInfoArray.size() ; i++){
            if (nd == m_netDeviceInfoArray[i].nd_pointer)
            {
                return m_netDeviceInfoArray[i]; //From the index initially set
            }
        }
        NetDeviceInfo r;
        return r; //if the specified node was not found        
    }
    
    uint8_t
    NOCRouter::GetNDevices(void){
        return m_netDevices.GetN();
    }    
    
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    
    
    bool NOCRouter::PacketUnicast (Ptr<const Packet> pck, uint8_t network_id, 
            int32_t destination_x, int32_t destination_y){
        
        uint8_t out = RouteTo(COLUMN_FIRST, destination_x, destination_y);
        
//        TODO: here the information with the origin and destination should be added
//        to a second header, which is added to the packet, in a multilayer style.
        
        return this->PacketSendSingle(pck->Copy(), GetNetDevice(network_id, out), P0);
        
        
//        Ptr<NOCNetDevice> nd = device->GetObject<NOCNetDevice>();
        
//        if ()
//        {
////        uint8_t i = this->GetNetDeviceInfoIndex(network_id, output_port);
////        if (m_netDeviceInfoArray[i].pck_buffered == false)
////        {
////            m_netDeviceInfoArray[i].pck_buffer = pck->Copy();
////            m_netDeviceInfoArray[i].pck_buffered = true;
////            Simulator::Schedule(PicoSeconds(1000), &NOCRouter::ServePackets, this);
//            
//            return true;
//        }
//        else
//        {
//            cout << "Packet dropped at "
//                 << m_addressX << "," << m_addressY <<" Buffer was full" << endl;
//        }
//
//        return false;
    }

    bool NOCRouter::PacketMulticast (Ptr<const Packet> pck, uint8_t network_id){
        return false;
    }

    bool NOCRouter::PacketBroadcast (Ptr<const Packet> pck, uint8_t network_id){
        
        NOCHeader h;
        
        h.SetProtocol(NOCHeader::PROTOCOL_BROADCAST);
        h.SetSourceAddress(0);
        
        pck->Copy()->AddHeader(h);
        PacketSendMultiple(pck->Copy(), network_id, DIRECTION_MASK_ALL_EXCEPT_LOCAL, P0);
        return false;
    }
    
    bool
    NOCRouter::PacketSendMultiple(Ptr<const Packet> pck, uint8_t network_id, uint8_t ports_mask, uint8_t priority){//, uint8_t optional network_id) {
        //TODO: This function should get the pck, the destination address, priority comes in the packet?, and network it should write to.
        // the rout should be calculated by the router itself. and the routing algorithms should be here (or in separate files).
        // the addressing scheme sould allow: Broadcast (with limited radius (hops)) and unicast (to an specific X,Y location)

        //TODO:
//        uint8_t out_ports_count = CountOnes(ports_mask);
        
        uint8_t sent = 0;

        if ( (ports_mask >> DIRECTION_N) & 1){
            if (PacketSendSingle(pck->Copy(), GetNetDevice(network_id, DIRECTION_N), priority));
                sent++;
        }
        if ( (ports_mask >> DIRECTION_S) & 1){
            if (PacketSendSingle(pck->Copy(), GetNetDevice(network_id, DIRECTION_S), priority));
                sent++;
        }
        if ( (ports_mask >> DIRECTION_E) & 1){
            if (PacketSendSingle(pck->Copy(), GetNetDevice(network_id, DIRECTION_E), priority));
                sent++;
        }
        if ( (ports_mask >> DIRECTION_W) & 1){
            if (PacketSendSingle(pck->Copy(), GetNetDevice(network_id, DIRECTION_W), priority));
                sent++;
        }
        if ( (ports_mask >> DIRECTION_L) & 1){
//            m_receiveCallBack(pck->Copy(), DIRECTION_L); //Loopback
        }
        
        if (sent>0) return true;
        else return false;
    }
    
    bool
    NOCRouter::PacketSendSingle(Ptr<const Packet> pck, Ptr<NOCNetDevice> nd, uint8_t priority){
        if (nd->Send(pck->Copy())){
            m_routerTxTrace(pck);
            return true;
        }
        else{
            m_routerTxDropTrace(pck);
            return false;
        }
        return false;
    }
       
    void
    NOCRouter::PacketReceived(Ptr<const Packet> pck, Ptr<NOCNetDevice> device) {

        m_routerRxTrace(pck->Copy());
        
        NOCHeader h;
        pck->PeekHeader(h);
        
        int32_t a = h.GetDestinationAddress();
        
        uint8_t p = h.GetProtocol();
        
        cout << a << std::endl;
        
        switch (p){
            case NOCHeader::PROTOCOL_BROADCAST:
               
               break;
               
            case NOCHeader::PROTOCOL_MULTICAST:
               
               break;
               
            case NOCHeader::PROTOCOL_UNICAST:
               
               break;
               
            default:
                cout << "Protocol not identified" << std::endl;
                break;
        }

                    
//
//                int32_t x = h.GetDestinationAddressX();
//                int32_t y = h.GetDestinationAddressY();
        
        
        
//        Ptr<NOCNetDevice> nd = device->GetObject<NOCNetDevice>();
        
//        uint8_t i = this->GetNetDeviceInfoIndex(nd);
//        if (m_netDeviceInfoArray[i].pck_buffered == false)
//        {
//            m_netDeviceInfoArray[i].pck_buffer = pck->Copy();
//            m_netDeviceInfoArray[i].pck_buffered = true;
//            Simulator::Schedule(PicoSeconds(1000), &NOCRouter::ServePackets, this);
//        }
//        else
//            cout << "Packet dropped at "
//                 << m_addressX << "," << m_addressY <<" Buffer was full" << endl;



        
//        return true;
    }
    
//    void
//    NOCRouter::RemoteWaitChanged(uint8_t signal, Ptr<NOCNetDevice> nd_this, bool wait_state){
//        
//        m_netDeviceInfoArray.at(nd_this->GetIfIndex()).wait_remote = wait_state;
//        cout << Simulator::Now() << " RWC " 
//                << m_addressX << "," << m_addressY
//                << " ND: " << m_netDeviceInfoArray.at(nd_this->GetIfIndex()).direction
//                << " s: " << wait_state << endl;
//    }
//    void
//    NOCRouter::LocalWaitChanged(uint8_t signal, Ptr<NOCNetDevice> nd_this, bool wait_state){
//        /* Here, check if wait was set low. In this case, check if there are buffered
//         * packets that want to use that port. Serve all the sources using round
//         * robin police.
//         */
//        m_netDeviceInfoArray.at(nd_this->GetIfIndex()).wait = wait_state;
//        
////        ServePorts();
//        
//        cout << Simulator::Now() << " LWC " << m_addressX << "," << m_addressY << " s: " << wait_state << endl;
//    }

    
    //Using XY routing
    uint8_t NOCRouter::RouteTo(uint8_t routing_alg, int32_t x, int32_t y) { //X-Y routing, with X first
        
        uint8_t dir;

        switch (routing_alg){       
            case COLUMN_FIRST:
                if      (m_addressY < y)    dir = NOCRouter::DIRECTION_S;
                else if (m_addressY > y)    dir = NOCRouter::DIRECTION_N;

                else if (m_addressX < x)    dir = NOCRouter::DIRECTION_E;
                else if (m_addressX > x)    dir = NOCRouter::DIRECTION_W;

                else if (m_addressX == x && m_addressY == y) 
                                            dir = NOCRouter::DIRECTION_L;
                break;
            
            case ROW_FIRST:
                if      (m_addressX < x)    dir = NOCRouter::DIRECTION_E;
                else if (m_addressX > x)    dir = NOCRouter::DIRECTION_W;
                
                else if (m_addressY < y)    dir = NOCRouter::DIRECTION_S;
                else if (m_addressY > y)    dir = NOCRouter::DIRECTION_N;

                else if (m_addressX == x && m_addressY == y) 
                                            dir = NOCRouter::DIRECTION_L;
                break;                
            case CLOCKWISE:
                break;
        }
        return dir;
    }
}

