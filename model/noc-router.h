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
        
        
        //names are architecture dependant
//        enum Networks{
//            MESH_W = 0,
//            MESH_R = 1,
//            MESH_X = 2
//        };
        
        enum Priority{
            P0,
            P1
        };
        
        enum Directions{
            DIRECTION_E        = 0, //east
            DIRECTION_S        = 1, //south
            DIRECTION_W        = 2, //west
            DIRECTION_N        = 3, //north
            DIRECTION_L        = 4  //Internal, local processor
        };

        enum DirectionsMasks {
            DIRECTION_MASK_E   = 0b00000001, //east
            DIRECTION_MASK_S   = 0b00000010, //south
            DIRECTION_MASK_W   = 0b00000100, //west
            DIRECTION_MASK_N   = 0b00001000, //north
            DIRECTION_MASK_L   = 0b00010000, //north
            DIRECTION_MASK_ALL = 0b00011111,
            DIRECTION_MASK_ALL_EXCEPT_LOCAL = 0b00001111
                    
        };

        typedef struct {
            uint32_t unique_id; //not used so far
            uint8_t cluster_id;
            uint8_t network_id;
            uint8_t direction;
            bool    wait;
            bool    wait_remote;
            Ptr<NOCNetDevice> nd_pointer;
//            Ptr<Packet> pck_buffer;
//            bool pck_buffered;
        }NetDeviceInfo;
        
        NOCRouter();
        virtual ~NOCRouter();
        
        // Basic comm functions//////////////////////

        bool PacketReceived(Ptr<NetDevice> device, Ptr<const Packet> packet);
        
//        void PacketSend(Ptr<const Packet> pck, uint8_t network_id, uint8_t ports_mask);
        
        
        
        // Basic comm abstractions of the router, which actually step on the basic functions //////////////////////
        
        bool PacketUnicast (Ptr<const Packet> pck, uint8_t network_id, int32_t destination_x, int32_t destination_y);

        bool PacketMulticast (Ptr<const Packet> pck, uint8_t network_id);

        bool PacketBroadcast (Ptr<const Packet> pck, uint8_t network_id);
        
//        
        
//        void SendPacket(Ptr<const Packet> pck, uint8_t ports);
        
//        void SendSignal(uint8_t bits, uint8_t ports);
        
        void AddNetDevice(Ptr<NOCNetDevice> nd, uint8_t cluster, uint32_t network, uint8_t direction);
        
        Ptr<NOCNetDevice> GetNetDevice(uint8_t network, uint8_t direction);
        
        NetDeviceInfo GetNetDeviceInfo(Ptr<NOCNetDevice> nd);
    
        NetDeviceInfo GetNetDeviceInfo(uint8_t network, uint8_t direction);
        
        int8_t GetNetDeviceInfoIndex(uint8_t network, uint8_t direction);
        int8_t GetNetDeviceInfoIndex(Ptr<NOCNetDevice> nd);
        
        uint8_t GetNDevices(void);
      
//        void ServePackets(void);
//        void ServePacket(uint8_t in, uint8_t out);
        
//        void RemoteWaitChanged(uint8_t signal, Ptr<NOCNetDevice> nd_this, bool wait_state);
//        void LocalWaitChanged(uint8_t signal, Ptr<NOCNetDevice> nd_this, bool wait_state);
 
        
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
        typedef Callback< void, Ptr<const Packet> > ReceiveCallback;

        /**
         * \param cb callback to invoke whenever a packet has been received and must
         *        be forwarded to the higher layers.
         *
         * Set the callback to be used to notify higher layers when a packet has been
         * received.
         */
        void SetReceiveCallback(ReceiveCallback);
        
    private:

        enum RoutingAlgos{
            COLUMN_FIRST,
            ROW_FIRST,
            CLOCKWISE
        };
        
        uint8_t RouteTo(uint8_t routing_alg, int32_t destination_x, int32_t destination_y);

        TracedCallback<Ptr<const Packet> > m_routerRxTrace;
        TracedCallback<Ptr<const Packet> > m_routerTxTrace;
        TracedCallback<Ptr<const Packet> > m_routerTxDropTrace;
        
        virtual void StartApplication(void);
        virtual void StopApplication(void);
        
//        bool PacketForward(Ptr<const Packet> pck, 
//            uint8_t network_id, uint8_t originPort, uint8_t destinationPort, uint8_t priority);
        
        bool PacketSendMultiple(Ptr<const Packet> pck, uint8_t network_id, uint8_t ports_mask, uint8_t priority);
        bool PacketSendSingle(Ptr<const Packet> pck, Ptr<NOCNetDevice> nd, uint8_t priority);
        
        ReceiveCallback m_receiveCallBack;

        bool m_running;
        EventId m_sendEvent;
        
        NetDeviceContainer m_netDevices;
        
        int32_t m_addressX, m_addressY;
        
        std::vector<NetDeviceInfo> m_netDeviceInfoArray;
        
        uint8_t m_port_to_serve;
        
        uint8_t m_channelCount;
    };

}

#endif /* NOC_SWITCH_H */

