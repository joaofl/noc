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


#ifndef USN_INPUT_DATA_H
#define USN_INPUT_DATA_H

#include <fstream>
#include <sstream>
#include "ns3/core-module.h"
#include "ns3/applications-module.h"

#include "usn-types.h"



using namespace std;
namespace ns3 {
    
    class UNSInputData{
    public:
        uint8_t LoadFromFile(string);
        uint32_t ReadNode(uint32_t t, uint32_t x, uint32_t y);
        

    private:
        //bool initialized;
                vector< vector<uint32_t> > m_data;
                vector< vector< vector <uint32_t> > > m_data_on_time;
        
        
        
    };
    
    class USNOutputData : public Application {
    public:
        
        static TypeId GetTypeId (void);
        
        USNOutputData();
        virtual ~USNOutputData();
        
        uint8_t WriteToFile(string, uint8_t);
        uint8_t WritePointsToFile(string filename, uint8_t x_size, uint8_t y_size);
        uint32_t AddFit(DataFit);
        uint32_t AddPoint(Point);
        

    private:
        //bool initialized;
        typedef vector< vector<uint16_t> > matrix;
        
        ofstream ss;
        
//        matrix m_data;
//        vector< matrix > m_data_on_time;
//        
        vector<DataFit> planes;
        vector<DataFit> curves;
        vector<Point> points;
        
        struct
        {
            int32_t x_min, x_max, y_min, y_max;
        }bounds;
        
        struct
        {
            DataFit up, down, left, right;
        }sink_neighbors;
        
    };

/* ... */

    
}

#endif /* USN_H */

