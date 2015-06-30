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

#include "calc.h"
#include "ns3/calc.h"


//#include "src/lte/helper/radio-bearer-stats-calculator.h"

using namespace arma;

namespace ns3 {

    NodeRef
    USNCalc::FindMax(std::vector<NodeRef> * nl) { //neighbor list
        NodeRef found;

        if (nl->size() > 0) {
            found = nl->at(0);

            //            if (found.value == 15) { //4 debugging only
            //                found.value = 15;
            //            }

            for (uint16_t i = 1; i < nl->size(); i++) {
                if (nl->at(i).value >= found.value) { // >= filters events. less events will
                                                //occur since if values are equal, they dont trigger
                    found = nl->at(i);
                }
            }

        }


        return found;
    }

    NodeRef
    USNCalc::FindMin(std::vector<NodeRef> * nl) {
        NodeRef found;

        if (nl->size() > 0) {
            found = nl->at(0);

            for (uint16_t i = 1; i < nl->size(); i++) {
                if (nl->at(i).value <= found.value) {
                    found = nl->at(i);
                }
            }
        }
        return found;
    }

    float
    USNCalc::FindGrad(std::vector<NodeRef> * sn) {
        NodeRef node;
        float grad = 0;
        float total_dist = 0;
        
        for (uint16_t i = 0; i < sn->size(); i++){
            NodeRef orig;
            orig.x = 0;
            orig.y = 0;
            node = sn->at(i);
            if (node.x != orig.x || node.y != orig.y){
                float dist = FindDistance(node, orig);
                grad += node.value / dist;
                total_dist += 1/dist;
            }
            else{
                grad += node.value * 2;
            }
        }
        
        grad = grad / total_dist;

        return grad;
    }

    
    float
    USNCalc::FindDistance(NodeRef a, NodeRef b){
        int32_t dx = 0;
        int32_t dy = 0;
        if(a.x > b.x) dx = abs(a.x - b.x);
        else dx = abs(b.x - a.x);
        if(a.y > b.y) dy = abs(a.y - b.y);
        else dy = abs(b.y - a.y);
        
        return sqrt(dx*dx + dy*dy);
    }
//    NodeRef
//    USNCalc::FindEdge(std::vector<NodeRef> * sn){
//        
//    }

    uint32_t
    USNCalc::FindArc(std::vector<NodeRef> * sn) {
        //        NodeRefPair nrp;

        NodeRef nr1, nr2, nr3;
        int32_t delta_x12, delta_x13, delta_y12, delta_y13;
        double dist_12, dist_13;
        double teta, h1; //, h1pc;

        //        uint32_t v = sn->at(0).value;

        nr1 = NodeAt(sn, SP_MIN, SP_ZERO);
        nr2 = NodeAt(sn, SP_ZERO, SP_ZERO);
        nr3 = NodeAt(sn, SP_MAX, SP_ZERO);

        delta_x12 = nr2.x - nr1.x;
        delta_x13 = nr3.x - nr1.x;
        //        delta_x = nr1.x - nr2.x * pow(2, 16); //normalized with the sensor full scale

        if (nr1.value >= nr2.value) delta_y12 = nr1.value - nr2.value;
        else if (nr2.value > nr1.value) delta_y12 = nr2.value - nr1.value;

        if (nr3.value >= nr1.value) delta_y13 = nr3.value - nr1.value;
        else if (nr1.value > nr3.value) delta_y13 = nr1.value - nr3.value;

        delta_y12 = nr2.value - nr1.value;
        delta_y13 = nr3.value - nr1.value;

        dist_12 = sqrt((double) (delta_x12 * delta_x12 + delta_y12 * delta_y12));
        dist_13 = sqrt((double) (delta_x13 * delta_x13 + delta_y13 * delta_y13));

        teta = (double) (acos(double(delta_x12 * delta_x13 + delta_y12 * delta_y13) / double(dist_12 * dist_13)));

        h1 = dist_12 * sin(teta);
        //        h1pc = (h1 * 100) / dist_13;


        return h1;
    }

    NodeRef
    USNCalc::NodeAt(std::vector<NodeRef> * sn, int8_t x, int8_t y) {
        NodeRef nr, r;
        //        uint16_t aux;

        if (sn->size() > 0) {
            r = sn->at(0);
            for (uint16_t i = 0; i < sn->size(); i++) {
                nr = sn->at(i);

                if (nr.x == x && nr.y == y)
                    return nr;
            }
            
        }
        return nr;
    }

