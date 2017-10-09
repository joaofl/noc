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
 * Author: João Loureiro, Pedro Santos <[joflo,pjsol]@isep.ipp.pt>
 */

#include "calc.h"

#define DEBUG 0


namespace ns3 {
    
    std::vector<double_t> 
    NOCCalc::GetRandomBinsDouble(double_t value_min, double_t value_max, uint16_t n_bins, double_t sum) {
        Ptr<UniformRandomVariable> m_random;
        m_random = CreateObject<UniformRandomVariable> ();
        std::vector<double_t> bins(n_bins, 0);
        
        double sum_n = 0;
        double n;
        
        for (int i = 0 ; i < n_bins ; i++){
            n = m_random->GetValue(value_min, value_max);
            bins[i] = n;
            sum_n += n;
        }
        
        double_t factor = sum / sum_n;
        uint16_t max_itr = 10;
        bool invalid = true;
        while (invalid){
            sum_n = 0;
            for (int i = 0 ; i < n_bins ; i++){
                n = bins[i] * factor;
                
                if(n > value_max)
                    n= value_max;
                if(n < value_min)
                    n=value_min;
                
                bins[i] = n;
                sum_n += n;
            }            
            factor = sum / sum_n;
            if (fabs(factor) > 0.98 && fabs(factor) < 1.02)
                invalid = false;
            if (max_itr == 0)
                invalid = false;
            max_itr--;
        }
        
        return bins;
    }
    
