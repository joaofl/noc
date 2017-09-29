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
#include "data-io.h"
/////////////////////////////////////////////////////////////////////////////////////////////
typedef struct stat Stat;

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_mkpath_c[] = "@(#)$Id: mkpath.c,v 1.13 2012/07/15 00:40:37 jleffler Exp $";
#endif /* lint */

int do_mkdir(const char *path, mode_t mode)
{
    Stat            st;
    int             status = 0;

    if (stat(path, &st) != 0)
    {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(path, mode) != 0 && errno != EEXIST)
            status = -1;
    }
    else if (!S_ISDIR(st.st_mode))
    {
        errno = ENOTDIR;
        status = -1;
    }

    return(status);
}

/**
** mkpath - ensure all directories in path exist
** Algorithm takes the pessimistic view and works top-down to ensure
** each directory in path exists, rather than optimistically creating
** the last element and working backwards.
*/
int mkpath(const char *path)
{
    mode_t mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
    char           *pp;
    char           *sp;
    int             status;
    char           *copypath = strdup(path);

    status = 0;
    pp = copypath;
    while (status == 0 && (sp = strchr(pp, '/')) != 0)
    {
        if (sp != pp)
        {
            /* Neither root nor double slash in path */
            *sp = '\0';
            status = do_mkdir(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if (status == 0)
        status = do_mkdir(path, mode);
    free(copypath);
    return (status);
}
/////////////////////////////////////////////////////////////////////////////////////////////

namespace ns3 {

    bool
    DataIO::LoadArray(string fn) {

        ifstream file(fn.c_str());
        string sv;
        float v;

        while (file.good()) {
            getline(file, sv, '\n');
            v = std::atof(sv.c_str());
            m_data_delay.push_back(v);
        }
        
        if (m_data_delay.size() == 0)
            return false;
        else
            return true;
    }
    uint32_t 
    DataIO::GetArraySize() {
        return m_data_delay.size();
    }

    
    float
    DataIO::GetValue(uint32_t i) {
        return m_data_delay.at(i);
    } 

    uint8_t
    DataIO::LoadArray3D(string fn) {

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
            vector < vector<int64_t> > matrix_value;

            while (n_matrix.good()) {
                getline(n_matrix, tmp_b, '\n');
                stringstream n_line(tmp_b);
                vector<int64_t> line_value;
                
                while (n_line.good()) {
                    getline(n_line, tmp_c, ',');
                    int64_t item = atof(tmp_c.c_str());
                    line_value.push_back(item);
                }
                matrix_value.push_back(line_value);
            }
            matrix_value.pop_back();
            m_data_sensors_on_time.push_back(matrix_value);   
        }


        return r;
    }
    
    uint32_t 
    DataIO::GetArraySize3D() {
        return m_data_delay.size();
    }
    
    int64_t 
    DataIO::GetValue3D(uint32_t t, uint32_t x, uint32_t y) {
        //
        //        vector<uint32_t> line;
        //        uint32_t data;
        //
        //        line = m_data.at(y);
        //        data = line.at(x);
        //        return data;
        
        vector< vector <int64_t> > matrix = m_data_sensors_on_time.at(t);
        vector<int64_t> line = matrix.at(y);
        return line.at(x);
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    bool
    NOCRouterShapingConf::LoadData(string fn) {

        ifstream file(fn.c_str());
        string sv = "";
        string lv = "";
        float v;

        while (file.good()) {
            getline(file, sv, '\n');
            stringstream line(sv);
            vector<float> line_value;

            while (line.good()) {
                getline(line, lv, ',');
                v = atof(lv.c_str());
                v=v;
                line_value.push_back(v);
            }
            m_data.push_back(line_value);
        }
        
        if (m_data.size() == 0)
            return false;
        else
            return true;
    }
    uint32_t 
    NOCRouterShapingConf::GetArraySize() {
        return m_data.size();
    }
    
    bool
    NOCRouterShapingConf::IsShaped(int32_t x_in, int32_t y_in, uint8_t p_in) {
            
        if (m_data.size() > 0){
            int32_t x;
            int32_t y;
            uint8_t p;  
            for (uint32_t i = 0 ; i < m_data.size() - 1; i++){
                x = m_data.at(i).at(0);
                y = m_data.at(i).at(1);
                p = m_data.at(i).at(2);         

                if (x_in == x && y_in == y && p_in == p){
                    return true;
                }
            }
        }
        
        return false;
    }

    float 
    NOCRouterShapingConf::GetBurstiness(int32_t x_in, int32_t y_in, uint8_t p_in) {
        int32_t x;
        int32_t y;
        uint8_t p;        
        for (uint32_t i = 0 ; i < m_data.size() - 1; i++){
            x = m_data.at(i).at(0);
            y = m_data.at(i).at(1);
            p = m_data.at(i).at(2);         
            
            if (x_in == x && y_in == y && p_in == p){
                return(m_data.at(i).at(3));
            }
        }
        return 1;
    }

    uint32_t NOCRouterShapingConf::GetMsgSize(int32_t x_in, int32_t y_in, uint8_t p_in) {
        int32_t x;
        int32_t y;
        uint8_t p;        
        for (uint32_t i = 0 ; i < m_data.size() - 1; i++){
            x = m_data.at(i).at(0);
            y = m_data.at(i).at(1);
            p = m_data.at(i).at(2);         
            
            if (x_in == x && y_in == y && p_in == p){
                return(m_data.at(i).at(5));
            }
        }
        return 0;
    }

    float NOCRouterShapingConf::GetOffset(int32_t x_in, int32_t y_in, uint8_t p_in) {
        int32_t x;
        int32_t y;
        uint8_t p;        
        for (uint32_t i = 0 ; i < m_data.size() - 1; i++){
            x = m_data.at(i).at(0);
            y = m_data.at(i).at(1);
            p = m_data.at(i).at(2);         
            
            if (x_in == x && y_in == y && p_in == p){
                return(m_data.at(i).at(4));
            }
        }
        return 0;
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    


    XDenseSensorModel::XDenseSensorModel()
    : m_running(false){
    }
    
    XDenseSensorModel::~XDenseSensorModel() {
    }
    
    void
    XDenseSensorModel::StartApplication(void) {
        Ptr<Node> nd = this->GetNode();
        m_time_instant = 0;
    }
    
    int64_t 
    XDenseSensorModel::ReadSensor(void){
        
        int64_t r = InputData->GetValue3D(m_time_instant, SensorPosition.x, SensorPosition.y);
        m_time_instant++;
        
        return r;
    }    
    void
    XDenseSensorModel::StopApplication(void) {
        m_running = false;

        if (m_sendEvent.IsRunning()) {
            Simulator::Cancel(m_sendEvent);
        }
    }
    
    
    
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    
    NOCRouterDelayModel::NOCRouterDelayModel()
    : m_running(false){
    }

    NOCRouterDelayModel::~NOCRouterDelayModel() {
    }


    void
    NOCRouterDelayModel::StartApplication(void) {
        m_random = CreateObject<UniformRandomVariable> ();
        random_counter = 0;
    }
    
    Time 
    NOCRouterDelayModel::GetDelay(void) {
        uint32_t i;
        float v;

        Time t = Time::FromInteger(0, Time::NS);
        uint32_t size = InputData->GetArraySize();
        if (size > 0){
            i = m_random->GetInteger(0, size -2);
            random_counter++;
            
//            cout << "i=" << random_counter << " n=" << i << "\n";
            
            v = InputData->GetValue(i);
            t = Time::FromDouble(v, Time::NS);            
        }        
        return t;
    }

    
    void
    NOCRouterDelayModel::StopApplication(void) {
        m_running = false;

        if (m_sendEvent.IsRunning()) {
            Simulator::Cancel(m_sendEvent);
        }
    }
    
    
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    
    TypeId
    NOCOutputDataSensors::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::NOCOutputData")
                .SetParent<Application> ()
                .AddConstructor<NOCOutputDataSensors> ()
                ;
        return tid;
    }
        
    NOCOutputDataSensors::NOCOutputDataSensors() {
        sink_neighbors.up.y = 100;
        sink_neighbors.down.y = -100;
        sink_neighbors.right.x = 100;
        sink_neighbors.left.x = -100;
    }

    NOCOutputDataSensors::~NOCOutputDataSensors(){
        
    }
    
    uint8_t NOCOutputDataSensors::WriteToFile(string filename, uint8_t n_size){
        
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
    
    uint8_t NOCOutputDataSensors::WritePointsToFile(string filename, uint8_t x_size, uint8_t y_size){
        
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
    
    uint32_t NOCOutputDataSensors::AddFit(DataFit p){
        
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
  
    uint32_t NOCOutputDataSensors::AddPoint(Point p){
        points.push_back(p);
        return 0;
    }


    
}