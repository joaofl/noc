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


using namespace std;
namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("NOCRouter");

    TypeId
    NOCRouter::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::NOCRouter")
                .SetParent<Application> ()
                .AddConstructor<NOCRouter> ()
                .AddTraceSource("SwitchRxTrace", "The packets received by the router of each node", MakeTraceSourceAccessor(&NOCRouter::m_routerRxTrace))
                .AddTraceSource("SwitchTxTrace", "The packets sent by the router of each node", MakeTraceSourceAccessor(&NOCRouter::m_routerTxTrace))
        
                .AddAttribute("ChannelCount",
                "Defines the number of NOCNetDevices installed at each direction",
                UintegerValue(true),
                MakeUintegerAccessor(&NOCRouter::m_channelCount),
                MakeUintegerChecker<uint8_t>())
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
   
    void NOCRouter::PacketUnicast (Ptr<const Packet> pck, uint8_t network_id, int32_t destination_x, int32_t destination_y){
        Ptr<Packet> pck_cp = pck->Copy();
//        NOCHeader hd;
//        pck_cp->RemoveHeader(hd);
        
        //modifications on the header here.
        
        
        uint8_t output_port_mask = RouteTo(destination_x, destination_y);
       
//        pck_cp->AddHeader(hd);
        this->PacketSend(pck_cp, network_id, output_port_mask, 0);
    }

    void NOCRouter::PacketMulticast (Ptr<const Packet> pck, uint8_t network_id, uint8_t hops){
        
    }

    void NOCRouter::PacketBroadcast (Ptr<const Packet> pck, uint8_t network_id){
        
    }
    
    void
    NOCRouter::PacketSend(Ptr<const Packet> pck, uint8_t network_id, uint8_t ports_mask, uint8_t priority){//, uint8_t optional network_id) {
        //TODO: This function should get the pck, the destination address, priority comes in the packet?, and network it should write to.
        // the rout should be calculated by the router itself. and the routing algorithms should be here (or in separate files).
        // the addressing scheme sould allow: Broadcast (with limited radius (hops)) and unicast (to an specific X,Y location)


        Ptr<NOCNetDevice> nd;
        uint8_t i = network_id;
//            //TODO: the arbitration policy should be considered here. Round robin for example
        if ( (ports_mask >> DIRECTION_E) & 1){
            nd = this->GetNetDevice(i, DIRECTION_E);
            if (nd != NULL) nd->Send(pck->Copy());
        }
        
        m_routerTxTrace(pck);
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

    bool
    NOCRouter::PacketReceived(Ptr<NetDevice> device, Ptr<const Packet> pck_rcv, uint16_t protocol, const Address& sourceAddress) {

        //TODO: It should check which port it requires to continue its trajectory. 
        //if not available, it stops, and block any port which migh require the same port.
        //this blocking should propagate till the sender.

        m_routerRxTrace(pck_rcv->Copy()); //trance the packet after changing 
        m_receiveCallBack(pck_rcv->Copy());

        return true;
    }
    
    void NOCRouter::SetReceiveCallback(ReceiveCallback cb){
        m_receiveCallBack = cb;
    }
    
    //Using XY routing
    uint8_t NOCRouter::RouteTo(int32_t x, int32_t y) { //X-Y routing, with X first
        
        uint8_t dir = 0b00000000;

        //with this algorithm, the nodes will first send the pck in order to make
        // the delta x = 0, then, start moving along the y. 
        //TODO: implement the clockwise or counter cw routing algorithms

        //        if (n.x < 0) dir |= 0b00000001;
        //        else if (n.x > 0) dir |= 0b00000100;
        //
        //        else if (n.y < 0) dir |= 0b00000010;
        //        else if (n.y > 0) dir |= 0b00001000;


        //clockwise routing

        //find out which quadrant it is
        if (x < 0 && y < 0) {
            dir |= 0b00000001;
            return dir;
        } //send right first
        if (x < 0 && y > 0) {
            dir |= 0b00001000;
            return dir;
        } //send up first
        if (x > 0 && y < 0) {
            dir |= 0b00000010;
            return dir;
        } //send down first
        if (x > 0 && y > 0) {
            dir |= 0b00000100;
            return dir;
        } //send left first

        //from now on, it is align to the sink in one of the 4 dir
        if (x > 0) {
            dir |= 0b00000100;
            return dir;
        } //send left then
        if (x < 0) {
            dir |= 0b00000001;
            return dir;
        } //send right then
        if (y > 0) {
            dir |= 0b00001000;
            return dir;
        } //send up then
        if (y < 0) {
            dir |= 0b00000010;
            return dir;
        } //send down then

        return dir;
    }
}

