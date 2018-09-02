#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cassert>
#include <iomanip>
#include <typeinfo>
#include <algorithm>
#include <math.h>
#include "chipMgr.h"
#include "util.h"
#include "include/gurobi_c++.h"

// #define DEBUG
using namespace std;


// find the area/fringe capacitance by using a hashmap
double chipManager::calCapicitance(double area, int type, int layer1, int layer2)
{
    int key = type * (layer_num + 1) * (layer_num + 1) + layer1 * (layer_num + 1) + layer2;

    if (total_Cap_List.find(key) == total_Cap_List.end())
    {
        cout << "[Error] can't find correspond cap rule key ="<< key <<"\n";
        return 0;
    }
    Capacitance* c = total_Cap_List[key];
    return c->getCapacitance(area) * pow(10, 10);
}

// find the lateral capacitance by using a hashmap
double chipManager::calCapicitance(int overlap, int space, int layer)
{
    int type = LATERAL;
    int key = type * (layer_num+1) * (layer_num+1) + layer * (layer_num+1) + layer;

    if (total_Cap_List.find(key) == total_Cap_List.end())
    {
        cout << "[Error] can't find correspond cap rule key =" << key << "\n";
        return 0;
    }
    Capacitance *c = total_Cap_List[key];
    return c->getCapacitance(overlap, space) * pow(10, 10);
}

