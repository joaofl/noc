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


#ifndef NOC_INPUT_DATA_H
#define NOC_INPUT_DATA_H

#include <errno.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <vector>
//#include <string>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>


#include "ns3/core-module.h"
#include "ns3/applications-module.h"

#include "noc-types.h"



using namespace std;    

int do_mkdir(const char *path, mode_t mode);
int mkpath(const char *path, mode_t mode);

//TODO: make it static maybe.. Make different classes?. Improve it!
namespace ns3 {
    ////////////////////////////////////////////////////////////////////////////
    class DataIO{
    public:
        uint8_t LoadArray3D(string);
        uint32_t GetArraySize3D(void);
        uint32_t GetValue3D(uint32_t t, uint32_t x, uint32_t y);
        
        
        bool LoadArray(string file_name);
        uint32_t GetArraySize(void);
        float GetValue(uint32_t i);
        
        
    private:
        //bool initialized;

        vector<float> m_data_delay;

        vector< vector<uint32_t> > m_data_sensors;
        vector< vector< vector <uint32_t> > > m_data_sensors_on_time;
    };

    /**************************************************************************/
    ////////////////////////////////////////////////////////////////////////////
    class NOCRouterShapingConf{
    public:

        float GetBurstiness(int32_t x, int32_t y, uint8_t p);        
        float GetOffset(int32_t x, int32_t y, uint8_t p);        
        uint32_t GetMsgSize(int32_t x, int32_t y, uint8_t p);        
        bool IsShaped(int32_t x, int32_t y, uint8_t p);        
        bool LoadData(string file_name);
        uint32_t GetArraySize(void);
        float GetValue(uint32_t i);
        
        
    private:
        vector< vector<float> > m_data; //TODO: this could be a multidimensional
                                        //array to allow storing info of nodes/ports
                                        //and avoid iterating though lists
        
        
        
    };

    /**************************************************************************/
    class NOCOutputDataSensors : public Application {
    public:
        
        static TypeId GetTypeId (void);
        
        NOCOutputDataSensors();
        virtual ~NOCOutputDataSensors();
        
        uint8_t WriteToFile(string, uint8_t);
        uint8_t WritePointsToFile(string filename, uint8_t x_size, uint8_t y_size);
        uint32_t AddFit(DataFit);
        uint32_t AddPoint(Point);
        

    private:
        //bool initialized;
        typedef vector< vector<uint16_t> > matrix;
        
        ofstream ss;
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

    ////////////////////////////////////////////////////////////////////////////

    class XDenseSensorModel : public Application {
    public:
        
        Coordinate SensorPosition;
        
        DataIO * InputData;
        
        uint32_t ReadSensor(void);
//        Time GetDelay(void);
        
        XDenseSensorModel();
        virtual ~XDenseSensorModel();
    private:
       virtual void StartApplication(void);
        virtual void StopApplication(void);

        bool m_running;
        uint32_t m_time_instant;
        EventId m_sendEvent;
        
//        Ptr<UniformRandomVariable> m_random;
    };
    
    ////////////////////////////////////////////////////////////////////////////
    
    class NOCRouterDelayModel : public Application {
    public:
        
        Coordinate SensorPosition;
        
        DataIO * InputData;
        
        Time GetDelay(void);
        
        NOCRouterDelayModel();
        virtual ~NOCRouterDelayModel();
        
    private:
        virtual void StartApplication(void);
        virtual void StopApplication(void);

        bool m_running;
        EventId m_sendEvent;
        
        
        Ptr<UniformRandomVariable> m_random;
        uint32_t random_counter;
    };
}

#endif /* NOC_H */

