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

//        TimeStartOffset = Seconds(1);

        m_router = this->GetNode()->GetApplication(INSTALLED_NOC_SWITCH)->GetObject<NOCRouter>();

//        ScheduleValueAnnouncement(SamplingCycles, Time::FromInteger(SamplingPeriod, Time::US));
    }

    void
    EpiphanyApp::StopApplication(void) {
        m_running = false;
    }

//    void
//    EpiphanyApp::ScheduleValueAnnouncement(uint8_t n_times, Time period) {
//
//        for (uint8_t i = 0; i < n_times; i++) {
//            Time t = MilliSeconds(period.GetMilliSeconds() * i + TimeStartOffset.GetMilliSeconds() + period.GetMilliSeconds());
//            Simulator::Schedule(t, &EpiphanyApp::ValueAnnouncement, this);
//        }
//
//    }
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
        
        
    }
    void
    EpiphanyApp::WriteDataReceived(){
        
        
    } 
   
    
    }

    


