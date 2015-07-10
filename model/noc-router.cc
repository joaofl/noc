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


#include "noc-header.h"
#include "noc-net-device.h"
#include "noc-application.h"
#include "noc-router.h"
#include "src/core/model/object.h"
#include "ns3/noc-types.h"
#include "ns3/noc-header.h"
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
    NOCRouter::StartApplication(void) {


//        PacketsReceived.assign(P_COUNT, 0);
//        PacketsSent.assign(P_COUNT, 0);

        //        for (uint8_t i = 0; i < P_COUNT; i++) {
        //            PacketTrace.push_back(new stringstream);
        //        }


        //Configure ();
        Ptr<Node> nd = this->GetNode();
        uint8_t n_devices = nd->GetNDevices();

        for (uint8_t i = 0; i < n_devices; i++) {
            nd->GetDevice(i)->SetReceiveCallback(MakeCallback(&NOCRouter::ReceivePacket, this));
        }

        //m_app = this->GetNode()->GetObject<NOCApp>();

    }

    void
    NOCRouter::StopApplication(void) {
        m_running = false;

        if (m_sendEvent.IsRunning()) {
            Simulator::Cancel(m_sendEvent);
        }
    }

//    void
//    NOCRouter::SendSignal(uint8_t bits, uint8_t ports) {
//        Ptr<Node> node = this->GetNode();
//        uint8_t n_devices = node->GetNDevices();
//
//        Ptr<Packet> pck = Create<Packet>();
//        NOCAddress address;
//        //         pck->;
//
//        for (uint32_t i = 0; i < node->GetNDevices(); i++) { //sends one packet per netdevice installed, according to the bitmask received
//            uint8_t p = ports >> i;
//            if ((p & 1) == 1) {
//                for (uint32_t j = 0; j < n_devices; j++) { //iterate to find which netdevice is the correct one
//                    address = node->GetDevice(j)->GetObject<NOCNetDevice>()->GetNOCAddress();
//                    if (address == i + 1) {
//
//                        node->GetDevice(j)->GetObject<NOCNetDevice>()->SendSignal(pck);
//
//                        break;
//                    }
//                }
//            }
//        }
//    }
    
//    void
//    NOCRouter::SendPacket(Ptr<const Packet> pck, uint8_t ports){
//        //check if it will be sent to more then one port, if positive, some time
//        //drift will be inserted between each transmission in order to guarantee
//        //the expected transmission sequence, from east to north, or 1 to 4
//        
//        
////        #ifdef DESYNCRONIZE
////
////        uint8_t n_ports = 0;
////        
////        for (uint8_t i = 0 ; i < NumNetDevices; i++){ //loops to all the bits to check which port should the packet be sent to
////            if (((ports >> i) & 0b00000001) == 1){
////                uint8_t port = 0b00000001 << i;
////                
////                Time t = NanoSeconds(n_ports);
////                Simulator::Schedule(t, &NOCRouter::SendSinglePacket, this, pck, port);
//////                SendSinglePacket(pck, port);
////                n_ports++;
////            }
////        }
////        
////        return;
////        #endif
//
//        SendSinglePacket(pck,ports);
//    }
    
    void
    NOCRouter::SendPacket(Ptr<const Packet> pck, uint8_t ports_mask){//, uint8_t optional network_id) {
        //TODO: This function should get the pck, the destination address, priority comes in the packet?, and network it should write to.
        // the rout should be calculated by the router itself. and the routing algorithms should be here (or in separate files).
        // the addressing scheme sould allow: Broadcast (with limited radius (hops)) and unicast (to an specific X,Y location)

        Ptr<Node> node = this->GetNode();
//        uint8_t n_devices = node->GetNDevices();
//        uint8_t priority = 0;

        NOCHeader hd;
        pck->PeekHeader(hd);

//        NodeRef nr;
//        nr.x = 30;
//        nr.y = 44;

        Time t = Simulator::Now();
        t = t;
        m_routerTxTrace(pck);

        //starts sending the packets from port 1 to port 4.
        for (uint32_t i = 0 ; i < this->GetNDevices() ; i++) { //sends one packet per netdevice installed, according to the bitmask received
            uint8_t p = ports_mask >> i;
            if ((p & 1) == 1) {
                
//                for (uint32_t j = 0; j < n_devices; j++) { //iterate to find which netdevice is the correct one
////                    Ptr<NOCRouter> my_noc_router = node->GetApplication(INSTALLED_NOC_SWITCH)->GetObject<NOCRouter>();
//                    
//                    
////                    if (node->GetDevice(j)->GetObject<NOCNetDevice>()->GetAddress() == i + 1) {
////
////                        //TODO: this should be replaced by callbacks urgently
////                        if (hd.GetNOCProtocol() == P_NETWORK_DISCOVERY) {
////                            priority = 0;
////                            PacketsSent [P_NETWORK_DISCOVERY]++;
////                        } else if (hd.GetNOCProtocol() == P_VALUE_ANNOUNCEMENT) {
////                            priority = 0;
////                            PacketsSent [P_VALUE_ANNOUNCEMENT]++;
////                        } else if (hd.GetNOCProtocol() == P_EVENT_ANNOUNCEMENT) {
////                            priority = 1;
////                            PacketsSent [P_EVENT_ANNOUNCEMENT]++;
////                        }
////
////                        node->GetDevice(j)->GetObject<NOCNetDevice>()->Send(pck->Copy(), priority);
////
////                        break;
////                    }
//                }
            }
        }
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
    NOCRouter::ReceivePacket(Ptr<NetDevice> device, Ptr<const Packet> pck_rcv, uint16_t protocol, const Address& sourceAddress) {

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
}

