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

#include "sensor.h"
#include "ns3/noc-types.h"

namespace ns3 {

    USNSensor::USNSensor()
    : m_running(false){
    }

    USNSensor::~USNSensor() {
    }


    void
    USNSensor::StartApplication(void) {

        Ptr<Node> nd = this->GetNode();
        m_time_instant = 0;
        
    }
    
    uint32_t USNSensor::ReadSensor(void){
        
        uint32_t r = InputData->ReadNode(m_time_instant, SensorPosition.x, SensorPosition.y);
        m_time_instant++;
        
        return r;
                        //in another class, and send it to the RAM. Each sensor goes
                        //there and pick its value, for every time t.
    }

    void
    USNSensor::StopApplication(void) {
        m_running = false;

        if (m_sendEvent.IsRunning()) {
            Simulator::Cancel(m_sendEvent);
        }
    }
    
}

//Todo: implement callback based sensor conversions... Whenver the sensor 
//finishes its "conversion", it calls the app layer