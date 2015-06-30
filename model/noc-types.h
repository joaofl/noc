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

#ifndef NOC_TYPES_H
#define	NOC_TYPES_H

#include "ns3/core-module.h"

enum EventType {
    EV_MAX,
    EV_MIN,
    EV_DELTA,
    EV_GRAD,
    EV_ARC,
    EV_PLANE,
    EV_CURVE,
    
    EV_COUNT
};

enum OperationalMode{
    OP_READ_ALL,
    OP_DETECT_EVENTS,
    OP_COUNT
};

enum ProtocolNumber {
    P_NETWORK_DISCOVERY,
    //P_NEIGHBORHOOD_DISCOVERY,
    P_VALUE_ANNOUNCEMENT,
    P_EVENT_ANNOUNCEMENT,
    P_TOTAL,
    P_COUNT //should always be the last item
};

enum SearchParam {
    SP_MAX,
    SP_MIN,
    SP_ZERO
};

typedef struct {
    int32_t x, y;
    uint32_t value;
    uint32_t serial_number; 
    bool updated;
} NodeRef;

typedef struct {
    NodeRef n1, n2;
    uint32_t delta;
} NodeRefPair;

typedef struct {
    int32_t x, y;

} Coordinate;

typedef struct {
    EventType type;
    int32_t a, b, c;
    int64_t time;
    int16_t x, y;
} DataFit;

typedef struct {
    int32_t value;
    int64_t time;
    int16_t x, y;
} Point;

typedef struct {
    EventType type;
    uint16_t data[3];
    uint32_t serial_number;
    bool detected;
} EventRef;



#endif	/* DASDAS_H */

