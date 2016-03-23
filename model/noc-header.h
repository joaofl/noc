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

        /*
         * [0:1]    0       relative addressing
         *          1       absolute addressing
         * 
         * [4:7]    0000     broadcast
         *          0001     multicast to a predefined given radius (n_hops)
         *          0010     unicast from a single node to another
        */
        
        enum NOCControlBits{
            // [0:1]
            ADDRESS_RELATIVE    =   0b00000000,
            ADDRESS_ABSOLUTE    =   0b10000000,
            
            ADDRESS_BITMASK     =   0b11000000,
            ADDRESS_BITSHIFT    =   6
        };

        enum NOCProtocols{
            // [4:7]
            PROTOCOL_BROADCAST              =   0b00000000,
            PROTOCOL_MULTICAST_AREA         =   0b00000001,
            PROTOCOL_MULTICAST_RADIUS       =   0b00100001, //whatever
            PROTOCOL_MULTICAST_INDIVIDUALS  =   0b00100010, //whatever
            PROTOCOL_UNICAST                =   0b00000010,
            PROTOCOL_UNICAST_HIGHWAY        =   0b00000100
            
//            PROTOCOL_BITMASK                =   0b00001111,
//            PROTOCOL_BITSHIFT               =   0
        };        
        
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


//        void SetDestinationAddress(uint8_t x, uint8_t y);
//        void SetDestinationAddress(uint32_t add);
        
        void SetDestinationAddressXY(int32_t x, int32_t y);
//        void SetDestinationAddressY(int32_t x);
//        void SetDestinationAddressX(int32_t y);
        
        
//        void GetDestinationAddress(uint8_t x, uint8_t y);
//        uint32_t GetDestinationAddress(void);
        
        int32_t GetDestinationAddressX(void);
        int32_t GetDestinationAddressY(void);
        
//        void SetSourceAddress(uint8_t x, uint8_t y);
//        void SetSourceAddress(uint32_t add);
        void SetSourceAddressXY(int32_t x, int32_t y);
        
//        void GetSourceAddress(uint8_t x, uint8_t y);
//        uint32_t GetSourceAddress(void);
        int32_t GetSourceAddressX(void);
        int32_t GetSourceAddressY(void);        
//        uint32_t ConvertXYtoAddress(uint8_t x, uint8_t y);
//        
//        Coordinate ConvertAddresstoXY(uint32_t address);
        
        void AddtoSourceAddress(int8_t x, int8_t y);
        void AddtoDestinationAddress(int8_t x, int8_t y);
        
        uint8_t GetProtocol(void);
        void SetProtocol(uint8_t p);
        
        
    private:

        

        
        static const uint8_t m_headerSize = 5;
        
        
        int32_t m_destAddressX;
        int32_t m_destAddressY;        
        
        
        int32_t m_srcAddressX;
        int32_t m_srcAddressY; 
       
        uint8_t m_controlBits; 
        
        uint8_t m_protocol; 
        
    };
    

    
} // namespace ns3


#endif /* EPIPHANY_HEADER_H */
