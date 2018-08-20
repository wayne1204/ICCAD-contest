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

// record the valid area range
void Capacitance::setRange(const string& str)
{
    int num;
    string token;
    int pos = 0;
    while (myStrGetTok(str, token, pos) != string::npos)
    {
        pos = myStrGetTok(str, token, pos);
        myStr2Int(token, num);
        range.push_back(num);
    }
}

// record the weights and bias in capacitance formula
void Capacitance::setParameter(const string &str)
{
    double w, b;
    int left = 0;
    while (str.find('(', left) != string::npos)
    {
        left = str.find('(', left);
        int comma = str.find(',', ++left);
        w = stod(str.substr(left, comma - left));
        int right = str.find(')', left);
        ++comma;
        b = stod(str.substr(comma, right - comma));
        weights.push_back(w);
        bias.push_back(b);
        left = right;
    }

}

// get capacitance from corresponding area
double Capacitance::getCapacitance(double area)
{
    for(int i = 1; i < weights.size(); ++i)
    {
        if(i < range[i])
            return area * weights[i] + bias[i];
    }
    // exceed maximun area
    double ret = range.back() * weights.back() + bias.back();
    return ret * area / range.back();
}

// parse rule file
void chipManager::parseRuleFile(const string &fileName)
{
    string line, token;
    int num1, num2, num3;
    double d1, d2;
    ifstream ifs(fileName.c_str(), ios::in);
    if(!ifs.is_open()){
        cout << "[Error] can't open rule file \"" << fileName << "\" !!\n";
        return;
    }
    _LayerList = new Layer[layer_num];
    while (getline(ifs, line))
    {
        if(line == "")
            break;
        size_t pos = 0;
        pos = myStrGetTok(line, token, pos);
        int index = stoi(token);
        pos = myStrGetTok(line, token, pos);

        pos = myStrGetTok(line, token, pos);
        myStr2Int(token, num1);
        pos = myStrGetTok(line, token, pos);
        myStr2Int(token, num2);
        pos = myStrGetTok(line, token, pos);
        myStr2Int(token, num3);
        pos = myStrGetTok(line, token, pos);
        d1 = stod(token);
        pos = myStrGetTok(line, token, pos);
        d2 = stod(token);
        _LayerList[index-1].init_rule(num1, num2, num3, d1, d2);
        // cout << "Layer#" << index << " "  << num1 << " " << num2 << " " << num3 << 
        // " " << d1 << " " << d2 <<endl;
    }
    // cout << "=== Finish parsing rule file \"" << fileName << "\" ===\n";
}

// parse process file
void chipManager::parseProcessFile(const string &fileName)
{
    string line, header, token;
    Capacitance *c;
    ifstream ifs(fileName.c_str(), ios::in);
    if(!ifs.is_open()){
        cout << "[Error] can't open process file \"" << fileName << "\" !!\n";
        return;
    }
    while(getline(ifs, line))
    {
        if(line != "") {
            int pos = line.find(' ', 0);          
            header = line.substr(0, pos);
            if (header == "window:"){
                token = line.substr(pos +1, 5);
                int size = stoi(token);
                setWindow(size);
            }
            else if (line == "; table matrix header"){
                parseTable(ifs);
            }
            else if (line == "; area cap tables"){
                parseCapRules(ifs, AREA);
            }
            else if (line == "; lateral cap tables"){
                parseCapRules(ifs, LATERAL);
            }
            else if (line == ";fringe cap tables"){
                parseCapRules(ifs, FRINGE);
            }
        }
    }
    // cout << "=== Finish parsing porcess file \"" << fileName << "\" ===\n";
    // cout << "    Area    |   Lateral  |   Fringe   |    Total   |\n";
    // cout << setw(7) << area_mapping.size() << setw(6) << "|";
    // cout << setw(7) << layer_num << setw(6) << "|";
    // cout << setw(7) << fringe_mapping.size() - layer_num << setw(6) << "|";
    // cout << setw(7) << total_Cap_List.size() << "     |\n";
    ifs.close();
}

// parse table mapping rules
void chipManager::parseTable(ifstream &ifs)
{
    int layer_n;
    int left, right, comma, pos = 0;
    string line, token;
    vector<int> index;

    // table matrix index
    getline(ifs, line);
    getline(ifs, line);
    while (pos != string::npos)
    {
        pos = myStrGetTok(line, token, pos);
        myStr2Int(token, layer_n);
        index.push_back(layer_n);
    }
    layer_num = index.size();
    
    while (getline(ifs, line))
    {
        int other_layer = 0;
        if (line == "")
            break;
        pos = myStrGetTok(line, token, 0);
        myStr2Int(token, layer_n);

        while (line.find('(', pos) != string::npos)
        {        
            left = line.find('(', pos);
            comma = line.find(',', ++left);
            string s1 = line.substr(left, comma - left);
            right = line.find(')', comma);
            comma += 2;
            string s2 = line.substr(comma, right - comma);
            if (s1 != "*")
            {
                pair<int, int> p(layer_n, index[other_layer]);
                area_mapping.emplace(s1, p);
                // cout << s1 << " " << layer_n << index[other_layer] << endl;
            }
            if (s2 != "*")
            {
                pair<int, int> p(layer_n, index[other_layer]);
                fringe_mapping.emplace(s2, p);
                // cout << s2 << " " << layer_n << index[other_layer] << endl;
            }
            pos = right;
            ++other_layer;
        }    
    }
}

