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




#include <assert.h>

#include "xdense-header.h"
#include "src/core/model/int64x64-128.h"
//#include "noc-types.h"


NS_LOG_COMPONENT_DEFINE("XDenseHeader");

namespace ns3 {
    


    NS_OBJECT_ENSURE_REGISTERED(XDenseHeader)
    ;

    XDenseHeader::XDenseHeader() {
        
    }

    XDenseHeader::~XDenseHeader() {
    }

    TypeId
    XDenseHeader::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::XDenseHeader")
                .SetParent<Header> ()
                .AddConstructor<XDenseHeader> ()
                ;
        return tid;
    }

    TypeId
    XDenseHeader::GetInstanceTypeId(void) const {
        return GetTypeId();
    }

    void
    XDenseHeader::Print(std::ostream &os) const {
        os << (int) this->m_protocol;    
        os << ",";
        for (uint8_t i = 0 ; i < HEADER_SIZE-1 ; i++){
            os <<  (int) m_data[i];
        }
        
//        os << this->m_data;       
    }

    uint32_t
    XDenseHeader::GetSerializedSize(void) const {
        return HEADER_SIZE;
    }
    
    void
    XDenseHeader::Serialize(Buffer::Iterator start) const {

        start.WriteU8(m_protocol);
        start.Write(m_data, HEADER_SIZE-1);
    }

    uint32_t
    XDenseHeader::Deserialize(Buffer::Iterator start) {
        
        (m_protocol) = start.ReadU8(); // 1 byte
        start.Read(m_data, HEADER_SIZE-1); //10 bytes
        return GetSerializedSize();
    }

    void
    XDenseHeader::SetXDenseProtocol(uint8_t protocol) {
        m_protocol = protocol;
    }

    uint8_t
    XDenseHeader::GetXDenseProtocol(void) {
        return m_protocol;
    }
    
    void 
    XDenseHeader::SetData64(int64_t data, uint8_t index) { //8 bytes out of 11
        for (uint8_t i = 0 ; i < HEADER_SIZE-1 ; i++){
            m_data[i] = data >> (i*8);
        }
    }
    
    int64_t 
    XDenseHeader::GetData64(uint8_t index) {        
        int64_t data = 0;
        
        for (uint8_t i = 0 ; i < 8 ; i++){
            data |= ( (m_data[i] << (i*8)) );
        }
        return data;
    }
    void 
    XDenseHeader::SetData24(int32_t data, uint8_t index) {
        uint8_t rooms = floor(float(HEADER_SIZE-1) / float(3));
        assert(index < rooms); // 3 bytes
        
        m_data[index*3+0] = data >> (0*8);
        m_data[index*3+1] = data >> (1*8);
        m_data[index*3+2] = data >> (2*8);
    }
    int32_t
    XDenseHeader::GetData24(uint8_t index) {
        uint8_t rooms = floor(float(HEADER_SIZE-1) / float(3)); //3 bytes = 24bits
        assert(index < rooms); // 3 bytes
        
        int32_t data = 0;
        
        data |= m_data[index*3+0] << (8) ;
        data |= m_data[index*3+1] << (16) ;
        data |= m_data[index*3+2] << (24) ;
        data = data >> 8;

        return data;
    }
    
    void
    XDenseHeader::SetData8(int8_t data, uint8_t index) {
        uint8_t rooms = (HEADER_SIZE-1);
        assert(index < rooms); // 10 bytes
        
        m_data[index] = data;
    }
    int8_t
    XDenseHeader::GetData8(uint8_t index) {
        uint8_t rooms = (HEADER_SIZE-1); //10 bytes = 80bits
        assert(index < rooms); // 3 bytes
        
        return m_data[index];
    }





} // namespace ns3
