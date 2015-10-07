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

#include "ns3/assert.h"
#include "ns3/log.h"

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
//                MakeIntegerAccessor(&EpiphanyHeader::SetDestinationAddressX, &EpiphanyHeader::GetDestinationAddressX),
//                MakeIntegerChecker<int> ())
        
                ;
        return tid;
    }

    NOCHeader::NOCHeader() {
    }

    NOCHeader::~NOCHeader() {
    }
    
    ::GetInstanceTypeId(void) const {
        return GetTypeId();
    }
    
    ::Print(std::ostream &os) const {
//        uint8_t i;
//        
//        for (i = 0 ; i < m_packetSize ; i++){
//            os << "[" << i << "] " << m_data[i];
//        }
//        std::cout << std::endl;
        
        os << "M " << (int) this->m_mode;
        os << " DA " << this->m_destAddress;
        os << " DA " << (int) this->m_destAddressX;
        os << "," << (int) this->m_destAddressY;
        
        if (m_write == RW_MODE_WRITE){
            os << " D " << (int) this->m_data[0];
        }
        else if (m_write == RW_MODE_READ){
            os << " D " << (int) this->m_data[0];
            os << " SA " << this->m_srcAddress;
        }
        os << std::endl;
    }

}