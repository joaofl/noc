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


#ifndef NOC_ROUTING_H
#define NOC_ROUTING_H

//#include <math.h>
#include <stdio.h>
#include "ns3/core-module.h"
#include "ns3/packet.h"

#include "noc-header.h"

namespace ns3 {

    class NOCRouting {
    public:
        
        enum Directions{
            DIRECTION_E        = 0, //east
            DIRECTION_S        = 1, //south
            DIRECTION_W        = 2, //west
            DIRECTION_N        = 3, //north
            DIRECTION_L        = 4  //Internal, local processor
        };

        enum DirectionsMasks {
            DIRECTION_MASK_NONE   = 0b00000000, //none
            DIRECTION_MASK_E   = 0b00000001, //east
            DIRECTION_MASK_S   = 0b00000010, //south
            DIRECTION_MASK_W   = 0b00000100, //west
            DIRECTION_MASK_N   = 0b00001000, //north
            DIRECTION_MASK_L   = 0b00010000, //local
            DIRECTION_MASK_ALL = 0b00011111,
            DIRECTION_MASK_ALL_EXCEPT_LOCAL = 0b00001111
                    
        };
        
        enum RoutingProtocols{
            ROUTING_PROTOCOL_XY_CLOCKWISE,
            ROUTING_PROTOCOL_XY_CCLOCKWISE,
            ROUTING_PROTOCOL_YFIRST,
            ROUTING_PROTOCOL_XFIRST
        };
        
        
//        enum RoutingAlgos{
//            ROUTING_COLUMN_FIRST,
//            ROUTING_ROW_FIRST,
//            ROUTING_CLOCKWISE,
//            ROUTING_COUNTERCLOCKWISE,
//            ROUTING_HIGHWAY
//        };
        
//        static uint8_t RouteTo(uint8_t routing_alg, int32_t x_source, int32_t y_source, int32_t x_dest, int32_t y_dest);
        
        static uint8_t Route(Ptr<const Packet> pck, RoutingProtocols param);
        
        static uint8_t Unicast(int32_t x_dest, int32_t y_dest, RoutingProtocols);
        
        static uint8_t UnicastFirstY(int32_t x_dest, int32_t y_dest);
        
        static uint8_t UnicastFirstX(int32_t x_dest, int32_t y_dest);
        
        static uint8_t UnicastClockwiseXY(int32_t x_dest, int32_t y_dest);
        
        
        
        static uint8_t UnicastClockwiseOffsetXY(int32_t x_dest, int32_t y_dest, int32_t x_orig, int32_t y_orig);
        
        static uint8_t MulticastRadius(int32_t x_source, int32_t y_source, uint16_t n_size);
        
        static uint8_t MulticastArea(int32_t x_source, int32_t y_source, int32_t x_dest, int32_t y_dest);
        
        static uint8_t MulticastIndividuals(int32_t x_source, int32_t y_source, int32_t x_dest, int32_t y_dest);
        
        static uint8_t Broadcast(int32_t x_source, int32_t y_source);
        
        

        
        static int32_t CalculateTimeSlot(int32_t x_source, int32_t y_source, int32_t x_size, int32_t y_size);
        
        
        
//        
        
    private:
//        static NodeRef NodeAt(std::vector<NodeRef> * sn, int8_t, int8_t);

        static char FindQuadrant(int32_t x_dest, int32_t y_dest);  

    };
}

#endif /* NOC_H */