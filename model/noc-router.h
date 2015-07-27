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

#ifndef NOC_SWITCH_H
#define NOC_SWITCH_H


#include <fstream>
#include <sstream>

#include "ns3/callback.h"
//#include "ns3/object.h"
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/data-rate.h"
#include "ns3/ptr.h"
#include "ns3/net-device-container.h"

#include "noc-net-device.h"
#include "noc-router.h"
#include "noc-types.h"

using namespace std;
namespace ns3 {

    class NOCRouter : public Application {
    public:

        static TypeId GetTypeId (void);
        
//        const static uint8_t NumNetDevices = 4;
        
        
//        bool IsSink;
//        uint32_t MaxHops;
//        Time TimeStartOffset;
//        TracedValue< stringstream* > PacketTrace;
        
        enum Directions{
        DIRECTION_E        = 1, //east
        DIRECTION_S        = 2, //south
        DIRECTION_W        = 3, //west
        DIRECTION_N        = 4 //north
        };

        enum DirectionsMasks {
        DIRECTION_MASK_ALL = 0b00001111,
        DIRECTION_MASK_E   = 0b00000001, //east
        DIRECTION_MASK_S   = 0b00000010, //south
        DIRECTION_MASK_W   = 0b00000100, //west
        DIRECTION_MASK_N   = 0b00001000 //north
        };

        
        NOCRouter();
        virtual ~NOCRouter();
        
        // Basic comm functions//////////////////////

        bool PacketReceived(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, const Address& sourceAddress);
        
//        void PacketSend(Ptr<const Packet> pck, uint8_t network_id, uint8_t ports_mask);
        
        void PacketSend(Ptr<const Packet> pck, uint8_t network_id, uint8_t ports_mask, uint8_t priority);
        
        // Basic comm abstractions of the router, which actually step on the basic functions //////////////////////
        
        void PacketUnicast (Ptr<const Packet> pck, uint8_t network_id, int32_t destination_x, int32_t destination_y);

        void PacketMulticast (Ptr<const Packet> pck, uint8_t network_id, uint8_t hops);

        void PacketBroadcast (Ptr<const Packet> pck, uint8_t network_id);
        
//        
        
//        void SendPacket(Ptr<const Packet> pck, uint8_t ports);
        
//        void SendSignal(uint8_t bits, uint8_t ports);
        
        void AddNetDevice(Ptr<NOCNetDevice> nd, uint8_t cluster, uint32_t x, uint32_t y, uint32_t network, uint8_t direction);
        
        Ptr<NOCNetDevice> GetNetDevice(uint8_t network, uint8_t direction);
        
        
        uint8_t GetNetDeviceInfo(Ptr<NOCNetDevice> nd);
    
        uint8_t GetNDevices(void);
        
        
        
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
        typedef Callback< void, Ptr<const Packet>, uint16_t > ReceiveCallback;

        /**
         * \param cb callback to invoke whenever a packet has been received and must
         *        be forwarded to the higher layers.
         *
         * Set the callback to be used to notify higher layers when a packet has been
         * received.
         */
        void SetReceiveCallback(ReceiveCallback);
        
    private:

        uint8_t RouteTo(int32_t destination_x, int32_t destination_y);

        TracedCallback<Ptr<const Packet> > m_routerRxTrace;
        TracedCallback<Ptr<const Packet> > m_routerTxTrace;
        
        virtual void StartApplication(void);
        virtual void StopApplication(void);
        
        ReceiveCallback m_receiveCallBack;

        bool m_running;
        EventId m_sendEvent;
        
        NetDeviceContainer m_netDevices;
        
        typedef struct {
            uint8_t container_index;
            uint8_t cluster_id;
            int32_t x; 
            int32_t y;
            uint8_t network_id;
            uint8_t direction;
        }NetDeviceInfo;
        
        int32_t m_addressX, m_addressY;
        
        std::vector<NetDeviceInfo> m_netDeviceInfoArray;
        
        uint8_t m_channelCount;
    };

}

#endif /* NOC_SWITCH_H */

