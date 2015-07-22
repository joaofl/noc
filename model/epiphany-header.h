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

#ifndef EPIPHANY_HEADER_H
#define EPIPHANY_HEADER_H

#include "ns3/header.h"
#include "noc-types.h"

namespace ns3 {

    /**
     * \ingroup NOC
     * \brief NoC generalized packet header
     *
     */
    class EpiphanyHeader : public Header {
    public:

        /**
         * \brief Construct a NoC header.
         */
        EpiphanyHeader();

        /**
         * \brief Destroy the NoC header.
         */
        virtual ~EpiphanyHeader();
        
        static TypeId GetTypeId(void);
        virtual TypeId GetInstanceTypeId(void) const;
        virtual void Print(std::ostream &os) const;
        virtual void Serialize(Buffer::Iterator start) const;
        virtual uint32_t Deserialize(Buffer::Iterator start);
        virtual uint32_t GetSerializedSize(void) const;

        void SetPacketSize(uint8_t size);
       uint8_t GetPacketSize(void);
        
        //Returns the number of bytes written.
        uint8_t SetPacketData(uint8_t &data);
        
        //Returns the number of bytes written to the pointer
        uint8_t GetPacketData(uint8_t &data);
        
        void SetPacketAddress(uint8_t x, uint8_t y);

        Coordinate GetPacketAddress(void);
    private:

        /**
         * \brief The Epiphany packet
         */
        
//        104 bits per cycle (32 bit address, 64 bits data, 4 bits control-mode,
//        2 bits datamode, 2 bits for read/write access)
        
        uint8_t m_packetSize;
        
        uint8_t m_packetData[8];
        
        uint32_t m_packetAddress;
        
        uint8_t m_controlBits; //(4 control mode, 2 data mode, 2 r/w)
        
        //Control bits are as 

    };

} // namespace ns3


#endif /* EPIPHANY_HEADER_H */