void chipManager::init_polygon(string &filename, unordered_set<int> &cnet_set, vector<bool>&VorH_v)
{
    // cout<<"init poly..."<<endl;
    ifstream ifs(filename);
    int filesize;
    ifs.seekg(0, ios::end);
    filesize = ifs.tellg();
    ifs.seekg(0, ios::beg);
    char* buff = new char[filesize+1];
    ifs.read(buff, filesize);
    char* buff_beg = buff;
    char* buff_end = buff + filesize;
    string token="haha";
    int num;
    bool first_line = true;
    vector< vector< vector<int> > > insert_vec;
    vector<int> tokens;
    vector<int> layer_bound;
    for (int i = 0; i < layer_num; ++i){
        vector< vector<int> > vec;
        insert_vec.push_back(vec);
    }
    Polygon* poly;
    int aa = 0, bb = 1;
    int _bl_bound_x1, _bl_bound_y1, _tr_bound_x1, _tr_bound_y1; 
    int x_len = 0, y_len = 0, x_len_big = 0, y_len_big = 0;
    bool VorH[layer_num];
    while (token != ""){
        if (first_line){
            while ( (token = next_token(buff_beg, buff_end)) != ""){
                if (token[0] == '\n') {first_line = false; break;}
                if (myStr2Int(token, num)){
                    layer_bound.push_back(num);
                }
                else{
                    _bl_bound_x = layer_bound[0];
                    _bl_bound_y = layer_bound[1];
                    _tr_bound_x = layer_bound[2];
                    _tr_bound_y = layer_bound[3];
                }
            }
        }
        else {
            while ( (token = next_token(buff_beg, buff_end)) != ""){
                if (token[0] == '\n') break;
                if (myStr2Int(token, num)){
                    tokens.push_back(num);
                }
                else {
                    if (insert_vec[tokens[6]-1].size() < 100){
                        insert_vec[tokens[6]-1].push_back(tokens);
                        if(insert_vec[tokens[6]-1].size() == 100)
                        {
                            for (int i = 0; i < 100; ++i){
                                x_len = insert_vec[tokens[6]-1][i][3]-insert_vec[tokens[6]-1][i][1];
                                y_len = insert_vec[tokens[6]-1][i][4]-insert_vec[tokens[6]-1][i][2];
                                if (x_len > y_len){
                                    x_len_big = x_len_big + 1;
                                }
                                else if (y_len > x_len){
                                    y_len_big = y_len_big + 1;
                                }
                            }
                            //不用轉就是true
                            if(x_len_big > y_len_big) {
                                VorH[tokens[6]-1] = false;
                                _bl_bound_y1 = layer_bound[0];
                                _bl_bound_x1 = layer_bound[1];
                                _tr_bound_y1 = layer_bound[2];
                                _tr_bound_x1 = layer_bound[3];
                            } 
                            else {
                                VorH[tokens[6]-1] = true;
                                _bl_bound_x1 = layer_bound[0];
                                _bl_bound_y1 = layer_bound[1];
                                _tr_bound_x1 = layer_bound[2];
                                _tr_bound_y1 = layer_bound[3];
                            }
                            cout<<"================= layer id = "<<tokens[6]-1<<"---";
                            // if (VorH[tokens[6]-1]) cout<<"same coordinate ==============="<<endl;
                            // else cout<<"rotated coordinate ============"<<endl;
                            // cout<<"y_big = "<<y_len_big<<", x_big = "<<x_len_big<<endl;
                            // cout<<"1 = "<<layer_bound[0]<<" 2 = "<<layer_bound[1]<<endl;
                            y_len_big = 0, x_len_big = 0;
                            _LayerList[tokens[6]-1].init_layer(_bl_bound_x1, _bl_bound_y1, _tr_bound_x1, _tr_bound_y1);
                            for (int i = 0; i < 100; ++i){
                                poly = new Polygon();
                                poly->setToSolid();
                                if (VorH[tokens[6]-1]) poly->set_coordinate_V(insert_vec[tokens[6]-1][i]);
                                else poly->set_coordinate_H(insert_vec[tokens[6]-1][i]);
                                if (cnet_set.count(insert_vec[tokens[6]-1][i][5])) {
                                    poly->setToCNet();
                                }
                                _LayerList[poly->get_layer_id()-1].insert(poly, true, _LayerList[poly->get_layer_id()-1].get_dummy());
                                ++aa;
                            }
                        }
                    }
                    else{
                        poly = new Polygon();
                        poly->setToSolid();
                        if (VorH[tokens[6]-1]) poly->set_coordinate_V(tokens);
                        else poly->set_coordinate_H(tokens);
                        if (cnet_set.count(tokens[5])){
                            poly->setToCNet();
                        }
                        _LayerList[poly->get_layer_id()-1].insert(poly, true, _LayerList[poly->get_layer_id()-1].get_dummy());
                        ++aa;
                    }
                    tokens.clear();
                }
            }
            #ifdef DEBUG
                //aa++;
                //cout<<"insert num = "<<aa<<endl;
                cout<<"..............layer id = "<<poly->get_layer_id()<<" .................."<<endl;
                cout<<".................finish poly....................."<<endl;
            #endif
        }
    }
    cout << "===    Finish inserting "<< aa << " polygon    ===" << endl;
    for (int i = 0; i < layer_num; ++i){
        VorH_v.push_back(VorH[i]);
    }
}
void chipManager::rotate_dummy(Layer layer)
{
/*              _________
before:     __7_|___8___|_1__          
            |   |       |   |
            | 6 |       | 2 |    
            |___|_______|___|
              5 |___4___| 3
                _________
after:      __3_|___2___|_1__         
            |   |       |   |
            | 4 |       | 8 |    
            |___|_______|___|
              5 |___6___| 7
*/
    Polygon* d4 = layer.get_dummy(); Polygon* d5 = d4->get_bl(); Polygon* d6 = d5->get_rt(); Polygon* d7 = d6->get_rt();
    Polygon* d8 = d7->get_tr(); Polygon* d1 = d8->get_tr(); Polygon* d2 = d1->get_lb(); Polygon* d3 = d2->get_lb();
    /* dummy botttom swap(4) */
    d4->swap_xy(); d4->swap_top_right(); d4->set_lb(d5); d4->set_bl(0);
    /* dummy botttom left(5) swap */
    d5->swap_xy(); d5->swap_top_right(); d5->set_lb(0); d5->set_bl(0);
    /* dummy botttom right(3) swap */
    d3->swap_xy(); d3->set_tr(d2); d3->set_lb(d4); d3->set_rt(0); d3->set_bl(0); 
    /* dummy right swap(2) */
    d2->swap_xy(); d2->swap_bottom_left(); d2->set_tr(d1); d2->set_rt(0);
    /* dummy right top(1) swap */
    d1->swap_xy(); d1->swap_top_right(); d2->set_tr(0); d2->set_rt(0);
    /* dummy top swap(8) */
    d8->swap_xy(); d8->swap_bottom_left(); d8->set_rt(d1); d8->set_tr(0);
    /* dummy top left swap(7) */
    d7->swap_xy(); d7->set_rt(d8); d7->set_bl(d6); d7->set_tr(0); d7->set_tr(0);
    /* dummy left swap(6) */
    d6->swap_xy(); d6->swap_top_right(); d6->set_bl(d5); d6->set_lb(0);
}



