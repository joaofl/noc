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


#ifndef NOC_CALC_H
#define NOC_CALC_H

#include "noc-types.h"
#include <math.h>
#include <stdio.h>
//#include <armadillo>

namespace ns3 {

    class NOCCalc {
    public:
        
        
        static NodeRef FindMax(std::vector<NodeRef> * sn);
        
        static NodeRef FindMin(std::vector<NodeRef> * sn);
        
//        static NodeRefPair FindGrad(std::vector<NodeRef> * sn);
        
        static float FindGrad(std::vector<NodeRef> * sn);

        static uint32_t FindArc(std::vector<NodeRef> * sn);
        
        static float FindDistance(NodeRef a, NodeRef b);
        
        static DataFit FindPlane(std::vector<NodeRef> * sn);
        
        static DataFit FindCurve(std::vector<NodeRef> * sn);
        
        static DataFit CannyEdge(std::vector<NodeRef> * sn);
        

        static int32_t FindPoly(double * x1, double *x2, double *y, int32_t n, double * returned_b, double * returned_sse);

        
    private:
        static NodeRef NodeAt(std::vector<NodeRef> * sn, int8_t, int8_t);
        
        static int32_t PrintVector (int int32_t, double *x);
        static void PrintMatrix (int32_t nr, int32_t nc, double **A);
        static void Gauss(double **a, double *b, double *x, int32_t n);
        static double SumProduct(double * value_x1, double * value_x2, int32_t n);

//        

    };
}

#endif /* NOC_H */