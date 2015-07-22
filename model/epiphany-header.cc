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
#include "epiphany-header.h"
#include "ns3/noc-types.h"
#include "src/network/model/buffer.h"

NS_LOG_COMPONENT_DEFINE("EpiphanyHeader");

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED(EpiphanyHeader);

    EpiphanyHeader::EpiphanyHeader() {
    }

    EpiphanyHeader::~EpiphanyHeader() {
    }

    TypeId
    EpiphanyHeader::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::EpiphanyHeader")
                .SetParent<Header> ()
                .AddConstructor<EpiphanyHeader> ()
                ;
        return tid;
    }

    TypeId
    EpiphanyHeader::GetInstanceTypeId(void) const {
        return GetTypeId();
    }

    void
    EpiphanyHeader::Print(std::ostream &os) const {
        uint8_t i;
        
        for (i = 0 ; i < m_packetSize ; i++){
            std::cout << "[" << i << "] " << m_packetData[i];
        }
        std::cout << std::endl;
    }

    uint32_t
    EpiphanyHeader::GetSerializedSize(void) const {

        return m_packetSize;

    }
    
    void
    EpiphanyHeader::Serialize(Buffer::Iterator start) const {

        uint8_t i;
        
        for (i = 0 ; i < m_packetSize ; i++){
            start.WriteU8(m_packetData[i]);
        }
        
    }

    uint32_t
    EpiphanyHeader::Deserialize(Buffer::Iterator start) {
        
        //TODO: do some safety check here.
        
        uint8_t i;
        
        for (i = 0 ; i < m_packetSize ; i++){
            m_packetData[i] = start.ReadU8();
            
        }
        
        return GetSerializedSize();
    }
    
    

} // namespace ns3