void chipManager::report_density(bool init_cnet){
    int x, y, wnd_num;
    double density = 0;
    double count[9] = {0}, count2[9] = {0};
    int half_wnd = window_size / 2;
    int horizontal_cnt = (_tr_bound_x - _bl_bound_x)  / half_wnd -1;
    int vertical_cnt = (_tr_bound_y - _bl_bound_y)  / half_wnd -1 ;
    vector<Polygon*> critical_nets;

    for (int i = 0; i < layer_num; ++i){
        for(int row = 0; row < vertical_cnt; ++row){
            for (int col = 0; col < horizontal_cnt; ++col){
                x = _bl_bound_x + col * half_wnd;
                y = _bl_bound_y + row * half_wnd;
                wnd_num = i * horizontal_cnt * vertical_cnt + row * horizontal_cnt + col;
                density = _LayerList[i].density_calculate(x, y, window_size, critical_nets);
                if(init_cnet)
                    // total_Cnet_List[].pus(critical_nets);
                if(density >= _LayerList[i].get_min_density() && density < _LayerList[i].get_min_density()+0.01)
                    count[0]+=1;
                else if (density >= _LayerList[i].get_min_density()+0.01 && density < _LayerList[i].get_min_density()+0.04)
                    count[1]+=1;
                else if (density >= _LayerList[i].get_min_density()+0.04 && density < _LayerList[i].get_min_density()+0.07)
                    count[2]+=1;
                else if (density >= _LayerList[i].get_min_density()+0.07 && density < _LayerList[i].get_min_density()+0.1)
                    count[3]+=1;
                else if (density >= _LayerList[i].get_min_density()+0.1)
                    count[4]+=1;
                else{
                    //cout << i+1 << " row " <<row/2 << " col "<<col/2 <<" pos " <<(row%2)*2 + (col%2) <<endl;
                }
                if (density >= _LayerList[i].get_min_density())
                    ++count2[0];
            }
        }       
    }
    cout << "\n============[denity report]============\n";
    for (int i = 0; i < 5; ++i){
        cout<<"Density "<<0.4 + 0.01*i<<" - "<< 0.4 + 0.01*(i+1) <<" : "<<count[i]/count2[0]*100<<"% \n";
    }
    // for (int i = 0; i < 9; i++)
    //     cout << "Layer #" << i + 1 << " | density:" << count[i] / count2[i] * 100 << "% \n";
}

