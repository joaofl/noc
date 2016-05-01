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
#include "ns3/buffer.h"

#include "noc-header.h"

NS_LOG_COMPONENT_DEFINE("NOCHeader");

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED(NOCHeader);

    TypeId
    NOCHeader::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::NOCHeader")
                .SetParent<Header> ()
                .AddConstructor<NOCHeader> ()
        
//                .AddAttribute("DestinationAddressX", "Destination address X coordinate",
//                IntegerValue(0),
//                MakeIntegerAccessor(&NOCHeader::SetDestinationAddressX, &NOCHeader::GetDestinationAddressX),
//                MakeIntegerChecker<int> ())
        
                ;
        return tid;
    }

    NOCHeader::NOCHeader() {
    }

    NOCHeader::~NOCHeader() {
    }
    
    TypeId
    NOCHeader::GetInstanceTypeId(void) const {
        return GetTypeId();
    }

    void
    NOCHeader::Print(std::ostream &os) const {
//        uint8_t i;
//        
//        for (i = 0 ; i < m_packetSize ; i++){
//            os << "[" << i << "] " << m_data[i];
//        }
//        std::cout << std::endl;
        
        os << (int) this->m_controlBits << ",";
        os << (int) this->m_protocol << ",";
        os << this->m_srcAddressX << "," << this->m_srcAddressY << ",";
        os << this->m_destAddressX << "," << this->m_destAddressY;
//        os << std::endl;
    }

    uint32_t
    NOCHeader::GetSerializedSize(void) const {

        return m_headerSize;

    }
    
    void
    NOCHeader::Serialize(Buffer::Iterator start) const {

        /*
         * [0:1]    0       relative addressing
         *          1       absolute addressing
         * 
         * [4:7]    0000     broadcast
         *          0001     multicast to a predefined given radius (n_hops)
         *          0010     unicast from a single node to another
         */
       
//        start.WriteU8((ADDRESS_BITSHIFT << m_controlBits) | 
//                (PROTOCOL_BITSHIFT << m_protocol));
        
        start.WriteU8(m_protocol);
        
        start.WriteU8(m_destAddressX);
        start.WriteU8(m_destAddressY);
                        
        start.WriteU8(m_srcAddressX);        
        start.WriteU8(m_srcAddressY);        
    }

    uint32_t
    NOCHeader::Deserialize(Buffer::Iterator start) {
 
        m_protocol = start.ReadU8();
        
//        m_controlBits = (d & ADDRESS_BITMASK) >> ADDRESS_BITSHIFT;
//        m_protocol = (d & PROTOCOL_BITMASK) >> PROTOCOL_BITSHIFT;
        
        m_destAddressX  =  (int8_t) start.ReadU8();
        m_destAddressY  =  (int8_t) start.ReadU8();
        m_srcAddressX   =  (int8_t) start.ReadU8();
        m_srcAddressY   =  (int8_t) start.ReadU8();
   
        return m_headerSize;
    }

    
    void
    NOCHeader::AddtoSourceAddress(int8_t x, int8_t y) 
    {
        m_srcAddressX += x;
        m_srcAddressY += y;
    }
    void
    NOCHeader::AddtoDestinationAddress(int8_t x, int8_t y) 
    {
        m_destAddressX += x;
        m_destAddressY += y;
    }
    void 
    NOCHeader::SetDestinationAddressXY(int32_t x, int32_t y){
        m_destAddressX = x;
        m_destAddressY = y;
    }
    void 
    NOCHeader::SetSourceAddressXY(int32_t x, int32_t y){
        m_srcAddressX = x;
        m_srcAddressY = y;
    } 
    int32_t 
    NOCHeader::GetSourceAddressX(void){
        return m_srcAddressX;
    }
    int32_t 
    NOCHeader::GetSourceAddressY(void){
        return m_srcAddressY;
    }
    int32_t 
    NOCHeader::GetDestinationAddressX(void){
        return m_destAddressX;
    }
    int32_t 
    NOCHeader::GetDestinationAddressY(void){
        return m_destAddressY;
    }  
    uint8_t
    NOCHeader::GetProtocol() {
        return m_protocol;
    }
    void
    NOCHeader::SetProtocol(uint8_t p) {
        m_protocol = p;
    }
   

} // namespace ns3
