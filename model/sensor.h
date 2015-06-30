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
 * 
 */

#ifndef SENSOR_H
#define SENSOR_H

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
//#include <gsl/gsl_rng.h>
#include <stdio.h>
#include "noc-types.h"
#include "sensor-data-io.h"

namespace ns3 {

    class SENSOR : public Application {
    public:
        
        Coordinate SensorPosition;
        
        UNSInputData * InputData;
        
        uint32_t ReadSensor(void);
        
        SENSOR();
        virtual ~SENSOR();
    private:
       virtual void StartApplication(void);
        virtual void StopApplication(void);

        bool m_running;
        uint32_t m_time_instant;
        EventId m_sendEvent;
    };
}
#endif

