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

#include "epiphany-header.h"
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
        m_running = true;
        
        NetDeviceContainer::Iterator nd;
        for (nd = m_netDevices.Begin() ; nd != m_netDevices.End() ; nd++){    
            (*nd)->GetObject<NOCNetDevice>()->SetReceiveCallback(MakeCallback(&NOCRouter::PacketReceived, this));
            (*nd)->GetObject<NOCNetDevice>()->SetRemoteSignalChangedCallback(MakeCallback(&NOCRouter::RemoteWaitChanged, this));
            (*nd)->GetObject<NOCNetDevice>()->SetLocalSignalChangedCallback(MakeCallback(&NOCRouter::LocalWaitChanged, this));
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
        nd_info.pck_buffered = false;
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
    
    
    
    void 
    NOCRouter::ServePackets(void) {

        //TODO: It should check which port it requires to continue its trajectory. 
        //if not available, it stops, and block any port which might require the same port.
        //this blocking should propagate till the sender.
        
        //TODO: dont know how to not specify a packet type.. It better be compatible
        // with any packet with attribute "DestinationAddressX"
        //          int32_t x = h->GetAttribute("DestinationAddressX");
        //          int32_t y = h->GetAttribute("DestinationAddressY");
        
        
//        static 
        
//        bool packet_served = false;
        uint8_t input_network = 0;
        
        int8_t i = GetNetDeviceInfoIndex(input_network, m_port_to_serve);
        
        uint8_t n_times = 0;
        
        while (m_netDeviceInfoArray[i].pck_buffered == false && n_times < 4 ){
            m_port_to_serve++;
            if (m_port_to_serve > DIRECTION_N){
                m_port_to_serve = DIRECTION_E;
            }
            i = GetNetDeviceInfoIndex(input_network, m_port_to_serve);
            n_times++;
        }
        if (n_times == 4) 
            return;
        
        if (i >= 0)
        if (m_netDeviceInfoArray[i].pck_buffered == true){
            Ptr<Packet> pck = m_netDeviceInfoArray[i].pck_buffer;
            EpiphanyHeader h;
            pck->PeekHeader(h);
            
            int32_t x = h.GetDestinationAddressX();
            int32_t y = h.GetDestinationAddressY();
        
            uint8_t output_port = this->RouteTo(COLUMN_FIRST, x, y);
            int8_t o = this->GetNetDeviceInfoIndex(input_network, output_port);
            
            if (output_port == DIRECTION_L){ 
                //Reached its destination, send it to upper layers
                m_receiveCallBack(pck->Copy(), m_port_to_serve);
                //Packet was consumed, no need to wait anymore
                m_netDeviceInfoArray[i].nd_pointer->SetLocalWait(false);
                m_netDeviceInfoArray[i].pck_buffered = false;
//                packet_served = true;
            }
            else if (o >= 0){
                if (m_netDeviceInfoArray[o].wait_remote == false){
                    this->ServePacket(i,o);
//                    packet_served = true;
                }
            }            
        }

        
        m_port_to_serve++;
        if (m_port_to_serve > DIRECTION_N){
            m_port_to_serve = DIRECTION_E;
        }
            
//        }
        //delay one cycle here, and re-execute it if there is still packets to be sent
        //if no packet was sent, serve another port without delay (best effort)
//        ServePorts();
//        if (packet_served)
//            packet_served = packet_served;
//        cout << Simulator::Now() << "Packets served" << endl;
            Simulator::Schedule(PicoSeconds(1000), &NOCRouter::ServePackets, this);
//        else
//            Simulator::ScheduleNow(&NOCRouter::ServePackets, this);
        
    }
    
    void 
    NOCRouter::ServePacket(uint8_t in, uint8_t out) {
        this->PacketSendSingle(m_netDeviceInfoArray[in].pck_buffer->Copy(),
                m_netDeviceInfoArray[out].nd_pointer, P0);

        m_netDeviceInfoArray[in].nd_pointer->SetLocalWait(false);
        m_netDeviceInfoArray[in].pck_buffered = false;
    }

    
    
    bool NOCRouter::PacketUnicast (Ptr<const Packet> pck, uint8_t network_id, 
            int32_t destination_x, int32_t destination_y){
        
        uint8_t output_port = RouteTo(COLUMN_FIRST, destination_x, destination_y);
//        return this->PacketSendSingle(pck->Copy(), GetNetDevice(network_id, output_port), 0);
        
        
//        Ptr<NOCNetDevice> nd = device->GetObject<NOCNetDevice>();
        
        uint8_t i = this->GetNetDeviceInfoIndex(network_id, output_port);
        if (m_netDeviceInfoArray[i].pck_buffered == false){
            m_netDeviceInfoArray[i].pck_buffer = pck->Copy();
            m_netDeviceInfoArray[i].pck_buffered = true;
            Simulator::Schedule(PicoSeconds(1000), &NOCRouter::ServePackets, this);
        }
        else
            cout << "Packet dropped at "
                 << m_addressX << "," << m_addressY <<" Buffer was full" << endl;
        
        return false;
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

        if ( (ports_mask >> DIRECTION_N) & 1){
            PacketSendSingle(pck->Copy(), GetNetDevice(network_id, DIRECTION_N), priority);
        }
        if ( (ports_mask >> DIRECTION_S) & 1){
            PacketSendSingle(pck->Copy(), GetNetDevice(network_id, DIRECTION_S), priority);
        }
        if ( (ports_mask >> DIRECTION_E) & 1){
            PacketSendSingle(pck->Copy(), GetNetDevice(network_id, DIRECTION_E), priority);
        }
        if ( (ports_mask >> DIRECTION_W) & 1){
            PacketSendSingle(pck->Copy(), GetNetDevice(network_id, DIRECTION_W), priority);
        }
        if ( (ports_mask >> DIRECTION_L) & 1){
            m_receiveCallBack(pck->Copy(), DIRECTION_L); //Loopback
        }
        return false;
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
       
    
    
    bool
    NOCRouter::PacketReceived(Ptr<NetDevice> device, Ptr<const Packet> pck, 
                        uint16_t direction, const Address& sourceAddress) {

        m_routerRxTrace(pck->Copy());
        
        Ptr<NOCNetDevice> nd = device->GetObject<NOCNetDevice>();
        
        uint8_t i = this->GetNetDeviceInfoIndex(nd);
        if (m_netDeviceInfoArray[i].pck_buffered == false){
            m_netDeviceInfoArray[i].pck_buffer = pck->Copy();
            m_netDeviceInfoArray[i].pck_buffered = true;
            Simulator::Schedule(PicoSeconds(1000), &NOCRouter::ServePackets, this);
        }
        else
            cout << "Packet dropped at "
                 << m_addressX << "," << m_addressY <<" Buffer was full" << endl;



        
        return true;
    }
    
    void
    NOCRouter::RemoteWaitChanged(uint8_t signal, Ptr<NOCNetDevice> nd_this, bool wait_state){
        
        m_netDeviceInfoArray.at(nd_this->GetIfIndex()).wait_remote = wait_state;
        cout << Simulator::Now() << " RWC " 
                << m_addressX << "," << m_addressY
                << " ND: " << m_netDeviceInfoArray.at(nd_this->GetIfIndex()).direction
                << " s: " << wait_state << endl;
    }
    void
    NOCRouter::LocalWaitChanged(uint8_t signal, Ptr<NOCNetDevice> nd_this, bool wait_state){
        /* Here, check if wait was set low. In this case, check if there are buffered
         * packets that want to use that port. Serve all the sources using round
         * robin police.
         */
        m_netDeviceInfoArray.at(nd_this->GetIfIndex()).wait = wait_state;
        
//        ServePorts();
        
        cout << Simulator::Now() << " LWC " << m_addressX << "," << m_addressY << " s: " << wait_state << endl;
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