void chipManager::insert_tile(string& output_fill){ 
    vector<Polygon*> critical_nets;
    int fillnum = 1;
    stringstream out_fill;

    for (int _time = 0; _time < 4; ++_time){
        int x, y, wnd_num;
        double density = 0;
        int half_wnd = window_size / 2;
        int horizontal_cnt = (_tr_bound_x - _bl_bound_x) / window_size;
        int vertical_cnt = (_tr_bound_y - _bl_bound_y) / window_size;
        if(_time == 1 || _time == 3)
            horizontal_cnt -= 1;
        if(_time == 2 || _time == 3)
            vertical_cnt -= 1;
        for (int i = 0; i < layer_num; ++i){
            for(int row = 0; row < vertical_cnt; ++row){
                for (int col = 0; col < horizontal_cnt; ++col){
                    x = _bl_bound_x + col * window_size;
                    y = _bl_bound_y + row * window_size;
                    if(_time == 1 or _time == 3)
                        x += half_wnd;
                    if(_time == 2 or _time == 3)
                        y += half_wnd;
                    wnd_num = i * horizontal_cnt * vertical_cnt + row * horizontal_cnt + col;                
                    density = _LayerList[i].density_calculate(x, y, window_size, critical_nets);
                    
                    if(density < _LayerList[i].get_min_density()){
                        cout << "error\n";
                        double new_density = density;
                        // cout << "\n==========[ Layer: " << i + 1 << " | Window: " << row * horizontal_cnt + col << "/"
                        //    << horizontal_cnt * vertical_cnt << "]=========="<< endl;
                        // cout << "\n=============[ Layer: " << i + 1 << " | Row: " << row/2 << "| Col:"
                        //     << col/2 << " | Pos:" << (row%2)*2+col%2 << "]=============" << endl;
                        string out = "";
                        //cout<<"insert dummy in Layer: "<<setw(1)<<i+1<<".......................\r";
                        _LayerList[i].layer_fill(x, y, window_size, new_density, i + 1, out, fillnum);
                        out_fill<<out;
                        // cout<<"新的密度 "<<new_density<<" "<<x<<","<<y<<" windownum= "<<wnd_num<<" layer id= "<<i+1<<endl;
                    }
                }
            }       
        }
    }
    //cout << endl;
    int horizontal_cnt = (_tr_bound_x - window_size - _bl_bound_x) * 2 / window_size + 1;
    int vertical_cnt = (_tr_bound_y - window_size - _bl_bound_y) * 2 / window_size + 1;

    int x, y, wnd_num;
    double density = 0;
    int half_wnd = window_size / 2;

    for (int i = 0; i < layer_num; ++i){
        for(int row = 0; row < vertical_cnt; ++row){
            for (int col = 0; col < horizontal_cnt; ++col){
                x = _bl_bound_x + col * window_size / 2;
                y = _bl_bound_y + row * window_size / 2;
                wnd_num = i * horizontal_cnt * vertical_cnt + row * horizontal_cnt + col;                
                density = _LayerList[i].density_calculate(x, y, window_size, critical_nets);
                if(density < _LayerList[i].get_min_density()){  
                    cout<<"error\n";
                    double new_density = density;
                    cout << "\n==========[ Layer: " << i + 1 << " | Window: " << row * horizontal_cnt + col << "/"
                         << horizontal_cnt * vertical_cnt << "]==========" << endl;
                    string out = "";
                    //cout<<"insert dummy in Layer: "<<setw(1)<<i+1<<".......................\r";
                    _LayerList[i].layer_fill(x, y, window_size, new_density, i + 1, out, fillnum);
                    out_fill<<out;
                    // cout<<"新的密度 "<<new_density<<" "<<x<<","<<y<<" windownum= "<<wnd_num<<" layer id= "<<i+1<<endl;
                }
            }
        }       
    }
    ///cout << endl;
    output_fill = out_fill.str();
}

void chipManager::write_fill(string output, string output_fill){
    ofstream ofs(output);

    //ofs.open(output.c_str());
    if(!ofs.is_open()){
        cout << "[Error] can't open rule file \"" << output << "\" !!\n";
        return;
    }
    ofs<<output_fill;
    ofs.close();
    cout<<"...........finish.............."<<output<<endl;
}

