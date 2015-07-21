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


#include <iostream>
#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "xdense-header.h"
#include "ns3/noc-types.h"
#include "src/network/model/buffer.h"

NS_LOG_COMPONENT_DEFINE("NOCHeader");

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED(NOCHeader)
    ;

    NOCHeader::NOCHeader() {
    }

    NOCHeader::~NOCHeader() {
    }

    TypeId
    NOCHeader::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::NOCHeader")
                .SetParent<Header> ()
                .AddConstructor<NOCHeader> ()
                ;
        return tid;
    }

    TypeId
    NOCHeader::GetInstanceTypeId(void) const {
        return GetTypeId();
    }

    void
    NOCHeader::Print(std::ostream &os) const {
        //        std::string proto;
        //
        //        router (m_protocol) {
        //            case 0x0021: /* IPv4 */
        //                proto = "IP (0x0021)";
        //                break;
        //            case 0x0057: /* IPv6 */
        //                proto = "IPv6 (0x0057)";
        //                break;
        //            default:
        //                NS_ASSERT_MSG(false, "PPP Protocol number not defined!");
        //        }
        //        os << "Point-to-Point Protocol: " << proto;

        //        std::cout << "\tP=" << m_protocol;
        //        std::cout << "\t(x,y)=" << CurrentX << "," << CurrentY;
        //std::cout << std::endl;
    }

    uint32_t
    NOCHeader::GetSerializedSize(void) const {
        uint8_t s = 0;
        switch (m_noc_protocol) {
            case P_NETWORK_DISCOVERY:
                s += sizeof (m_noc_protocol);
                
                s += sizeof (CurrentX);
                s += sizeof (CurrentY);
                
                s += sizeof (OperationalMode);
                
                s += sizeof (SerialNumber);
                return s;
                break;

            case P_VALUE_ANNOUNCEMENT:
                s += sizeof (m_noc_protocol);
                
                s += sizeof (CurrentX);
                s += sizeof (CurrentY);
                
                s += sizeof (SensorValue);
                
                s += sizeof (SerialNumber);
                
                s += 5; //junk... just to make the packets with the same transmission time
                
                return s;
                break;

            case P_EVENT_ANNOUNCEMENT:
                s += sizeof (m_noc_protocol);
                
                s += sizeof (CurrentX);
                s += sizeof (CurrentY);
                  
                s += sizeof (EventData);
                s += sizeof (EventType);
                
                s += sizeof (SerialNumber);
                return s;
                break;

            default:
                return m_max_header_size;
                break;
        }


    }
    
    void
    NOCHeader::Serialize(Buffer::Iterator start) const {

        switch (m_noc_protocol) {
            case P_NETWORK_DISCOVERY:
                start.WriteU8(m_noc_protocol);
                
                start.WriteU8(CurrentX);
                start.WriteU8(CurrentY);
                
                start.WriteU8(OperationalMode);
                
                start.WriteU8(SerialNumber);
                break;

            case P_VALUE_ANNOUNCEMENT:
                start.WriteU8(m_noc_protocol);
                
                start.WriteU8(CurrentX);
                start.WriteU8(CurrentY);
                
//                start.WriteU8(HopsCount);
                start.WriteHtonU16(SensorValue);
                start.WriteU8(SerialNumber);
                break;

            case P_EVENT_ANNOUNCEMENT:
                start.WriteU8(m_noc_protocol);
                
                start.WriteU8(CurrentX);
                start.WriteU8(CurrentY);
                
                start.WriteHtonU16(EventData[0]);
                start.WriteHtonU16(EventData[1]);
                start.WriteHtonU16(EventData[2]);
                
                start.WriteU8(EventType);
                
                start.WriteU8(SerialNumber);
                break;

            default:
                std::cout << "Error while serializing data into the buffer. No known protocol defined in the header.\n";
                exit(1);
        }
    }

    uint32_t
    NOCHeader::Deserialize(Buffer::Iterator start) {
        
        (m_noc_protocol) = start.ReadU8();
        
        switch (m_noc_protocol) {
            case P_NETWORK_DISCOVERY:
                (CurrentX) = start.ReadU8();
                (CurrentY) = start.ReadU8();
                
                (OperationalMode) = start.ReadU8();
                
                (SerialNumber) = start.ReadU8();
                break;

            case P_VALUE_ANNOUNCEMENT:
                (CurrentX) = start.ReadU8();
                (CurrentY) = start.ReadU8();
                
                (SensorValue) = start.ReadNtohU16();
                
                (SerialNumber) = start.ReadU8();
                break;

            case P_EVENT_ANNOUNCEMENT:
                (CurrentX) = start.ReadU8();
                (CurrentY) = start.ReadU8();
                
                (EventData[0]) = start.ReadNtohU16();
                (EventData[1]) = start.ReadNtohU16();
                (EventData[2]) = start.ReadNtohU16();
                
                (EventType) = start.ReadU8();
                (SerialNumber) = start.ReadU8();
                break;

            default:
                std::cout << "Error while deserializing data into the buffer. No known protocol defined in the header.\n";
                exit(1);
        }
        return GetSerializedSize();
    }

    void
    NOCHeader::SetNOCProtocol(uint16_t protocol) {
        m_noc_protocol = protocol;
    }

    uint16_t
    NOCHeader::GetNOCProtocol(void) {
        return m_noc_protocol;
    }

    //    void NOCHeader::SetCurrentX(int32_t x) {
    //        m_currentX = x;
    //    }
    //
    //    int32_t NOCHeader::GetCurrentX(void) {
    //        return m_currentX;
    //    }
    //    
    //    int32_t NOCHeader::AddCurrentX(int32_t n) {
    //        m_currentX += n;
    //        return m_currentX;
    //    }
    //
    //    void NOCHeader::SetCurrentY(int32_t y) {
    //        m_currentY = y;
    //    }
    //
    //    int32_t NOCHeader::GetCurrentY(void) {
    //        return m_currentY;
    //    }
    //    
    //    int32_t NOCHeader::AddCurrentY(int32_t n) {
    //        m_currentY += n;
    //        return m_currentY;
    //    }

    //    void NOCHeader::SetHopsCount(uint32_t n){
    //        m_hops_count = n;
    //    }
    //    void NOCHeader::SetSensorValue(uint32_t n){
    //        m_sensor_value = n;
    //    }
    //    uint32_t NOCHeader::GetHopsCount(void){
    //        return m_hops_count;
    //    }
    //    uint32_t NOCHeader::GetSensorValue(void){
    //        return m_sensor_value;
    //    }

} // namespace ns3
