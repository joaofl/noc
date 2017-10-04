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

#define PAYLOAD_SIZE 11

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
            CLUSTER_DATA_RESPONSE,
            CLUSTER_DATA,
            ACTION_NODES_DATA_TO_CLUSTER_HEADS,
            ACTION_NETWORK_SETUP,
            TRACE,
            PING,
            PING_RESPONSE
        };

        /**
         * \brief Construct a PPP header.
         */
        XDenseHeader();

        /**
         * \brief Destroy a PPP header.
         */
        virtual ~XDenseHeader();
        
//        uint8_t Data[PAYLOAD_SIZE];   
//        int8_t Protocol;
     
        

        static TypeId GetTypeId(void);
        virtual TypeId GetInstanceTypeId(void) const;
        virtual void Print(std::ostream &os) const;
        virtual void Serialize(Buffer::Iterator start) const;
        virtual uint32_t Deserialize(Buffer::Iterator start);
        virtual uint32_t GetSerializedSize(void) const;

        /**
         * \brief Set the protocol type carried by this PPP packet
         *
         * The type numbers to be used are defined in \RFC{3818}
         *
         * \param protocol the protocol type being carried
         */
//        void SetProtocol(uint16_t protocol);

        void SetData(uint64_t);
        
        uint64_t GetData(void);
        
        /**
         * \brief Set the protocol type carried by this PPP packet
         *
         * The type numbers to be used are defined in \RFC{3818}
         *
         * \param protocol the protocol type being carried
         */
        void SetXdenseProtocol(uint8_t protocol);

        /**
         * \brief Get the protocol type carried by this PPP packet
         *
         * The type numbers to be used are defined in \RFC{3818}
         *
         * \return the protocol type being carried
         */
        uint8_t GetXdenseProtocol(void);


    private:

        /**
         * \brief The PPP protocol type of the payload packet
         */
        uint8_t m_protocol;
//        uint8_t m_data[PAYLOAD_SIZE - 1];
        uint64_t m_data;
    };

} // namespace ns3


#endif /* XDENSE_HEADER_H */
