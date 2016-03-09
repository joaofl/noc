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

#include "noc-routing-protocols.h"


namespace ns3 {
   //Using XY routing
//    uint8_t NOCRoutingProtocols::RouteTo(uint8_t routing_alg, int32_t x_dest, int32_t y_dest) { //X-Y routing, with X first
//        
//        uint8_t dir = 0;
//
//        switch (routing_alg){       
//            case ROUTING_COLUMN_FIRST:
//                if      (y_dest < 0)    dir = NOCRouter::DIRECTION_MASK_S;
//                else if (y_dest > 0)    dir = NOCRouter::DIRECTION_MASK_N;
//
//                else if (x_dest > 0)    dir = NOCRouter::DIRECTION_MASK_E;
//                else if (x_dest < 0)    dir = NOCRouter::DIRECTION_MASK_W;
//
//                else if (0 == x_dest && 0 == y_dest) dir = NOCRouter::DIRECTION_MASK_L;
//                break;
//            
//            case ROUTING_ROW_FIRST:
//                if      (x_dest > 0)    dir = NOCRouter::DIRECTION_MASK_E;
//                else if (x_dest < 0)    dir = NOCRouter::DIRECTION_MASK_W;
//                
//                else if (y_dest > 0)    dir = NOCRouter::DIRECTION_MASK_S;
//                else if (y_dest < 0)    dir = NOCRouter::DIRECTION_MASK_N;
//
//                else if (0 == x_dest && 0 == y_dest) dir = NOCRouter::DIRECTION_MASK_L;
//                break;    
//        }
//        
//        return dir;
//    }
    
    uint8_t
    NOCRoutingProtocols::Unicast(RoutingAlgos ra, int32_t x_dest, int32_t y_dest) {
        //Check in which quadrant the packet is in:

        /*              |
         *       B      |     A
         *              |
         * -------------|-------------
         *              |
         *       C      |     D
         *              |
         */
         uint8_t dir = 0;

        dir = NOCRouter::DIRECTION_MASK_L; //It sends the packet inside anyway, since it
                                 //was received, but not sent to the app yet

        //A: [+x +y[
        if (x_dest >= 0 && y_dest > 0){
           dir = NOCRouter::DIRECTION_MASK_E; //send it up, dir +y
           if (x_dest == 0) //if on the axis, send it right too
               dir = NOCRouter::DIRECTION_MASK_N;
        }
        //B: ]-x +y]
        else if (x_dest < 0 && y_dest >= 0){
           dir = NOCRouter::DIRECTION_MASK_N;
           if (y_dest == 0) 
               dir = NOCRouter::DIRECTION_MASK_W;
        }
        //C: [-x -y[
        else if (x_dest <= 0 && y_dest < 0){
           dir = NOCRouter::DIRECTION_MASK_W;
           if (x_dest == 0) 
               dir = NOCRouter::DIRECTION_MASK_S;
        }
        //D: ]+x -y]
        else if (x_dest > 0 && y_dest <= 0){
           dir = NOCRouter::DIRECTION_MASK_S;
           if (y_dest == 0) 
               dir = NOCRouter::DIRECTION_MASK_E;
        }
        else if (x_dest == 0 && y_dest == 0){ //it is to me, consume it
            dir = NOCRouter::DIRECTION_MASK_L;
        }
        
        return dir;
    }

    
    uint8_t
    NOCRoutingProtocols::Broadcast(RoutingAlgos ra, int32_t x_source, int32_t y_source) {
        return MulticastRadius(ROUTING_CLOCKWISE, x_source, y_source, 0);
    }

    
    uint8_t
    NOCRoutingProtocols::MulticastRadius(RoutingAlgos ra, int32_t x_source, int32_t y_source, uint16_t n_hops) {
 //Check in which quadrant the packet is in:

        /*              |
         *       B      |     A
         *              |
         * -------------|-------------
         *              |
         *       C      |     D
         *              |
         */

        uint8_t dir = NOCRouter::DIRECTION_MASK_L; //It sends the packet inside anyway, since it
                                 //was received, but not sent to the app yet

        //First of all, check if have not exceeded the radius defined by n_hops

        if (abs(x_source) + abs(y_source) < n_hops || n_hops == 0) {

            //A: [+x +y[
            if (x_source >= 0 && y_source > 0){
               dir |= NOCRouter::DIRECTION_MASK_E; //send it up, dir +y
               if (x_source == 0) //if on the axis, send it right too
                   dir |= NOCRouter::DIRECTION_MASK_N;
            }
            //B: ]-x +y]
            else if (x_source < 0 && y_source >= 0){
               dir |= NOCRouter::DIRECTION_MASK_N;
               if (y_source == 0) 
                   dir |= NOCRouter::DIRECTION_MASK_W;
            }
            //C: [-x -y[
            else if (x_source <= 0 && y_source < 0){
               dir |= NOCRouter::DIRECTION_MASK_W;
               if (x_source == 0) 
                   dir |= NOCRouter::DIRECTION_MASK_S;
            }
            //D: ]+x -y]
            else if (x_source > 0 && y_source <= 0){
               dir |= NOCRouter::DIRECTION_MASK_S;
               if (y_source == 0) 
                   dir |= NOCRouter::DIRECTION_MASK_E;
            }
            else if (x_source == 0 && y_source == 0) //generated by myself.
                                                     //send it to all neighbors
                dir = NOCRouter::DIRECTION_MASK_ALL_EXCEPT_LOCAL;

        }
        
        return dir;
    }
    
