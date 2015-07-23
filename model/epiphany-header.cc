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
#include "epiphany-header.h"
#include "ns3/noc-types.h"
#include "src/network/model/buffer.h"

NS_LOG_COMPONENT_DEFINE("EpiphanyHeader");

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED(EpiphanyHeader);

    EpiphanyHeader::EpiphanyHeader() {
    }

    EpiphanyHeader::~EpiphanyHeader() {
    }

    TypeId
    EpiphanyHeader::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::EpiphanyHeader")
                .SetParent<Header> ()
                .AddConstructor<EpiphanyHeader> ()
                ;
        return tid;
    }

    TypeId
    EpiphanyHeader::GetInstanceTypeId(void) const {
        return GetTypeId();
    }

    void
    EpiphanyHeader::Print(std::ostream &os) const {
        uint8_t i;
        
        for (i = 0 ; i < m_packetSize ; i++){
            std::cout << "[" << i << "] " << m_packetData[i];
        }
        std::cout << std::endl;
    }

    uint32_t
    EpiphanyHeader::GetSerializedSize(void) const {

        return m_packetSize;

    }
    
    void
    EpiphanyHeader::Serialize(Buffer::Iterator start) const {

        uint8_t i;
        
        for (i = 0 ; i < m_packetSize ; i++){
            start.WriteU8(m_packetData[i]);
        }
        
    }

    uint32_t
    EpiphanyHeader::Deserialize(Buffer::Iterator start) {
        
        //TODO: do some safety check here.
        
        uint8_t i;
        
        for (i = 0 ; i < m_packetSize ; i++){
            m_packetData[i] = start.ReadU8();
            
        }
        
        return GetSerializedSize();
    }
    
    
    //Returns the number of bytes written.
    void 
    EpiphanyHeader::SetPacketData(uint8_t data[], uint8_t bytes_count){
        for (uint8_t i = 0 ; i < m_packetSize ; i++){
            if (i < bytes_count)
                m_packetData[i] = data[i];
        }
    }

    //Returns the number of bytes written to the pointer
    void 
    EpiphanyHeader::GetPacketData(uint8_t data[], uint8_t bytes_count){
        for (uint8_t i = 0 ; i < m_packetSize ; i++){
            if (i < bytes_count)
                data[i] = m_packetData[i];
        }
    }

    void 
    EpiphanyHeader::SetDestinationAddress(uint32_t add){
        m_destAddress = add;
    }

    uint32_t 
    EpiphanyHeader::GetDestinationAddress(void){
        return m_destAddress;
    }

    void 
    EpiphanyHeader::SetSourceAddress(uint32_t add){
        m_srcAddress = add;
    }

    uint32_t 
    EpiphanyHeader::GetSourceAddress(void){
    
        return m_srcAddress;
    }

    uint32_t 
    EpiphanyHeader::ConvertXYtoAddress(uint8_t x, uint8_t y){
    
        
        return 0;
    }

    Coordinate 
    EpiphanyHeader::ConvertAddresstoXY(uint32_t address){
        Coordinate c;
        
        return c;
    }

    void 
    EpiphanyHeader::SetRWMode(uint8_t rw_mode){
        m_mode = (m_mode & 0b11111100) | (rw_mode << 0); //check it
        m_access = rw_mode & 0b00000001 >> 0;
        m_access = rw_mode & 0b00000010 >> 1;
    }
    
    uint8_t 
    EpiphanyHeader::GetRWMode(void){
        return m_write & m_access;
    }
    
    void 
    EpiphanyHeader::SetDataMode(uint8_t data_mode){
        m_mode = (m_mode & 0b11110011) | (data_mode << 2); //check it
        m_dataMode = data_mode;
    }
    
    uint8_t 
    EpiphanyHeader::GetDataMode(void){
        return m_dataMode;
    }

    void 
    EpiphanyHeader::SetCrtlMode(uint8_t ctrl_mode){
        m_mode = (m_mode & 0b00001111) | (ctrl_mode << 4); //check it
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
        m_ctrlMode  = (mode & 0b11110000) >> 4;     // [7:4]
    }
    
    uint8_t 
    EpiphanyHeader::GetMode(void){
        return m_mode;
    }
    

} // namespace ns3
