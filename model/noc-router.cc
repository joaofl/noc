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

    void 
    NOCRouter::SetAttribute (std::string name, const uint8_t value){
        if (name.compare("ChannelCount") == 0) //Is the same
        {
            m_channelCount = value;
        }
    }
    
    void
    NOCRouter::StartApplication(void) {

//        uint8_t n = m_netDevices.GetN();
//        uint8_t m = m_netDeviceInfoArray.size();
//        n=n;
//        m=m;
        
        
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

//        Ptr<Node> node = this->GetNode();
//        uint8_t n_devices = node->GetNDevices();
//        uint8_t priority = 0;

//        NOCHeader hd;
//        pck->PeekHeader(hd);

//        NodeRef nr;
//        nr.x = 30;
//        nr.y = 44;

//        Time t = Simulator::Now();
//        t = t;
        
//        uint8_t s = m_netDeviceInfoArray.size();
        
        for (uint8_t i = 0; i < m_channelCount; i++) 
        {
            //TODO: the arbitration policy should be considered here. Round robin for example
            if ( (ports_mask >> DIRECTION_E) & 1) this->GetNetDevice(i, DIRECTION_E)->Send(pck->Copy());
            if ( (ports_mask >> DIRECTION_S) & 1) this->GetNetDevice(i, DIRECTION_S)->Send(pck->Copy());
            if ( (ports_mask >> DIRECTION_W) & 1) this->GetNetDevice(i, DIRECTION_W)->Send(pck->Copy());
            if ( (ports_mask >> DIRECTION_N) & 1) this->GetNetDevice(i, DIRECTION_N)->Send(pck->Copy());
        }

        
        m_routerTxTrace(pck);
        
        
        
        

        //starts sending the packets from port 1 to port 4.
//        for (uint32_t i = 0 ; i < this->GetNDevices() ; i++) { //sends one packet per netdevice installed, according to the bitmask received
//            uint8_t p = ports_mask >> i;
//            if ((p & 1) == 1) {
//                
////                for (uint32_t j = 0; j < n_devices; j++) { //iterate to find which netdevice is the correct one
//////                    Ptr<NOCRouter> my_noc_router = node->GetApplication(INSTALLED_NOC_SWITCH)->GetObject<NOCRouter>();
////                    
////                    
//////                    if (node->GetDevice(j)->GetObject<NOCNetDevice>()->GetAddress() == i + 1) {
//////
//////                        //TODO: this should be replaced by callbacks urgently
//////                        if (hd.GetNOCProtocol() == P_NETWORK_DISCOVERY) {
//////                            priority = 0;
//////                            PacketsSent [P_NETWORK_DISCOVERY]++;
//////                        } else if (hd.GetNOCProtocol() == P_VALUE_ANNOUNCEMENT) {
//////                            priority = 0;
//////                            PacketsSent [P_VALUE_ANNOUNCEMENT]++;
//////                        } else if (hd.GetNOCProtocol() == P_EVENT_ANNOUNCEMENT) {
//////                            priority = 1;
//////                            PacketsSent [P_EVENT_ANNOUNCEMENT]++;
//////                        }
//////
//////                        node->GetDevice(j)->GetObject<NOCNetDevice>()->Send(pck->Copy(), priority);
//////
//////                        break;
//////                    }
////                }
//            }
//        }
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
        return 0; //if the specified node was not found
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

        //Check the origin of the packet to determine from which direction it
        //came from and the address of the sink. It also prepares the header in
        //case it is forwarded to neighbors.

        // Net devices Address reference

        //            4               4       
        //         _______         _______    
        //        |       |       |       |   
        //     3  |  SN   |  1 3  |  SN   |  1
        //        |_______|       |_______|   
        //                                    
        //            2               2               
        //            4               4       
        //         _______         _______    
        //        |       |       |       |   
        //     3  |  SN   |  1 3  |  SN   |  1
        //        |_______|       |_______|   
        //                                    
        //            2               2       
        //
        //   ------------> +x
        //   |
        //   |
        //   |
        //   |
        //   +y

        //Protocol Numbering:
        // 1: Network Discovery
        // 2: Values Exchanging
        // 3: Detection Announcement

//        Ptr<Node> nd = this->GetNode();
//
//        Ptr<Packet> pck_cp = pck_rcv->Copy();
//
//        NOCHeader hd;
//        pck_cp->RemoveHeader(hd);

//        NodeRef nr;
//        nr.x = 30;
//        nr.y = 44;

//        PacketsReceived[P_TOTAL]++;
        //TODO: replace all this by callbacks
        

//        uint8_t noc_protocol = hd.GetNOCProtocol();
//
//        NOCAddress ad = device->GetObject<NOCNetDevice>()->GetAddress(); //Get the address of the device which generated the interruption
//
//        if (ad == 1) { //came from the right
//            hd.CurrentX -= 1;
//        } 
//        else if (ad == 2) {
//            hd.CurrentY -= 1;
//        } 
//        else if (ad == 3) {
//            hd.CurrentX += 1;
//        } 
//        else if (ad == 4) {
//            hd.CurrentY += 1;
//        }
////        hd.HopsCount += 1;
//        pck_cp->AddHeader(hd);
//
//        m_routerRxTrace(pck_cp); //trance the packet after changing 
//        
//        uint8_t origin_port = 0b00000001 << (ad - 1);



        //TODO: This should be in the app layer... Unless this are specific networking functions, like Broadcast or unicast
//        if (noc_protocol == P_NETWORK_DISCOVERY) {
//            PacketsReceived[P_NETWORK_DISCOVERY]++;
//            Ptr<NOCApp> m_app = this->GetNode()->GetApplication(INSTALLED_NOC_APP)->GetObject<NOCApp>();
//            m_app->NetworkDiscoveryReceived(pck_cp, origin_port);
//
//        } else if (noc_protocol == P_VALUE_ANNOUNCEMENT) {
//            PacketsReceived[P_VALUE_ANNOUNCEMENT]++;
//            Ptr<NOCApp> m_app = this->GetNode()->GetApplication(INSTALLED_NOC_APP)->GetObject<NOCApp>();
//            m_app->ValueAnnoucementReceived(pck_cp, origin_port);
//
//        } else if (noc_protocol == P_EVENT_ANNOUNCEMENT) {
//            PacketsReceived[P_EVENT_ANNOUNCEMENT]++;
//
//            Ptr<NOCApp> m_app = this->GetNode()->GetApplication(INSTALLED_NOC_APP)->GetObject<NOCApp>();
//            m_app->EventAnnoucementReceived(pck_cp, origin_port);
//        }


        return true;
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

