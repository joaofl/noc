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
    void ReadDataReceived(Ptr<const Packet> pck);
    
    /*
     Write dumb data to an specific node given by x, y
     */
    void WriteData(int32_t dest_x, int32_t dest_y);
        
//    void WriteData(Ptr<const Packet> pck);
    
    void WriteDataReceived(Ptr<const Packet> pck);
    
    void DataReceived(Ptr<const Packet>, uint16_t direction);
    
    void AddRouter(Ptr<NOCRouter>  r);
    
    void ScheduleDataWrites(uint8_t n_times, Time period, int32_t x, int32_t y);


    private:

        virtual void StartApplication(void);
        virtual void StopApplication(void);

//        void ScheduleTx(void);

        bool m_running;
//        vector<EventRef> m_lastEvents;

        Ptr<NOCRouter> m_router;
    };

}

#endif /* EPIPHANY_APP_H */