    DataFit
    USNCalc::FindPlane(std::vector<NodeRef> * sn)
    {
        double sumxx, sumxy, sumyy, sumxz, sumyz, sumx, sumy, sumz;
        sumxx = 0;
        sumxy = 0;
        sumyy = 0;
        sumxz = 0;
        sumyz = 0;
        sumx = 0;
        sumy = 0;
        sumz = 0;
        uint8_t n = sn->size();
        
        for (uint8_t i = 0; i < n; i++)
        {
            NodeRef n = sn->at(i);
            sumxx += n.x * n.x;
            sumyy += n.y * n.y;
            sumxy += n.x * n.y;
            sumxz += (double)n.x * (double)n.value;
            sumyz += (double)n.y * (double)n.value;
            sumx += n.x;
            sumy += n.y;
            sumz += n.value;
        }
        
        mat A, B;

        A
                << sumxx << sumxy << sumx << endr
                << sumxy << sumyy << sumy << endr
                << sumx  << sumy  << n    << endr;
        B
                << sumxz << endr
                << sumyz << endr
                << sumz  << endr;
        
        // A * [a;b;c] = B
        
        vec vplane = solve(A, B);
        
        DataFit p;
        
        p.type = EV_PLANE;
        p.a = (int32_t)vplane.at(0);
        p.b = (int32_t)vplane.at(1);
        p.c = (int32_t)vplane.at(2);
        
        return p;
//        USNCalc::CalculatePlane(sumxx, sumxy, sumyy, sumxz, sumyz, sumzz, sumx, sumy, sumz, sn->size(), a, b, c);
        
        //http://www.had2know.com/academics/least-squares-plane-regression.html
    }

    DataFit
    USNCalc::FindCurve(std::vector<NodeRef> * sn)
    {
//        Exponential Function z = abxcy
        //        z = abxcy
        //ln(z) = ln(abxcy)
        //ln(z) = ln(a) + x*ln(b) + y*ln(c)
        
        double sumxx, sumxy, sumyy, sumxz, sumyz, sumx, sumy, sumz, sumlnz, sumxlnz, sumylnz, t1, t2, t3;
        sumxx = 0;
        sumxy = 0;
        sumyy = 0;
        sumxz = 0;
        sumyz = 0;
        sumx = 0;
        sumy = 0;
        sumz = 0;
        sumlnz = 0;
        sumylnz = 0;
        sumxlnz = 0;
        
        uint8_t n = sn->size();
        
        for (uint8_t i = 0; i < n; i++)
        {
            NodeRef n = sn->at(i);
            sumxx += n.x * n.x;
            sumyy += n.y * n.y;
            sumxy += n.x * n.y;
            sumxz += (double)n.x * (double)n.value;
            sumyz += (double)n.y * (double)n.value;
            
            sumlnz += log((double)n.value);
            sumylnz += (double)n.y * log((double)n.value);
            sumxlnz += (double)n.x * log((double)n.value);
            
            sumx += n.x;
            sumy += n.y;
            sumz += n.value;
        }
        
        mat A, B;

        A
                << n     << sumx  << sumy  << endr
                << sumx << sumxx  << sumxy << endr
                << sumy << sumxy  << sumyy << endr;
        B
                << sumlnz << endr
                << sumxlnz << endr
                << sumylnz  << endr;
        
        // A * [ln a; ln b; ln c] = B
        
        vec vcurve = solve(A, B);   
        
        t1 = vcurve.at(0) * 100;
        t2 = vcurve.at(1) * 1000000000000000;
        t3 = vcurve.at(2) * 1000000000000000;
        
        DataFit p;
        
        p.type = EV_CURVE;
        p.a = (int32_t)(t1);
        p.b = (int32_t)(t2);
        p.c = (int32_t)(t3);
        
        return p;
//        USNCalc::CalculatePlane(sumxx, sumxy, sumyy, sumxz, sumyz, sumzz, sumx, sumy, sumz, sn->size(), a, b, c);
        
        //http://www.had2know.com/academics/least-squares-plane-regression.html
    }
    
    DataFit
    USNCalc::CannyEdge(std::vector<NodeRef> * sn)
    {
        //        https://code.google.com/p/fast-edge/
        
        
        
        DataFit f;
        
        return f;
    }
}