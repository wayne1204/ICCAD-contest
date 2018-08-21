#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cassert>
#include <iomanip>
#include <typeinfo>
#include "chipMgr.h"
#include "util.h"
// #define DEBUG
using namespace std;

// find the corrseponding type of capacitance by using a hashmap
double chipManager::calCapicitance(double area, int type, int layer1, int layer2)
{
    int key = (type * 100) + (layer1 * 10) + layer2;

    if (total_Cap_List.find(key) == total_Cap_List.end())
    {
        cout << "[Error] can't find correspond cap rule\n";
        return 0;
    }
    Capacitance* c = total_Cap_List[key];
    return c->getCapacitance(area);
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
    int aa = 0;
    int bb = 1;
    int x_len = 0, y_len = 0, x_len_big = 0, y_len_big = 0;
    bool VorH[layer_num];
    while (token != ""){
        if (first_line){
            while ( (token = next_token(buff_beg, buff_end)) != ""){
                if (token[0] == '\n') {first_line = false; break;}
                if (myStr2Int(token, num)){
                    layer_bound.push_back(num);
                }
            }
        }
        else {
            bool if_new=false;
            while ( (token = next_token(buff_beg, buff_end)) != ""){
                if (token[0] == '\n') break;
                if (myStr2Int(token, num)){
                    tokens.push_back(num);
                }
                else {
                    #ifdef DEBUG
                    //cout<<"start new.....tokens size = "<<tokens.size()<<endl;
                    #endif

                    if (insert_vec[tokens[6]-1].size() < 100){
                        insert_vec[tokens[6]-1].push_back(tokens);
                        // cout<<"vec"<<endl;
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
                            if(x_len_big > y_len_big) VorH[tokens[6]-1] = false; 
                            else VorH[tokens[6]-1] = true;

                            if(VorH[tokens[6]-1]){
                                _bl_bound_x = layer_bound[0];
                                _bl_bound_y = layer_bound[1];
                                _tr_bound_x = layer_bound[2];
                                _tr_bound_y = layer_bound[3];
                            }
                            else{
                                _bl_bound_y = layer_bound[0];
                                _bl_bound_x = layer_bound[1];
                                _tr_bound_y = layer_bound[2];
                                _tr_bound_x = layer_bound[3];
                            }
                            cout<<"================= layer id = "<<tokens[6]-1<<"---";
                            if (VorH[tokens[6]-1]) cout<<"same coordinate ==============="<<endl;
                            else cout<<"rotated coordinate ============"<<endl;

                            cout<<"y_big = "<<y_len_big<<", x_big = "<<x_len_big<<endl;
                            y_len_big = 0;
                            x_len_big = 0;
                            
                            _LayerList[tokens[6]-1].init_layer(_bl_bound_x, _bl_bound_y, _tr_bound_x, _tr_bound_y);
                            for (int i = 0; i < 100; ++i){
                                poly = new Polygon();
                                poly->setToSolid();
                                if (VorH[tokens[6]-1]) poly->set_coordinate_V(insert_vec[tokens[6]-1][i]);
                                else poly->set_coordinate_H(insert_vec[tokens[6]-1][i]);
                                if (cnet_set.count(insert_vec[tokens[6]-1][i][5])) poly->setToCNet();
                                // cout<<"10 : insert..........."<<endl;
                                _LayerList[poly->get_layer_id()-1].insert(poly, true, _LayerList[poly->get_layer_id()-1].get_dummy());
                                ++aa;
                                // cout<<"10 : insert...........back"<<endl;
                                
                            }
                        }
                    }
                    else{
                        poly = new Polygon();
                        poly->setToSolid();
                        if (VorH[tokens[6]-1]) poly->set_coordinate_V(tokens);
                        else poly->set_coordinate_H(tokens);
                        if (cnet_set.count(tokens[5])) poly->setToCNet();
                        // cout<<"out : insert............."<<endl;
                        _LayerList[poly->get_layer_id()-1].insert(poly, true, _LayerList[poly->get_layer_id()-1].get_dummy());
                        ++aa;
                        // cout<<"out : insert.............back"<<endl;
                        }
                    tokens.clear();
                }
            }
            // _LayerList[poly->get_layer_id()-1].insert(poly, true, _LayerList[poly->get_layer_id()-1].get_dummy());
            // cout<<"parse poly....number of poly = "<<setw(6)<<aa<<"...."<<"\r";
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

void chipManager::preproccess(vector<bool> VorH){
    for(int i=0;i < layer_num ;i++){
        
        vector<Polygon*> temp;
        _LayerList[i].region_query(_LayerList[i].get_dummy()->get_bl(),_LayerList[i].get_tr_boundary_x(),
            _LayerList[i].get_tr_boundary_y(),_LayerList[i].get_bl_boundary_x(),
            _LayerList[i].get_bl_boundary_y(), temp);
        if(VorH[i] == false){
            rotate_dummy(_LayerList[i]);
            _LayerList[i].rotate();
            for (int j = 0; j < temp.size(); ++j){
                temp[j]->rotate();
            }
        }
        cout<<"start slot split in layer "<<i+1<<endl;
        for(int ii=0; ii<temp.size(); ii++){
            if(temp[ii]->getType() == "space"){
                int w = temp[ii]->_top_right_x() - temp[ii]->_bottom_left_x();
                int h = temp[ii]->_top_right_y() - temp[ii]->_bottom_left_y();
                if(w >= _LayerList[i].get_width() && h >= _LayerList[i].get_width()){
                    vector<int> coordinate_y;
                    vector<int> coordinate_x;
                    int w_y = find_optimal_width(_LayerList[i],temp[ii]->_bottom_left_y() , h, coordinate_y);
                    int w_x = find_optimal_width(_LayerList[i],temp[ii]->_bottom_left_x() , w, coordinate_x);
                    for(int j=0 ;j < coordinate_y.size(); j++){
                        for(int k=0;k < coordinate_x.size();k++){
                            int x1 = coordinate_x[k] + w_x/2 ;
                            int y1 = coordinate_y[j] + w_y/2 ;
                            int x2 = coordinate_x[k] - w_x/2 ;
                            int y2 = coordinate_y[j] - w_y/2 ;
                            Polygon* T = new Polygon("slot");
                            T -> set_layer_id(i+1);
                            T -> set_xy(x1,y1,x2,y2);
                            //_LayerList[i].insert(T, true, _LayerList[i].get_dummy());
                        }
                    }
                }
            }
        }
    }
    for(int i=0;i<layer_num;i++){
        vector<Polygon*> tmp;
        _LayerList[i].region_query(_LayerList[i].get_dummy()->get_bl(),_LayerList[i].get_tr_boundary_x(),
            _LayerList[i].get_tr_boundary_y(),_LayerList[i].get_bl_boundary_x(),
            _LayerList[i].get_bl_boundary_y(), tmp);
           
        for(int ii=0 ;ii<tmp.size();ii++){
            if(tmp[ii]->getType() == "slot")
                print_Polygon(tmp[ii]);
        }
    }
    
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
                    total_Cnet_List.emplace(wnd_num, critical_nets);
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
    //cout<<"///////////////"<<output_fill<<endl;
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

