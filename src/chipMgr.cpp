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

// 
// 

void chipManager::write_fill(string output, string output_fill){
    ofstream ofs(output);

    //ofs.open(output.c_str());
    if(!ofs.is_open()){
        cout << "[Error] can't open rule file \"" << output << "\" !!\n";
        return;
    }
    vector<Polygon*> query_list;
    int num = 1;
    for (int layer = 0; layer < layer_num; ++layer ){
        _LayerList[layer].region_query(_LayerList[layer].get_dummy(),_tr_bound_x,_tr_bound_y,_bl_bound_x,_bl_bound_y,query_list);
        for(int i=0 ; i <query_list.size();i++){
            if(query_list[i]->getType() == "slot"){
                ofs<<num<<" "<<query_list[i]->_bottom_left_x()<<" "<<query_list[i]->_bottom_left_y()
                << " " << query_list[i]->_top_right_x() << " " << query_list[i]->_top_right_y() << " 0 "
                << layer << " " << "Fill"<<endl;
            }
        }
    }
    ofs.close();
    cout<<"...........finish.............."<<output<<endl;
}
void chipManager::final_check(){
    int x, y, wnd_num;
    double density = 0;
    int half_wnd = window_size / 2;
    int horizontal_cnt = (_tr_bound_x - _bl_bound_x) / half_wnd -1;
    int vertical_cnt = (_tr_bound_y - _bl_bound_y) / half_wnd -1;
    for (int i = 0; i < layer_num; ++i){
         for(int row = 0; row < vertical_cnt; ++row){
             for (int col = 0; col < horizontal_cnt; ++col){
                x = _bl_bound_x + col * half_wnd;
                y = _bl_bound_y + row * half_wnd; 
                vector<Polygon*> query_list;
                _LayerList[i].region_query(_LayerList[i].get_dummy(), x + window_size, y + window_size, x, y, query_list);     
                         
                // for (int j=0;j<query_list.size();j++){
                //     if(query_list[j]->getType() == "slot"){
                //         //cout<<"checking"<<endl;
                //         //檢查最大最小寬度

                //         // 3567150 1960845 3567214 1960931
                //         assert(query_list[j]->_top_right_x()-query_list[j]->_bottom_left_x() >=_LayerList[i].get_width());
                //         assert(query_list[j]->_top_right_x()-query_list[j]->_bottom_left_x() <=_LayerList[i].get_max_width());
                //         assert(query_list[j]->_top_right_y()-query_list[j]->_bottom_left_y() <=_LayerList[i].get_max_width());
                //         assert(query_list[j]->_top_right_y()-query_list[j]->_bottom_left_y() >=_LayerList[i].get_width());
                //         //檢查間距
                //         // 3412365 1969539 3412487 1969935
                //         if (query_list[i]->_bottom_left_x() == 3412365 && query_list[i]->_bottom_left_y() == 1969539
                //             && query_list[i]->_top_right_x() == 3412487 && query_list[i]->_top_right_y() == 1969935){
                //             cout<<" min width = "<<_LayerList[i].get_width()<<endl;
                //             print_Polygon(query_list[i]);
                //         }
                //         vector<Polygon*> check;
                //         _LayerList[i].region_query(_LayerList[i].get_dummy(),
                //             query_list[j]->_top_right_x()+_LayerList[i].get_gap(), 
                //             query_list[j]->_top_right_y()+_LayerList[i].get_gap(),
                //             query_list[j]->_bottom_left_x() - _LayerList[i].get_gap(),
                //             query_list[j]->_bottom_left_y() - _LayerList[i].get_gap(), check);
                //         int soild = 0;
                //         for(int k=0;k<check.size();k++){
                //             if(check[k]->is_solid())soild++;
                //             assert(soild<=1);

                //         }
                //     }
                // }
                //檢查密度有沒有對
                density = _LayerList[i].density_calculate(x, y, window_size, query_list);
                cout<<"density = "<<density<<endl;
                //assert(density >= _LayerList[i].get_min_density());
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
    //     if(tmp[ii]->getType() == "slot"){
    //         if(tmp[ii]->_top_right_x()-tmp[ii]->_bottom_left_x() >= _LayerList[layer].get_max_width())
    //             cout<<"------------boom--------------\n";
    //         if(tmp[ii]->_top_right_y()-tmp[ii]->_bottom_left_y() >= _LayerList[layer].get_max_width())
    //             cout<<"------------clap--------------\n";
    //         // assert(tmp[ii]->get_slot_id() != 0);
    //         ++count;
    //     }
    // }
    // cout << "total slot count = "<<count << endl;
    // cout<<"X = "<<_LayerList[layer].get_bl_boundary_x()<<" Y = "<<_LayerList[layer].get_bl_boundary_y()<<" Xr = "<<_LayerList[layer].get_bl_boundary_x()+window_size
    //     <<" Yr = "<<_LayerList[layer].get_bl_boundary_y()+window_size<<endl;
}

// specify constraints in every window 
void chipManager::layer_constraint(GRBModel* model, int layer_id , int x ,int y){
    int x_l, y_l;
    int half_wnd = window_size / 2;
    // int horizontal_cnt = (_LayerList[layer_id].get_tr_boundary_x() - _LayerList[layer_id].get_bl_boundary_x()) / half_wnd - 1;
    // int vertical_cnt = (_LayerList[layer_id].get_tr_boundary_y() - _LayerList[layer_id].get_bl_boundary_y()) / half_wnd - 1;
    int horizontal_cnt = (2 * window_size) / half_wnd - 1;
    int vertical_cnt = (2 * window_size) / half_wnd - 1;
    vector<Polygon *> slots;/////////////////////////////////////////////////////////////////////
    _LayerList[layer_id].region_query(_LayerList[layer_id].get_dummy(), x + 2 * window_size,
                                      y + 2 * window_size, x, y, slots);
    for (int i = 0; i < slots.size(); ++i){
        if (slots[i]->getType() == "slot" )
            slots[i]->setVariable(model);
    }
    slots.clear();
    for (int row = 0; row < vertical_cnt; ++row)
    {
        for (int col = 0; col < horizontal_cnt; ++col)
        {
            x_l = x + col * half_wnd;
            y_l = y + row * half_wnd;
            int area = _LayerList[layer_id].slot_area(x_l, y_l, window_size, slots);
            cout<< "(layer cons) pre density = "<< double(area)/(window_size * window_size) <<endl;
            int min_area = _LayerList[layer_id].get_min_density() * window_size * window_size;//////////////////
            GRBQuadExpr slot_exp = slot_constraint(model, x_l, y_l, slots);
            // cout << "x: " << x/1000.0 << "k y: " << y/1000.0 << "k slot size: " << slots.size() <<endl;
            // cout << "window slot expression size: " << slot_exp.size() <<endl;

            // density constraint
            string name = to_string(layer_id) + '_' + to_string(row) + '_' + to_string(col);
            model->addQConstr(slot_exp + area  >= min_area, name);
            // model->addQConstr(slot_exp + area  >= 0, name);
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
        // cout<<"slot yij = "<<slots[i]->get_Wi_coord(-1)<<"  "<<int(y + window_size)<<endl;
        assert(middle > 0);
        assert(width > 0);

        middle = max(middle, y);
        for (int j = 0; j < 8; ++j)
        {
            int w = min(slots[i]->get_Wi_coord(j), int(y + window_size));
            w = max(w, y);
            height += abs(w - middle) * slots[i]->getVariable(j);

            // cout<<"x = "<< x <<", y = "<<y<<endl;
            // cout<<" w num = "<<j<< " , coor = "<< slots[i]->get_Wi_coord(j) <<endl;
            // cout<< "y = " <<slots[i]->get_Wi_coord(-1)<<endl;
            // cout<<" ============== "<<abs(w - middle)<<" ================"<<endl;
            // cout<<"width = "<<width<<endl;
            // print_Polygon(slots[i]);
        }
        
        // GRBQuadExpr express = slots[i]->getVariable(-1) * height * width;
        GRBQuadExpr express = height * width;
        slot_exp += express;
    }
    return slot_exp;
}

// minimize slot coupleing cap with critical net
void chipManager::minimize_cap(GRBModel *model, int layer_id, int x,int  y){
    GRBLinExpr cap_expression = 0;
    for (int i = 0; i < total_Cnet_List[layer_id].size(); ++i)
    {
        Polygon* C = total_Cnet_List[layer_id][i]; /////////////////////////////
        if (classify(C->_top_right_x(), C->_bottom_left_x(), x + 2 * window_size, x) 
            * classify(C->_top_right_y(), C->_bottom_left_y(), y + 2 * window_size, y) != 0)
        {
            vector<Polygon*> poly_list;
            int min_space = _LayerList[layer_id].get_gap();

            _LayerList[layer_id].critical_find_lr(C, poly_list, x, y, window_size);////////////////

            for (int j = 0; j < poly_list.size(); ++j)
            {
                int overlap = min(C->_top_right_y(), poly_list[j]->_top_right_y()) - 
                              max(C->_bottom_left_y(), poly_list[j]->_bottom_left_y());
                GRBLinExpr single_cap = calCapicitance(overlap, min_space, layer_id + 1) * poly_list[j]->getPortion();
                assert(calCapicitance(overlap, min_space, layer_id + 1) >= 0);
                cap_expression += single_cap ;//* poly_list[j]->getVariable(-1);
            }

            _LayerList[layer_id].critical_find_top(C, poly_list, x, y, window_size);
            for (int j = 0; j < poly_list.size(); ++j)
            {
                int overlap = min(C->_top_right_x(), poly_list[j]->_top_right_x()) - 
                              max(C->_bottom_left_x(), poly_list[j]->_bottom_left_x());
                GRBLinExpr single_cap = 0;
                for (int k = 4; k < 8; ++k){
                    int space = poly_list[j]->get_Wi_coord(k) - C->_top_right_y();
                    double cap = calCapicitance(overlap, space, layer_id+1);
                    assert(cap > 0);
                    single_cap += cap * poly_list[j]->getVariable(k);
                }
                // cap_expression += single_cap * poly_list[j]->getVariable(-1);
                cap_expression += single_cap;
            }

            // cout<<"start bo...."<<endl;
            _LayerList[layer_id].critical_find_bottom(C, poly_list, x, y, window_size);
            for (int j = 0; j < poly_list.size(); ++j)
            {
                int overlap = min(C->_top_right_x(), poly_list[j]->_top_right_x()) -
                              max(C->_bottom_left_x(), poly_list[j]->_bottom_left_x());
                GRBLinExpr single_cap = 0;
                for (int k = 0; k < 8; ++k){
                    int space = C->_bottom_left_y() - poly_list[j]->get_Wi_coord(k);
                    double cap = calCapicitance(overlap, space, layer_id+1);
                    assert(cap > 0);
                    single_cap += cap * poly_list[j]->getVariable(k);
                }
                // cap_expression += single_cap * poly_list[j]->getVariable(-1);
                cout << single_cap.size() << endl;
                cap_expression += single_cap;
            }
        }
        
    }

    // for(int i = 0; i < cap_expression.size(); ++i){
    //     cout << cap_expression.getCoeff(i) << " \n";
    //     cout << cap_expression.getVar1(i).get(GRB_StringAttr_QCName) << ' ';
    //     cout << cap_expression.getVar2(i).get(GRB_StringAttr_QCName) << ' ' << endl;
    // }

    cout << "===== finish adding objective function (minimize capacitance) exp size = " << cap_expression.size() << endl;
    // cap_expression = 0;
    model->setObjective(cap_expression, GRB_MINIMIZE);
    // vector<Polygon*> slot_list = _LayerList[0].getSlots();
    // cout<<slot_list.size()<<endl;
    // model->setObjective(slot_list[1]->getVariable(0) + slot_list[1]->getVariable(5), GRB_MAXIMIZE);
}


void chipManager::write_output(GRBModel* g, int layer, int x, int y){
    vector<Polygon*> polygon_list;
    int slot_id = 0;
    int space_count = 0;
    int threshold = _LayerList[layer].get_width() + 2 * _LayerList[layer].get_gap();
    
    _LayerList[layer].region_query(
        _LayerList[layer].get_dummy()->get_bl(), x + 2 *window_size, ///////////////////////////////
        y + 2 *window_size, x, y, polygon_list);
    for(int i=0; i <polygon_list.size();i++){
        if(polygon_list[i]->getType()=="slot"){
            int y_top = INT_MAX, y_bottom = INT_MAX;
            bool fill = false;
            // int a;
            // if(polygon_list[i]->getVariable(0).get(GRB_DoubleAttr_X)+polygon_list[i]->getVariable(1).get(GRB_DoubleAttr_X)+polygon_list[i]->getVariable(2).get(GRB_DoubleAttr_X)+polygon_list[i]->getVariable(3).get(GRB_DoubleAttr_X)>1)
            //     cin>>a;
            //     //cout<<"--------------error--------------\n";
            // if(polygon_list[i]->getVariable(4).get(GRB_DoubleAttr_X)+polygon_list[i]->getVariable(5).get(GRB_DoubleAttr_X)+polygon_list[i]->getVariable(6).get(GRB_DoubleAttr_X)+polygon_list[i]->getVariable(7).get(GRB_DoubleAttr_X)>1)
            //     cin>>a;
                //cout<<"--------------error--------------\n";
            for (int j=0;j<8;j++){
                //cout<<"x = "<<x<<" ,y = "<<y<<endl;
                // print_Polygon(polygon_list[i]);
                GRBVar x = polygon_list[i]->getVariable(j);
                if(x.get(GRB_DoubleAttr_X) > 0){
                    // cout<< x.get(GRB_StringAttr_VarName) << " ,value = "<< x.get(GRB_DoubleAttr_X)<<endl;
                    // cout<<"Y_ij name: "<<polygon_list[i]->getVariable(-1).get(GRB_StringAttr_VarName)<<" , value = "<<polygon_list[i]->getVariable(-1).get(GRB_DoubleAttr_X)<<endl;
                    fill = true;
                    if (j < 4){
                        y_top = polygon_list[i]->get_Wi_coord(j);
                    }
                    else{
                        y_bottom = polygon_list[i]->get_Wi_coord(j);
                    }
                }

            }
            polygon_list[i]->reset_var();
            if (y_top != INT_MAX && y_bottom == INT_MAX){
                y_bottom = polygon_list[i]->get_Wi_coord(-1);
            }
            else if (y_top == INT_MAX && y_bottom != INT_MAX){
                y_top = polygon_list[i]->get_Wi_coord(-1);
            }
            if (fill){
                Polygon* T = new Polygon("fill", true);
                T->set_xy(polygon_list[i]->_top_right_x(), y_top, polygon_list[i]->_bottom_left_x(), y_bottom);
                T->set_layer_id(layer);
                _LayerList[layer].insert(T, true, _LayerList[layer].get_dummy());

                delete T;
                T = NULL;
            }
        }
    }
}


