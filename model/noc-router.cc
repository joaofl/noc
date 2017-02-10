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
#include "src/core/model/nstime.h"


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
                MakeIntegerAccessor(&NOCRouter::AddressX),
                MakeIntegerChecker<int32_t>())

                .AddAttribute("AddressY",
                "The Y coordinate of the router in the grid (required depending on the protocol)",
                IntegerValue(0),
                MakeIntegerAccessor(&NOCRouter::AddressY),
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
            (*nd)->GetObject<NOCNetDevice>()->SetReceiveCallback(MakeCallback(&NOCRouter::PacketServe, this));
//            (*nd)->GetObject<NOCNetDevice>()->SetRemoteSignalChangedCallback(MakeCallback(&NOCRouter::RemoteWaitChanged, this));
//            (*nd)->GetObject<NOCNetDevice>()->SetLocalSignalChangedCallback(MakeCallback(&NOCRouter::LocalWaitChanged, this));
        }
        
        m_input_ports.Start();
        rr_n.Start();
        rr_w.Start();
        rr_s.Start();       
        
        m_queueFactory.SetTypeId ("ns3::DropTailQueue");
        m_queue_app_output = m_queueFactory.Create<Queue> ();
        
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
    NOCRouter::SetDataRate(DataRate bps) {
        NS_LOG_FUNCTION_NOARGS();
        m_bps = bps;
    }

    void 
    NOCRouter::SetShaper(float b, float rd, uint8_t ms , uint8_t port) {
        
//        uint8_t n = m_netDevices.GetN();

          GetNetDevice(0, port)->SetShaper(b, rd, ms);
                
//        if ( (ports_mask >> NOCRouting::DIRECTION_N) & 1){
//            GetNetDevice(0, NOCRouting::DIRECTION_N)->SetShaper(b, rd);
//        }
//        else if ( (ports_mask >> NOCRouting::DIRECTION_S) & 1){
//            GetNetDevice(0, NOCRouting::DIRECTION_S)->SetShaper(b, rd);
//        }
//        else if ( (ports_mask >> NOCRouting::DIRECTION_E & 1) & 1){
//            GetNetDevice(0, NOCRouting::DIRECTION_E)->SetShaper(b, rd);
//        }
//        else if ( (ports_mask >> NOCRouting::DIRECTION_W) & 1){
//            GetNetDevice(0, NOCRouting::DIRECTION_W)->SetShaper(b, rd);
//        }
    }

    
    Time
    NOCRouter::GetTransmissionTime(Ptr<const Packet> pck) {
        Time t = m_bps.CalculateBitsTxTime(pck->GetSize() * 10);
        return t;
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
    
    Ptr<Queue>
    NOCRouter::GetNetDeviceQueue(uint8_t network, uint8_t direction){
        
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
    NOCRouter::SetRoutingProtocolUnicast(NOCRouting::RoutingProtocols rp) {
        m_routing_conf = rp;
    }

    
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    bool 
    NOCRouter::PacketUnicast (Ptr<const Packet> pck, uint8_t network_id, 
            int32_t destination_x, int32_t destination_y, bool absolute_address)
    {
        if (absolute_address){
                destination_x = destination_x - AddressX;
                destination_y = destination_y - AddressY;
        }
        
        if (destination_x == 0 && destination_y == 0)
            return 0; // dont send it to myself
        
        NOCHeader h;
        h.SetProtocol(NOCHeader::PROTOCOL_UNICAST);
        h.SetSourceAddressXY(0,0);
        h.SetDestinationAddressXY(destination_x,destination_y);
        
        Ptr<Packet> pck_c = pck->Copy();
        pck_c->AddHeader(h);
        
        m_routerGxTrace(pck_c); //the router receives a pck from the application
        
        PacketServe(pck_c, NOCRouting::DIRECTION_L);
        return true; 
    }
    
    bool 
    NOCRouter::PacketUnicastOffset (Ptr<const Packet> pck, uint8_t network_id, 
            int32_t destination_x, int32_t destination_y, bool absolute_address)
    {
        if (absolute_address){
                destination_x = destination_x - AddressX;
                destination_y = destination_y - AddressY;
        }
        
        if (destination_x == 0 && destination_y == 0)
            return 0; // dont send it to myself
        
        NOCHeader h;
        h.SetProtocol(NOCHeader::PROTOCOL_UNICAST_OFFSET);
        h.SetSourceAddressXY(0,0);
        h.SetDestinationAddressXY(destination_x, destination_y);
        
        Ptr<Packet> pck_c = pck->Copy();
        pck_c->AddHeader(h);
        
        m_routerGxTrace(pck_c); //the router receives a pck from the application
        
        PacketServe(pck_c, NOCRouting::DIRECTION_L);
        return true; 
    }

    bool NOCRouter::PacketMulticastRadius (Ptr<const Packet> pck, uint8_t network_id, uint8_t n_hops){
        NOCHeader h;
        h.SetProtocol(NOCHeader::PROTOCOL_MULTICAST_RADIUS);
        h.SetSourceAddressXY(0,0);
        h.SetDestinationAddressXY(n_hops,0);
        
        Ptr<Packet> pck_c = pck->Copy();
        pck_c->AddHeader(h);
        
        PacketServe(pck_c, NOCRouting::DIRECTION_L);
        return true; 
    }

    bool NOCRouter::PacketMulticastArea(Ptr<const Packet> pck, uint8_t network_id, int32_t x_destination, int32_t y_destination){
        NOCHeader h;
        h.SetProtocol(NOCHeader::PROTOCOL_MULTICAST_AREA);
        h.SetSourceAddressXY(0,0);
        h.SetDestinationAddressXY(x_destination,y_destination);
        
        Ptr<Packet> pck_c = pck->Copy();
        pck_c->AddHeader(h);
        
        PacketServe(pck_c, NOCRouting::DIRECTION_L);
        return true; 
    }
    
    bool 
    NOCRouter::PacketMulticastIndividuals(Ptr<const Packet> pck, uint8_t network_id, int32_t x_position, int32_t y_position) {
        NOCHeader h;
        h.SetProtocol(NOCHeader::PROTOCOL_MULTICAST_INDIVIDUALS);
        h.SetSourceAddressXY(0,0);
        h.SetDestinationAddressXY(x_position*2 + 1, y_position*2 + 1);
        
        Ptr<Packet> pck_c = pck->Copy();
        pck_c->AddHeader(h);
        
        PacketServe(pck_c, NOCRouting::DIRECTION_L);
        return true;
    }


    bool NOCRouter::PacketBroadcast (Ptr<const Packet> pck, uint8_t network_id){
        NOCHeader h;
        h.SetProtocol(NOCHeader::PROTOCOL_BROADCAST);
        h.SetSourceAddressXY(0,0);
        h.SetDestinationAddressXY(0,0);
        
        Ptr<Packet> pck_c = pck->Copy();
        pck_c->AddHeader(h);
        
        PacketServe(pck_c, NOCRouting::DIRECTION_L);
        return true;         
    }
    
    uint8_t
    NOCRouter::Transmit(Ptr<const Packet> pck, uint8_t network_id, uint8_t ports_mask, uint8_t priority){
        //TODO: This function should get the pck, the destination address, priority comes in the packet?, and network it should write to.
        // the rout should be calculated by the router itself. and the routing algorithms should be here (or in separate files).
        // the addressing scheme sould allow: Broadcast (with limited radius (hops)) and unicast (to an specific X,Y location)
        //TODO: uint8_t out_ports_count = CountOnes(ports_mask);
        
        uint8_t sent = 0;

        if ( (ports_mask >> NOCRouting::DIRECTION_N) & 1)
        {
            Ptr<Packet> pck_c = pck->Copy();
            if (TransmitSingle(pck_c, network_id, NOCRouting::DIRECTION_N, priority)){
                sent++;
            }
        }
        if ( (ports_mask >> NOCRouting::DIRECTION_S) & 1){
            Ptr<Packet> pck_c = pck->Copy();
            if (TransmitSingle(pck_c, network_id, NOCRouting::DIRECTION_S, priority)){
                sent++;
            }
        }
        if ( (ports_mask >> NOCRouting::DIRECTION_E & 1) & 1){
            Ptr<Packet> pck_c = pck->Copy();
            if (TransmitSingle(pck_c, network_id, NOCRouting::DIRECTION_E, priority)){
                sent++;
            }
        }
        if ( (ports_mask >> NOCRouting::DIRECTION_W) & 1){
            Ptr<Packet> pck_c = pck->Copy();      
            if (TransmitSingle(pck_c, network_id, NOCRouting::DIRECTION_W, priority)){
                sent++;
            }
        }
        if ( (ports_mask >> NOCRouting::DIRECTION_L) & 1){
            m_routerCxTrace(pck);
            Ptr<Packet> pck_c = pck->Copy();
            NOCHeader h;
            pck_c->RemoveHeader(h);
            m_receiveCallBack(pck_c, h.GetProtocol(), h.GetSourceAddressX(), h.GetSourceAddressY(), h.GetDestinationAddressX(), h.GetDestinationAddressY());           
        }
        
        if (sent > 0) m_routerTxTrace(pck);
        
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

    void
    NOCRouter::PacketTrace(Ptr<const Packet> pck, Ptr<NOCNetDevice> nd){
        Ptr<Packet> pck_c = pck->Copy();
        
        Time packet_duration = this->GetTransmissionTime(pck); 
        
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
    NOCRouter::PacketServe(Ptr<const Packet> pck_rcv, NOCRouting::Directions input) {
        m_routerRxTrace(pck_rcv);  
        
//        if (this->m_addressX == 1 && this->m_addressY == 1)
//            cout << "Junktion\n";
        
        Ptr<QueueItem> item;
        Ptr<Packet> pck;// = pck->Copy();
        uint8_t out;
        Time t_ns;
        Ptr<NOCNetDevice> nd;
        
        if (input == NOCRouting::DIRECTION_L){ //Generated locally, then queue it
            this->m_queue_app_output->Enqueue(Create<QueueItem> (pck_rcv->Copy()));
        }
        
        switch (ServerPolicy){
            case FIFO:
                if (input == NOCRouting::DIRECTION_L){
                    item = m_queue_app_output->Dequeue();
                    pck = item->GetPacket();
                }
                else
                    pck = this->GetNetDevice(0, input)->DequeueReceived();
                
                out = NOCRouting::Route(pck, m_routing_conf);
                
                this->Transmit(pck, 0, out, P0);
                break;
                
            case ROUND_ROBIN:               
                if (m_server_state == IDLE){
//                    m_server_state = BUSY;
                    t_ns = Time::FromInteger(0, Time::NS);
                    Simulator::Schedule(t_ns, &NOCRouter::RoundRobin, this);                 
                }

                break;       
        }
    }
    
    void
    NOCRouter::RoundRobin(void) {

        Time t_ns;
        Time packet_duration;
        
        Ptr<Packet> pck;
        Ptr<const Packet> pck_e, pck_n, pck_w, pck_s, pck_l;
        Ptr<NOCNetDevice> nd; 
        
        uint8_t out_from_e, out_from_n, out_from_w, out_from_s, out_from_l;
        
        uint8_t queue_e, queue_n, queue_w, queue_s, queue_l;
        uint16_t queue_total;
        
        queue_e = 0;
        Ptr<NOCNetDevice> nd_e = GetNetDevice(0, NOCRouting::DIRECTION_E);
        if (nd_e != NULL){
            queue_e = nd_e->GetInputQueueSize();
            if (queue_e > 0){
                pck_e = nd_e->PeekReceived();
                out_from_e = NOCRouting::Route(pck_e, m_routing_conf);
            }
        }
        
        queue_n = 0;
        Ptr<NOCNetDevice> nd_n = GetNetDevice(0, NOCRouting::DIRECTION_N);
        if (nd_n != NULL){
            queue_n = nd_n->GetInputQueueSize();
            if (queue_n > 0){
                pck_n = nd_n->PeekReceived();
                out_from_n = NOCRouting::Route(pck_n, m_routing_conf);
            }
        }
        
        queue_w = 0;
        Ptr<NOCNetDevice> nd_w = GetNetDevice(0, NOCRouting::DIRECTION_W);
        if (nd_w != NULL){
            queue_w = nd_w->GetInputQueueSize();
            if (queue_w > 0){
                pck_w = nd_w->PeekReceived();
                out_from_w = NOCRouting::Route(pck_w, m_routing_conf);
            }
        }
        
        queue_s = 0;
        Ptr<NOCNetDevice> nd_s = GetNetDevice(0, NOCRouting::DIRECTION_S);
        if (nd_s != NULL){ 
            queue_s = nd_s->GetInputQueueSize();
            if (queue_s > 0){
                pck_s = nd_s->PeekReceived();
                out_from_s = NOCRouting::Route(pck_s, m_routing_conf);
            }
        }
        
        queue_l = 0; 
            queue_l = m_queue_app_output->GetNPackets();
            if (queue_l > 0){
                Ptr<const QueueItem> item = m_queue_app_output->Peek();
                pck_l = item->GetPacket();
                out_from_l = NOCRouting::Route(pck_l, m_routing_conf);
            }
        
        //The local port should be accounted here
        queue_total = queue_e + queue_n + queue_s + queue_w + queue_l;
        
        if (queue_total == 0) {
            m_server_state = IDLE;
        }
        else{
            m_server_state = BUSY;
        }
        
        nd = GetNetDevice(0, m_input_ports.m_actual_port);
        
//        if (this->m_addressX == 1 && this->m_addressY == 1)
//            cout << "Junktion\n";

        switch (m_server_state) {
            
            case (IDLE):
                break;
            
            case (BUSY):
                switch (m_input_ports.m_actual_port) {
                        /////////////////////// EAST ////////////////////////////////////////////////
                    case NOCRouting::DIRECTION_E:
                        if (queue_e > 0) {
                            pck = nd->DequeueReceived();
                            Transmit(pck, 0, out_from_e, P0);
                            
                            //There is no conflict of interest for output ports
                            if (queue_n > 0 && !(out_from_n ^ out_from_e) == 0){ 
                                pck = nd_n->DequeueReceived();
                                Transmit(pck, 0, out_from_n, P0);
                            }
                            if (queue_w > 0 && !(out_from_w ^ out_from_e) == 0){ 
                                pck = nd_w->DequeueReceived();
                                Transmit(pck, 0, out_from_w, P0);
                            }
                            if (queue_s > 0 && !(out_from_s ^ out_from_e) == 0){ 
                                pck = nd_s->DequeueReceived();
                                Transmit(pck, 0, out_from_s, P0);
                            }     
                            if (queue_l > 0 && !(out_from_l ^ out_from_e) == 0){ 
                                Ptr<QueueItem> item = m_queue_app_output->Dequeue();
                                pck = item->GetPacket();
                                Transmit(pck, 0, out_from_l, P0);
                            }
                            
                            packet_duration = this->GetTransmissionTime(pck);
                            Simulator::Schedule(packet_duration, &NOCRouter::RoundRobin, this);
                        } else {
                            t_ns = Time::FromInteger(0, Time::NS);
                            Simulator::Schedule(t_ns, &NOCRouter::RoundRobin, this);
                        }

                        m_input_ports.NextPort();
                        break;
                        /////////////////////// EAST ////////////////////////////////////////////////
                    case NOCRouting::DIRECTION_N:
                        if (queue_n > 0) {
                            pck = nd->DequeueReceived();
                            Transmit(pck, 0, out_from_n, P0);
                            
                            //There is no conflict of interest for output ports
                            if (queue_e > 0 && !(out_from_e ^ out_from_n) == 0){ 
                                pck = nd_e->DequeueReceived();
                                Transmit(pck, 0, out_from_e, P0);
                            }
                            if (queue_w > 0 && !(out_from_w ^ out_from_n) == 0){ 
                                pck = nd_w->DequeueReceived();
                                Transmit(pck, 0, out_from_w, P0);
                            }
                            if (queue_s > 0 && !(out_from_s ^ out_from_n) == 0){ 
                                pck = nd_s->DequeueReceived();
                                Transmit(pck, 0, out_from_s, P0);
                            }
                            if (queue_l > 0 && !(out_from_l ^ out_from_n) == 0){ 
                                Ptr<QueueItem> item = m_queue_app_output->Dequeue();
                                pck = item->GetPacket();
                                Transmit(pck, 0, out_from_l, P0);
                            }
                            
                            packet_duration = this->GetTransmissionTime(pck);
                            Simulator::Schedule(packet_duration, &NOCRouter::RoundRobin, this);
                        } else {
                            t_ns = Time::FromInteger(0, Time::NS);
                            Simulator::Schedule(t_ns, &NOCRouter::RoundRobin, this);
                        }

                        m_input_ports.NextPort();
                        break;
                        /////////////////////// EAST ////////////////////////////////////////////////
                    case NOCRouting::DIRECTION_W:
                        if (queue_w > 0) {
                            pck = nd->DequeueReceived();
                            Transmit(pck, 0, out_from_w, P0);
                            
                            //There is no conflict of interest for output ports
                            if (queue_e > 0 && !(out_from_e ^ out_from_w) == 0){ 
                                pck = nd_e->DequeueReceived();
                                Transmit(pck, 0, out_from_e, P0);
                            }
                            if (queue_n > 0 && !(out_from_n ^ out_from_w) == 0){ 
                                pck = nd_n->DequeueReceived();
                                Transmit(pck, 0, out_from_n, P0);
                            }
                            if (queue_s > 0 && !(out_from_s ^ out_from_w) == 0){ 
                                pck = nd_s->DequeueReceived();
                                Transmit(pck, 0, out_from_s, P0);
                            }
                            if (queue_l > 0 && !(out_from_l ^ out_from_w) == 0){
                                Ptr<QueueItem> item = m_queue_app_output->Dequeue();
                                pck = item->GetPacket();
                                Transmit(pck, 0, out_from_l, P0);
                            }
                            
                            packet_duration = this->GetTransmissionTime(pck);
                            Simulator::Schedule(packet_duration, &NOCRouter::RoundRobin, this);
                        } else {
                            t_ns = Time::FromInteger(0, Time::NS);
                            Simulator::Schedule(t_ns, &NOCRouter::RoundRobin, this);
                        }

                        m_input_ports.NextPort();
                        break;
                        
                    /////////////////////// EAST ////////////////////////////////////////////////
                    case NOCRouting::DIRECTION_S:
                        if (queue_s > 0) {
                            pck = nd->DequeueReceived();
                            Transmit(pck, 0, out_from_s, P0);
                            
                            //There is no conflict of interest for output ports
                            if (queue_e > 0 && !(out_from_e ^ out_from_s) == 0){ 
                                pck = nd_e->DequeueReceived();
                                Transmit(pck, 0, out_from_e, P0);
                            }
                            if (queue_w > 0 && !(out_from_w ^ out_from_s) == 0){ 
                                pck = nd_w->DequeueReceived();
                                Transmit(pck, 0, out_from_w, P0);
                            }
                            if (queue_n > 0 && !(out_from_n ^ out_from_s) == 0){ 
                                pck = nd_n->DequeueReceived();
                                Transmit(pck, 0, out_from_n, P0);
                            }
                            if (queue_l > 0 && !(out_from_l ^ out_from_s) == 0){ 
                                Ptr<QueueItem> item = m_queue_app_output->Dequeue();
                                pck = item->GetPacket();
                                Transmit(pck, 0, out_from_l, P0);
                            }
                            
                            packet_duration = this->GetTransmissionTime(pck);
                            Simulator::Schedule(packet_duration, &NOCRouter::RoundRobin, this);
                        } else {
                            t_ns = Time::FromInteger(0, Time::NS);
                            Simulator::Schedule(t_ns, &NOCRouter::RoundRobin, this);
                        }

                        m_input_ports.NextPort();
                        break;
                        
                    /////////////////////// LOCAL ////////////////////////////////////////////////
                    case NOCRouting::DIRECTION_L:
                        if (queue_l > 0) {
                            Ptr<QueueItem> item = m_queue_app_output->Dequeue();
                            pck = item->GetPacket();
                            
                            Transmit(pck, 0, out_from_l, P0);
                            
                            //There is no conflict of interest for output ports
                            if (queue_e > 0 && !(out_from_e ^ out_from_l) == 0){ 
                                pck = nd_e->DequeueReceived();
                                Transmit(pck, 0, out_from_e, P0);
                            }
                            if (queue_w > 0 && !(out_from_w ^ out_from_l) == 0){ 
                                pck = nd_w->DequeueReceived();
                                Transmit(pck, 0, out_from_w, P0);
                            }
                            if (queue_n > 0 && !(out_from_n ^ out_from_l) == 0){ 
                                pck = nd_n->DequeueReceived();
                                Transmit(pck, 0, out_from_n, P0);
                            }
                            if (queue_s > 0 && !(out_from_s ^ out_from_l) == 0){ 
                                pck = nd_s->DequeueReceived();
                                Transmit(pck, 0, out_from_s, P0);
                            }
                            
                            packet_duration = this->GetTransmissionTime(pck);
                            Simulator::Schedule(packet_duration, &NOCRouter::RoundRobin, this);
                        } else {
                            t_ns = Time::FromInteger(0, Time::NS);
                            Simulator::Schedule(t_ns, &NOCRouter::RoundRobin, this);
                        }

                        m_input_ports.NextPort();
                        break;

                    default:
                        break;
                }
  
                break;
        }
    }
    
        
    void
    RoundRobinState::NextPort(void) {
        switch (m_actual_port) {
            /////////////////////// LOCAL ////////////////////////////////////////////////
            case NOCRouting::DIRECTION_L:
                if (m_LC < m_L){
                    m_LC++;
                }
                else{
                    m_LC = 0;
                    m_actual_port = NOCRouting::DIRECTION_E;
                }
                break;
            /////////////////////// EAST ////////////////////////////////////////////////
            case NOCRouting::DIRECTION_E:
                if (m_EC < m_E){
                    m_EC++;
                }
                else{
                    m_EC = 0;
                    m_actual_port = NOCRouting::DIRECTION_N;
                }
                break;
            /////////////////////// NORTH ////////////////////////////////////////////////
            case NOCRouting::DIRECTION_N:
                if (m_NC < m_N){
                    m_NC++;
                }
                else{
                    m_NC = 0;
                    m_actual_port = NOCRouting::DIRECTION_W;
                }
                break;
            /////////////////////// WEST ////////////////////////////////////////////////
            case NOCRouting::DIRECTION_W:
                if (m_WC < m_W){
                    m_WC++;
                }
                else{
                    m_WC = 0;
                    m_actual_port = NOCRouting::DIRECTION_S;
                }
                break;
            /////////////////////// SOUTH ////////////////////////////////////////////////
            case NOCRouting::DIRECTION_S:
                if (m_SC < m_S){
                    m_SC++;
                }
                else{
                    m_SC = 0;
                    m_actual_port = NOCRouting::DIRECTION_L;
                }
                break;

            default:
                break;
        }
    }
    void 
    RoundRobinState::Start() {
        m_actual_port = NOCRouting::DIRECTION_L;
        
        m_E = 1;
        m_N = 1;
        m_W = 1;
        m_S = 1;
        m_L = 1;
        
        m_EC = 0;
        m_NC = 0;
        m_WC = 0;
        m_SC = 0;
        m_LC = 0;
    }



    


 

}