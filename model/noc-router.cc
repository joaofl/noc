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
#include "ns3/noc-router.h"


using namespace std;
namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("NOCRouter");

    TypeId
    NOCRouter::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::NOCRouter")
                .SetParent<Application> ()
                .AddConstructor<NOCRouter> ()
        
                .AddTraceSource("RouterRxTrace", 
                "The packets received by the router of each node",
                MakeTraceSourceAccessor(&NOCRouter::m_routerRxTrace),
                "ns3::NOCRouter::SwitchRxTrace")
        
                .AddTraceSource("RouterTxTrace", 
                "The packets sent by the router of each node",
                MakeTraceSourceAccessor(&NOCRouter::m_routerTxTrace),
                "ns3::NOCRouter::SwitchTxTrace")
        
                .AddTraceSource("RouterTxDropTrace", 
                "The packets sent by the router of each node",
                MakeTraceSourceAccessor(&NOCRouter::m_routerTxDropTrace),
                "ns3::NOCRouter::RouterTxDropTrace")
        
                .AddAttribute("ChannelCount",
                "Defines the number of NOCNetDevices installed at each direction",
                UintegerValue(1),
                MakeUintegerAccessor(&NOCRouter::m_channelCount),
                MakeUintegerChecker<uint8_t>())
        
                .AddAttribute("UnicastProtocol",
                "Defines the routing protocol utilized for unicasting",
                UintegerValue(ROUTING_COLUMN_FIRST),
                MakeUintegerAccessor(&NOCRouter::m_routing_unicast),
                MakeUintegerChecker<uint8_t>())
        
                .AddAttribute("MultiProtocol",
                "Defines the routing protocol utilized for multicasting",
                UintegerValue(ROUTING_MULTICAST),
                MakeUintegerAccessor(&NOCRouter::m_routing_multicast),
                MakeUintegerChecker<uint8_t>())
        
                .AddAttribute("BroadcastProtocol",
                "Defines the routing protocol utilized for broadcasting",
                UintegerValue(ROUTING_BROADCAST),
                MakeUintegerAccessor(&NOCRouter::m_routing_broadcast),
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
//        m_port_to_serve = DIRECTION_E;
        m_useRelativeAddress = 1;
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
    
    Ptr<NOCNetDevice>
    NOCRouter::GetNetDevice(uint8_t i){
        if (i < m_netDeviceInfoArray.size()){
            NetDeviceInfo nd_info = m_netDeviceInfoArray.at(i);
            return nd_info.nd_pointer;
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
    
    
//    bool NOCRouter::PacketUnicast (Ptr<const Packet> pck, uint8_t network_id, 
//        int32_t destination_x, int32_t destination_y){
//        return PacketUnicast (pck, network_id, destination_x, destination_y, false);
//    }
    
    
    bool NOCRouter::PacketUnicast (Ptr<const Packet> pck, uint8_t network_id, 
            int32_t destination_x, int32_t destination_y, bool absolute_address)
    {
        if (absolute_address){
                destination_x = destination_x - m_addressX;
                destination_y = destination_y - m_addressY;
        }
        
        NOCHeader h;
        h.SetProtocol(NOCHeader::PROTOCOL_UNICAST);
        h.SetSourceAddressXY(0,0);
        h.SetDestinationAddressXY(destination_x,destination_y);
        
        Ptr<Packet> pck_c = pck->Copy();
        pck_c->AddHeader(h);
        
        uint8_t out = RouteTo(m_routing_unicast,0,0, destination_x, destination_y);
        
        return PacketSendMultiple(pck_c, network_id, out, P0);
    }

    bool NOCRouter::PacketMulticast (Ptr<const Packet> pck, uint8_t network_id){
        return false;
    }

    bool NOCRouter::PacketBroadcast (Ptr<const Packet> pck, uint8_t network_id){
        NOCHeader h;
        h.SetProtocol(NOCHeader::PROTOCOL_BROADCAST);
        h.SetSourceAddressXY(0,0);
        h.SetDestinationAddressXY(0,0);
        
        Ptr<Packet> pck_c = pck->Copy();
        pck_c->AddHeader(h);
        
        uint8_t out = RouteTo(m_routing_broadcast,0,0,0,0);
        
        if (PacketSendMultiple(pck_c, network_id, out, P0) > 0)
           return true;
        
        return false;    
    }
    
    uint8_t
    NOCRouter::PacketSendMultiple(Ptr<const Packet> pck, uint8_t network_id, uint8_t ports_mask, uint8_t priority){//, uint8_t optional network_id) {
        //TODO: This function should get the pck, the destination address, priority comes in the packet?, and network it should write to.
        // the rout should be calculated by the router itself. and the routing algorithms should be here (or in separate files).
        // the addressing scheme sould allow: Broadcast (with limited radius (hops)) and unicast (to an specific X,Y location)

        //TODO:
//        uint8_t out_ports_count = CountOnes(ports_mask);
        
        uint8_t sent = 0;

        if ( (ports_mask >> DIRECTION_N) & 1)
        {
            Ptr<Packet> pck_c = pck->Copy();
            if (PacketSendSingle(pck_c, network_id, DIRECTION_N, priority)){
                sent++;
            }
        }
        if ( (ports_mask >> DIRECTION_S) & 1){
            Ptr<Packet> pck_c = pck->Copy();
            if (PacketSendSingle(pck_c, network_id, DIRECTION_S, priority)){
                sent++;
            }
        }
        if ( (ports_mask >> DIRECTION_E & 1) & 1){
            Ptr<Packet> pck_c = pck->Copy();
            if (PacketSendSingle(pck_c, network_id, DIRECTION_E, priority)){
                sent++;
            }
        }
        if ( (ports_mask >> DIRECTION_W) & 1){
            Ptr<Packet> pck_c = pck->Copy();      
            if (PacketSendSingle(pck_c, network_id, DIRECTION_W, priority)){
                sent++;
            }
        }
        if ( (ports_mask >> DIRECTION_L) & 1){
            Ptr<Packet> pck_c = pck->Copy();
            if (PacketSendSingle(pck_c, network_id, DIRECTION_L, priority)){
                sent++;
            }            
        }
        
        return sent;
    }
    
    bool
    NOCRouter::PacketSendSingle(Ptr<const Packet> pck, uint8_t network_id, uint8_t port_direction, uint8_t priority){
        Ptr<NOCNetDevice> nd;
        
        if (port_direction != DIRECTION_L){
            nd = GetNetDevice(network_id, port_direction);
            if (nd == NULL) //That node does not have a net device in that direction
                return false;
        }
        else{  //Packet reached its destination, send it to the upper layers
            m_receiveCallBack(pck->Copy(), DIRECTION_L);
            return true;
        }
        
        if (nd->Send(pck->Copy())){
            m_routerTxTrace(pck);
            return true;
        }
        else{
            m_routerTxDropTrace(pck);
        }
        return false;
    }
       
    void
    NOCRouter::PacketReceived(Ptr<const Packet> pck, Ptr<NOCNetDevice> nd) {
        m_routerRxTrace(pck);
        Ptr<Packet> pck_c = pck->Copy();
        NetDeviceInfo nd_i = GetNetDeviceInfo(nd);
        
        NOCHeader h;
        pck_c->RemoveHeader(h);
        
        switch (nd_i.direction){
            case DIRECTION_S: 
                h.AddtoSourceAddress( 0, 1);
                h.AddtoDestinationAddress( 0, -1);
                break;
            case DIRECTION_N:
                h.AddtoSourceAddress( 0,-1); 
                h.AddtoDestinationAddress( 0, 1);
                break;
            case DIRECTION_E: 
                h.AddtoSourceAddress(-1, 0); 
                h.AddtoDestinationAddress( 1, 0);
                break;
            case DIRECTION_W: 
                h.AddtoSourceAddress( 1, 0); 
                h.AddtoDestinationAddress( -1, 0);
                break;
        }
        
        pck_c->AddHeader(h);
        
        
        
        int32_t adx = h.GetDestinationAddressX();
        int32_t ady = h.GetDestinationAddressY();
        int32_t asx = h.GetSourceAddressX();
        int32_t asy = h.GetSourceAddressY();
        uint8_t p = h.GetProtocol();
        
        uint8_t out = 0;
        switch (p){
            case NOCHeader::PROTOCOL_BROADCAST:
                out = RouteTo(m_routing_broadcast,asx,asy,adx,ady);
                PacketSendMultiple(pck_c, nd_i.network_id, out, P0);
                break;
               
            case NOCHeader::PROTOCOL_MULTICAST:
               
                break;
               
            case NOCHeader::PROTOCOL_UNICAST:
                out = RouteTo(m_routing_unicast,asx,asy,adx,ady);
                PacketSendMultiple(pck_c, nd_i.network_id, out, P0);
                break;
               
            default:
                cout << "Protocol unknown" << std::endl;
                break;
        }
    }
    
    
    //Using XY routing
    uint8_t NOCRouter::RouteTo(uint8_t routing_alg, int32_t x_source, int32_t y_source, int32_t x_dest, int32_t y_dest) { //X-Y routing, with X first
        
        uint8_t dir = 0;

        switch (routing_alg){       
            case ROUTING_COLUMN_FIRST:
                if      (y_dest < 0)    dir = DIRECTION_MASK_S;
                else if (y_dest > 0)    dir = DIRECTION_MASK_N;

                else if (x_dest > 0)    dir = DIRECTION_MASK_E;
                else if (x_dest < 0)    dir = DIRECTION_MASK_W;

                else if (0 == x_dest && 0 == y_dest) dir = DIRECTION_MASK_L;
                break;
            
            case ROUTING_ROW_FIRST:
                if      (x_dest > 0)    dir = DIRECTION_MASK_E;
                else if (x_dest < 0)    dir = DIRECTION_MASK_W;
                
                else if (y_dest > 0)    dir = DIRECTION_MASK_S;
                else if (y_dest < 0)    dir = DIRECTION_MASK_N;

                else if (0 == x_dest && 0 == y_dest) dir = DIRECTION_MASK_L;
                break;                
            case ROUTING_CLOCKWISE:
                break;
                
            case ROUTING_BROADCAST:
                
                //Check in which quadrant the packet is in:
                
                /*              |
                 *       B      |     A
                 *              |
                 * -------------|-------------
                 *              |
                 *       C      |     D
                 *              |
                 */
                
                dir = DIRECTION_MASK_L; //It sends the packet inside anyway, since it
                                         //was received, but not sent to the app yet
                
                //A: [+x +y[
                if (x_source >= 0 && y_source > 0){
                   dir |= DIRECTION_MASK_E; //send it up, dir +y
                   if (x_source == 0) //if on the axis, send it right too
                       dir |= DIRECTION_MASK_N;
                }
                //B: ]-x +y]
                else if (x_source < 0 && y_source >= 0){
                   dir |= DIRECTION_MASK_N;
                   if (y_source == 0) 
                       dir |= DIRECTION_MASK_W;
                }
                //C: [-x -y[
                else if (x_source <= 0 && y_source < 0){
                   dir |= DIRECTION_MASK_W;
                   if (x_source == 0) 
                       dir |= DIRECTION_MASK_S;
                }
                //D: ]+x -y]
                else if (x_source > 0 && y_source <= 0){
                   dir |= DIRECTION_MASK_S;
                   if (y_source == 0) 
                       dir |= DIRECTION_MASK_E;
                }
                else if (x_source == 0 && y_source == 0) //generated by myself.
                                                         //send it to all neighbors
                    dir = DIRECTION_MASK_ALL_EXCEPT_LOCAL;
                
                break;
        }
        return dir;
    }
}

