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
            (*nd)->SetReceiveCallback(MakeCallback(&NOCRouter::PacketReceived, this));
        }

    }

    void
    NOCRouter::StopApplication(void) {
        m_running = false;

        if (m_sendEvent.IsRunning()) {
            Simulator::Cancel(m_sendEvent);
        }
    }
   
    bool NOCRouter::PacketUnicast (Ptr<const Packet> pck, uint8_t network_id, int32_t destination_x, int32_t destination_y){
        uint8_t output_port = RouteTo(destination_x, destination_y);
        return this->PacketSendSingleDir(pck->Copy(), network_id, output_port, 0);
    }

    bool NOCRouter::PacketMulticast (Ptr<const Packet> pck, uint8_t network_id, uint8_t hops){
        return false;
    }

    bool NOCRouter::PacketBroadcast (Ptr<const Packet> pck, uint8_t network_id){
        return false;
    }
    
    bool
    NOCRouter::PacketSend(Ptr<const Packet> pck, uint8_t network_id, uint8_t ports_mask, uint8_t priority){//, uint8_t optional network_id) {
        //TODO: This function should get the pck, the destination address, priority comes in the packet?, and network it should write to.
        // the rout should be calculated by the router itself. and the routing algorithms should be here (or in separate files).
        // the addressing scheme sould allow: Broadcast (with limited radius (hops)) and unicast (to an specific X,Y location)

        //TODO:
//        uint8_t out_ports_count = CountOnes(ports_mask);

        Ptr<NOCNetDevice> nd;
//            //TODO: the arbitration policy should be considered here. Round robin for example
        if ( (ports_mask >> DIRECTION_N) & 1){
            PacketSendSingleDir(pck->Copy(), network_id, DIRECTION_N, priority);
        }
        if ( (ports_mask >> DIRECTION_S) & 1){
            PacketSendSingleDir(pck->Copy(), network_id, DIRECTION_S, priority);
        }
        if ( (ports_mask >> DIRECTION_E) & 1){
            PacketSendSingleDir(pck->Copy(), network_id, DIRECTION_E, priority);
        }
        if ( (ports_mask >> DIRECTION_W) & 1){
            PacketSendSingleDir(pck->Copy(), network_id, DIRECTION_W, priority);
        }
        return false;
    }
    
        bool
    NOCRouter::PacketSendSingleDir (Ptr<const Packet> pck, uint8_t network_id, uint8_t port, uint8_t priority){
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
    NOCRouter::PacketForward(Ptr<const Packet> pck, 
            uint8_t network_id, uint8_t originPort, uint8_t destinationPort, uint8_t priority){
        //TODO: This function should get the pck, the destination address, priority comes in the packet?, and network it should write to.
        // the rout should be calculated by the router itself. and the routing algorithms should be here (or in separate files).
        // the addressing scheme sould allow: Broadcast (with limited radius (hops)) and unicast (to an specific X,Y location)
        
        if (GetNetDevice(0, destinationPort)->GetRemoteWait() == false){
           PacketSendSingleDir(pck->Copy(), network_id, destinationPort, priority);
           GetNetDevice(0, originPort)->SetLocalWait(false);
           return true;
        }
        m_routerTxDropTrace(pck);
        return false;
    }
    

    
    bool
    NOCRouter::PacketReceived(Ptr<NetDevice> device, Ptr<const Packet> pck_rcv, uint16_t direction, const Address& sourceAddress) {

        //TODO: It should check which port it requires to continue its trajectory. 
        //if not available, it stops, and block any port which might require the same port.
        //this blocking should propagate till the sender.
        
        //TODO: dont know how to not specify a packet type.. It better be compatible
        // with any packet with attribute "DestinationAddressX"
        m_routerRxTrace(pck_rcv->Copy());
        EpiphanyHeader h;
        pck_rcv->PeekHeader(h);
        
        Ptr<NOCNetDevice> nd = device->GetObject<NOCNetDevice>();
        
        
        
//        int32_t x = h->GetAttribute("DestinationAddressX");
//        int32_t y = h->GetAttribute("DestinationAddressY");
  
        int32_t x = h.GetDestinationAddressX();
        int32_t y = h.GetDestinationAddressY();
        
        if (x == m_addressX && y == m_addressY){ //Reached its destination
            m_receiveCallBack(pck_rcv->Copy(), direction);
        }
        else{ //IF UNICAST
            uint8_t input_port = m_netDeviceInfoArray[nd->GetIfIndex()].direction;
            uint8_t output_port = RouteTo(x, y);
            this->PacketForward(pck_rcv->Copy(), 0, input_port, output_port, 0);    
//            this->PacketSend(pck_rcv->Copy(), 0, output_port_mask, 0);
        }
        
        return true;
    }
    
    void NOCRouter::SetReceiveCallback(ReceiveCallback cb){
        m_receiveCallBack = cb;
    }
    
    void
    NOCRouter::AddNetDevice(Ptr<NOCNetDevice> nd, uint8_t cluster, uint32_t x, uint32_t y, uint32_t network, uint8_t direction){
        NetDeviceInfo info;
        
        m_netDevices.Add(nd);
        
        info.cluster_id = cluster;
        info.x = x;
        info.y = y;
        info.network_id = network;
        info.direction = direction;
        info.container_index = m_netDevices.GetN() - 1; //get the index in which it is stored at the net device container
       
        
        m_netDeviceInfoArray.push_back(info);
        
        nd->SetIfIndex(m_netDeviceInfoArray.size() - 1);
    }
    
    Ptr<NOCNetDevice>
    NOCRouter::GetNetDevice(uint8_t network, uint8_t direction){
        for (uint8_t i = 0 ; i < m_netDeviceInfoArray.size() ; i++){
            NetDeviceInfo nd_info = m_netDeviceInfoArray[i];
            if (nd_info.direction == direction && nd_info.network_id == network){
                Ptr<NOCNetDevice> nd = m_netDevices.Get(nd_info.container_index)->GetObject<NOCNetDevice>();
                return nd;
            }
        }
        return NULL; //if the specified node was not found
    }
    
//    NetDeviceInfo
    uint8_t
    NOCRouter::GetNetDeviceInfo(Ptr<NOCNetDevice> nd){
//        NetDeviceInfo nd_info;
        for (uint8_t i = 0 ; i < m_netDeviceInfoArray.size() ; i++){
            if (nd->GetAddress() == m_netDevices.Get(i)->GetAddress())
            {
//                return m_netDeviceInfoArray.at(nd->GetIfIndex()); //From the index initially set
            }
        }
        return 0; //if the specified node was not found
    }
    
    uint8_t
    NOCRouter::GetNDevices(void){
        return m_netDevices.GetN();
    }

    
    //Using XY routing
    uint8_t NOCRouter::RouteTo(int32_t x, int32_t y) { //X-Y routing, with X first
        
        uint8_t dir;

        //with this algorithm, the nodes will first send the pck in order to make
        // the delta x = 0, then, start moving along the y. 
        //TODO: implement the clockwise or counter cw routing algorithms
        
        if      (m_addressY < y)    dir = NOCRouter::DIRECTION_S;
        else if (m_addressY > y)    dir = NOCRouter::DIRECTION_N;

        else if (m_addressX < x)    dir = NOCRouter::DIRECTION_E;
        else if (m_addressX > x)    dir = NOCRouter::DIRECTION_W;



        return dir;

        //clockwise routing

        //find out which quadrant it is
//        if (x < 0 && y < 0) {
//            dir |= NOCRouter::DIRECTION_MASK_E;
//            return dir;
//        } //send right first
//        if (x < 0 && y > 0) {
//            dir |= NOCRouter::DIRECTION_MASK_N;
//            return dir;
//        } //send up first
//        if (x > 0 && y < 0) {
//            dir |= NOCRouter::DIRECTION_MASK_S;
//            return dir;
//        } //send down first
//        if (x > 0 && y > 0) {
//            dir |= NOCRouter::DIRECTION_MASK_W;
//            return dir;
//        } //send left first
//
//        //from now on, it is align to the sink in one of the 4 dir
//        if (x > 0) {
//            dir |= NOCRouter::DIRECTION_MASK_W;
//            return dir;
//        } //send left then
//        if (x < 0) {
//            dir |= NOCRouter::DIRECTION_MASK_E;
//            return dir;
//        } //send right then
//        if (y > 0) {
//            dir |= NOCRouter::DIRECTION_MASK_N;
//            return dir;
//        } //send up then
//        if (y < 0) {
//            dir |= NOCRouter::DIRECTION_MASK_E;
//            return dir;
//        } //send down then
//
//        return dir;
    }
}

