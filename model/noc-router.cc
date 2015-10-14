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
            int32_t destination_x, int32_t destination_y)
    {
        
        uint8_t out = RouteTo(ROUTING_COLUMN_FIRST,0,0, destination_x, destination_y);
        
//        TODO: here the information with the origin and destination should be added
//        to a second header, which is added to the packet, in a multilayer style.
        
        return this->PacketSendSingle(pck->Copy(), network_id, out, P0);
        
    }

    bool NOCRouter::PacketMulticast (Ptr<const Packet> pck, uint8_t network_id){
        return false;
    }

    bool NOCRouter::PacketBroadcast (Ptr<const Packet> pck, uint8_t network_id){
        
        NOCHeader h;
        
        h.SetProtocol(NOCHeader::PROTOCOL_BROADCAST);
        h.SetSourceAddressXY(0,0);
        h.SetDestinationAddressXY(0,0);
        
//        if (m_useRelativeAddress)
//        {
//            h.SetSourceAddressXY(0,0);
//        }
        
        Ptr<Packet> pck_c = pck->Copy();
        pck_c->AddHeader(h);
        
//        pck->Copy()->AddHeader(h);
        PacketSendMultiple(pck_c, network_id, DIRECTION_MASK_ALL_EXCEPT_LOCAL, P0);
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
            NOCHeader h;
            pck_c->RemoveHeader(h);
            h.AddtoSourceAddress(-1,0);
            pck_c->AddHeader(h);
            
            if (PacketSendSingle(pck_c, network_id, DIRECTION_N, priority)){
                sent++;
            }
        }
        if ( (ports_mask >> DIRECTION_S) & 1){
            Ptr<Packet> pck_c = pck->Copy();
            NOCHeader h;
            pck_c->RemoveHeader(h);
            h.AddtoSourceAddress(1,0);
            pck_c->AddHeader(h);
            
            if (PacketSendSingle(pck_c, network_id, DIRECTION_S, priority)){
                sent++;
            }
        }
        if ( (ports_mask >> DIRECTION_E & 1) & 1){
            Ptr<Packet> pck_c = pck->Copy();
            NOCHeader h;
            pck_c->RemoveHeader(h);
            h.AddtoSourceAddress(0,1);
            pck_c->AddHeader(h);
            
            if (PacketSendSingle(pck_c, network_id, DIRECTION_E, priority)){
                sent++;
            }
        }
        if ( (ports_mask >> DIRECTION_W) & 1){
            Ptr<Packet> pck_c = pck->Copy();
            NOCHeader h;
            pck_c->RemoveHeader(h);
            h.AddtoSourceAddress(0,-1);
            pck_c->AddHeader(h);
            
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
        
        if (port_direction != DIRECTION_L)
        {
            nd = GetNetDevice(network_id, port_direction);            
        }
        else
        {  //Packet reached its destination, send it to the upper layers
            m_receiveCallBack(pck->Copy(), DIRECTION_L);
        }
        
        if (nd == NULL) //That node does not have a net device in that direction
            return false;
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
        m_routerRxTrace(pck);
        
        Ptr<Packet> pck_c = pck->Copy();

        NOCHeader h;
        pck_c->PeekHeader(h);
        
        int32_t adx = h.GetDestinationAddressX();
        int32_t ady = h.GetDestinationAddressY();
        int32_t asx = h.GetSourceAddressX();
        int32_t asy = h.GetSourceAddressY();
        uint8_t p = h.GetProtocol();
        
        uint8_t out = 0;
        switch (p){
            case NOCHeader::PROTOCOL_BROADCAST:
                out = RouteTo(ROUTING_BROADCAST,asx,asy,adx,ady);
                PacketSendMultiple(pck_c->Copy(), 0, out, P0);
                break;
               
            case NOCHeader::PROTOCOL_MULTICAST:
               
                break;
               
            case NOCHeader::PROTOCOL_UNICAST:
               
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
                if      (m_addressY < y_dest)    dir = DIRECTION_MASK_S;
                else if (m_addressY > y_dest)    dir = DIRECTION_MASK_N;

                else if (m_addressX < x_dest)    dir = DIRECTION_MASK_E;
                else if (m_addressX > x_dest)    dir = DIRECTION_MASK_W;

                else if (m_addressX == x_dest && m_addressY == y_dest) 
                                            dir = DIRECTION_MASK_L;
                break;
            
            case ROUTING_ROW_FIRST:
                if      (m_addressX < x_dest)    dir = DIRECTION_MASK_E;
                else if (m_addressX > x_dest)    dir = DIRECTION_MASK_W;
                
                else if (m_addressY < y_dest)    dir = DIRECTION_MASK_S;
                else if (m_addressY > y_dest)    dir = DIRECTION_MASK_N;

                else if (m_addressX == x_dest && m_addressY == y_dest) 
                                                dir = DIRECTION_MASK_L;
                break;                
            case ROUTING_CLOCKWISE:
                break;
                
            case ROUTING_BROADCAST:
                if ((x_source == m_addressX) && (y_source == m_addressY)) //myself generating the packet
                    dir = DIRECTION_MASK_ALL_EXCEPT_LOCAL;
                
                else if((x_source == 0) && (y_source < 0)) //Going up, aligned with the sink
                    dir = DIRECTION_MASK_N | DIRECTION_MASK_W; //send it inside, north and west
                else if((x_source > 0) && (y_source < 0)) //Turned left, keep straight
                    dir = DIRECTION_MASK_W; //send it inside and west
                
//                else if((x_source == 0) && (y_source > 0)) //Going up, aligned with the sink
//                    dir = DIRECTION_MASK_S | DIRECTION_MASK_E | DIRECTION_MASK_L; //send it inside, north and west
//                else if((x_source > 0) && (y_source > 0)) //Turned left, keep straight
//                    dir = DIRECTION_MASK_E | DIRECTION_MASK_L; //send it inside and west
//                
//                else if((x_source > 0) && (y_source == 0)) //Going up, aligned with the sink
//                    dir = DIRECTION_MASK_W | DIRECTION_MASK_S | DIRECTION_MASK_L; //send it inside, north and west
//                else if((x_source > 0) && (y_source < 0)) //Turned left, keep straight
//                    dir = DIRECTION_MASK_S | DIRECTION_MASK_L; //send it inside and west
//                
//                else if((x_source < 0) && (y_source == 0)) //Going up, aligned with the sink
//                    dir = DIRECTION_MASK_E | DIRECTION_MASK_N | DIRECTION_MASK_L; //send it inside, north and west
//                else if((x_source < 0) && (y_source > 0)) //Turned left, keep straight
//                    dir = DIRECTION_MASK_N | DIRECTION_MASK_L; //send it inside and west
                
                
                

                dir |= DIRECTION_MASK_L;
                     
                break;
        }
        return dir;
    }
}

