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
#include "src/network/model/buffer.h"

#include "xdense-header.h"
#include "noc-types.h"


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

    }

    uint32_t
    XDenseHeader::GetSerializedSize(void) const {


        return PAYLOAD_SIZE;
    }
    
    void
    XDenseHeader::Serialize(Buffer::Iterator start) const {

        start.WriteU8(m_protocol);
        start.WriteU8(m_data[0], PAYLOAD_SIZE - 1);
        
    }

    uint32_t
    XDenseHeader::Deserialize(Buffer::Iterator start) {
        
        (m_protocol) = start.ReadU8();
//        
//        for (uint8_t i = 0 ; i < PAYLOAD_SIZE ; i++)
//        {
//            m_data[i] = start.ReadU8();
        start.Read(m_data, PAYLOAD_SIZE - 1);
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

    uint16_t
    XDenseHeader::GetXdenseProtocol(void) {
        return m_protocol;
    }

} // namespace ns3
