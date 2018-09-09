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
    int small = min(layer1, layer2);
    int large = max(layer1, layer2);
    int key = type * (layer_num + 1) * (layer_num + 1) + small * (layer_num + 1) + large;
    int key2 = type * (layer_num + 1) * (layer_num + 1) + large * (layer_num + 1) + small;

    if (total_Cap_List.find(key) == total_Cap_List.end())
    {
        cout << "[Error] can't find correspond cap rule key ="<< key <<"\n";
        return 0;
    }
    if(type == AREA)
        return total_Cap_List[key]->getCapacitance(area);
    else
        return total_Cap_List[key]->getCapacitance(area) + total_Cap_List[key2]->getCapacitance(area);
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
    return c->getCapacitance(overlap, space);
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
                    insert_vec[tokens[6]-1].push_back(tokens);
                    tokens.clear();
                }
            }
        }
    }
    for (int layer_id = 0; layer_id < layer_num; ++layer_id){
        for (int i = 0; i < insert_vec[layer_id].size(); ++i)
        {
            x_len = insert_vec[layer_id][i][3]-insert_vec[layer_id][i][1];
            y_len = insert_vec[layer_id][i][4]-insert_vec[layer_id][i][2];
            if ( x_len > y_len){
                x_len_big ++;
            }
            else if (y_len > x_len){
                y_len_big++;
            }
        }
        if (x_len_big > y_len_big && layer_id != 8){
            VorH[layer_id] = false;
            _bl_bound_y1 = layer_bound[0];
            _bl_bound_x1 = layer_bound[1];
            _tr_bound_y1 = layer_bound[2];
            _tr_bound_x1 = layer_bound[3];

        }
        else {
            VorH[layer_id] = true;
            _bl_bound_x1 = layer_bound[0];
            _bl_bound_y1 = layer_bound[1];
            _tr_bound_x1 = layer_bound[2];
            _tr_bound_y1 = layer_bound[3];
        }
        if (VorH[layer_id]) cout<<"same coordinate ==============="<<endl;
        else cout<<"rotated coordinate ============"<<endl;
        y_len_big = 0, x_len_big = 0;

        _LayerList[layer_id].init_layer(_bl_bound_x1, _bl_bound_y1, _tr_bound_x1, _tr_bound_y1);

        for (int i = 0; i < insert_vec[layer_id].size(); ++i){
            poly = new Polygon();
            poly->setToSolid();
            if (VorH[layer_id]) poly->set_coordinate_V(insert_vec[layer_id][i]);
            else poly->set_coordinate_H(insert_vec[layer_id][i]);
            if (cnet_set.count(insert_vec[layer_id][i][5])) {
                poly->setToCNet();
            }
            _LayerList[poly->get_layer_id()-1].insert(poly, true, _LayerList[poly->get_layer_id()-1].get_dummy());
            ++aa;
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
    
    int num = 1;
    for (int layer = 0; layer < layer_num; ++layer ){
        vector<Polygon*> query_list;
        _LayerList[layer].region_query(_LayerList[layer].get_dummy(),_tr_bound_x,_tr_bound_y,_bl_bound_x,_bl_bound_y,query_list);
        for(int i=0 ; i <query_list.size();i++){
            if(query_list[i]->getType() == "fill"){
                ofs<<num<<" "<<query_list[i]->_bottom_left_x()<<" "<<query_list[i]->_bottom_left_y()
                << " " << query_list[i]->_top_right_x() << " " << query_list[i]->_top_right_y() << " 0 "
                << layer+1<< " " << "Fill"<<endl;
                num++;
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
    // for (int i = 0; i < layer_num ; ++i){
        int i = 8;
        for(int row = 0; row < vertical_cnt; ++row){
            for (int col = 0; col < horizontal_cnt; ++col){
                x = _bl_bound_x + col * half_wnd;
                y = _bl_bound_y + row * half_wnd;
                vector<Polygon*> query_list;
                _LayerList[i].region_query(_LayerList[i].get_dummy(), x + window_size, y + window_size, x, y, query_list);     
                         
                for (int j=0;j<query_list.size();j++){
                    if(query_list[j]->getType() == "fill"){
                        //cout<<"checking"<<endl;
                        //檢查最大最小寬度
                        assert(query_list[j]->_top_right_x()-query_list[j]->_bottom_left_x() >=_LayerList[i].get_width());
                        assert(query_list[j]->_top_right_x()-query_list[j]->_bottom_left_x() <=_LayerList[i].get_max_width());
                        assert(query_list[j]->_top_right_y()-query_list[j]->_bottom_left_y() <=_LayerList[i].get_max_width());
                        assert(query_list[j]->_top_right_y()-query_list[j]->_bottom_left_y() >=_LayerList[i].get_width());
                        //檢查間距
                        vector<Polygon*> check;
                        _LayerList[i].region_query(_LayerList[i].get_dummy(),
                            query_list[j]->_top_right_x()+_LayerList[i].get_gap(), 
                            query_list[j]->_top_right_y()+_LayerList[i].get_gap(),
                            query_list[j]->_bottom_left_x() - _LayerList[i].get_gap(),
                            query_list[j]->_bottom_left_y() - _LayerList[i].get_gap(), check);
                        int soild = 0;
                        for(int k=0;k<check.size();k++){
                            if(check[k]->is_solid())soild++;
                            assert(soild<=1);

                        }
                    }
                }
                //檢查密度有沒有對
                density = _LayerList[i].density_calculate(x, y, window_size, query_list);
                // cout << "row = " << row << ", col = " << col <<", density = "<<density<<endl;
                if (density < _LayerList[i].get_min_density()){
                    cout<<"layer = "<<i + 1 << " ,row = "<<row<<", col = "<<col<<endl;
                }
                //assert(density >= _LayerList[i].get_min_density());
            }
        } 
        cout<<" ==================== check finish ============================="<<endl;
        cout<<" layer id = "<<i+1<<" ...................."<<endl;
    // }      
}


void chipManager::chip_rotate(vector<bool> VorH)
{
    cout << "start rotating chip \n";
    for(int layer = 0; layer < layer_num; ++layer){
        vector<Polygon *> polygon_list;
        _LayerList[layer].region_query(
            _LayerList[layer].get_dummy()->get_bl(), _LayerList[layer].get_tr_boundary_x(),
            _LayerList[layer].get_tr_boundary_y(), _LayerList[layer].get_bl_boundary_x(),
            _LayerList[layer].get_bl_boundary_y(), polygon_list);
        if (VorH[layer] == false)
        {
            rotate_dummy(_LayerList[layer]);
            _LayerList[layer].layer_rotate();
        }

        for (int j = 0; j < polygon_list.size(); ++j)
        {
            if (VorH[layer] == false)
                polygon_list[j]->rotate();
            if (polygon_list[j]->is_critical())
                total_Cnet_List[layer].push_back(polygon_list[j]);
        }
        total_poly_List.push_back(polygon_list);
    }
}


void chipManager::set_variable(GRBModel* model, int layer)
{
    cout << "start insert_slots in layer# " << layer + 1 << endl;
    vector<Polygon*> polygon_list;
    int slot_id = 0;
    int space_count = 0;
    int threshold = _LayerList[layer].get_width() + 2 * _LayerList[layer].get_gap();
    vector< vector<int> > rest;

    for (int i = 0; i < total_poly_List[layer].size(); i++)
    {
        if (total_poly_List[layer][i]->getType() == "space")
        {
            space_count++;
            int poly_w = total_poly_List[layer][i]->_top_right_x() - total_poly_List[layer][i]->_bottom_left_x();
            int poly_h = total_poly_List[layer][i]->_top_right_y() - total_poly_List[layer][i]->_bottom_left_y();
            if (poly_w >= threshold && poly_h >= threshold)
                _LayerList[layer].insert_slots(model, total_poly_List[layer][i], poly_w, poly_h, slot_id);
            else{
                vector<int> coordinate;
                coordinate.push_back(total_poly_List[layer][i]->_top_right_x());
                coordinate.push_back(total_poly_List[layer][i]->_top_right_y());
                coordinate.push_back(total_poly_List[layer][i]->_bottom_left_x());
                coordinate.push_back(total_poly_List[layer][i]->_bottom_left_y());
                rest.push_back(coordinate);
            }
        }
    }
    for(int i=0;i<rest.size();i++){
        // int t_x = query_list[i]->_top_right_x(), t_y = query_list[i]->_top_right_y();
        // int b_x = query_list[i]->_bottom_left_x(), b_y = query_list[i]->_bottom_left_y();
        int t_x = rest[i][0], t_y = rest[i][1], b_x = rest[i][2], b_y = rest[i][3];
        cout<<"haha==============   "<<endl;
        if (_LayerList[layer].expand(t_x, t_y, b_x, b_y, _bl_bound_x, _bl_bound_y, 10000000, 4))
        {
            // cout << "..............expanding " << cnt  << "/" << rest.size() <<'\r';
            Polygon* T = new Polygon();
            T->set_layer_id(layer);
            T->set_xy(t_x, t_y, b_x, b_y);
            cout<<"fuck..."<<endl;
            _LayerList[layer].insert_slots(model, T, t_x-b_x, t_y-b_y, slot_id);
        }
    }
}

// specify constraints in every window 
void chipManager::layer_constraint(GRBModel* model, int layer_id , int x ,int y, int cons){
    int x_l, y_l;
    int half_wnd = window_size / 2;
    int horizontal_cnt = (_LayerList[layer_id].get_tr_boundary_x() - _LayerList[layer_id].get_bl_boundary_x()) / half_wnd - 1;
    int vertical_cnt = (_LayerList[layer_id].get_tr_boundary_y() - _LayerList[layer_id].get_bl_boundary_y()) / half_wnd - 1;
    vector<Polygon *> slots;

    _LayerList[layer_id].region_query(_LayerList[layer_id].get_dummy(),
                        _tr_bound_x, _tr_bound_y, _bl_bound_x, _bl_bound_y, slots);
    for (int i = 0; i < slots.size(); ++i){
        if (slots[i]->getType() == "slot" )
            slots[i]->setVariable(model, _LayerList[layer_id].get_width());
    }
    slots.clear();
    for (int row = 0; row < vertical_cnt; ++row)
    {
        for (int col = 0; col < horizontal_cnt; ++col)
        {
            x_l = x + col * half_wnd;
            y_l = y + row * half_wnd;
            int area = _LayerList[layer_id].slot_area(x_l, y_l, window_size, slots);

            int min_area = _LayerList[layer_id].get_min_density() * window_size * window_size;//////////////////
            GRBLinExpr slot_exp = slot_constraint(model, x_l, y_l, slots, layer_id);
            // cout << "x: " << x_l/1000.0 << "k y: " << y_l/1000.0 << "k slot size: " << slots.    size() <<endl;
            // cout << "window slot expression size: " << slot_exp.size() <<endl;

            //// density constraint
            string name = to_string(layer_id) + '_' + to_string(row) + '_' + to_string(col);
            model->addConstr(slot_exp + area  >= min_area, name);
            if (cons == 1){
                if(layer_id < layer_num - 2)
                    model->addConstr((_LayerList[layer_id].get_min_density() + 0.05)* window_size * window_size >= slot_exp + area);
            }
            else if (cons == 2){
                if (layer_id < layer_num - 2)
                    model->addConstr((_LayerList[layer_id].get_min_density() + 0.1) * window_size * window_size >= slot_exp + area);
            }
        }
    }
    cout << "=====[ finish adding layer#" << layer_id +1 << " constraints ( slot + metal >= 0.4 ) ]=====" << endl;
}


GRBLinExpr chipManager::slot_constraint(GRBModel *model, const int &x, const int &y, vector<Polygon *> &slots, int layer_id)
{
    GRBLinExpr slot_exp = GRBLinExpr();
    // if(layer_id == 4 && x == 3425000 && y == 1800000){
    //     vector<Polygon *> p_v;
    //     _LayerList[4].region_query(_LayerList[4].get_dummy(), 3426907,
    //                                 1802562, 3425657, 1801889, p_v);
    //     // _LayerList[4].region_query(_LayerList[i].get_dummy(), 3427704,
    //     //                                                             1802682, 3426948, 1801170, p_v);
    //     for(int j = 0; j < p_v.size(); ++j){
    //         cout << j << " | " << p_v[j]->getType() <<endl;
    //     }
    // }
    for (int i = 0; i < slots.size(); ++i)
    {   
        int middle = min(slots[i]->get_Wi_coord(-1), int(y + window_size));
        middle = max(middle, y);
        assert(slots[i]->is_slot());
        int width = min(int(x + window_size), slots[i]->_top_right_x()) - max(x, slots[i]->_bottom_left_x());
        GRBLinExpr express = overlap(slots[i], x+window_size, y+window_size, x, y, middle) * width;
        slot_exp += express;
    }
    return slot_exp;
}

// minimize slot coupleing cap with critical net
void chipManager::minimize_cap(GRBModel *model, int layer_id){
    GRBLinExpr cap_expression = 0;
    for (int i = 0; i < total_Cnet_List[layer_id].size(); ++i)
    {
        Polygon* C = total_Cnet_List[layer_id][i];
        vector<Polygon*> poly_list;
        int min_space = _LayerList[layer_id].get_gap();

        //cout<<"start lr ..."<<endl;
        _LayerList[layer_id].critical_find_lr(C, poly_list);
        for (int j = 0; j < poly_list.size(); ++j)
        {
            GRBLinExpr expr = overlap(poly_list[j], C->_top_right_x(), C->_top_right_y(),
                                        C->_bottom_left_x(), C->_bottom_left_y(), poly_list[j]->get_Wi_coord(-1));

            for (int k = 0; k < expr.size(); ++k)
            {
                double cap = calCapicitance(expr.getCoeff(k), min_space, layer_id + 1);
                cap_expression += cap * poly_list[j]->getVariable(k);
            }
        }

        //cout<<"start top..."<<endl;
        _LayerList[layer_id].critical_find_top(C, poly_list);
        for (int j = 0; j < poly_list.size(); ++j)
        {
            int overlap = min(C->_top_right_x(), poly_list[j]->_top_right_x()) - 
                            max(C->_bottom_left_x(), poly_list[j]->_bottom_left_x());
            for (int k = 0; k < poly_list[j]->getVarSize(); ++k){
                int space = poly_list[j]->get_Wi_coord(k) - C->_top_right_y();
                if (k < 4) 
                    space = poly_list[j]->get_Wi_coord(-1) - C->_top_right_y();
                
                double cap = calCapicitance(overlap, space, layer_id+1);
                assert(cap > 0);
                cap_expression += cap * poly_list[j]->getVariable(k);
            }
        }

        //cout<<"start bo...."<<endl;
        _LayerList[layer_id].critical_find_bottom(C, poly_list);
        // if (layer_id == 4) cout<<" find bottom size = "<<poly_list.size()<<"  =====================" <<endl;
        for (int j = 0; j < poly_list.size(); ++j)
        {
            int overlap = min(C->_top_right_x(), poly_list[j]->_top_right_x()) -
                          max(C->_bottom_left_x(), poly_list[j]->_bottom_left_x());
            for (int k = 0; k < poly_list[j]->getVarSize(); ++k){
                int space = C->_bottom_left_y() - poly_list[j]->get_Wi_coord(k);
                if (k > 3) 
                    space = C->_bottom_left_y() - poly_list[j]->get_Wi_coord(-1);
         
                double cap = calCapicitance(overlap, space, layer_id+1) * 10;
                assert(cap > 0);
                cap_expression += cap * poly_list[j]->getVariable(k);
            }
        }
    }

    cout << "=====[ finish adding lateral capacitance in layer#" << layer_id + 1 << " ]===== exp size = " << cap_expression.size() << endl;
    if (layer_id - 1 >= 0)
        cap_expression += minimize_area_cap(model, layer_id, false);
    if (layer_id + 1 < layer_num)
        cap_expression += minimize_area_cap(model, layer_id, true);

    model->setObjective(cap_expression, GRB_MINIMIZE);
}


GRBLinExpr chipManager::minimize_area_cap(GRBModel* model, int layer_id, bool is_up){
    GRBLinExpr total_expr;
    vector<Polygon*> slot_list;
    int next_layer_id = is_up ? layer_id + 1 : layer_id - 1;

    for (int i = 0; i < total_Cnet_List[next_layer_id].size(); ++i)
    {
        Polygon* C = total_Cnet_List[next_layer_id][i];
        _LayerList[layer_id].critical_find_vertical(C, slot_list, _LayerList[layer_id].get_gap());
        for (int j = 0; j < slot_list.size(); ++j)
        {
            for (int k = 0; k < slot_list[j]->getVarSize(); ++k)
            {
                double overlap_x, overlap_y, cap;
                int slot_top, slot_bot, space;

                if (k < slot_list[j]->getVarSize()/2 || slot_list[j]->getVarSize() == 1)
                {    // 上半部
                    slot_top = slot_list[j]->get_Wi_coord(k);
                    slot_bot = slot_list[j]->get_Wi_coord(-1);
                }
                else
                {   // 下半部
                    slot_top = slot_list[j]->get_Wi_coord(-1);
                    slot_bot = slot_list[j]->get_Wi_coord(k);
                }

                overlap_x = classify(C->_top_right_x(), C->_bottom_left_x(), slot_list[j]->_top_right_x(), slot_list[j]->_bottom_left_x());
                overlap_y = classify(C->_top_right_y(), C->_bottom_left_y(), slot_top, slot_bot);

                if(overlap_x > 0 && overlap_y > 0){
                    cap = calCapicitance(overlap_x * overlap_y, AREA, layer_id+1, next_layer_id+1);
                    total_expr += cap * slot_list[j]->getVariable(k);
                    assert(cap > 0);
                }

                else if(overlap_x > 0){
                    if (C->_bottom_left_y() > slot_bot)
                        space = C->_bottom_left_y() - slot_top;
                    else
                        space = slot_bot - C->_top_right_y();
                    cap = calCapicitance(overlap_x, FRINGE, layer_id+1, next_layer_id+1) * space;
                    total_expr += cap * slot_list[j]->getVariable(k);
                    assert(cap >= 0);
                }                
                else if(overlap_y > 0){
                    if (C->_bottom_left_x() > slot_list[j]->_bottom_left_x())
                        space = C->_bottom_left_x() - slot_list[j]->_top_right_x();
                    else
                        space = slot_list[j]->_bottom_left_x() - C->_top_right_x();
                    cap = calCapicitance(overlap_y, FRINGE, layer_id+1, next_layer_id+1) * space;
                    total_expr += cap * slot_list[j]->getVariable(k);
                    assert(cap >= 0);
                }
            }
        }        
    }
    cout << "=====[ finish adding area/fringe capacitance from " << next_layer_id + 1
         << " to " << layer_id + 1 << "]===== exp size = " << total_expr.size() << endl;
    return total_expr;
}


void chipManager::write_output(GRBModel* g, int layer, int x, int y){
    vector<Polygon*> polygon_list;
    
    // _LayerList[layer].region_query(
    //     _LayerList[layer].get_dummy()->get_bl(), x + 1 *window_size, ///////////////////////////////
    //     y + 1 *window_size, x, y, polygon_list);
    _LayerList[layer].region_query(_LayerList[layer].get_dummy(),_tr_bound_x, _tr_bound_y, _bl_bound_x, _bl_bound_y, polygon_list);
    for(int i=0; i <polygon_list.size();i++){
        if(polygon_list[i]->getType()=="slot"){
            int y_top = INT_MAX, y_bottom = INT_MAX;
            bool fill = false;
            
            for (int j=0; j<polygon_list[i]->getVarSize(); j++){
                GRBVar x = polygon_list[i]->getVariable(j);
                if(x.get(GRB_DoubleAttr_X) > 0){
                    fill = true;
                    if (j < 4){
                        y_top = polygon_list[i]->get_Wi_coord(j);
                    }
                    else{
                        y_bottom = polygon_list[i]->get_Wi_coord(j);
                    }
                }
            }

            if (y_top != INT_MAX && y_bottom == INT_MAX){
                y_bottom = polygon_list[i]->get_Wi_coord(-1);
            }
            else if (y_top == INT_MAX && y_bottom != INT_MAX){
                y_top = polygon_list[i]->get_Wi_coord(-1);
            }
            if (fill){
                Polygon* T = new Polygon("fill", true);
                T->set_xy(polygon_list[i]->_top_right_x(), y_top, polygon_list[i]->_bottom_left_x(), y_bottom);
                if(y_top- y_bottom <_LayerList[layer].get_width())
                    cout<<"----------------Error--------------\n";
                T->set_layer_id(layer);
                _LayerList[layer].insert(T, true, _LayerList[layer].get_dummy());

                delete T;
                T = NULL;
            }
        }
    }
}


