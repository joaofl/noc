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

//TODO: eliminate this file, and spread the definitions each one on its own class
//file

#ifndef NOC_TYPES_H
#define	NOC_TYPES_H

#include "ns3/core-module.h"

#define INSTALLED_NOC_ROUTER 0
#define INSTALLED_NOC_APP 1
#define INSTALLED_SENSOR 2

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

//typedef struct {
//    uint8_t group;
//    uint8_t id;
//}NOCNetDeviceAddress;

//typedef uint16_t NOCNetDeviceAddress;

#endif	/* DASDAS_H */