    uint8_t
    NOCRoutingProtocols::MulticastArea(RoutingAlgos ra, int32_t x_source, int32_t y_source, int32_t x_dest, int32_t y_dest) {
 //Check in which quadrant the packet is in:

        /*              |
         *       B      |     A
         *              |
         * -------------|-------------
         *              |
         *       C      |     D
         *              |
         */

        uint8_t dir = NOCRouter::DIRECTION_MASK_L; //It sends the packet inside anyway, since it
                                 //was received, but not sent to the app yet

        //First of all, check if have not exceeded the radius defined by n_hops

        if (abs(x_source) < x_dest || abs(y_source) < y_dest) 
        {

            //A: [+x +y[
            if (x_source >= 0 && y_source > 0){
                if (abs(x_source) < x_dest)
                    dir |= NOCRouter::DIRECTION_MASK_E; //send it up, dir +y
                if (x_source == 0 && abs(y_source) < y_dest) //if on the axis, send it right too
                    dir |= NOCRouter::DIRECTION_MASK_N;
            }
            //B: ]-x +y]
            else if (x_source < 0 && y_source >= 0){
                if (abs(y_source) < y_dest)
                    dir |= NOCRouter::DIRECTION_MASK_N;
               if (y_source == 0 && abs(x_source) < x_dest) 
                   dir |= NOCRouter::DIRECTION_MASK_W;
            }
            //C: [-x -y[
            else if (x_source <= 0 && y_source < 0){
                if (abs(x_source) < x_dest)
                    dir |= NOCRouter::DIRECTION_MASK_W;
                if (x_source == 0 && abs(y_source) < y_dest) 
                   dir |= NOCRouter::DIRECTION_MASK_S;
            }
            //D: ]+x -y]
            else if (x_source > 0 && y_source <= 0){
                if (abs(y_source) < y_dest)
                    dir |= NOCRouter::DIRECTION_MASK_S;
                if (y_source == 0 && abs(x_source) < x_dest) 
                   dir |= NOCRouter::DIRECTION_MASK_E;
            }
            else if (x_source == 0 && y_source == 0) //generated by myself.
                                                     //send it to all neighbors
                dir = NOCRouter::DIRECTION_MASK_ALL_EXCEPT_LOCAL;

        }
//        else if ((abs(x_source) == x_dest && abs(y_source) < y_dest-1)
//                || (abs(x_source) < x_dest-1 && abs(y_source) == y_dest))
//        {
//            //A: [+x +y[
//            if (x_source >= 0 && y_source > 0){
//               dir |= NOCRouter::DIRECTION_MASK_E; //send it up, dir +y
//            }
//            //B: ]-x +y]
//            else if (x_source < 0 && y_source >= 0){
//               dir |= NOCRouter::DIRECTION_MASK_N;
//            }
//            //C: [-x -y[
//            else if (x_source <= 0 && y_source < 0){
//               dir |= NOCRouter::DIRECTION_MASK_W;
//            }
//            //D: ]+x -y]
//            else if (x_source > 0 && y_source <= 0){
//               dir |= NOCRouter::DIRECTION_MASK_S;
//            }
//            
//        }
        
        return dir;
    }
    

        
//        Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
        
//        t_wait = x->GetInteger (0, x_size * y_size);
////        
//        t_wait = 0;
//        
//        uint16_t t_wait = (origin_x - 1) * (y_size-2);
        
//        uint32_t t_wait = (x_source - 1) * (y_source - 2);
//         t_wait = 1;

//        t_wait = t_wait;
//        t_wait = int32_t(100 * myRandomNo);
//        return t_wait;
//    }

    int32_t
    NOCRoutingProtocols::ScheduleTransmission(int32_t x_source, int32_t y_source, int32_t x_size, int32_t y_size) {
        
        
        uint32_t t_wait = -1;
        
        //A: [+x +y[
        if (x_source >= 0 && y_source > 0){
            t_wait = (y_source - 1) * (x_size - 2);
        }
        //B: ]-x +y]
        else if (x_source < 0 && y_source >= 0){
            t_wait = (x_source * -1 - 1) * (y_size - 2);
        }
        //C: [-x -y[
        else if (x_source <= 0 && y_source < 0){
            t_wait = (y_source * -1 - 1) * (x_size - 2);
        }
        //D: ]+x -y]
        else if (x_source > 0 && y_source <= 0){
            t_wait = (x_source - 1) * (y_size - 2);
        }
        
        
        return t_wait;
    }
    
}