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


#include "ns3/noc-net-device.h"
#include "ns3/noc-router.h"
#include "ns3/calc.h"

#include "noc-types.h"
#include "epiphany-application.h"
#include "epiphany-header.h"
#include "ns3/epiphany-header.h"

//using namespace ns3NOCCalc;
using namespace std;
namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("EpiphanyApplication");

    EpiphanyApp::EpiphanyApp()
    :
//    m_packetSize(0),
    m_running(false)
    //m_packetsSent(0),
    //m_packetsReceived(0)
    {
    }

    EpiphanyApp::~EpiphanyApp() {
        //  m_socket = 0;
    }

    void
    EpiphanyApp::StartApplication(void) {
        m_running = true;
        
        //ScheduleValueAnnouncement(SamplingCycles, Time::FromInteger(SamplingPeriod, Time::US));
        
        m_router->SetReceiveCallback(MakeCallback(&EpiphanyApp::DataReceived, this));
    }

    void
    EpiphanyApp::StopApplication(void) {
        m_running = false;
    }
    
    void
    EpiphanyApp::AddRouter(Ptr<NOCRouter> r) {
        m_router = r;
    }

    void
    EpiphanyApp::ScheduleDataWrites(uint8_t n_times, Time period) {

        for (uint8_t i = 0; i < n_times; i++) {
            Time t = MilliSeconds(period.GetMilliSeconds() * i + period.GetMilliSeconds());
            Simulator::Schedule(t, &EpiphanyApp::WriteData, this);
        }

    }
//
//
//    NodeRef
//    EpiphanyApp::GetNeighborAt(uint8_t i) {
//        return m_neighborsList.at(i);
//    }
//
//    uint8_t
//    EpiphanyApp::GetNeighborN(void) {
//        return m_neighborsList.size();
    
    void
    EpiphanyApp::ReadData(void){
        
        
    }
    void
    EpiphanyApp::ReadDataReceived(){
        
        
    }
    void
    EpiphanyApp::WriteData(){
        
        EpiphanyHeader h;
        
        h.SetRWMode(EpiphanyHeader::RW_MODE_WRITE);
//        h.SetCrtlMode(EpiphanyHeader::CRTL_MODE_0A);
        h.SetDataMode(EpiphanyHeader::DATA_MODE_64b);
        h.SetDestinationAddress(0);
        
        Ptr<Packet> pck = Create<Packet>();
        pck->AddHeader(h);
        
        m_router->PacketSend(pck, 0, 0xFF, 0);
        
        std::cout << "Sent from: "<< Now() << ", " << this << endl;
    }
    void
    EpiphanyApp::WriteDataReceived(){
        
        
    } 
   
    void
    EpiphanyApp::DataReceived(Ptr<const Packet>){ 
        std::cout << "Received by: " << Now() << ", " << this << endl;
    }
        
}

    

    


