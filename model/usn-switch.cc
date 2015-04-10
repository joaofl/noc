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


#include "usn-header.h"
#include "usn-net-device.h"
#include "usn-application.h"
#include "usn-switch.h"
#include "src/core/model/object.h"
#include "ns3/usn-types.h"
#include "ns3/usn-header.h"
#include "ns3/usn-switch.h"


using namespace std;
namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("USNSwitch");

    TypeId
    USNSwitch::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::USNSwitch")
                .SetParent<Application> ()
                .AddConstructor<USNSwitch> ()
                .AddTraceSource("SwitchRxTrace", "The packets received by the switch of each node", MakeTraceSourceAccessor(&USNSwitch::m_switchRxTrace))
                .AddTraceSource("SwitchTxTrace", "The packets sent by the switch of each node", MakeTraceSourceAccessor(&USNSwitch::m_switchTxTrace))
                ;
        return tid;
    }

    USNSwitch::USNSwitch()
    : m_running(false) {
    }

    USNSwitch::~USNSwitch() {
    }

    //    void
    //    USNSwitch::Setup(bool IsSink) {
    //        IsSink = IsSink;
    //
    //    }

    void
    USNSwitch::StartApplication(void) {


        PacketsReceived.assign(P_COUNT, 0);
        PacketsSent.assign(P_COUNT, 0);

        //        for (uint8_t i = 0; i < P_COUNT; i++) {
        //            PacketTrace.push_back(new stringstream);
        //        }


        //Configure ();
        Ptr<Node> nd = this->GetNode();
        uint8_t n_devices = nd->GetNDevices();

        for (uint8_t i = 0; i < n_devices; i++) {
            nd->GetDevice(i)->SetReceiveCallback(MakeCallback(&USNSwitch::PacketReceive, this));
        }

        //m_app = this->GetNode()->GetObject<USNApp>();

    }

    void
    USNSwitch::StopApplication(void) {
        m_running = false;

        if (m_sendEvent.IsRunning()) {
            Simulator::Cancel(m_sendEvent);
        }
    }

    void
    USNSwitch::SendSignal(uint8_t bits, uint8_t ports) {
        Ptr<Node> node = this->GetNode();
        uint8_t n_devices = node->GetNDevices();

        Ptr<Packet> pck = Create<Packet>();
        //         pck->;

        for (uint32_t i = 0; i < NumNetDevices; i++) { //sends one packet per netdevice installed, according to the bitmask received
            uint8_t p = ports >> i;
            if ((p & 1) == 1) {
                for (uint32_t j = 0; j < n_devices; j++) { //iterate to find which netdevice is the correct one
                    if (node->GetDevice(j)->GetObject<USNNetDevice>()->GetUSNAddress() == i + 1) {

                        node->GetDevice(j)->GetObject<USNNetDevice>()->SendSignal(pck);

                        break;
                    }
                }
            }
        }
    }
    
    void
    USNSwitch::SendPacket(Ptr<const Packet> pck, uint8_t ports){
        //check if it will be sent to more then one port, if positive, some time
        //drift will be inserted between each transmission in order to guarantee
        //the expected transmission sequence, from east to north, or 1 to 4
        
        
//        #ifdef DESYNCRONIZE
//
//        uint8_t n_ports = 0;
//        
//        for (uint8_t i = 0 ; i < NumNetDevices; i++){ //loops to all the bits to check which port should the packet be sent to
//            if (((ports >> i) & 0b00000001) == 1){
//                uint8_t port = 0b00000001 << i;
//                
//                Time t = NanoSeconds(n_ports);
//                Simulator::Schedule(t, &USNSwitch::SendSinglePacket, this, pck, port);
////                SendSinglePacket(pck, port);
//                n_ports++;
//            }
//        }
//        
//        return;
//        #endif

        SendSinglePacket(pck,ports);
    }
    
    void
    USNSwitch::SendSinglePacket(Ptr<const Packet> pck, uint8_t ports) {

        Ptr<Node> node = this->GetNode();
        uint8_t n_devices = node->GetNDevices();
        uint8_t priority = 0;

        USNHeader hd;
        pck->PeekHeader(hd);

//        NodeRef nr;
//        nr.x = 30;
//        nr.y = 44;

        Time t = Simulator::Now();
        t = t;
        m_switchTxTrace(pck);

        //starts sending the packets from port 1 to port 4.
        for (uint32_t i = 0; i < NumNetDevices; i++) { //sends one packet per netdevice installed, according to the bitmask received
            uint8_t p = ports >> i;
            if ((p & 1) == 1) {
                for (uint32_t j = 0; j < n_devices; j++) { //iterate to find which netdevice is the correct one
                    if (node->GetDevice(j)->GetObject<USNNetDevice>()->GetUSNAddress() == i + 1) {

                        //TODO: this should be replaced by callbacks urgently
                        if (hd.GetUSNProtocol() == P_NETWORK_DISCOVERY) {
                            priority = 0;
                            PacketsSent [P_NETWORK_DISCOVERY]++;
                        } else if (hd.GetUSNProtocol() == P_VALUE_ANNOUNCEMENT) {
                            priority = 0;
                            PacketsSent [P_VALUE_ANNOUNCEMENT]++;
                        } else if (hd.GetUSNProtocol() == P_EVENT_ANNOUNCEMENT) {
                            priority = 1;
                            PacketsSent [P_EVENT_ANNOUNCEMENT]++;
                        }

                        node->GetDevice(j)->GetObject<USNNetDevice>()->Send(pck->Copy(), priority);

                        break;
                    }
                }
            }
        }
    }

    bool
    USNSwitch::PacketReceive(Ptr<NetDevice> device, Ptr<const Packet> pck_rcv, uint16_t protocol, const Address& sourceAddress) {

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

        Ptr<Node> nd = this->GetNode();

        Ptr<Packet> pck_cp = pck_rcv->Copy();

        USNHeader hd;
        pck_cp->RemoveHeader(hd);

//        NodeRef nr;
//        nr.x = 30;
//        nr.y = 44;

        PacketsReceived[P_TOTAL]++;
        //TODO: replace all this by callbacks
        

        uint8_t usn_protocol = hd.GetUSNProtocol();

        uint8_t ad = device->GetObject<USNNetDevice>()->GetUSNAddress(); //Get the address of the device which generated the interruption

        if (ad == 1) { //came from the right
            hd.CurrentX -= 1;
            //            hd.OriginX += 1; //only in this case that this header will contain this field
            //PortOfOrigin = 0b00000001;
        } else if (ad == 2) {
            hd.CurrentY -= 1;
            //            hd.OriginY += 1;
            //PortOfOrigin = 0b00000010;
        } else if (ad == 3) {
            hd.CurrentX += 1;
            //            hd.OriginX -= 1;
            //PortOfOrigin = 0b00000100;
        } else if (ad == 4) {
            hd.CurrentY += 1;
            //            hd.OriginY -= 1;
            //PortOfOrigin = 0b00001000;
        }
//        hd.HopsCount += 1;
        pck_cp->AddHeader(hd);

        m_switchRxTrace(pck_cp); //trance the packet after changing 
        
        uint8_t origin_port = 0b00000001 << (ad - 1);



        if (usn_protocol == P_NETWORK_DISCOVERY) {
            PacketsReceived[P_NETWORK_DISCOVERY]++;
            Ptr<USNApp> m_app = this->GetNode()->GetApplication(0)->GetObject<USNApp>();
            m_app->NetworkDiscoveryReceived(pck_cp, origin_port);

        } else if (usn_protocol == P_VALUE_ANNOUNCEMENT) {
            PacketsReceived[P_VALUE_ANNOUNCEMENT]++;
            Ptr<USNApp> m_app = this->GetNode()->GetApplication(0)->GetObject<USNApp>();
            m_app->ValueAnnoucementReceived(pck_cp, origin_port);

        } else if (usn_protocol == P_EVENT_ANNOUNCEMENT) {
            PacketsReceived[P_EVENT_ANNOUNCEMENT]++;

            Ptr<USNApp> m_app = this->GetNode()->GetApplication(0)->GetObject<USNApp>();
            m_app->EventAnnoucementReceived(pck_cp, origin_port);
        }


        return true;
    }
}