void chipManager::check_layer(string &filename)
{
    ifstream ifs(filename);
    int filesize;
    ifs.seekg(0, ios::end);
    filesize = ifs.tellg();
    ifs.seekg(0, ios::beg);
    char* buff = new char[filesize+1];
    ifs.read(buff, filesize);
    char* buff_beg = buff;
    char* buff_end = buff + filesize;
    string token = "haha";
    int num;
    vector<int> tokens;

    while (token != ""){
        while ( (token = next_token(buff_beg, buff_end)) != "" ){
            if (token[0] == '\n') { break; }
            if (myStr2Int(token, num)){
                tokens.push_back(num);
            }
            else{
                int a = tokens[6]-1;
                cout<<"layer id = "<<a<<endl;
                int count = 0;
                vector<Polygon*> query_list;
                int gap = _LayerList[a].get_gap();
                int y2 = tokens[4], y1 = tokens[2], x2 = tokens[1], x1 = tokens[3];
                int bl_x, bl_y, tr_x, tr_y;
                (x2 - _LayerList[a].get_gap() >= _LayerList[a].get_bl_boundary_x()) ? bl_x = x2 - _LayerList[a].get_gap()      : bl_x = _LayerList[a].get_bl_boundary_x();
                (y2 - _LayerList[a].get_gap() >= _LayerList[a].get_bl_boundary_y()) ? bl_y = y2 - _LayerList[a].get_gap()      : bl_y = _LayerList[a].get_bl_boundary_y();
                (x1 + _LayerList[a].get_gap() >  _LayerList[a].get_tr_boundary_x()) ? tr_x = _LayerList[a].get_tr_boundary_x() : tr_x = x1 + _LayerList[a].get_gap()     ;
                (y1 + _LayerList[a].get_gap() >  _LayerList[a].get_tr_boundary_y()) ? tr_y = _LayerList[a].get_tr_boundary_y() : tr_y = y1 + _LayerList[a].get_gap()     ; 
                       
                _LayerList[a].region_query( _LayerList[a].get_dummy(), tr_x, tr_y, bl_x, bl_y, query_list);
                for (int i = 0; i < query_list.size(); ++i){
                    if (query_list[i]->is_solid())
                    {
                        int area = (query_list[i]->_top_right_x()-query_list[i]->_bottom_left_x())*(query_list[i]->_top_right_y()-query_list[i]->_bottom_left_y());
                        cout<<"..............area = "<<area<<endl;
                        cout<<"..............maxa = "<< _LayerList[a].get_max_width()*_LayerList[a].get_max_width()<<endl;
                        assert(area < _LayerList[a].get_max_width()*_LayerList[a].get_max_width());
                        cout<<"a "<<query_list[i]->_top_right_x() - query_list[i]->_bottom_left_x()<<endl;
                        cout<<"b "<<_LayerList[a].get_max_width()<<endl;
                        assert(query_list[i]->_top_right_x() - query_list[i]->_bottom_left_x() <= _LayerList[a].get_max_width());
                        assert(query_list[i]->_top_right_y() - query_list[i]->_bottom_left_y() <= _LayerList[a].get_max_width());
                        assert(query_list[i]->_top_right_x() - query_list[i]->_bottom_left_x() >= _LayerList[a].get_width());
                        assert(query_list[i]->_top_right_y() - query_list[i]->_bottom_left_y() >= _LayerList[a].get_width());
                        assert(area >= _LayerList[a].get_width()*_LayerList[a].get_width());
                        count++;
                        if (count > 1){
                            cout<<".............gap error.....Layer id = "<<a<<"  ............."<<endl;
                            print_Polygon(query_list[i]);
                        }
                    }
                }
            }

        }
    }
}

