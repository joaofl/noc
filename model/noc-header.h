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

#ifndef USN_HEADER_H
#define USN_HEADER_H

#include "ns3/header.h"
#include "noc-types.h"

namespace ns3 {

    /**
     * \ingroup point-to-point
     * \brief Packet header for PPP
     *
     * This class can be used to add a header to PPP packet.  Currently we do not
     * implement any of the state machine in \RFC{1661}, we just encapsulate the
     * inbound packet send it on.  The goal here is not really to implement the
     * point-to-point protocol, but to encapsulate our packets in a known protocol
     * so packet sniffers can parse them.
     *
     * if PPP is transmitted over a serial link, it will typically be framed in
     * some way derivative of IBM SDLC (HDLC) with all that that entails.
     * Thankfully, we don't have to deal with all of that -- we can use our own
     * protocol for getting bits across the serial link which we call an ns3 
     * Packet.  What we do have to worry about is being able to capture PPP frames
     * which are understandable by Wireshark.  All this means is that we need to
     * teach the PcapWriter about the appropriate data link type (DLT_PPP = 9),
     * and we need to add a PPP header to each packet.  Since we are not using
     * framed PPP, this just means prepending the sixteen bit PPP protocol number
     * to the packet.  The ns-3 way to do this is via a class that inherits from
     * class Header.
     */
    class USNHeader : public Header {
    public:

        /**
         * \brief Construct a PPP header.
         */
        USNHeader();

        /**
         * \brief Destroy a PPP header.
         */
        virtual ~USNHeader();
        
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
         * \brief Get the protocol type carried by this PPP packet
         *
         * The type numbers to be used are defined in \RFC{3818}
         *
         * \return the protocol type being carried
         */
//        uint16_t GetProtocol(void);
        
        
        /**
         * \brief Set the protocol type carried by this PPP packet
         *
         * The type numbers to be used are defined in \RFC{3818}
         *
         * \param protocol the protocol type being carried
         */
        void SetUSNProtocol(uint16_t protocol);

        /**
         * \brief Get the protocol type carried by this PPP packet
         *
         * The type numbers to be used are defined in \RFC{3818}
         *
         * \return the protocol type being carried
         */
        uint16_t GetUSNProtocol(void);

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
        uint8_t m_usn_protocol;
        static const uint8_t m_max_header_size = 1 + 4 + 4 + 4 + 4 + 4;
        //int32_t m_currentX, m_currentY;
        //uint32_t m_hops_count, m_sensor_value;

    };

} // namespace ns3


#endif /* USN_HEADER_H */
