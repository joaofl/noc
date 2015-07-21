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

#ifndef NOC_HEADER_H
#define NOC_HEADER_H

#include "ns3/header.h"
#include "noc-types.h"

namespace ns3 {

    /**
     * \ingroup NOC
     * \brief NoC generalized packet header
     *
     */
    class NOCHeader : public Header {
    public:

        /**
         * \brief Construct a NoC header.
         */
        NOCHeader();

        /**
         * \brief Destroy the NoC header.
         */
        virtual ~NOCHeader();
        
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

    private:

        /**
         * \brief The NoC protocol type of the payload packet
         */
        
        uint8_t m_packetSize;
        uint8_t m_packetData[];

    };

} // namespace ns3


#endif /* NOC_HEADER_H */