void chipManager::preprocess(GRBModel* model, int layer, vector<bool> VorH)
{
    vector<Polygon*> polygon_list;
    int slot_id = 0;
    int space_count = 0;
    int threshold = _LayerList[layer].get_width() + 2 * _LayerList[layer].get_gap();
    
    _LayerList[layer].region_query(
        _LayerList[layer].get_dummy()->get_bl(),_LayerList[layer].get_tr_boundary_x(),
        _LayerList[layer].get_tr_boundary_y(),_LayerList[layer].get_bl_boundary_x(),
        _LayerList[layer].get_bl_boundary_y(), polygon_list);
    if (VorH[layer] == false)
    {
        rotate_dummy(_LayerList[layer]);
        _LayerList[layer].layer_rotate();
    }

    for (int j = 0; j < polygon_list.size(); ++j){
        if (VorH[layer] == false)   
            polygon_list[j]->rotate();
        if (polygon_list[j]->is_critical())
            total_Cnet_List[layer].push_back(polygon_list[j]);
    }

    cout<<"start slot split in layer "<<layer+1<<endl;
    for(int i=0; i < polygon_list.size(); i++){
        if(polygon_list[i]->getType() == "space"){
            space_count++;
            int poly_w = polygon_list[i]->_top_right_x() - polygon_list[i]->_bottom_left_x();
            int poly_h = polygon_list[i]->_top_right_y() - polygon_list[i]->_bottom_left_y();
            if (poly_w >= threshold && poly_h >= threshold)
            {
                // print_Polygon(polygon_list[i]);
                _LayerList[layer].insert_slots(model, polygon_list[i], poly_w, poly_h, slot_id);
            }
            // cout << "space_count = "<<space_count<<endl;
        }
        else if (polygon_list[i]->is_critical()){

        }
    }
    // total_Cnet_List.emplace(layer, critical_nets);
    cout<<"end insert slots"<<endl;
    // vector<Polygon*> tmp;
    // _LayerList[layer].region_query(_LayerList[layer].get_dummy(),
    //     _LayerList[layer].get_tr_boundary_x(),
    //     _LayerList[layer].get_tr_boundary_y(), 
    //     _LayerList[layer].get_bl_boundary_x(),
    //     _LayerList[layer].get_bl_boundary_y(), 
    //     tmp);
    // cout<<"(preprocess) end query"<<endl;
    // int count = 0;
    // for(int ii=0 ;ii<tmp.size()/100;ii++){
    //     cout << tmp[ii]->getType() <<endl;
    //     if(tmp[ii]->getType() == "slot"){
    //         // assert(tmp[ii]->get_slot_id() != 0);
    //         ++count;
    //     }
    // }
    // cout << "total slot count = "<<count << endl;
    // cout<<"X = "<<_LayerList[layer].get_bl_boundary_x()<<" Y = "<<_LayerList[layer].get_bl_boundary_y()<<" Xr = "<<_LayerList[layer].get_bl_boundary_x()+window_size
    //     <<" Yr = "<<_LayerList[layer].get_bl_boundary_y()+window_size<<endl;
}

// specify constraints in every window 
void chipManager::layer_constraint(GRBModel* model, int layer_id){
    int x, y;
    int half_wnd = window_size / 2;
    int horizontal_cnt = (_LayerList[layer_id].get_tr_boundary_x() - _LayerList[layer_id].get_bl_boundary_x()) / half_wnd - 1;
    int vertical_cnt = (_LayerList[layer_id].get_tr_boundary_y() - _LayerList[layer_id].get_bl_boundary_y()) / half_wnd - 1;
    vector<Polygon *> slots;
    for (int row = 0; row < vertical_cnt; ++row)
    {
        for (int col = 0; col < horizontal_cnt; ++col)
        {
            x = _LayerList[layer_id].get_bl_boundary_x() + col * half_wnd;
            y = _LayerList[layer_id].get_bl_boundary_y() + row * half_wnd;
            int area = _LayerList[layer_id].slot_area(x, y, window_size, slots);
            int min_area = _LayerList[layer_id].get_min_density() * window_size * window_size;
            GRBQuadExpr slot_exp = slot_constraint(model, x, y, slots);
            // cout << "x: " << x/1000.0 << "k y: " << y/1000.0 << "k slot size: " << slots.size() <<endl;
            // cout << "window slot expression size: " << slot_exp.size() <<endl;

            // density constraint
            string name = to_string(layer_id) + '_' + to_string(row) + '_' + to_string(col);
            model->addQConstr(slot_exp + area  >= min_area, name);
        }
    }
    cout << "===== finish adding layer constraints  ( slot + metal >= 0.4 ) " << endl;
}

// 
GRBQuadExpr chipManager::slot_constraint(GRBModel *model, const int &x, const int &y, vector<Polygon *> &slots)
{
    GRBQuadExpr slot_exp = GRBQuadExpr();
    for (int i = 0; i < slots.size(); ++i)
    {
        assert(slots[i]->is_slot());
        vector<int> W_ij_coordinate;
        GRBLinExpr height = GRBLinExpr();

        int middle = min(slots[i]->get_Wi_coord(-1), int(y + window_size));
        int width = min(int(x + window_size), slots[i]->_top_right_x()) - max(x, slots[i]->_bottom_left_x());
        assert(width > 0);

        middle = max(middle, y);
        for (int j = 0; j < 8; ++j)
        {
            int w = min(slots[i]->get_Wi_coord(j), int(y + window_size));
            w = max(w, y);
            height += abs(w - middle) * slots[i]->getVariable(j);
        }
        
        GRBQuadExpr express = slots[i]->getVariable(-1) * height * width;
        slot_exp += express;
    }
    return slot_exp;
}

