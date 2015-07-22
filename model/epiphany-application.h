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

#ifndef EPIPHANY_APP_H
#define EPIPHANY_APP_H


#include <fstream>
#include <math.h>
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/data-rate.h"
#include "ns3/ptr.h"

#include "epiphany-header.h"
#include "noc-net-device.h"
#include "noc-router.h"
#include "noc-types.h"


////MESH snumeration
//enum{
//    C_MESH,
//    R_MESH,
//    X_MESH
//    
//};

namespace ns3 {

    class EpiphanyApp : public Application {
    public:

    EpiphanyApp();
    virtual ~EpiphanyApp();

    void ReadData(void);
    void ReadDataReceived(void);
    void WriteData(void);
    void WriteDataReceived(void);


    private:

        virtual void StartApplication(void);
        virtual void StopApplication(void);

//        void ScheduleTx(void);
//        //void SendPacket(void);
//
//        uint8_t DetectEvents(void);

        //void SendPacket(uint8_t ports, Ptr<Packet> pck);



//        std::vector<NodeRef> m_sinksList;
//        std::vector<NodeRef> m_neighborsList;
//
//        uint32_t m_nPackets;
//        vector <uint32_t> m_SerialNumber; //Serial NUmber
//        DataRate m_dataRate;
        bool m_running;
//        vector<EventRef> m_lastEvents;

        Ptr<NOCRouter> m_router;
    };

}

#endif /* EPIPHANY_APP_H */