// parse the line into different types of capacitance (area, lateral, fringe)
void chipManager::parseCapRules(ifstream &ifs, int type)
{
    int layer_1, layer_2, count = 0;
    string line, token;
    Capacitance *c;
    unordered_map<string, pair <int, int> >map;
    map = (type == AREA) ? area_mapping : fringe_mapping ;
    int thresold = map.size();
    if (type == LATERAL)
        thresold = layer_num;

    while (getline(ifs, line) && count < thresold)
    {
        if(line == "" || line == ";")
            continue;
        int pos = line.find(' ');
        token = line.substr(pos+1);

        if (map.find(token) != map.end()) {
            layer_1 = map[token].first;
            layer_2 = map[token].second;
            c = new Capacitance(type, layer_1, layer_2);
            // cout << "parsed " << type << " " << layer_1<< " " << layer_2 << " " << endl;
            int key = (type * 100) + (layer_1 * 10) + layer_2 ;
            if (total_Cap_List.find(key) != total_Cap_List.end())
                cout <<"[Error] collision happens!!"<<endl;
            total_Cap_List.emplace(key, c);
        }
        else{
            cout <<"[Error] can't find cap rule "<< token <<endl;
        }
        getline(ifs, line);
        getline(ifs, line);
        c->setRange(line);
        getline(ifs, line);
        c->setParameter(line);
        ++count;
    }
}

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
/*
bool chipManager::check_VorH(string &filename){
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
    int num, line_num = 0;
    bool first_line = true;
    vector<int> tokens_layer;
    vector<int> tokens_poly;
    vector<int> 
    Polygon* poly;
    int x_len = 0;
    int y_len = 0;
    int x_len_big = 0;
    int y_len_big = 0;
    while (token != "" || line_num < 10){
        if (first_line){
            while ( (token = next_token(buff_beg, buff_end)) != ""){
                if (token[0] == '\n') {first_line = false; break;}
                if (myStr2Int(token, num)){
                    tokens_layer.push_back(num);
                }
            }
        }
        else{
            bool if_new=false;
            while ( (token = next_token(buff_beg, buff_end)) != ""){
                if (token[0] == '\n') break;
                if (myStr2Int(token, num)){
                    tokens_poly.push_back(num);
                }
                else{
                    ++ line_num;
                    x_len = tokens_poly[3]-tokens_poly[1];
                    y_len = tokens_poly[4]-tokens_poly[2];
                    if (x_len > y_len){
                        x_len_big = x_len_big + 1;
                    }
                    else if (y_len > x_len){
                        y_len_big = y_len_big + 1;
                    }

                }
            }
        }
    }
    if(x_len_big > y_len_big) return false;
    else return true;
}
*/

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
            //     _bl_bound_y = tokens[0];
            //     _bl_bound_x = tokens[1];
            //     _tr_bound_y = tokens[2];
            //     _tr_bound_x = tokens[3];

            // for (int i = 0; i < layer_num; ++i){
            //     _LayerList[i].init_layer(_bl_bound_x, _bl_bound_y, _tr_bound_x, _tr_bound_y);
            //     #ifdef DEBUG
            //     cout<<"layer num = "<<bb<<endl;
            //     bb++;
            //     #endif
            // }
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
            
            
            
            
            // if(if_new){
            //     delete poly;
            //     poly=NULL;
            // }
        }
    }
    cout << "===    Finish inserting "<< aa << " polygon    ===" << endl;
    for (int i = 0; i < layer_num; ++i){
        VorH_v.push_back(VorH[i]);
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
void chipManager::preproccess(vector<bool> VorH){
    for(int i=0;i < layer_num ;i++){
        
        vector<Polygon*> temp;
        cout<<i+1<<endl;
        print_Polygon(_LayerList[i].get_dummy()->get_bl());
         _LayerList[i].region_query(_LayerList[i].get_dummy()->get_bl(),_LayerList[i].get_tr_boundary_x(),
             _LayerList[i].get_tr_boundary_y(),_LayerList[i].get_bl_boundary_x(),
             _LayerList[i].get_bl_boundary_y(), temp);
        if(VorH[i] == false){
            for (int j = 0; j < temp.size(); ++j){
                temp[j]->rotate();
            }
        }
        
        // for(int ii=0; ii<temp.size(); ii++){
        //     if(temp[ii]->getType() == "space"){
        //         int w = temp[ii]->_top_right_x() - temp[ii]->_bottom_left_x();
        //         int h = temp[ii]->_top_right_y() - temp[ii]->_bottom_left_y();
        //         if(w >= _LayerList[i].get_width() && h >= _LayerList[i].get_width()){
        //             vector<int> coordinate_y;
        //             vector<int> coordinate_x;
        //             int w_y = find_optimal_width(_LayerList[i],temp[ii]->_bottom_left_y() , h, coordinate_y);
        //             int w_x = find_optimal_width(_LayerList[i],temp[ii]->_bottom_left_x() , w, coordinate_x);
        //             for(int j=0 ;j < coordinate_y.size(); j++){
        //                 for(int k=0;k < coordinate_x.size();k++){
        //                     int x1 = coordinate_x[k] + w_x/2 ;
        //                     int y1 = coordinate_y[j] + w_y/2 ;
        //                     int x2 = coordinate_x[k] - w_x/2 ;
        //                     int y2 = coordinate_y[j] - w_y/2 ;
        //                     Polygon* T = new Polygon("slot");
        //                     T -> set_layer_id(i+1);
        //                     T -> set_xy(x1,y1,x2,y2);
        //                     _LayerList[i].insert(T, true, _LayerList[i].get_dummy());
        //                 }
        //             }
        //         }
        //     }
        // }
    }
    
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