    std::vector<int32_t> 
    NOCCalc::GetRandomBinsInt(int32_t value_min, int32_t value_max, int32_t n_bins, int32_t sum) {
        Ptr<UniformRandomVariable> m_random;
        m_random = CreateObject<UniformRandomVariable> ();
        std::vector<int32_t> bins(n_bins, 0);
        
        int32_t sum_n = 0;
        int32_t n = 0;
        
        for (uint16_t i = 0 ; i < n_bins ; i++){
            n = m_random->GetInteger(value_min, value_max);
            bins[i] = n;
            sum_n += n;
        }
        
        double_t factor = (double_t) sum / sum_n;

        bool invalid = true;
        while (invalid){
            uint16_t j = m_random->GetInteger(0, n_bins-1);
            
            if (factor > 1)
                n = bins[j] + 1;
            if (factor < 1)
                n = bins[j] - 1;

            if(n > value_max)
                n= value_max;
            if(n < value_min)
                n=value_min;
            
            bins[j] = n;
            
            sum_n = 0;
            for (uint16_t i = 0 ; i < n_bins ; i++){
                sum_n += bins[i];
            }
            
            factor = (double_t) sum / sum_n;
            if (fabs(factor) == 1)
                invalid = false;
        }
        
        return bins;
    }


    
    NodeRef
    NOCCalc::FindMax(std::vector<NodeRef> * nl) { //neighbor list
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
    NOCCalc::FindMin(std::vector<NodeRef> * nl) {
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
    NOCCalc::FindGrad(std::vector<NodeRef> * sn) {
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
    NOCCalc::FindDistance(NodeRef a, NodeRef b){
        int32_t dx = 0;
        int32_t dy = 0;
        if(a.x > b.x) dx = abs(a.x - b.x);
        else dx = abs(b.x - a.x);
        if(a.y > b.y) dy = abs(a.y - b.y);
        else dy = abs(b.y - a.y);
        
        return sqrt(dx*dx + dy*dy);
    }
//    NodeRef
//    NOCCalc::FindEdge(std::vector<NodeRef> * sn){
//        
//    }

    uint32_t
    NOCCalc::FindArc(std::vector<NodeRef> * sn) {
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
    NOCCalc::NodeAt(std::vector<NodeRef> * sn, int8_t x, int8_t y) {
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
    NOCCalc::FindPlane(std::vector<NodeRef> * sn)
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
        uint16_t n = sn->size();
        
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
        p.a = (int64_t)vplane.at(0);
        p.b = (int64_t)vplane.at(1);
        p.c = (int64_t)vplane.at(2);
        
        p.time = -1;
        p.x = 0;
        p.y = 0;
        
        return p;
        
//        NOCCalc::CalculatePlane(sumxx, sumxy, sumyy, sumxz, sumyz, sumzz, sumx, sumy, sumz, sn->size(), a, b, c);
        //http://www.had2know.com/academics/least-squares-plane-regression.html
    }

    DataFit
    NOCCalc::FindCurve(std::vector<NodeRef> * sn)
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
        //http://www.had2know.com/academics/least-squares-plane-regression.html
    }
    
    DataFit
    NOCCalc::CannyEdge(std::vector<NodeRef> * sn)
    {
        //        https://code.google.com/p/fast-edge/
        
        
        
        DataFit f;
        
        return f;
    }
    
    
    
    double 
    NOCCalc::SumProduct(double * value_x1, double * value_x2, int32_t n){
	int i;
    double result=0;
	for(i = 0; i < n; i++){
		result += (value_x1[i] * value_x2[i]) ;
	}
	return result;
}
    void 
    NOCCalc::PrintMatrix (int32_t nr, int32_t nc, double **A){
    int i,j;


    for (i = 0; i < nr; i++) {

        for (j = 0; j < nc; j++) {
            printf ("%9.4f  ", A[i][j]);
        }

        printf("\n");
    }
    return;
}
int32_t 
NOCCalc::PrintVector (int32_t nr, double *x){
    int i;
      
    if ( nr <= 0 ) return (-1);

    for (i = 0; i < nr; i++) {
        printf ("%9.4f  \n", x[i]);
    }
    printf("\n"); 
    return (0);
}

void 
NOCCalc::Gauss(double **a, double *b, double *x, int32_t n) {
    int   i,j,k,m,rowx;
    double xfac,temp,temp1,amax;


    /////////////////////////////////////////
    // Do the forward reduction step. 
    /////////////////////////////////////////

    rowx = 0;   // Keep count of the row interchanges 
    for (k=0; k<=n-2; ++k) {
        
         amax = (double) fabs(a[k][k]) ;
         m = k;
         for (i=k+1; i<=n-1; i++){   // Find the row with largest pivot 
                   xfac = (double) fabs(a[i][k]);
                   if(xfac > amax) {amax = xfac; m=i;}
         }
         if(m != k) {  // Row interchanges 
                     rowx++;
                     temp1 = b[k];
                     b[k]  = b[m];
                     b[m]  = temp1;
                     for(j=k; j<=n-1; j++) {
                           temp = a[k][j];
                           a[k][j] = a[m][j];
                           a[m][j] = temp;
                     }
          }
           for (i=k+1; i<=n-1; ++i) {
              xfac = a[i][k]/a[k][k];

                   for (j=k+1; j<=n-1; ++j) {
                       a[i][j] = a[i][j]-xfac*a[k][j];
                   }
              b[i] = b[i]-xfac*b[k];
           }

    if(DEBUG == 1) {printf("\n Matrix [A] after decomposition step [%d]\n\n",k);
                     PrintMatrix(n, n, a);}        

    }
    
    /////////////////////////////////////////
    // Do the back substitution step 
    /////////////////////////////////////////

    for (j=0; j<=n-1; ++j) {
      k=(n-(j+1)+1)-1;
      x[k] = b[k];
           for(i=k+1; i<=n-1; ++i) {
             x[k] = x[k]-a[k][i]*x[i];
           }
      if(a[k][k]) {
        x[k] = x[k]/a[k][k];
      } else {
        x[k] = 0;
      }
    }

    if(DEBUG == 1) printf("\nNumber of row exchanges = %d\n",rowx);

}
int32_t 
NOCCalc::FindPoly(double * x1, double *x2, double *y, int32_t n, double * returned_b, double * returned_sse){
	int i, j;
	int result = 0;

	int __n 		= n;
	double *__1 	= (double *) malloc(sizeof(double) * n);
	double *__x1 	= (double *) malloc(sizeof(double) * n);
	double *__x2 	= (double *) malloc(sizeof(double) * n);
	double *__x1_2 	= (double *) malloc(sizeof(double) * n);
	double *__x2_2 	= (double *) malloc(sizeof(double) * n);
	double *__x1x2 	= (double *) malloc(sizeof(double) * n);
	double *__y 	= (double *) malloc(sizeof(double) * n);

	//[row][col]
	int __matrixA_rows = 6;
	int __matrixA_cols = 7;
	double ** __matrixA = (double **) malloc(__matrixA_rows * sizeof(double*));
	for(i = 0; i < __matrixA_rows; i++) __matrixA[i] = (double *)malloc(__matrixA_cols * sizeof(double));
	//double __matrixA[6][6]= {0};
	int __matrixB_rows = 6;
	double * __matrixB = (double *) malloc(__matrixB_rows * sizeof(double));
	//double __matrixB[6][1]= {0};
	int __matrixC_rows = 6;
	double * __matrixC = (double *) malloc(__matrixC_rows * sizeof(double));
	//double __matrixC[6][1]= {0};

	for ( i = 0; i < __n; i++){
		__1[i] = 1;
		__x1[i] = x1[i];
		__x2[i] = x2[i];
		__x1_2[i] = x1[i] * x1[i];
		__x2_2[i] = x2[i] * x2[i];
		__x1x2[i] = x1[i] * x2[i];
		__y[i] = y[i];
	}

	//Calc Matrix A
	for ( i = 0; i < 6; i++){
		switch(i){
			case 0:
					__matrixA[0][i] = __matrixA[i][0] = SumProduct(__1,    __1, __n);
					__matrixA[1][i] = __matrixA[i][1] = SumProduct(__x1,   __1, __n);
					__matrixA[2][i] = __matrixA[i][2] = SumProduct(__x2,   __1, __n);
					__matrixA[3][i] = __matrixA[i][3] = SumProduct(__x1_2, __1, __n);
					__matrixA[4][i] = __matrixA[i][4] = SumProduct(__x2_2, __1, __n);
					__matrixA[5][i] = __matrixA[i][5] = SumProduct(__x1x2, __1, __n);
			break;
			case 1: //*x1
					__matrixA[1][i] = __matrixA[i][1] = SumProduct(__x1,   __x1, __n);
					__matrixA[2][i] = __matrixA[i][2] = SumProduct(__x2,   __x1, __n);
					__matrixA[3][i] = __matrixA[i][3] = SumProduct(__x1_2, __x1, __n);
					__matrixA[4][i] = __matrixA[i][4] = SumProduct(__x2_2, __x1, __n);
					__matrixA[5][i] = __matrixA[i][5] = SumProduct(__x1x2, __x1, __n);
			break;
			case 2: //*x2
					__matrixA[2][i] = __matrixA[i][2] = SumProduct(__x2,   __x2, __n);
					__matrixA[3][i] = __matrixA[i][3] = SumProduct(__x1_2, __x2, __n);
					__matrixA[4][i] = __matrixA[i][4] = SumProduct(__x2_2, __x2, __n);
					__matrixA[5][i] = __matrixA[i][5] = SumProduct(__x1x2, __x2, __n);
			break;
			case 3: //*x1_2
					__matrixA[3][i] = __matrixA[i][3] = SumProduct(__x1_2, __x1_2, __n);
					__matrixA[4][i] = __matrixA[i][4] = SumProduct(__x2_2, __x1_2, __n);
					__matrixA[5][i] = __matrixA[i][5] = SumProduct(__x1x2, __x1_2, __n);
			break;
			case 4: //*x2_2
					__matrixA[4][i] = __matrixA[i][4] = SumProduct(__x2_2, __x2_2, __n);
					__matrixA[5][i] = __matrixA[i][5] = SumProduct(__x1x2, __x2_2, __n);
			break;
			case 5: //*x1_x2
					__matrixA[5][i] = __matrixA[i][5] = SumProduct(__x1x2, __x1x2, __n);
			break;
		}
	}
    if(DEBUG == 1) {
        printf("Matrix A = \n");
        for (i=0; i<6; i++)
        {
            for(j=0; j<6; j++)
                printf("%lf  ", __matrixA[i][j]);
            printf("\n");
        }
        printf("\n");
    }

	//Calc Matrix C
	__matrixC[0] = SumProduct(__y, __1, __n);
	__matrixC[1] = SumProduct(__y, __x1, __n);
	__matrixC[2] = SumProduct(__y, __x2, __n);
	__matrixC[3] = SumProduct(__y, __x1_2, __n);
	__matrixC[4] = SumProduct(__y, __x2_2, __n);
	__matrixC[5] = SumProduct(__y, __x1x2, __n);

    if(DEBUG == 1) {
        printf("Matrix C = \n");
        for (i=0; i<6; i++)
        {
            printf("C[%d] = %lf     ", i, __matrixC[i]);
            printf("\n");
        }
        printf("\n");
    }
    
    //gauss elimination
    Gauss(__matrixA, __matrixC, __matrixB, 6);
    
    //copy from local var to external var
    for (i=0; i<6; i++){
        returned_b[i]=__matrixB[i];
    }
        
    if(DEBUG == 1) {
        printf("Matrix B = \n");
        for (i=0; i<6; i++)
        {
            printf("B[%d] = %lf     ", i, __matrixB[i]);
            printf("\n");
        }
        printf("\n");
    }
    
    //Mean Squared Error Calc
    int __ms_error=0;
    for(i=0; i<n; i++){
        __ms_error += pow(__y[i] - (__matrixB[0] + __matrixB[1] * __x1[i] + __matrixB[2] * __x2[i] + __matrixB[3] * __x1_2[i] + __matrixB[4] * __x2_2[i] + __matrixB[5] * __x1x2[i]), 2);
    }
    __ms_error /= n;
    
    *returned_sse = __ms_error;
	
    free(__x1);
	free(__x2);
	free(__x1_2);
	free(__x2_2);
	free(__x1x2);
    free(__y);
    
    free(__matrixA);
	free(__matrixB);
    free(__matrixC);
    

	return result;
}

/*
int main(){
	printf("Hello World!\n");

	double x[16] = {1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4};
	double y[16] = {1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4};
	double v[16] =  {10,10,10,10,10,10,10,10,10,10,10,10,12,12,12,12};
    //double v[16] =  {10, 11, 12, 13, 14, 15, 16, 17, 18, 11, 12, 12, 13, 13, 14, 16};
	double coeff[6];
    double ms_error;
    
	polyfit2indepentvars(x, y, v, 16, coeff, &ms_error);
    
    //print equation
    printf("\nf(x,y)= ");
    if (coeff[0]>0.00000001 ||  coeff[0]<-0.00000001)
        printf("%c %lf ", coeff[0] > 0 ? '+' : ' ', coeff[0]);
    if (coeff[1]>0.00000001 ||  coeff[1]<-0.00000001)
        printf("%c %lf * x ", coeff[1] > 0 ? '+' : ' ', coeff[1]);
    if (coeff[2]>0.00000001 ||  coeff[2]<-0.00000001)
        printf("%c %lf * y ", coeff[2] > 0 ? '+' : ' ', coeff[2]);
    if (coeff[3]>0.00000001 ||  coeff[3]<-0.00000001)
        printf("%c %lf * x^2", coeff[3] > 0 ? '+' : ' ', coeff[3]);
    if (coeff[4]>0.00000001 ||  coeff[4]<-0.00000001)
        printf("%c %lf * y^2", coeff[4] > 0 ? '+' : ' ', coeff[4]);
    if (coeff[5]>0.00000001 ||  coeff[5]<-0.00000001)
        printf("%c %lf * x.y", coeff[5] > 0 ? '+' : ' ', coeff[5]);
    printf("\n\n");
    
    printf("Mean Square Error = %lf", ms_error);
    printf("\n\n");
    
    return 0;
}

*/
    
    
}