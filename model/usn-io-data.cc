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

#include <complex>

#include "usn-io-data.h"
#include "ns3/usn-types.h"

namespace ns3 {

    uint8_t
    UNSInputData::LoadFromFile(string fn) {

        //        float data[38][27];
        ifstream file(fn.c_str());

        string line_value = "";
        string item_value = "";
        string tmp_a = "";
        string tmp_t = "";
        string tmp_b = "";
        string tmp_c = "";

        uint8_t r = 0; //error on reading the file
        
        while (file.good()) {
            r = 1;
            getline(file, tmp_a, '@');
            getline(file, tmp_t, '\n');
            stringstream n_matrix(tmp_a);
            vector < vector<uint32_t> > matrix_value;

            while (n_matrix.good()) {
                getline(n_matrix, tmp_b, '\n');
                stringstream n_line(tmp_b);
                vector<uint32_t> line_value;
                
                while (n_line.good()) {
                    getline(n_line, tmp_c, ',');
                    uint32_t item = atof(tmp_c.c_str());
                    line_value.push_back(item);
//                    cout << item << ", ";
                }
                matrix_value.push_back(line_value);
//                cout << "\n";
            }
            matrix_value.pop_back();
            m_data_on_time.push_back(matrix_value);   
//            cout << "@";
//            cout << m_data_on_time.size();
        }


        return r;
    }
    
    uint32_t UNSInputData::ReadNode(uint32_t t, uint32_t x, uint32_t y) {
        //
        //        vector<uint32_t> line;
        //        uint32_t data;
        //
        //        line = m_data.at(y);
        //        data = line.at(x);
        //        return data;
        
        vector< vector <uint32_t> > matrix = m_data_on_time.at(t);
        vector<uint32_t> line = matrix.at(y);
        return line.at(x);
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    
    
        TypeId
    USNOutputData::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::USNOutputData")
                .SetParent<Application> ()
                .AddConstructor<USNOutputData> ()
                ;
        return tid;
    }
        
    USNOutputData::USNOutputData() {
        sink_neighbors.up.y = 100;
        sink_neighbors.down.y = -100;
        sink_neighbors.right.x = 100;
        sink_neighbors.left.x = -100;
    }

    USNOutputData::~USNOutputData(){
        
    }
    
    uint8_t USNOutputData::WriteToFile(string filename, uint8_t n_size){
        
//        DataFit p_sink;
//        p_sink.x = 0; p_sink.y = 0; 
//        //take the mean a b c from the closest cluster heads around it.
//        p_sink.a = (sink_neighbors.up.a + sink_neighbors.down.a + sink_neighbors.left.a + sink_neighbors.right.a) / 4; 
//        p_sink.b = (sink_neighbors.up.b + sink_neighbors.down.b + sink_neighbors.left.b + sink_neighbors.right.b) / 4; 
//        p_sink.c = (sink_neighbors.up.c + sink_neighbors.down.c + sink_neighbors.left.c + sink_neighbors.right.c) / 4; 
//        
//        AddFit(p_sink);
        
        uint8_t x_size = (bounds.x_max - bounds.x_min + 2 * n_size + 1);
        uint8_t y_size = (bounds.y_max - bounds.y_min + 2 * n_size + 1);
        
        matrix data(x_size, vector <uint16_t>(y_size));
        
        //TODO: this is not taking into account the time of each annoucement.
        
        for (uint16_t i = 0 ; i < planes.size() ; i++){
            DataFit p = planes.at(i);
            
            for (int8_t x = n_size*-1 ; x <= n_size ; x++){
                
                for (int8_t y = n_size*-1 ; y <= n_size ; y++){
                    
                    if (abs(x) + abs(y) <= n_size) {
                        int32_t z;
                        double dz;
                        if (p.type == EV_PLANE)
                            z = x * p.a + y * p.b + p.c;
                        
                        else if (p.type == EV_CURVE) //z = a * b^x * c^y
                        {
                            dz = (double)p.a /100 + (double)p.b / 1000000000000000 * x + (double)p.c / 1000000000000000 * y;
                            z = (int32_t)exp(dz);
                        }
                            
                        
                        int16_t x_m = p.x + x + ceil(float(x_size-1)/2) ;
                        int16_t y_m = p.y + y + ceil(float(y_size-1)/2) ;
                        
                        if(x_m >= 0 && x_m < x_size && y_m >= 0 && y_m < y_size)
                        {
//                            if (data[x_m][y_m] == 0)
                            if (z < 0) z=0;
                            if (z > 65535) z=65535;
                            if (data[x_m][y_m] == 0)
                                data[x_m][y_m] = (uint16_t)z;
                            else
//                                data[x_m][y_m] = (data[x_m][y_m] + (uint16_t) z) / 2;
                                data[x_m][y_m] = (data[x_m][y_m]);
                        }
                    }
                }
            }
        }
        
        
        // write to disk
        ss.open(filename.c_str(), ios::out);
        
        for (int32_t j = data[0].size()-1 ; j > -1  ; j--){
            for (int32_t i = data.size()-1 ; i > -1 ; i--){
                if (i > 0) ss << data[i][j] << ",";
                else ss << data[i][j];
            }
            ss << endl;
        }
        ss << "@0" << endl;
        ss.close();
        
        return 0;
    }
    
    uint8_t USNOutputData::WritePointsToFile(string filename, uint8_t x_size, uint8_t y_size){
        
        matrix data(x_size, vector <uint16_t>(y_size));
        
        for (uint16_t i = 0 ; i < points.size() ; i++){
            Point p = points.at(i);
            
            int16_t x_m = p.x + ceil(float(x_size-1)/2) ;
            int16_t y_m = p.y + ceil(float(y_size-1)/2) ;
            
            data[x_m][y_m] = (uint16_t)p.value;
        }
        
        ss.open(filename.c_str(), ios::out);
        
        for (int32_t j = data[0].size()-1 ; j > -1  ; j--){
            for (int32_t i = data.size()-1 ; i > -1 ; i--){
                if (i > 0) ss << data[i][j] << ",";
                else ss << data[i][j];
            }
            ss << endl;
        }
        ss << "@0" << endl;
        ss.close();
        
        return 0;
        
    }
    
    uint32_t USNOutputData::AddFit(DataFit p){
        
        if (p.x > bounds.x_max) bounds.x_max = p.x;
        if (p.x < bounds.x_min) bounds.x_min = p.x;
        if (p.y > bounds.y_max) bounds.y_max = p.y;
        if (p.y < bounds.y_min) bounds.y_min = p.y;
        
        if (p.y == 0 && p.x > 0 && p.x < sink_neighbors.right.x)
            sink_neighbors.right = p; //up neighbour
        if (p.y == 0 && p.x < 0 && p.x > sink_neighbors.left.x)
            sink_neighbors.left = p; //botom neighbour
        if (p.x == 0 && p.y > 0 && p.y < sink_neighbors.up.y)
            sink_neighbors.up = p; //right neighbour
        if (p.x == 0 && p.y < 0 && p.y > sink_neighbors.down.y)
            sink_neighbors.down = p; //left neighbour
        
        planes.push_back(p);
        
        return 0;
    }
   

    
    
    uint32_t USNOutputData::AddPoint(Point p){
        
        
        points.push_back(p);
        
        return 0;
    }

}

