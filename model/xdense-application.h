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
 */

#ifndef NOC_APP_H
#define NOC_APP_H


#include <fstream>
#include <math.h>
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/data-rate.h"
#include "ns3/ptr.h"

//#include "noc-application.h"
#include "xdense-header.h"
//#include "noc-header.h"
#include "noc-net-device.h"
#include "noc-router.h"
#include "noc-types.h"
#include "sensor.h"
#include "calc.h"
#include "sensor-data-io.h"


#define USE_ABSOLUTE_ADDRESS true
#define USE_RELATIVE_ADDRESS false

namespace ns3 {

    class XDenseApp : public Application {
    public:

        
        enum NetworkId {
            NETWORK_ID_0 = 0,
            NETWORK_ID_1 = 1,
            NETWORK_ID_2 = 2
        };

        

        bool IsSink;
        Time PacketDuration;
        uint8_t ClusterSize_x, ClusterSize_y; 
        

        Ptr <NOCOutputData> SinkReceivedData;
        
        XDenseApp();
        virtual ~XDenseApp();

        void Setup(bool IsSink);
        
        void Tests(void);
        
        void DataReceived(Ptr<const Packet> pck, uint8_t protocol, int32_t origin_x,int32_t origin_y, int32_t dest_x,int32_t dest_y);
        
        void NetworkSetup();
        bool NetworkSetupReceived(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y);

        void DataSharingRequest();
        void DataSharingRequestReceived(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y);
        
        void DataAnnouncement(int32_t x_dest, int32_t y_dest);
        bool DataAnnoucementReceived(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y);
        
        void ClusterDataRequest();
        void ClusterDataResponse(int32_t x_dest, int32_t y_dest);
        bool ClusterDataRequestReceived(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y, int32_t size_x, int32_t size_y);
        
//        void DataSharingSchedule(uint8_t n_times, Time period);
//        void DataSharing();
//        bool DataSharingReceived(Ptr<const Packet> pck, int32_t origin_x, int32_t origin_y);
        //        
        void AddRouter(Ptr<NOCRouter> r);
//        NodeRef NearestClusterHead(void);
//        void ScheduleTransmission(Ptr<const Packet> pck, uint8_t destination_port);
//        NodeRef GetSinkAt(uint8_t);
//        uint8_t GetSinkN(void);
//        NodeRef GetNeighborAt(uint8_t);
//        uint8_t GetNeighborN(void);
//        IsPresent(vector<m_sink_type> *, m_sink_type *);
//        uint32_t packetsReceived;

    private:
        
        typedef struct {
            int32_t x, y;
//            uint32_t value;
//            uint32_t serial_number; 
//            bool updated;
        } node_ref;
        
//        node_ref cluster_data_request_remetent;


        virtual void StartApplication(void);
        virtual void StopApplication(void);

        void CalculateTimeSlot(void);
        uint8_t DetectEvents(void);
        
        bool m_running;
//        uint32_t m_baudrate;
//        uint32_t m_packetSize;
//        uint32_t m_packetDuration;
        Ptr<NOCRouter> m_router;
//        std::vector<NodeRef> m_sinksList;
        

        
        
//        std::vector<NodeRef> m_neighborsList;
//        uint32_t m_nPackets;
//        vector <uint32_t> m_SerialNumber; //Serial NUmber
//        DataRate m_dataRate;
//        vector<EventRef> m_lastEvents;
    };

}

#endif /* NOC_APP_H */

