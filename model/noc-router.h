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

#define DIRECTION_E 1 //east
#define DIRECTION_S 2 //south
#define DIRECTION_W 3 //west
#define DIRECTION_N 4 //north



#define DIRECTION_MASK_ALL  0b00001111
#define DIRECTION_MASK_E    0b00000001 //east
#define DIRECTION_MASK_S    0b00000010 //south
#define DIRECTION_MASK_W    0b00000100 //west
#define DIRECTION_MASK_N    0b00001000 //north

//#define DESYNCRONIZE

using namespace std;
namespace ns3 {

    class NOCRouter : public Application {
    public:

        static TypeId GetTypeId (void);
        
//        const static uint8_t NumNetDevices = 4;
        
        
        bool IsSink;
        uint32_t MaxHops;
        Time TimeStartOffset;
//        uint32_t ValueAnnouncementsDone;
//        vector <uint32_t> PacketsReceived; //allocate for the number of protocols used, with the value 0 initialized on it
//        vector <uint32_t> PacketsSent;
//        
//        vector < stringstream* > PacketTrace;
        TracedValue< stringstream* > PacketTrace;
        

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
         * Set an attribute value of a network general parameter.
         *
         * \param name the name of the attribute to set
         * \param value the value of the attribute to set
         *
         * These parameters are the required info for the helper to create a grid of nodes
         */
        void SetAttribute (std::string name, const uint8_t value);
        
        
    private:

//        TracedCallback<Ptr<const Packet>, NodeRef > m_routerRxTrace;
//        TracedCallback<Ptr<const Packet>, NodeRef > m_routerTxTrace;
        
        
        
        uint8_t RouteTo(int32_t destination_x, int32_t destination_y);

        TracedCallback<Ptr<const Packet> > m_routerRxTrace;
        TracedCallback<Ptr<const Packet> > m_routerTxTrace;
        
        virtual void StartApplication(void);
        virtual void StopApplication(void);

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
        
        std::vector<NetDeviceInfo> m_netDeviceInfoArray;
        
        uint8_t m_channelCount;
    };

}

#endif /* NOC_SWITCH_H */

