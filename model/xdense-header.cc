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




#include "xdense-header.h"
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
        os << this->m_data;       
    }

    uint32_t
    XDenseHeader::GetSerializedSize(void) const {
        return PAYLOAD_SIZE;
    }
    
    void
    XDenseHeader::Serialize(Buffer::Iterator start) const {

        start.WriteU8(m_protocol);
        start.WriteU64(m_data);
//        start.Write(m_data, PAYLOAD_SIZE - 1);
        
    }

    uint32_t
    XDenseHeader::Deserialize(Buffer::Iterator start) {
        
        (m_protocol) = start.ReadU8();
        (m_data) = start.ReadU64();
//        
//        for (uint8_t i = 0 ; i < PAYLOAD_SIZE ; i++)
//        {
//            m_data[i] = start.ReadU8();
//        start.Read(m_data, PAYLOAD_SIZE - 1);
//        }
//            default:
//                std::cout << "Error while deserializing data into the buffer. No known protocol defined in the header.\n";
//                exit(1);
//        }
        return GetSerializedSize();
    }

    void
    XDenseHeader::SetXdenseProtocol(uint8_t protocol) {
        m_protocol = protocol;
    }

    uint8_t
    XDenseHeader::GetXdenseProtocol(void) {
        return m_protocol;
    }

    uint64_t
    XDenseHeader::GetData(void) {
//        uint64_t data = 0;
        
//        data = m_data[7]
//        for (uint8_t i = 0 ; i < 8 ; i++){
//            data &= m_data[i] << (i * 8);
//        }
                
        return m_data;
    }
    
    
//        uint8_t n_size_x = (hd.GetData8() & 0b1111111100000000) >> 8;
//        uint8_t n_size_y = (hd.GetData8() & 0b0000000011111111) >> 0;
    
    void
    XDenseHeader::SetData(int64_t data) {
        
        m_data = data;
        
//        for (uint8_t i = 0 ; i < 8 ; i++){
//            m_data[i] = data >> (i * 8);
//        }
        
//        m_data[0] = data >> 0;
//        m_data[1] = data >> 8;
//        m_data[2] = data >> 16;
//        m_data[3] = data >> 24;
//        m_data[4] = data >> 32;
//        m_data[5] = data >> 40;
//        m_data[6] = data >> 48;
//        m_data[7] = data >> 56;
    }

    void 
    XDenseHeader::SetDataChunk(int64_t data, uint8_t n, uint8_t n_bits) {

    }
    int64_t 
    XDenseHeader::GetDataChunk(int64_t data, uint8_t n, uint8_t n_bits) {

        return 1;
    }




} // namespace ns3
