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


#ifndef NOC_ROUTING_PROTOCOLS_H
#define NOC_ROUTING_PROTOCOLS_H

//#include <math.h>
#include <stdio.h>
#include"noc-router.h"

namespace ns3 {

    class NOCRoutingProtocols {
    public:
        
//        enum RoutingAlgos{
//            ROUTING_COLUMN_FIRST,
//            ROUTING_ROW_FIRST,
//            ROUTING_CLOCKWISE,
//            ROUTING_COUNTERCLOCKWISE,
//            ROUTING_HIGHWAY
//        };
        
//        static uint8_t RouteTo(uint8_t routing_alg, int32_t x_source, int32_t y_source, int32_t x_dest, int32_t y_dest);
        
        static uint8_t MulticastRadius(int32_t x_source, int32_t y_source, uint16_t n_size);
        
        static uint8_t MulticastArea(int32_t x_source, int32_t y_source, int32_t x_dest, int32_t y_dest);
        
        static uint8_t MulticastIndividuals(int32_t x_source, int32_t y_source, int32_t x_dest, int32_t y_dest);
        
        static uint8_t Broadcast(int32_t x_source, int32_t y_source);
        
        static uint8_t UnicastFirstY(int32_t x_dest, int32_t y_dest);
        
        static uint8_t UnicastFirstX(int32_t x_dest, int32_t y_dest);
        
        static uint8_t UnicastClockwiseXY(int32_t x_dest, int32_t y_dest);
        
        static uint8_t UnicastClockwiseOffsetXY(int32_t x_dest, int32_t y_dest, int32_t x_orig, int32_t y_orig);
        
        static int32_t CalculateTimeSlot(int32_t x_source, int32_t y_source, int32_t x_size, int32_t y_size);
        
        
        
//        
        
    private:
//        static NodeRef NodeAt(std::vector<NodeRef> * sn, int8_t, int8_t);

        static char FindQuadrant(int32_t x_dest, int32_t y_dest);
//        

    };
}

#endif /* NOC_H */