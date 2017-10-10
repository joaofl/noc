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

#ifndef XDENSE_HEADER_H
#define XDENSE_HEADER_H

#include <iostream>
#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "src/network/model/buffer.h"

#define HEADER_SIZE 11 //5 from the NOCHeader, and 11 here, total 16 bytes

namespace ns3 {
    /*
     * \ingroup NOC
     * \brief NoC generalized packet header
     */
    class XDenseHeader : public Header {
    public:
        enum{
            DATA_SHARING_REQUEST,   
            DATA_SHARING,
            CLUSTER_DATA_REQUEST,
            CLUSTER_DATA,
            ACTION_NODES_DATA_TO_CLUSTER_HEADS,
            ACTION_NETWORK_SETUP,
            TRACE,
            PING,
            PING_RESPONSE
        };

        /**
         * \brief Construct the header.
         */
        XDenseHeader();

        /**
         * \brief Destroy the header.
         */
        virtual ~XDenseHeader();
        
        static TypeId GetTypeId(void);
        virtual TypeId GetInstanceTypeId(void) const;
        virtual void Print(std::ostream &os) const;
        virtual void Serialize(Buffer::Iterator start) const;
        virtual uint32_t Deserialize(Buffer::Iterator start);
        virtual uint32_t GetSerializedSize(void) const;

        void SetData64(int64_t data, uint8_t index); //max 1 byte
        int64_t GetData64(uint8_t index);
        
        void SetData32(int32_t data, uint8_t index); //max 2 
        int32_t GetData32(uint8_t index);
        
        void SetData24(int32_t data, uint8_t index); //max 3 
        int32_t GetData24(uint8_t index);
        
        void SetData16(int16_t data, uint8_t index); //max 5 
        int16_t GetData16(uint8_t index);

        void SetData8(int8_t data, uint8_t index); //max 10
        int8_t GetData8(uint8_t index);
        
        
        /**
         * \brief Set the protocol type carried by the packet
         *
         * \param protocol the protocol type being carried
         */
        void SetXDenseProtocol(uint8_t protocol);

        /**
         * \brief Get the protocol type carried by this packet
         *
         * \return the protocol type being carried
         */
        uint8_t GetXDenseProtocol(void);


    private:

        uint8_t m_protocol;
        uint8_t m_data[HEADER_SIZE];
//        int8_t m_data;
    };

} // namespace ns3


#endif /* XDENSE_HEADER_H */
