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

#include "noc-net-device.h"
#include "noc-router.h"
#include "src/core/model/object.h"
#include "ns3/noc-types.h"
#include "ns3/noc-router.h"
#include "src/network/model/node.h"
#include "ns3/epiphany-header.h"


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

    //    void
    //    NOCRouter::Setup(bool IsSink) {
    //        IsSink = IsSink;
    //
    //    }

//    void 
//    NOCRouter::SetAttribute (std::string name, const uint8_t value){
//        if (name.compare("ChannelCount") == 0) //Is the same
//        {
//            m_channelCount = value;
//        }
//    }
    
    void
    NOCRouter::StartApplication(void) {

//        uint8_t n = m_netDevices.GetN();
//        uint8_t m = m_netDeviceInfoArray.size();
//        n=n;
//        m=m;
        
        m_running = true;
        
        NetDeviceContainer::Iterator nd;
        for (nd = m_netDevices.Begin() ; nd != m_netDevices.End() ; nd++){    
            (*nd)->GetObject<NOCNetDevice>()->SetReceiveCallback(MakeCallback(&NOCRouter::PacketReceived, this));
            (*nd)->GetObject<NOCNetDevice>()->AddRemoteTransmitStartedCallback(MakeCallback(&NOCRouter::RemoteTransmissionStarted, this));
        }

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
//        nd_info.x = x;
//        nd_info.y = y;
        nd_info.network_id = network;
        nd_info.direction = direction;
        nd_info.nd_pointer = nd;
        nd_info.wait = false;
        m_netDeviceInfoArray.push_back(nd_info);
        
        nd->SetIfIndex(m_netDeviceInfoArray.size() - 1);        
        m_netDevices.Add(nd);
    }
    
    Ptr<NOCNetDevice>
    NOCRouter::GetNetDevice(uint8_t network, uint8_t direction){
        for (uint8_t i = 0 ; i < m_netDeviceInfoArray.size() ; i++){
            NetDeviceInfo nd_info = m_netDeviceInfoArray[i];
            if (nd_info.direction == direction && nd_info.network_id == network){
                return nd_info.nd_pointer;
            }
        }
        return NULL; //if the specified node was not found
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
    
    bool NOCRouter::PacketUnicast (Ptr<const Packet> pck, uint8_t network_id, int32_t destination_x, int32_t destination_y){
        uint8_t output_port = RouteTo(COLUMN_FIRST, destination_x, destination_y);
        return this->PacketSendSingle(pck->Copy(), network_id, output_port, 0);
    }

    bool NOCRouter::PacketMulticast (Ptr<const Packet> pck, uint8_t network_id, uint8_t hops){
        return false;
    }

    bool NOCRouter::PacketBroadcast (Ptr<const Packet> pck, uint8_t network_id){
        return false;
    }
    
    bool
    NOCRouter::PacketSendMultiple(Ptr<const Packet> pck, uint8_t network_id, uint8_t ports_mask, uint8_t priority){//, uint8_t optional network_id) {
        //TODO: This function should get the pck, the destination address, priority comes in the packet?, and network it should write to.
        // the rout should be calculated by the router itself. and the routing algorithms should be here (or in separate files).
        // the addressing scheme sould allow: Broadcast (with limited radius (hops)) and unicast (to an specific X,Y location)

        //TODO:
//        uint8_t out_ports_count = CountOnes(ports_mask);

        Ptr<NOCNetDevice> nd;
//            //TODO: the arbitration policy should be considered here. Round robin for example
        if ( (ports_mask >> DIRECTION_N) & 1){
            PacketSendSingle(pck->Copy(), network_id, DIRECTION_N, priority);
        }
        if ( (ports_mask >> DIRECTION_S) & 1){
            PacketSendSingle(pck->Copy(), network_id, DIRECTION_S, priority);
        }
        if ( (ports_mask >> DIRECTION_E) & 1){
            PacketSendSingle(pck->Copy(), network_id, DIRECTION_E, priority);
        }
        if ( (ports_mask >> DIRECTION_W) & 1){
            PacketSendSingle(pck->Copy(), network_id, DIRECTION_W, priority);
        }
        if ( (ports_mask >> DIRECTION_L) & 1){
            m_receiveCallBack(pck->Copy(), DIRECTION_L); //Loopback
        }
        return false;
    }
    
    bool
    NOCRouter::PacketSendSingle(Ptr<const Packet> pck, uint8_t network_id, uint8_t port, uint8_t priority){
        Ptr<NOCNetDevice> nd;
//            //TODO: the arbitration policy should be considered here. Round robin for example
        
            nd = this->GetNetDevice(network_id, port);
            if (nd != NULL){   
                
                if (nd->Send(pck->Copy())){
                    m_routerTxTrace(pck);
                    return true;
                }
                else{
                    m_routerTxDropTrace(pck);
                    return false;
                }
            }
            return false;
    }
       
    
    
    bool
    NOCRouter::PacketReceived(Ptr<NetDevice> device, Ptr<const Packet> pck_rcv, 
            uint16_t direction, const Address& sourceAddress) {

        //TODO: It should check which port it requires to continue its trajectory. 
        //if not available, it stops, and block any port which might require the same port.
        //this blocking should propagate till the sender.
        
        //TODO: dont know how to not specify a packet type.. It better be compatible
        // with any packet with attribute "DestinationAddressX"
        m_routerRxTrace(pck_rcv->Copy());
        EpiphanyHeader h;
        pck_rcv->PeekHeader(h);
        
        Ptr<NOCNetDevice> nd = device->GetObject<NOCNetDevice>();
        uint8_t input_port = m_netDeviceInfoArray[nd->GetIfIndex()].direction;

//        int32_t x = h->GetAttribute("DestinationAddressX");
//        int32_t y = h->GetAttribute("DestinationAddressY");
  
        int32_t x = h.GetDestinationAddressX();
        int32_t y = h.GetDestinationAddressY();
        
        uint8_t output_port = RouteTo(COLUMN_FIRST, x, y);
        
        if (output_port == DIRECTION_L){ //Reached its destination
            m_receiveCallBack(pck_rcv->Copy(), input_port);
        }
        else{ //IF UNICAST   
            this->PacketSendSingle(pck_rcv->Copy(), 0, output_port, 0);
        }
        
        return true;
    }
    
    void 
    NOCRouter::RemoteTransmissionStarted(Ptr<NOCNetDevice> nd_this, Ptr<NOCNetDevice> nd_src, uint8_t direction){
        SetLocalWait(nd_this, true);
        std::cout  << Simulator::Now().GetPicoSeconds() << "Someone started transmitting to me: " << m_addressX << "," << m_addressY << endl;
    }
    void
    NOCRouter::RemoteWaitChanged(uint8_t network, uint8_t direction, bool wait_state){
        /* Here, check if wait was set low. In this case, check if there are buffered
         * packets that want to use that port. Serve all the sources using round
         * robin police.
         */
    }
    
    bool
    NOCRouter::GetRemoteWait(uint8_t network, uint8_t direction){
        
        return true;
    }
    bool
    NOCRouter::GetLocalWait(uint8_t network, uint8_t direction){
        return m_netDeviceInfoArray.at(nd->GetIfIndex()).wait;
    }
    void
    NOCRouter::SetLocalWait(Ptr<NOCNetDevice> nd, bool state){
        m_netDeviceInfoArray.at(nd->GetIfIndex()).wait = state;
        nd->SetWait(state);
    }
    
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