// minimize slot coupleing cap with critical net
void chipManager::minimize_cap(GRBModel *model, int layer_id){
    GRBQuadExpr cap_expression = 0;
    for (int i = 0; i < total_Cnet_List[layer_id].size(); ++i)
    {
        Polygon* C = total_Cnet_List[layer_id][i];
        vector<Polygon*> poly_list;
        int min_space = _LayerList[layer_id].get_gap();
    
        // cout<<"start left right...."<<endl;
        // _LayerList[layer_id].critical_find_lr(C, poly_list);

        // for (int j = 0; j < poly_list.size(); ++j)
        // {
        //     int overlap = min(C->_top_right_y(), poly_list[j]->_top_right_y()) - 
        //                   max(C->_bottom_left_y(), poly_list[j]->_bottom_left_y());
        //     GRBLinExpr single_cap = calCapicitance(overlap, min_space, layer_id + 1) * poly_list[j]->getPortion();
        //     cap_expression += single_cap * poly_list[j]->getVariable(-1);
        // }

        // cout<<"start top...."<<endl;
        _LayerList[layer_id].critical_find_top(C, poly_list);
        for (int j = 0; j < poly_list.size(); ++j)
        {
            int overlap = min(C->_top_right_x(), poly_list[j]->_top_right_x()) - 
                          max(C->_bottom_left_x(), poly_list[j]->_bottom_left_x());
            GRBLinExpr single_cap = 0;
            for (int k = 4; k < 8; ++k){
                int space = poly_list[j]->get_Wi_coord(k) - C->_top_right_y();
                double cap = calCapicitance(overlap, space, layer_id+1);
                assert(cap != 0);
                single_cap += cap * poly_list[j]->getVariable(k);
            }
            cap_expression += single_cap * poly_list[j]->getVariable(-1);
            cap_expression += single_cap;
            cout << cap_expression.size() << endl;
        }

        // cout<<"start bo...."<<endl;
        // _LayerList[layer_id].critical_find_bottom(C, poly_list);
        // for (int j = 0; j < poly_list.size(); ++j)
        // {
        //     int overlap = min(C->_top_right_x(), poly_list[j]->_top_right_x()) -
        //                   max(C->_bottom_left_x(), poly_list[j]->_bottom_left_x());
        //     GRBLinExpr single_cap = 0;
        //     for (int k = 0; k < 4; ++k){
        //         int space = C->_bottom_left_y() - poly_list[j]->get_Wi_coord(k);
        //         double cap = calCapicitance(overlap, space, layer_id+1);
        //         assert(cap != 0);
        //         single_cap += cap * poly_list[j]->getVariable(k);
        //     }
        //     // cap_expression += single_cap * poly_list[j]->getVariable(-1);
        //     cout << single_cap.size() << endl;
        //     cap_expression += single_cap;
        // }
    }

    // for(int i = 0; i < cap_expression.size(); ++i){
    //     cout << cap_expression.getCoeff(i) << " \n";
    //     cout << cap_expression.getVar1(i).get(GRB_StringAttr_QCName) << ' ';
    //     cout << cap_expression.getVar2(i).get(GRB_StringAttr_QCName) << ' ' << endl;
    // }

    cout << "===== finish adding objective function (minimize capacitance) exp size = " << cap_expression.size() << endl;
    model->setObjective(cap_expression, GRB_MINIMIZE);
    // vector<Slot*> slot_list = _LayerList[0].getSlots();
    // model->setObjective(slot_list[0]->getVariable(0) + slot_list[0]->getVariable(5), GRB_MAXIMIZE);
}
