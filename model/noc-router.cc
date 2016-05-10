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


#include "noc-router.h"
#include "ns3/noc-routing-protocols.h"
#include "ns3/noc-net-device.h"


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
        
                .AddTraceSource("RouterCxTrace", 
                "The packets consumed by the router of each node",
                MakeTraceSourceAccessor(&NOCRouter::m_routerCxTrace),
                "ns3::NOCRouter::SwitchCxTrace")
        
                .AddTraceSource("RouterGxTrace", 
                "The packets generated by the router of each node",
                MakeTraceSourceAccessor(&NOCRouter::m_routerGxTrace),
                "ns3::NOCRouter::SwitchGxTrace")
        
                .AddTraceSource("RouterTxDropTrace", 
                "The packets sent by the router of each node",
                MakeTraceSourceAccessor(&NOCRouter::m_routerTxDropTrace),
                "ns3::NOCRouter::RouterTxDropTrace")
        
                .AddAttribute("ChannelCount",
                "Defines the number of NOCNetDevices installed at each direction",
                UintegerValue(1),
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
        m_server_state = IDLE;
        
        NetDeviceContainer::Iterator nd;
        for (nd = m_netDevices.Begin() ; nd != m_netDevices.End() ; nd++){    
            (*nd)->GetObject<NOCNetDevice>()->SetReceiveCallback(MakeCallback(&NOCRouter::PacketReceived, this));
//            (*nd)->GetObject<NOCNetDevice>()->SetRemoteSignalChangedCallback(MakeCallback(&NOCRouter::RemoteWaitChanged, this));
//            (*nd)->GetObject<NOCNetDevice>()->SetLocalSignalChangedCallback(MakeCallback(&NOCRouter::LocalWaitChanged, this));
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
        nd_info.network_id = network;
        nd_info.direction = direction;
        nd_info.nd_pointer = nd;
//        nd_info.wait = false;
//        nd_info.wait_remote = false;
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

    void 
    NOCRouter::SetRoutingProtocolUnicast(NOCRoutingProtocols::RoutingProtocols rp) {
        unicast_routing_protocol = rp;
    }

    
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    bool 
    NOCRouter::PacketUnicast (Ptr<const Packet> pck, uint8_t network_id, 
            int32_t destination_x, int32_t destination_y, bool absolute_address)
    {
        
        
        if (absolute_address){
                destination_x = destination_x - m_addressX;
                destination_y = destination_y - m_addressY;
        }
        
        if (destination_x == 0 && destination_y == 0)
            return 0; // dont send it to myself
        
        NOCHeader h;
        h.SetProtocol(NOCHeader::PROTOCOL_UNICAST);
        h.SetSourceAddressXY(0,0);
        h.SetDestinationAddressXY(destination_x,destination_y);
        
        Ptr<Packet> pck_c = pck->Copy();
        pck_c->AddHeader(h);
        
        m_routerGxTrace(pck_c, 0); //the router receives a pck from the application
        
        uint8_t out = NOCRoutingProtocols::Unicast(destination_x, destination_y, unicast_routing_protocol);
        
        
        return Transmit(pck_c, network_id, out, P0);
    }
    
    bool 
    NOCRouter::PacketUnicastOffset (Ptr<const Packet> pck, uint8_t network_id, 
            int32_t destination_x, int32_t destination_y)
    {
           
        NOCHeader h;
        h.SetProtocol(NOCHeader::PROTOCOL_UNICAST_OFFSET);
        h.SetSourceAddressXY(0,0);
        h.SetDestinationAddressXY(destination_x, destination_y);
        
        Ptr<Packet> pck_c = pck->Copy();
        pck_c->AddHeader(h);
        
        m_routerGxTrace(pck_c, 0); //the router receives a pck from the application
        
        uint8_t out = NOCRoutingProtocols::UnicastClockwiseOffsetXY(destination_x, destination_y, 0, 0);
        
        return Transmit(pck_c, network_id, out, P0);
    }

    bool NOCRouter::PacketMulticastRadius (Ptr<const Packet> pck, uint8_t network_id, uint8_t n_hops){
        NOCHeader h;
        h.SetProtocol(NOCHeader::PROTOCOL_MULTICAST_RADIUS);
        h.SetSourceAddressXY(0,0);
        h.SetDestinationAddressXY(n_hops,0);
        
        Ptr<Packet> pck_c = pck->Copy();
        pck_c->AddHeader(h);
        
        uint8_t out = NOCRoutingProtocols::MulticastRadius(0,0,n_hops);
        
        if (Transmit(pck_c, network_id, out, P0) > 0)
           return true;
        
        return false;    
    }

    bool NOCRouter::PacketMulticastArea(Ptr<const Packet> pck, uint8_t network_id, int32_t x_destination, int32_t y_destination){
        NOCHeader h;
        h.SetProtocol(NOCHeader::PROTOCOL_MULTICAST_AREA);
        h.SetSourceAddressXY(0,0);
        h.SetDestinationAddressXY(x_destination,y_destination);
        
        Ptr<Packet> pck_c = pck->Copy();
        pck_c->AddHeader(h);
        
        uint8_t out = NOCRoutingProtocols::MulticastArea(0, 0, x_destination, y_destination);
        
        if (Transmit(pck_c, network_id, out, P0) > 0)
           return true;
        
        return false;    
    }
    
    bool 
    NOCRouter::PacketMulticastIndividuals(Ptr<const Packet> pck, uint8_t network_id, int32_t x_position, int32_t y_position) {
        NOCHeader h;
        h.SetProtocol(NOCHeader::PROTOCOL_MULTICAST_INDIVIDUALS);
        h.SetSourceAddressXY(0,0);
        h.SetDestinationAddressXY(x_position, y_position);
        
        Ptr<Packet> pck_c = pck->Copy();
        pck_c->AddHeader(h);
        
        uint8_t out = NOCRoutingProtocols::MulticastIndividuals(0, 0, x_position, y_position);
        
        if (Transmit(pck_c, network_id, out, P0) > 0)
           return true;
        
        return false;    

    }


    bool NOCRouter::PacketBroadcast (Ptr<const Packet> pck, uint8_t network_id){
        NOCHeader h;
        h.SetProtocol(NOCHeader::PROTOCOL_BROADCAST);
        h.SetSourceAddressXY(0,0);
        h.SetDestinationAddressXY(0,0);
        
        Ptr<Packet> pck_c = pck->Copy();
        pck_c->AddHeader(h);
        
        uint8_t out = NOCRoutingProtocols::Broadcast(0,0);
        
        if (Transmit(pck_c, network_id, out, P0) > 0)
           return true;
        
        return false;    
    }
    
    
    
    uint8_t
    NOCRouter::Transmit(Ptr<const Packet> pck, uint8_t network_id, uint8_t ports_mask, uint8_t priority){
        //TODO: This function should get the pck, the destination address, priority comes in the packet?, and network it should write to.
        // the rout should be calculated by the router itself. and the routing algorithms should be here (or in separate files).
        // the addressing scheme sould allow: Broadcast (with limited radius (hops)) and unicast (to an specific X,Y location)
        //TODO: uint8_t out_ports_count = CountOnes(ports_mask);
        
        uint8_t sent = 0;

        if ( (ports_mask >> NOCRoutingProtocols::DIRECTION_N) & 1)
        {
            Ptr<Packet> pck_c = pck->Copy();
            if (TransmitSingle(pck_c, network_id, NOCRoutingProtocols::DIRECTION_N, priority)){
                sent++;
            }
        }
        if ( (ports_mask >> NOCRoutingProtocols::DIRECTION_S) & 1){
            Ptr<Packet> pck_c = pck->Copy();
            if (TransmitSingle(pck_c, network_id, NOCRoutingProtocols::DIRECTION_S, priority)){
                sent++;
            }
        }
        if ( (ports_mask >> NOCRoutingProtocols::DIRECTION_E & 1) & 1){
            Ptr<Packet> pck_c = pck->Copy();
            if (TransmitSingle(pck_c, network_id, NOCRoutingProtocols::DIRECTION_E, priority)){
                sent++;
            }
        }
        if ( (ports_mask >> NOCRoutingProtocols::DIRECTION_W) & 1){
            Ptr<Packet> pck_c = pck->Copy();      
            if (TransmitSingle(pck_c, network_id, NOCRoutingProtocols::DIRECTION_W, priority)){
                sent++;
            }
        }
        if ( (ports_mask >> NOCRoutingProtocols::DIRECTION_L) & 1){
            m_routerCxTrace(pck, 0);
            Ptr<Packet> pck_c = pck->Copy();
            NOCHeader h;
            pck_c->RemoveHeader(h);
            m_receiveCallBack(pck_c, h.GetProtocol(), h.GetSourceAddressX(), h.GetSourceAddressY(), h.GetDestinationAddressX(), h.GetDestinationAddressY());           
        }
        
        if (sent > 0) m_routerTxTrace(pck, 0);
        
        return sent;
    }
    
    bool
    NOCRouter::TransmitSingle(Ptr<const Packet> pck, uint8_t network_id, uint8_t port_direction, uint8_t priority){
        Ptr<NOCNetDevice> nd;     
        nd = GetNetDevice(network_id, port_direction);
        if (nd == NULL) //That node does not have a net device in that direction
            return false;
        
        Time t = Time::FromInteger(0, Time::NS);
        t = RoutingDelays->GetDelay();

        //Tweak
        if (port_direction == NOCRoutingProtocols::DIRECTION_E)
            t = Time::FromInteger(1, Time::NS);
        
        int8_t (NOCNetDevice::*fp)(Ptr<Packet>) = &NOCNetDevice::Send;
        Simulator::Schedule(t, fp, nd, pck->Copy());
        
//        if (nd->Send(pck->Copy())){
            return true;
//        }
//        else{
//            m_routerTxDropTrace(pck, 0);
//        }
//        return false;
    }

//    void
//    NOCRouter::PacketTrace(Ptr<const Packet> packet, Ptr<NOCNetDevice> device) {
//
//    }

    
    void
    NOCRouter::PacketTrace(Ptr<const Packet> pck, Ptr<NOCNetDevice> nd){
        Ptr<Packet> pck_c = pck->Copy();
        
        Time packet_duration = nd->GetTransmissionTime(pck); 
        
        NOCHeader h;
        pck_c->RemoveHeader(h);
        
        XDenseHeader hxd;
        pck_c->PeekHeader(hxd);
        
        if (hxd.GetXdenseProtocol() == XDenseHeader::TRACE)
            cout << "Received at:" << Simulator::Now().GetNanoSeconds() 
                 << " tts:" << Simulator::Now().GetNanoSeconds() / packet_duration
                 << " Q:" << nd->GetInputQueueSize() << "\n";  
    }
    
    void
    NOCRouter::PacketReceived(Ptr<const Packet> pck, Ptr<NOCNetDevice> nd) {
        
        switch (ServerPolicy){
            case FIFO:
                ConsumePacket(pck, nd);
            break;
                
            case ROUND_ROBIN:
                if (m_server_state == IDLE){
//                    m_server_state = BUSY;
                    ServePorts();                    
                }

                break;
                       
                     
        }
    }

    void
    NOCRouter::ServePorts(void) {
        static uint8_t actual_port = NOCRoutingProtocols::DIRECTION_E;

        Time t_ns;
        Time packet_duration;
        
        Ptr<Packet> pck;
        Ptr<NOCNetDevice> nd = GetNetDevice(0, actual_port);
        
        uint8_t queue_e = 0;
        Ptr<NOCNetDevice> nd_e = GetNetDevice(0, NOCRoutingProtocols::DIRECTION_E);
        if (nd_e != NULL) 
            queue_e = nd_e->GetInputQueueSize();
        
        uint8_t queue_n = 0;
        Ptr<NOCNetDevice> nd_n = GetNetDevice(0, NOCRoutingProtocols::DIRECTION_N);
        if (nd_n != NULL) 
            queue_n = nd_n->GetInputQueueSize();
        
        uint8_t queue_w = 0;
        Ptr<NOCNetDevice> nd_w = GetNetDevice(0, NOCRoutingProtocols::DIRECTION_W);
        if (nd_w != NULL) 
            queue_w = nd_w->GetInputQueueSize();
        
        uint8_t queue_s = 0;
        Ptr<NOCNetDevice> nd_s = GetNetDevice(0, NOCRoutingProtocols::DIRECTION_S);
        if (nd_s != NULL) 
            queue_s = nd_s->GetInputQueueSize();
        
        uint16_t queue_total = queue_e + queue_n + queue_s + queue_w;

        if (queue_total == 0) {
            m_server_state = IDLE;
        }
        else{
            m_server_state = BUSY;
        }

        switch (m_server_state) {
            case (BUSY):
                switch (actual_port) {
                    /////////////////////// EAST ////////////////////////////////////////////////
                    case NOCRoutingProtocols::DIRECTION_E:
                        if (queue_e > 0) {
                            pck = nd->DequeueReceived();
                            this->ConsumePacket(pck, nd);
                            packet_duration = nd->GetTransmissionTime(pck);
                            Simulator::Schedule(packet_duration, &NOCRouter::ServePorts, this);
                        } else {
                            t_ns = Time::FromInteger(0, Time::NS);
                            Simulator::Schedule(t_ns, &NOCRouter::ServePorts, this);
                        }

                        actual_port = NOCRoutingProtocols::DIRECTION_N;
                        break;
                    /////////////////////// EAST ////////////////////////////////////////////////
                    case NOCRoutingProtocols::DIRECTION_N:
                        if (queue_n > 0) {
                            pck = nd->DequeueReceived();
                            this->ConsumePacket(pck, nd);
                            packet_duration = nd->GetTransmissionTime(pck);
                            Simulator::Schedule(packet_duration, &NOCRouter::ServePorts, this);
                        } else {
                            t_ns = Time::FromInteger(0, Time::NS);
                            Simulator::Schedule(t_ns, &NOCRouter::ServePorts, this);
                        }

                        actual_port = NOCRoutingProtocols::DIRECTION_W;
                        break;
                    /////////////////////// EAST ////////////////////////////////////////////////
                    case NOCRoutingProtocols::DIRECTION_W:
                        if (queue_w > 0) {
                            pck = nd->DequeueReceived();
                            this->ConsumePacket(pck, nd);
                            packet_duration = nd->GetTransmissionTime(pck);
                            Simulator::Schedule(packet_duration, &NOCRouter::ServePorts, this);
                        } else {
                            t_ns = Time::FromInteger(0, Time::NS);
                            Simulator::Schedule(t_ns, &NOCRouter::ServePorts, this);
                        }

                        actual_port = NOCRoutingProtocols::DIRECTION_S;
                        break;
                    /////////////////////// EAST ////////////////////////////////////////////////
                    case NOCRoutingProtocols::DIRECTION_S:
                        if (queue_s > 0) {
                            pck = nd->DequeueReceived();
                            this->ConsumePacket(pck, nd);
                            packet_duration = nd->GetTransmissionTime(pck);
                            Simulator::Schedule(packet_duration, &NOCRouter::ServePorts, this);
                        } else {
                            t_ns = Time::FromInteger(0, Time::NS);
                            Simulator::Schedule(t_ns, &NOCRouter::ServePorts, this);
                        }

                        actual_port = NOCRoutingProtocols::DIRECTION_E;
                        break;
                }
                break;

            case (IDLE):
                break;
        }
    }

    
    void
    NOCRouter::ConsumePacket(Ptr<const Packet> pck, Ptr<NOCNetDevice> nd) {
        //Debug only
        PacketTrace(pck, nd);
        
        m_routerRxTrace(pck, 0);
        Ptr<Packet> pck_c = pck->Copy();
        
        NetDeviceInfo nd_i = GetNetDeviceInfo(nd);
        
        NOCHeader h;
        pck_c->RemoveHeader(h);
        
        bool AddToDestination = false;
        
        if (h.GetProtocol() == NOCHeader::PROTOCOL_UNICAST ||
            h.GetProtocol() == NOCHeader::PROTOCOL_UNICAST_OFFSET)
            AddToDestination = true;
            
        
        switch (nd_i.direction){
            case NOCRoutingProtocols::DIRECTION_S: 
                h.AddtoSourceAddress( 0, 1);
                if (AddToDestination)
                    h.AddtoDestinationAddress( 0, -1);
                break;
            case NOCRoutingProtocols::DIRECTION_N:
                h.AddtoSourceAddress( 0,-1); 
                if (AddToDestination)
                    h.AddtoDestinationAddress( 0, 1);
                break;
            case NOCRoutingProtocols::DIRECTION_E: 
                h.AddtoSourceAddress(-1, 0);
                if (AddToDestination)
                    h.AddtoDestinationAddress( 1, 0);
                break;
            case NOCRoutingProtocols::DIRECTION_W: 
                h.AddtoSourceAddress( 1, 0); 
                if (AddToDestination)
                    h.AddtoDestinationAddress( -1, 0);
                break;
        }
        
        pck_c->AddHeader(h);
        
        
        
        //TODO: Instead of serving imediatelly, queue it and serve it within a
        //delay, captured from the hardware measurements. If set to 0, then it
        //is pretty much like doing it in parallel.
        
        //The input delay should be modeled here, and the output delay in the 
        //packet send function, as it is now.
        
             
        int32_t adx = h.GetDestinationAddressX();
        int32_t ady = h.GetDestinationAddressY();
        int32_t asx = h.GetSourceAddressX();
        int32_t asy = h.GetSourceAddressY();
        uint8_t p = h.GetProtocol();
        
        
        
        uint8_t out = 0;
        
        // Switches between the different possible protocols contained in the pck header
        
        switch (p){
            case NOCHeader::PROTOCOL_BROADCAST:
                out = NOCRoutingProtocols::Broadcast(asx,asy);
                Transmit(pck_c, nd_i.network_id, out, P0);
                break;
               
            case NOCHeader::PROTOCOL_MULTICAST_INDIVIDUALS:
                out = NOCRoutingProtocols::MulticastIndividuals(asx,asy,adx,ady);
                Transmit(pck_c, nd_i.network_id, out, P0);
                break;
               
            case NOCHeader::PROTOCOL_MULTICAST_RADIUS:
                out = NOCRoutingProtocols::MulticastRadius(asx,asy,adx);
                Transmit(pck_c, nd_i.network_id, out, P0);
                break;
               
            case NOCHeader::PROTOCOL_MULTICAST_AREA:
                out = NOCRoutingProtocols::MulticastArea(asx,asy,adx,ady);
                Transmit(pck_c, nd_i.network_id, out, P0);
                break;
               
            case NOCHeader::PROTOCOL_UNICAST:
                out = NOCRoutingProtocols::Unicast(adx,ady, unicast_routing_protocol);
                Transmit(pck_c, nd_i.network_id, out, P0);
                break;

            case NOCHeader::PROTOCOL_UNICAST_OFFSET:
                out = NOCRoutingProtocols::UnicastClockwiseOffsetXY(adx,ady,asx,asy);
                Transmit(pck_c, nd_i.network_id, out, P0);
                break;
               
            default:
                cout << "Unknown protocol" << std::endl;
                break;
                
        }
    }

 

}