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

#include "xdense-application.h"
#include "xdense-header.h"
#include "noc-net-device.h"
#include "noc-router.h"
#include "calc.h"
#include "sensor-data-io.h"
#include "noc-types.h"
#include "src/core/model/object-base.h"

//using namespace ns3NOCCalc;
using namespace std;
namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("XDenseApplication");

    XDenseApp::XDenseApp()
    :
    //    m_packetSize(0),
    m_nPackets(0),
    m_dataRate(0),
    //    m_sendEvent(),
    m_running(false)
    //IsSink(false),
    //m_packetsSent(0),
    //m_packetsReceived(0)
    {
    }

    XDenseApp::~XDenseApp() {
        //  m_socket = 0;
    }

    //    void
    //    XDenseApp::Setup(bool IsSink) {
    //        IsSink = IsSink;
    //
    //    }

    void
    XDenseApp::StartApplication(void) {

//        TimeStartOffset = Seconds(1);
        
        //This is a threshold, to decide the min neghborhood a node have to be 
        //to be able to do event annoucements.
        //These are all the possible neighbors times a factor, to include nodes in the edges,
        //which have less neighbors.
        MinNeighborhood = MaxHops * 2 * (MaxHops + 1) * 0.7; //add some tolerance.Ex 20%

        SensorValueLast = 0;

        EventRef er;
        er.detected = false;
        er.data[0] = 0;
        er.data[1] = 0;
        er.data[2] = 0;
        m_lastEvents.assign(EV_COUNT, er);
        m_SerialNumber.assign(P_COUNT, 0);
        //        PacketTrace.assign(P_COUNT, temp);


        m_router->SetReceiveCallback(MakeCallback(&XDenseApp::DataReceived, this));

        if (IsSink == true) {
//            Simulator::Schedule(TimeStartOffset, &XDenseApp::NetworkDiscovery, this);
            SinkReceivedData = CreateObject<NOCOutputData> ();
        }
        
        Time t = Time::FromInteger(0, Time::MS);
//        if (IsSink == true)
        
        uint16_t pck_duration = 7200;
        
        IntegerValue x, y;
        m_router->GetAttribute("AddressX", x);
        m_router->GetAttribute("AddressY", y);
        
//        if (x.Get() % 2 == 0 && y.Get() % 2 == 0){
        
        for (uint8_t j = 0 ; j < 1 ; j++)
        {
            for (uint8_t i = 0 ; i < 5 ; i++){
                Simulator::Schedule(t, &XDenseApp::DataAnnouncementTT, this);
                t += Time::FromInteger(pck_duration * 1, Time::NS); //add the packet duration, to make
                //data generation periodic.
            }
//            t += Time::FromInteger((20 * pck_duration),Time::NS);// + Time::FromInteger(1, Time::MS);
        }

            

//            for (uint8_t i = 0 ; i < 1 ; i++){
//                Simulator::Schedule(t, &XDenseApp::DataAnnouncementTT, this);
//                t += Time::FromInteger(pck_duration * 1, Time::NS); //add the packet duration, to make
//                //data generation periodic.
//            }
    //        ScheduleValueAnnouncement(SamplingCycles, Time::FromInteger(SamplingPeriod, Time::US));
        
//        }
    }
    
    void
    XDenseApp::StopApplication(void) {
        m_running = false;

        //        if (m_sendEvent.IsRunning()) {
        //            Simulator::Cancel(m_sendEvent);
        //        }
    }

    void
    XDenseApp::DataSharingSchedule(uint8_t n_times, Time period) {

        for (uint8_t i = 0; i < n_times; i++) {
            Time t = MilliSeconds(period.GetMilliSeconds() * i + TimeStartOffset.GetMilliSeconds() + period.GetMilliSeconds());
            Simulator::Schedule(t, &XDenseApp::DataSharing, this);
        }

    }

    NodeRef
    XDenseApp::GetSinkAt(uint8_t i) {
        return m_sinksList.at(i);
    }

    uint8_t
    XDenseApp::GetSinkN(void) {
        return m_sinksList.size();
    }

    NodeRef
    XDenseApp::GetNeighborAt(uint8_t i) {
        return m_neighborsList.at(i);
    }

    uint8_t
    XDenseApp::GetNeighborN(void) {
        return m_neighborsList.size();
    }

    void
    XDenseApp::AddRouter(Ptr<NOCRouter> r) {
        m_router = r;
    }
    
    uint8_t
    XDenseApp::DetectEvents(void) { //gets the array with all the neighbors data
        EventRef er;
        DataFit p;
        uint8_t count = 0;
        bool updated = true;
        er.detected = false;
        
        for (uint8_t i = 1; i < m_neighborsList.size(); i++) //except myself
        {
            if (m_neighborsList.at(i).updated == false)
            {
                updated = false;
                break;
            }
        }
        
        if (m_neighborsList.size() >= MinNeighborhood && updated == true) {

            if (OperationalMode == 0) //to collect data from all nodes
            {
                if (IsClusterHead == true) {

                    if (m_neighborsList.size() > 1)
                    {
                        p = NOCCalc::FindPlane(&m_neighborsList);
//                        p = NOCCalc::FindCurve(&m_neighborsList);
                    }
                        
                    else if(m_neighborsList.size() == 1)
                    {
                        p.a = 0;
                        p.b = 0;
                        p.c = m_neighborsList.at(0).value;
                    }

                    er.type = p.type;
                    er.data[0] = (uint16_t) (p.a + 32767); //offset them and and make positive
                    er.data[1] = (uint16_t) (p.b + 32767);
                    er.data[2] = (uint16_t) p.c;
                    er.detected = true;
                    //                er.value = p.A + p.B + p.C;
                    //                er.plane = p;

                    //                10% margin of variation from the previous to the actual detection
                    int32_t v = m_lastEvents.at(p.type).data[2];
                    if (er.data[2] > v + v * 0.2 || er.data[2] < v - v * 0.2) {

                        DataAnnouncement(er);
                        m_lastEvents.at(p.type) = er;
                        count++;
                    }
                }
            }
            else if (OperationalMode == 1) {
                //////// DETECT Edges /////////////
                float g = NOCCalc::FindGrad(&m_neighborsList);
                DataFit p;
//                float w = 1;
//                float c = 8.4;
//                uint32_t tl = 28000 / ((c / MinNeighborhood) * w);
//                uint32_t th = 55000 * ((c / MinNeighborhood) * w);
                
//                uint32_t tl = 28000; //best found for n_hops=2 and N = 12 * 0.8% = 9.6
//                uint32_t th = 55000;
                
//                uint32_t tl = 35000; //best found for n_hops=4 and N = 40 * 0.8 = 32
//                uint32_t th = 45000;                
                
//                finding a line that matches this values for tl
//                -8.4 a 	- b 	= -28000
//                -28 a 	- b 	= -35000 
                
//                { a = 357.1428571428571, b = 25000.0 }
                
//                finding a line that matches this values for th
                    //-8.4 a 	- b 	= -55000
                    //-28 a 	- b 	= -45000 
                
//                { a = -510.204081632653, b = 59285.71428571428 }
                
                uint32_t tl = 357.14 * MinNeighborhood + 25000; //best found for n_hops=4 and N = 40 * 0.8 = 32
                uint32_t th = -510.20 * MinNeighborhood + 59285.71;
                

                if (g > tl && g < th) {
                    er.data[0] = g;
//                    er.detected = true;
//                    EventsDetectedCount++;
//                    er.type = EV_DELTA;

                    //                10% margin of variation from the previous to the actual detection
//                    if (er.data[0] > m_lastEvents.at(EV_DELTA).data[0] * 1.2 || er.data[0] < m_lastEvents.at(EV_DELTA).data[0] * 0.8) {
//                        m_lastEvents.at(EV_DELTA) = er;
                        

                        if (m_neighborsList.size() > 1)
                        {
                            p = NOCCalc::FindPlane(&m_neighborsList);
//                            c = NOCCalc::FindCurve(&m_neighborsList);
                        }
                        else if(m_neighborsList.size() == 1)
                        {
                            p.a = 0;
                            p.b = 0;
                            p.c = m_neighborsList.at(0).value;
                        }

                        er.type = p.type;
                        er.data[0] = (uint16_t) (p.a + 32767); //offset them and and make positive
                        er.data[1] = (uint16_t) (p.b + 32767);
                        er.data[2] = (uint16_t) p.c;
                        er.detected = true;
                        
                        int32_t v = m_lastEvents.at(p.type).data[2];
                        if (er.data[2] > v + v * 0.2 || er.data[2] < v - v * 0.2) {

                        DataAnnouncement(er);
                        m_lastEvents.at(p.type) = er;
                        count++;
                        }
//                    }
                }
            }
        }
        
        if (count > 0){ //event was sent
            for (uint8_t i = 1; i < m_neighborsList.size(); i++) 
            {
                m_neighborsList.at(i).updated = false;
            }
        }
        return count;
    }



    NodeRef XDenseApp::NearestClusterHead(void) {
        NodeRef nr;
        
        if (MaxHops == 0) //if I'm reading from all nodes, there is no cluster head
        {
            nr.x = 0;
            nr.y = 0;
            return nr;
        }
        
        uint8_t rx = abs(m_sinksList.at(0).x) % (MaxHops);
        uint8_t ry = abs(m_sinksList.at(0).y) % (MaxHops);
        uint8_t rxy = (abs(m_sinksList.at(0).y) + abs(m_sinksList.at(0).x)) % (MaxHops * 2);

        if (rx + ry + rxy == 0) // select only intercalated nodes. I am a cluster head
        {
            nr.x = 0;
            nr.y = 0;
            return nr;
        }
        else
        {
            for (int8_t x = MaxHops*-1; x <= int8_t(MaxHops); x++) {
                for (int8_t y = MaxHops*-1; y <= int8_t(MaxHops); y++) {
                    if (abs(x) + abs(y) <= MaxHops) {
                        rx = abs(m_sinksList.at(0).x + x) % (MaxHops);
                        ry = abs(m_sinksList.at(0).y + y) % (MaxHops);
                        rxy = (abs(m_sinksList.at(0).y + y) + abs(m_sinksList.at(0).x + x)) % (MaxHops * 2);
            
                        if (rx + ry + rxy == 0) {
                            nr.x = x*-1;
                            nr.y = y*-1;
                            return nr;
                        }
                    }
                }
            }
        }
        return nr;
    }

    uint32_t packets_received_count;
    void 
    XDenseApp::DataReceived(Ptr<const Packet> pck, uint16_t direction) {
        IntegerValue x, y;
//        m_router->GetAttribute("AddressX", x);
//        m_router->GetAttribute("AddressY", y);
//        packets_received_count++;
//        cout << "Received at:" << (int) x.Get() << "," << (int) y.Get() << "Total: " << packets_received_count <<endl;
    }
    
    
    void
    XDenseApp::NetworkDiscovery() {

        if (IsSink == true) {

            NodeRef me;
            me.x = 0;
            me.y = 0;
            m_sinksList.push_back(me);

            XDenseHeader hd;
            hd.CurrentX = 0;
            hd.CurrentY = 0;
            hd.SetNOCProtocol(P_NETWORK_DISCOVERY);
            hd.OperationalMode = OperationalMode;
            hd.SerialNumber = m_SerialNumber.at(P_NETWORK_DISCOVERY);
            m_SerialNumber.at(P_NETWORK_DISCOVERY)++;

            Ptr<Packet> pck = Create<Packet>();
//            pck->AddHeader(hd);
            
            
          m_router->PacketBroadcast(pck, 0);
//            m_router->PacketUnicast(pck, 0, 2, 2);
            
        }

    }

    bool
    XDenseApp::NetworkDiscoveryReceived(Ptr<const Packet> pck, uint8_t origin_port) {


        //if (protocol == 1){ 
        // && m_IsSink == false) { //Network
        //            uint32_t id = nd->GetId();

        //            if (id == 43)
        //                cout << id;

        XDenseHeader hd;
        pck->PeekHeader(hd);

        bool IsPresent = false;
        for (uint8_t i = 0; i < m_sinksList.size(); i++) {
            if (m_sinksList.at(i).x == hd.CurrentX && m_sinksList.at(i).y == hd.CurrentY) { //check to see if the sink
                //is already in the list
                IsPresent = true;
                break;
            }

            //TODO: Redundant packets ++
        }

        if (IsPresent == false) {
            //            if (m_sinksList.size() == 0)
            //                ScheduleValueAnnouncement(SamplingCycles, Time::FromInteger(SamplingPeriod, Time::US)); //Schedule the values announcements once, 
            //Simulator::Schedule(Seconds(1), &XDenseApp::EventAnnouncement, this, 700);

            //when the first sinks is discovered

            NodeRef sink;
            sink.x = hd.CurrentX;
            sink.y = hd.CurrentY;
            m_sinksList.push_back(sink);
            OperationalMode = hd.OperationalMode;

            //Check if it is cluster head
            NodeRef ch = NearestClusterHead();
            if (ch.x == 0 && ch.y == 0) //if I am a cluster head
                IsClusterHead = true;
            else
                IsClusterHead = false;

//            m_router->PacketSend(pck, 0, NOCRouter::DIRECTION_MASK_ALL & (~origin_port), 0);
        }

        return true;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void
    XDenseApp::DataSharing(void) {


        //        uint32_t id = this->GetNode()->GetId();
        //        if (id == 100) { //for debuging reasons
        //            cout << "value announced by n100";
        //        }

        if (IsSink == false) {
            Ptr<Sensor> sensor = this->GetNode()->GetApplication(INSTALLED_SENSOR)->GetObject<Sensor>();

            XDenseHeader hd;
            hd.CurrentX = 0;
            hd.CurrentY = 0;
            hd.SensorValue = sensor->ReadSensor();
            hd.SerialNumber = m_SerialNumber.at(P_VALUE_ANNOUNCEMENT);
            SensorValueLast = hd.SensorValue;

            m_SerialNumber.at(P_VALUE_ANNOUNCEMENT)++; //one serial number per protocol is kept

            hd.SetNOCProtocol(P_VALUE_ANNOUNCEMENT);


            //for internal reference of the node app
            NodeRef me;
            me.x = hd.CurrentX;
            me.y = hd.CurrentY;
            me.serial_number = hd.SerialNumber;
            me.value = hd.SensorValue; //replace with function that call the sensor model

            //            m_neighborsList.clear();

            bool IsPresent = false;
            for (uint8_t i = 0; i < m_neighborsList.size(); i++) { //only get in here if there are elements already
                if (m_neighborsList.at(i).x == hd.CurrentX && m_neighborsList.at(i).y == hd.CurrentY) { //check to see if the sink
                    //is already in the list
                    IsPresent = true;

                    if (m_neighborsList.at(i).serial_number != hd.SerialNumber) {
                        m_neighborsList.at(i).value = hd.SensorValue; //update the sensor reading only, in the case it was in the list
                        m_neighborsList.at(i).serial_number = hd.SerialNumber;
                    }

                    break;
                }
            }

            if (IsPresent == false) {
                m_neighborsList.push_back(me);
            }

            Ptr<Packet> pck = Create<Packet>();
            pck->AddHeader(hd);
            //Start by sending to All Ports

            if (MaxHops > 0) {
                if (OperationalMode == 1) { //event detection
//                    m_router->PacketSend(pck, DIRECTION_ALL);
                    m_router->PacketMulticast(pck, 0);
                    
                } 
                else if (OperationalMode == OP_READ_ALL && IsClusterHead == 0) { //read all, send to cluster head only
//                    uint8_t port = RouteTo(NearestClusterHead()); port=port;
//                    m_router->PacketSend(pck, DIRECTION_ALL);
                    m_router->PacketMulticast(pck, 0);
                }
            }

            else if (MaxHops == 0 && m_SerialNumber.at(P_VALUE_ANNOUNCEMENT) > 1){ //skip the first round for 
                DetectEvents();
            }
        }
    }


    bool
    XDenseApp::DataSharingReceived(Ptr<const Packet> pck, uint8_t origin_port) {

        bool IsPresent = false;
        uint8_t i;

        uint8_t id = this->GetNode()->GetId();
        id = id; //avoid the compiling error for unused variable

        XDenseHeader hd;
        pck->PeekHeader(hd);
        
        uint8_t hops_count = abs(hd.CurrentX) + abs(hd.CurrentY);

        for (i = 0; i < m_neighborsList.size(); i++) {
            if (m_neighborsList.at(i).x == hd.CurrentX && m_neighborsList.at(i).y == hd.CurrentY) { //check to see if the neighbor
                //is already in the list
                //if present, it saves the index i to the last function
                IsPresent = true;
                break;
            }
        }

        if (IsPresent == false) { //only happens in the very first round of values announcements
            //while the nodes are still adding the neighbor to its list
            NodeRef remote_node;
            remote_node.x = hd.CurrentX;
            remote_node.y = hd.CurrentY;
            remote_node.value = hd.SensorValue; //filled with dummy data in order to distinguish the first cycle
            remote_node.serial_number = hd.SerialNumber;
            remote_node.updated = false;

            m_neighborsList.push_back(remote_node);

            //            uint8_t a = m_neighborsList.size();
            //            if (a > 39 && this->GetNode()->GetId() == 1400)
            //            {
            //                a++;
            //            }
            
            if (hops_count < MaxHops) { //forward the packet

                if (OperationalMode == 1) { //event detection
//                    m_router->PacketSend(pck, 0, NOCRouter::DIRECTION_MASK_ALL & (~origin_port), 0);
                }
                if (OperationalMode == 0 && IsClusterHead == 0) { //read all, send to cluster head only
//                    uint8_t port = RouteTo(NearestClusterHead());port=port;
//                    m_router->PacketSend(pck, 0, NOCRouter::DIRECTION_MASK_ALL & (~origin_port), 0);
//                    m_router->PacketSend(pck, DIRECTION_ALL & (~origin_port));
                }
                
                //                with a branch-based route
                //                if (hd.CurrentY == 0) //if the packet is in the same line that it was originated from
                //                    m_router->PacketSend(pck, DIRECTION_ALL & (~origin_port));
                //                else if(hd.CurrentY > 0)
                //                    m_router->PacketSend(pck, DIRECTION_S);
                //                else if(hd.CurrentY < 0)
                //                    m_router->PacketSend(pck, DIRECTION_N);


            }

            return true;

        } else { // IsPresent == true // it is present in the list, but still 
            //can be a redundant file or a new packed.
            // i check it, and only if it is new, I detect new events,
            // and forward it. If it is present, and old, it means
            // that it is redundant, so its dropped.

            if (m_neighborsList.at(i).serial_number != hd.SerialNumber) { //new packet received
                m_neighborsList.at(i).updated = true;
                m_neighborsList.at(i).serial_number = hd.SerialNumber; // update the serial number
                m_neighborsList.at(i).value = hd.SensorValue; //update the sensor reading
                //only, in the case it is different, 
                //if it is different, then events are
                //scanned

                //                if (hd.SensorValue != m_neighborsList.at(i).value) {
                //                    uint32_t id = this->GetNode()->GetId();
                //                    if (id == 2550) { //for debuging reasons
                //                        cout << "xxxx\n";
                //                    }

                if (hops_count < MaxHops) {

                    if (OperationalMode == 1) { //event detection
//                        m_router->PacketSend(pck, 0, NOCRouter::DIRECTION_MASK_ALL & (~origin_port), 0);
                    }
                    if (OperationalMode == 0 && IsClusterHead == 0) { //read all, send to cluster head only
//                        uint8_t port = RouteTo(NearestClusterHead());port=port;
//                        m_router->PacketSend(pck, 0, NOCRouter::DIRECTION_MASK_ALL & (~origin_port), 0);
                    }
                }
                
                DetectEvents(); // it first forward the packet, and then tries to detect any event

                return true;


            } else {
                //if the packet received was already in the list
                //it is droped in this case.
                return false;
            }

            //TODO: address this thing in time
        }

        return true;
    }

    void
    XDenseApp::DataAnnouncementTT(void) {
            Ptr<Packet> pck = Create<Packet>();
//            m_router->PacketUnicast(pck, NETWORK_ID_0, 0, 0, USE_ABSOLUTE_ADDRESS);  
            
//            m_router->PacketUnicast(pck, NETWORK_ID_0, 2, 2, USE_RELATIVE_ADDRESS);        
//            m_router->PacketUnicast(pck, NETWORK_ID_0, 2, -2, USE_RELATIVE_ADDRESS);        
//            m_router->PacketUnicast(pck, NETWORK_ID_0, -2, -2, USE_RELATIVE_ADDRESS);       
//            m_router->PacketUnicast(pck, NETWORK_ID_0, -2, 2, USE_RELATIVE_ADDRESS);        
            
//            m_router->PacketUnicast(pck, NETWORK_ID_0, 4, 4, USE_ABSOLUTE_ADDRESS);        
//            m_router->PacketUnicast(pck, NETWORK_ID_0, 4, 0, USE_ABSOLUTE_ADDRESS);        
            m_router->PacketUnicast(pck, NETWORK_ID_0, 0, 0, USE_ABSOLUTE_ADDRESS);        
//            m_router->PacketUnicast(pck, NETWORK_ID_0, 0, 4, USE_ABSOLUTE_ADDRESS);        
     
                  
    }
    
    void
    XDenseApp::DataAnnouncement(EventRef er) {

        //        uint32_t id = this->GetNode()->GetId();
        //        if (id != 43 && id != 397) { //for debuging reasons
        //            return;
        //        }

        //TODO:
        // could be done only by storing and forwarding too. Compare results with
        // the connection based transmission (wormhole)
        //        Is multiple of the neighborhood size? Defining cluster heads
        //        uint8_t r = m_sinksList.at(0).x % MaxHops;
        //        r += m_sinksList.at(0).y % MaxHops;

        //        if (m_sinksList.size() > 0 && r == 0) { //this minimizes drastically the number of EA
        if (m_sinksList.size() > 0) {
            NodeRef sink = m_sinksList.at(0); //the sink can vary, depending on the errors count etc...
//            uint8_t output_port = RouteTo(sink);

            
            XDenseHeader hd;
            //now, this x y will be used to count in the oposite direction, in order to track the origin of the packet
            hd.CurrentX = 0; 
            hd.CurrentY = 0;
            hd.SerialNumber = m_SerialNumber.at(P_EVENT_ANNOUNCEMENT);
            m_SerialNumber.at(P_EVENT_ANNOUNCEMENT)++;

            //this is the data computed after analysing all the neighbor data... Ex: dx/dt, ou dz/dxdy
            hd.EventData[0] = er.data[0]; 
            hd.EventData[1] = er.data[1];
            hd.EventData[2] = er.data[2]; 

            hd.EventType = er.type;

            hd.SetNOCProtocol(P_EVENT_ANNOUNCEMENT);

            Ptr<Packet> pck = Create<Packet>();
            pck->AddHeader(hd);
            m_router->PacketUnicast(pck, 0, sink.x, sink.y, false);           
        }


    }
    


    bool
    XDenseApp::DataAnnoucementReceived(Ptr<const Packet> pck, uint8_t origin_port) {
        //The pck is transported in order to forward a packet with the same unique ID for tracing
        //prorposes

        //        NodeRef sink;
        //        sink.x = hd.CurrentX; //the sink can vary, depending on the errors count etc...
        //        sink.y = hd.CurrentY;

        NodeRef sink = m_sinksList.at(0);


        //                uint32_t id = this->GetNode()->GetId();
        //                if (id == 10150) { //for debuging reasons
        //                    cout << "value announced by n100";
        //                }

        if (sink.x != 0 || sink.y != 0) {
//            uint8_t output_port = RouteTo(sink);
//            m_router->PacketSend(pck, output_port);
            m_router->PacketUnicast(pck, 0, sink.x, sink.y, false);
        }
        else if (IsSink) { //just to make sure it is the sink (test)

            

            XDenseHeader hd;
            pck->PeekHeader(hd);
            
            if (hd.EventType == EV_PLANE || hd.EventType == EV_CURVE) {
                DataFit fit;

                fit.time = Simulator::Now().GetNanoSeconds();
                fit.type = (EventType) hd.EventType;

                fit.x = hd.CurrentX;
                fit.y = hd.CurrentY;

                fit.a = (int32_t) hd.EventData[0] - 32767;
                fit.b = (int32_t) hd.EventData[1] - 32767;
                fit.c = (int32_t) hd.EventData[2];

                SinkReceivedData->AddFit(fit);               
            }
            
           
            else if (hd.EventType == EV_DELTA){
                Point p;
                
                p.x = hd.CurrentX;
                p.y = hd.CurrentY;
                p.value = hd.EventData[0];
                p.time = Simulator::Now().GetNanoSeconds();
                
                SinkReceivedData->AddPoint(p);
            }



        }


        return true;
    }


}

//static void
//CwndChange (uint32_t oldCwnd, uint32_t newCwnd)
//{
//  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
//}
//
//static void
//RxDrop (Ptr<const Packet> p)
//{
//  NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
//}


