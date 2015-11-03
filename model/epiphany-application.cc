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
//#include "ns3/calc.h"

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
        
//        m_router->SetReceiveCallback(MakeCallback(&EpiphanyApp::DataReceived, this));
        
//                this->WriteData(3, 3);
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
    EpiphanyApp::ScheduleDataWrites(uint64_t n_times, Time period, int32_t dest_x, int32_t dest_y) {

        for (uint8_t i = 0; i < n_times; i++) {
            Time t = PicoSeconds(period.GetPicoSeconds() * i + period.GetPicoSeconds());
            Simulator::Schedule(t, &EpiphanyApp::WriteData, this, dest_x, dest_y);
        }
        
//        this->WriteData(dest_x, dest_y);

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
    EpiphanyApp::WriteData(int32_t dest_x, int32_t dest_y){
        
        EpiphanyHeader h;
        uint8_t data[8];
        data[0] = 77;
        
        h.SetRWMode(EpiphanyHeader::RW_MODE_WRITE);
        h.SetCrtlMode(EpiphanyHeader::CRTL_MODE_0A);
        h.SetDataMode(EpiphanyHeader::DATA_MODE_64b);
        h.SetDestinationAddressXY(dest_x, dest_y);
        h.SetPacketData(data, 1);
        
        Ptr<Packet> pck = Create<Packet>();
        pck->AddHeader(h);
        
//        for (uint8_t i = 0 ; i < 10 ; i++)
        m_router->PacketUnicast(pck, 0, dest_x, dest_y, true);
        
//        std::cout << "Sent from: "<< Ntow() << ", " << this << endl;
    }
    
//    EpiphanyApp::WriteData(Ptr<const Packet> pck){
//    
//    }
    
      
    void
    EpiphanyApp::DataReceived(Ptr<const Packet> pck, uint16_t direction){ 
        EpiphanyHeader h;
        pck->PeekHeader(h);
        
        if (h.GetRWMode() == EpiphanyHeader::RW_MODE_WRITE){
            this->WriteDataReceived(pck);
        }
        else if (h.GetRWMode() == EpiphanyHeader::RW_MODE_READ){
            this->ReadDataReceived(pck);
        }
            
    }
    
    void
    EpiphanyApp::WriteDataReceived(Ptr<const Packet> pck){
        static Time ti = PicoSeconds(0);
        static Time tf = PicoSeconds(0);
        static double_t packet_count = 0;
        
        packet_count++;
        
        Time tnow = Simulator::Now();
        
        if (ti == PicoSeconds(0)){
            ti = tnow;
        }
        if (tnow > tf){
            tf = tnow;
            m_totalThroughput = (packet_count * 8) / (tf - ti).ToDouble(Time::US); // *8 is because each packet has 8 bytes for data. output in MB/s

            std::cout << "Packets count = " << packet_count <<", Total throughput = " << m_totalThroughput << " MB/s" << endl;            
        }    
    }   
    
    double_t
    EpiphanyApp::GetTotalThroughput(void){
        return m_totalThroughput;
    }
    
    void
    EpiphanyApp::ReadDataReceived(Ptr<const Packet> pck){
        
        std::cout << "Read data packet successfully received." << endl;
    }
        
}

    

    


