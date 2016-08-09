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

#ifndef NOC_ROUTER_H
#define NOC_ROUTER_H


#include <fstream>
#include <sstream>

#include "ns3/callback.h"
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/address.h"
#include "ns3/node.h"
//#include "ns3/data-rate.h"
#include "ns3/ptr.h"
#include "ns3/net-device-container.h"
#include "ns3/object.h"
#include "ns3/queue.h"

#include "noc-header.h"
#include "xdense-header.h"
#include "noc-net-device.h"
#include "noc-routing.h"
#include "data-io.h"

using namespace std;
namespace ns3 {

    class RoundRobinState {
    public:
        NOCRouting::Directions m_actual_port;

        uint8_t m_E, m_W, m_S, m_N, m_L;
        uint8_t m_EC, m_WC, m_SC, m_NC, m_LC;
        
        

        void Start(void);
        void NextPort(void);
    private:
        
    };

    class NOCRouter : public Application {
    public:

        static TypeId GetTypeId (void);
        
        enum Priority{
            P0,
            P1
        };
        
        enum ServerPolicy{
            FIFO,
            ROUND_ROBIN
        };
        
        ServerPolicy ServerPolicy;

        typedef struct {
            uint32_t unique_id; //not used so far
            uint8_t cluster_id;
            uint8_t network_id;
            uint8_t direction;
            bool    wait;
            bool    wait_remote;
            Ptr<NOCNetDevice> nd_pointer;
        }NetDeviceInfo;
        
        Ptr<NOCRouterDelayModel> RoutingDelays;
        
        NOCRouter();
        virtual ~NOCRouter();
        
        // Basic comm functions//////////////////////
        void PacketTrace(Ptr<const Packet> packet, Ptr<NOCNetDevice> device);
        
//        void PacketServe(Ptr<const Packet> packet, Ptr<NOCNetDevice> device);
        void PacketServe(Ptr<const Packet> packet, NOCRouting::Directions dir);
        
        
        bool PacketUnicast (Ptr<const Packet> pck, uint8_t network_id, int32_t destination_x, 
                            int32_t destination_y, bool absolute_address);
        
        bool PacketUnicastOffset (Ptr<const Packet> pck, uint8_t network_id, 
                                    int32_t destination_x, int32_t destination_y);

        bool PacketMulticastRadius (Ptr<const Packet> pck, uint8_t network_id, uint8_t n_hops);
        bool PacketMulticastArea (Ptr<const Packet> pck, uint8_t network_id, int32_t x_size, int32_t y_size);
        bool PacketMulticastIndividuals (Ptr<const Packet> pck, uint8_t network_id, int32_t x_position, int32_t y_position);
//        bool PacketMulticastReceived (Ptr<const Packet> pck, uint8_t network_id);

        bool PacketBroadcast (Ptr<const Packet> pck, uint8_t network_id);
//        bool PacketBroadcastReceived (Ptr<const Packet> pck, uint8_t network_id);
        
        void SetRoutingProtocolUnicast(NOCRouting::RoutingProtocols);
        
        void AddNetDevice(Ptr<NOCNetDevice> nd, uint8_t cluster, uint32_t network, uint8_t direction);
        
        Ptr<NOCNetDevice> GetNetDevice(uint8_t network, uint8_t direction);
        Ptr<NOCNetDevice> GetNetDevice(uint8_t i);
        
        Ptr<Queue> GetNetDeviceQueue(uint8_t network, uint8_t direction);
        
        NetDeviceInfo GetNetDeviceInfo(Ptr<NOCNetDevice> nd);
    
        NetDeviceInfo GetNetDeviceInfo(uint8_t network, uint8_t direction);
        
        int8_t GetNetDeviceInfoIndex(uint8_t network, uint8_t direction);
        int8_t GetNetDeviceInfoIndex(Ptr<NOCNetDevice> nd);
        
        uint8_t GetNDevices(void);
        
        Time GetTransmissionTime(Ptr<const Packet> pck);
        
        void SetDataRate(DataRate bps);
        
        void SetBurstiness(double_t b, uint8_t rd);
     
        
        //            void
        //    NOCNetDevice::SetAddress(Address address) {
        ////        m_address = Mac48Address::ConvertFrom(address);
        //        m_address = NOCAddress::ConvertFrom(address);
        //    }
        //
        //    Address
        //    NOCNetDevice::GetAddress(void) const {
        //        return m_address;
        //    }
 
        
        /**
         * \param device a pointer to the net device which is calling this callback
         * \param packet the packet received
         * \param protocol the 16 bit protocol number associated with this packet.
         *        This protocol number is expected to be the same protocol number
         *        given to the Send method by the user on the sender side.
         * \param sender the address of the sender
         * \returns true if the callback could handle the packet successfully, false
         *          otherwise.
         */
        typedef Callback< void, Ptr<const Packet>, uint8_t, int32_t, int32_t, int32_t, int32_t> ReceiveCallback;

        /**
         * \param cb callback to invoke whenever a packet has been received and must
         *        be forwarded to the higher layers.
         *
         * Set the callback to be used to notify higher layers when a packet has been
         * received.
         */
        void SetReceiveCallback(ReceiveCallback cb);
        
    private:
        
        TracedCallback<Ptr<const Packet> > m_routerRxTrace;
        TracedCallback<Ptr<const Packet> > m_routerTxTrace;
        TracedCallback<Ptr<const Packet> > m_routerCxTrace;
        TracedCallback<Ptr<const Packet> > m_routerGxTrace;
        TracedCallback<Ptr<const Packet> > m_routerTxDropTrace;
        TracedCallback< uint32_t > m_routerTxQueueSize;
        
        virtual void StartApplication(void);
        virtual void StopApplication(void);
        
        void ConsumePacket(Ptr<const Packet> packet, Ptr<NOCNetDevice> device);

        void RoundRobin(void);
        void NextPort(void);
        

        uint8_t Transmit(Ptr<const Packet> pck, uint8_t network_id, uint8_t ports_mask, uint8_t priority);
        bool TransmitSingle(Ptr<const Packet> pck, uint8_t network_id, uint8_t ports_mask, uint8_t priority);
        
        
        ReceiveCallback m_receiveCallBack;

        bool m_running;
        EventId m_sendEvent;
        
        NetDeviceContainer m_netDevices;
        
        int32_t m_addressX, m_addressY;
        
        std::vector<NetDeviceInfo> m_netDeviceInfoArray;
        
        uint8_t m_channelCount;
        
        NOCRouting::RoutingProtocols m_routing_conf;
        
        ObjectFactory m_queueFactory;
        Ptr<Queue> m_queue_app_output;
        
        DataRate m_bps;
        
        double_t m_burstiness;
        
        enum server_state_machine{
            IDLE,
            BUSY
        };
        
        server_state_machine m_server_state;
        
        RoundRobinState m_input_ports, rr_n, rr_w, rr_s;
        
    };
    
    
    

}

#endif /* NOC_SWITCH_H */

