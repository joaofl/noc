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
    class XDenseHeader : public Header {
    public:

        /**
         * \brief Construct a PPP header.
         */
        XDenseHeader();

        /**
         * \brief Destroy a PPP header.
         */
        virtual ~XDenseHeader();
        
//        uint8_t HopsCount;
        uint16_t SensorValue;
        uint8_t SerialNumber;        
        int8_t CurrentX, CurrentY; 
        int8_t OperationalMode;
//        OriginX, OriginY;
        
        // Event
        uint16_t EventData[3];
        uint8_t EventType;         
        

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

        
        
        /**
         * \brief Set the protocol type carried by this PPP packet
         *
         * The type numbers to be used are defined in \RFC{3818}
         *
         * \param protocol the protocol type being carried
         */
        void SetNOCProtocol(uint16_t protocol);

        /**
         * \brief Get the protocol type carried by this PPP packet
         *
         * The type numbers to be used are defined in \RFC{3818}
         *
         * \return the protocol type being carried
         */
        uint16_t GetNOCProtocol(void);

//        /**
//         * \brief Set the current X position of the packet, according to a given
//         * reference
//         * \param new X value (usually of the following node);
//         */
//        void SetCurrentX(int32_t x);
//
//        /**
//         * \brief Get the current X position of the packet, according to a given
//         * reference
//         */
//        int32_t GetCurrentX(void);
//
//        /**
//         * \brief Set the current X position of the packet, according to a given
//         * reference
//         * \param new X value (usually of the following node);
//         */
//        void SetCurrentY(int32_t y);
//
//        /**
//         * \brief Get the current X position of the packet, according to a given
//         * reference
//         */
//        int32_t GetCurrentY(void);
//
//        int32_t AddCurrentX(int32_t n);
//        int32_t AddCurrentY(int32_t n);
        
//        void SetHopsCount(uint32_t);
//        void SetSensorValue(uint32_t);
//        uint32_t GetHopsCount(void);
//        uint32_t GetSensorValue(void);

    private:

        /**
         * \brief The PPP protocol type of the payload packet
         */
//        uint16_t m_protocol; 
        uint8_t m_noc_protocol;
        static const uint8_t m_max_header_size = 1 + 4 + 4 + 4 + 4 + 4;
        //int32_t m_currentX, m_currentY;
        //uint32_t m_hops_count, m_sensor_value;

    };

} // namespace ns3


#endif /* NOC_HEADER_H */
