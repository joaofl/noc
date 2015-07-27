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

        enum DataMode{
            DATA_MODE_8b  = 0b00,
            DATA_MODE_16b = 0b01,
            DATA_MODE_32b = 0b10,
            DATA_MODE_64b = 0b11
        };
        
        enum RWMode{
            RW_MODE_WRITE = 0b00,
            RW_MODE_READ  = 0b01,
            RW_MODE_C     = 0b10, //? 
            RW_MODE_D     = 0b11  //?
        };
        
        //TODO: Understand each one, and name the modes
        enum CtrlMode{
            CRTL_MODE_0A  =  0b0000,
            CRTL_MODE_0B  =  0b0001,
            CRTL_MODE_0C  =  0b0010,
            CRTL_MODE_0D  =  0b0011,
            CRTL_MODE_0E  =  0b0100,
            CRTL_MODE_0F  =  0b0101,
            CRTL_MODE_0G  =  0b0110,
            CRTL_MODE_0H  =  0b0111,
            CRTL_MODE_0I  =  0b0000,
            CRTL_MODE_0J  =  0b0001,
            CRTL_MODE_0K  =  0b0010,
            CRTL_MODE_0L  =  0b0011,
            CRTL_MODE_0M  =  0b0100,
            CRTL_MODE_0N  =  0b0101,
            CRTL_MODE_0O  =  0b0110,
            CRTL_MODE_0P  =  0b0111,
            CRTL_MODE_1A  =  0b1000,
            CRTL_MODE_1B  =  0b1001,
            CRTL_MODE_1C  =  0b1010,
            CRTL_MODE_1D  =  0b1011,
            CRTL_MODE_1E  =  0b1100,
            CRTL_MODE_1F  =  0b1101,
            CRTL_MODE_1G  =  0b1110,
            CRTL_MODE_1H  =  0b1111,
            CRTL_MODE_1I  =  0b1000,
            CRTL_MODE_1J  =  0b1001,
            CRTL_MODE_1K  =  0b1010,
            CRTL_MODE_1L  =  0b1011,
            CRTL_MODE_1M  =  0b1100,
            CRTL_MODE_1N  =  0b1101,
            CRTL_MODE_1O  =  0b1110,
            CRTL_MODE_1P  =  0b1111
        };
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

//        void SetPacketSize(uint8_t size);
//       uint8_t GetPacketSize(void);
        
        //Returns the number of bytes written.
        void SetPacketData(uint8_t data[], uint8_t bytes_count);
        
        //Returns the number of bytes written to the pointer
        void GetPacketData(uint8_t data[], uint8_t bytes_count);
        
//        void SetDestinationAddress(uint8_t x, uint8_t y);
        void SetDestinationAddress(uint32_t add);
        
        void SetDestinationAddressXY(int32_t x, int32_t y);
//        void SetDestinationAddressY(int32_t x);
//        void SetDestinationAddressX(int32_t y);
        
        
//        void GetDestinationAddress(uint8_t x, uint8_t y);
        uint32_t GetDestinationAddress(void);
        
        int32_t GetDestinationAddressX(void);
        int32_t GetDestinationAddressY(void);
        
//        void SetSourceAddress(uint8_t x, uint8_t y);
        void SetSourceAddress(uint32_t add);
        
//        void GetDestinationAddress(uint8_t x, uint8_t y);
        uint32_t GetSourceAddress(void);
        
//        uint32_t ConvertXYtoAddress(uint8_t x, uint8_t y);
//        
//        Coordinate ConvertAddresstoXY(uint32_t address);
        
        void SetRWMode(uint8_t mode);
        uint8_t GetRWMode(void);
        
        void SetDataMode(uint8_t mode);
        uint8_t GetDataMode(void);
        
        void SetCrtlMode(uint8_t mode);
        uint8_t GetCrtlMode(void);

        void SetMode(uint8_t mode);
        uint8_t GetMode(void);
        
    private:

        /**
         * \brief The Epiphany packet
         */
        
//        104 bits per cycle (32 bit address, 64 bits data, 4 bits control-mode,
//        2 bits datamode, 2 bits for read/write access)
        
        /*
        EMESH PACKET FORMAT

        The elink was born out of a need to connect multiple Epiphany chips together
        and uses the eMesh 104 bit atomic packet structure for communication. 
        The eMesh atomic packet consists of the following sub fields.

        PACKET SUBFIELD     DESCRIPTION
        access              Indicates a valid packet
        write               A write transaction. Access & ~write indicates a read.
        datamode[1:0]       Datasize (00=8b,01=16b,10=32b,11=64b)
        ctrlmode[3:0]       Various packet modes for the Epiphany chip
        dstraddr[31:0]      Address for write, read-request, or read-responses
        data[31:0]          Data for write transaction, return data for read response
        srcaddr[31:0]       Return address for read-request, upper data for 64 bit write
        
        */
        

        
        static const uint8_t m_packetSize = 13;
        
        uint8_t m_data[8];
        
        uint32_t m_destAddress;
        
        int32_t m_destAddressX;
        int32_t m_destAddressY;        
        
        uint32_t m_srcAddress;
        
        uint8_t m_mode; //(4 control mode, 2 data mode, 2 r/w)
                            //this is the combination of all below:

        uint8_t m_access;      // [0:0]
        uint8_t m_write;       // [1:1]        
        uint8_t m_dataMode; // [3:2]
        uint8_t m_ctrlMode; // [7:4]

        
//        typedef struct AddressXY{
//            uint32_t x, y;
//        };

        
        

    };
    

    
} // namespace ns3


#endif /* EPIPHANY_HEADER_H */
