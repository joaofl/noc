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
#include "ns3/buffer.h"

#include "epiphany-header.h"

NS_LOG_COMPONENT_DEFINE("EpiphanyHeader");

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED(EpiphanyHeader);

    TypeId
    EpiphanyHeader::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::EpiphanyHeader")
                .SetParent<Header> ()
                .AddConstructor<EpiphanyHeader> ()
        
//                .AddAttribute("DestinationAddressX", "Destination address X coordinate",
//                IntegerValue(0),
//                MakeIntegerAccessor(&EpiphanyHeader::SetDestinationAddressX, &EpiphanyHeader::GetDestinationAddressX),
//                MakeIntegerChecker<int> ())
        
                ;
        return tid;
    }

    EpiphanyHeader::EpiphanyHeader() {
    }

    EpiphanyHeader::~EpiphanyHeader() {
    }
    
    TypeId
    EpiphanyHeader::GetInstanceTypeId(void) const {
        return GetTypeId();
    }

    void
    EpiphanyHeader::Print(std::ostream &os) const {
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

    uint32_t
    EpiphanyHeader::GetSerializedSize(void) const {

        return m_packetSize;

    }
    
    void
    EpiphanyHeader::Serialize(Buffer::Iterator start) const {

//        PACKET SUBFIELD     DESCRIPTION
            
//        access              Indicates a valid packet
//        write               A write transaction. Access & ~write indicates a read.
//        datamode[1:0]       Datasize (00=8b,01=16b,10=32b,11=64b)
//        ctrlmode[3:0]       Various packet modes for the Epiphany chip
        
//        dstraddr[31:0]      Address for write, read-request, or read-responses
//        data[31:0]          Data for write transaction, return data for read response
//        srcaddr[31:0]       Return address for read-request, upper data for 64 bit write
        
        start.WriteU8(m_mode);
        start.WriteU32(m_destAddress);
        
        if (m_write == RW_MODE_WRITE){
            start.Write(m_data, 8);
        }
        else if (m_write == RW_MODE_READ){
            start.Write(m_data, 4);
            start.WriteU32(m_srcAddress);
        }
        
    }

    uint32_t
    EpiphanyHeader::Deserialize(Buffer::Iterator start) {
        
        m_mode = start.ReadU8();
        this->SetMode(m_mode);
        
        m_destAddress = start.ReadU32();
        this->SetDestinationAddress(m_destAddress);
        
        if (m_write == RW_MODE_WRITE){
            start.Read(m_data, 8);
        }
        else if (m_write == RW_MODE_READ){
            start.Read(m_data, 4);
            m_srcAddress = start.ReadU32();
        }        
        
        return m_packetSize;
    }
    
    
    //Returns the number of bytes written.
    void 
    EpiphanyHeader::SetPacketData(uint8_t data[], uint8_t bytes_count){
        for (uint8_t i = 0 ; i < m_packetSize ; i++){
            if (i < bytes_count)
                m_data[i] = data[i];
        }
    }

    //Returns the number of bytes written to the pointer
    void 
    EpiphanyHeader::GetPacketData(uint8_t data[], uint8_t bytes_count){
        for (uint8_t i = 0 ; i < m_packetSize ; i++){
            if (i < bytes_count)
                data[i] = m_data[i];
        }
    }

    /*
     * Address space for epiphany:
     * 
     * Core     Start Address       End Address     Size
     * 0,0      00000000            00007FFF        32Kb
     * 0,1      00100000            00107FFF        32Kb
     * 0,2      00200000            00207FFF        32Kb
     * 0,3      00300000            00307FFF        32Kb
     * 
     * 1,0      04000000            04007FFF        32Kb
     * 1,1      04100000            04107FFF        32Kb
     * 1,2      04200000            04207FFF        32Kb
     * 1,3      04300000            04307FFF        32Kb
     * 
     * 2,0      08000000            08007FFF        32Kb
     * 2,1      08100000            08107FFF        32Kb
     * 2,2      08200000            08207FFF        32Kb
     * 2,3      08300000            08307FFF        32Kb
     * 
     * 3,0      0C000000            0C007FFF        32Kb
     * 3,1      0C100000            0C107FFF        32Kb
     * 3,2      0C200000            0C207FFF        32Kb
     * 3,3      0C300000            0C307FFF        32Kb
     */
    
    void 
    EpiphanyHeader::SetDestinationAddress(uint32_t add){
        //TODO check if it is valid
        m_destAddress = add;
        m_destAddressX = ((m_destAddress & (0xFF000000)) / 4 ) >> 6 * 4;
        m_destAddressY = ((m_destAddress &  0x00F00000) >> 5 * 4);
    }

    void 
    EpiphanyHeader::SetDestinationAddressXY(int32_t x, int32_t y){
        m_destAddress = ((x * 4) << 6 * 4) | (y << 5 * 4);
        m_destAddressX = x;
        m_destAddressY = y;
    }
    
    
    uint32_t 
    EpiphanyHeader::GetDestinationAddress(void){
        return m_destAddress;
    }
    int32_t 
    EpiphanyHeader::GetDestinationAddressX(void){
        return m_destAddressX;
    }
    int32_t 
    EpiphanyHeader::GetDestinationAddressY(void){
        return m_destAddressY;
    }    
    

    void 
    EpiphanyHeader::SetSourceAddress(uint32_t add){
        //TODO: implement some checking here
        m_srcAddress = add;
    }

    uint32_t 
    EpiphanyHeader::GetSourceAddress(void){
    
        return m_srcAddress;
    }

//    uint32_t 
//    EpiphanyHeader::ConvertXYtoAddress(uint8_t x, uint8_t y){
//    
//        
//        return 0;
//    }
//
//    Coordinate 
//    EpiphanyHeader::ConvertAddresstoXY(uint32_t address){
//        Coordinate c;
//        
//        return c;
//    }

    void 
    EpiphanyHeader::SetRWMode(uint8_t rw_mode){
        m_mode = (m_mode & 0b11111100) | (rw_mode << 0); 
        m_access = rw_mode & 0b00000001 >> 0;
        m_access |= rw_mode & 0b00000010 >> 1;
    }
    
    uint8_t 
    EpiphanyHeader::GetRWMode(void){
        return m_write & m_access;
    }
    
    void 
    EpiphanyHeader::SetDataMode(uint8_t data_mode){
        m_mode = (m_mode & 0b11110011) | (data_mode << 2); 
        m_dataMode = data_mode;
    }
    
    uint8_t 
    EpiphanyHeader::GetDataMode(void){
        return m_dataMode;
    }

    void 
    EpiphanyHeader::SetCrtlMode(uint8_t ctrl_mode){
        m_mode = (m_mode & 0b00001111) | (ctrl_mode << 4);
        m_ctrlMode = ctrl_mode;
    }
    
    uint8_t 
    EpiphanyHeader::GetCrtlMode(void){
        return m_ctrlMode;
    }
    


    void 
    EpiphanyHeader::SetMode(uint8_t mode){
        m_mode = mode;
        
        m_access    = (mode & 0b00000001) >> 0;     // [0:0]
        m_write     = (mode & 0b00000010) >> 1;     // [1:1]        
        m_dataMode  = (mode & 0b00001100) >> 2;     // [3:2]
        m_ctrlMode  = (mode & 0b11110000) >> 4;     // [7:4] //Should be flipped
    }
    
    uint8_t 
    EpiphanyHeader::GetMode(void){
        return m_mode;
    }
    

} // namespace ns3
