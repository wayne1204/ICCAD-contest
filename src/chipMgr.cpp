#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cassert>
#include <iomanip>
#include <typeinfo>
#include "chipMgr.h"
#include "util.h"
//#define DEBUG
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
        _LayerList[index].initRule(num1, num2, num3, d1, d2);
        // cout << "Layer#" << index << " "  << num1 << " " << num2 << " " << num3 << 
        // " " << d1 << " " << d2 <<endl;
    }
    cout << "=== Finish parsing rule file \"" << fileName << "\" ===\n";
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
    cout << "    Area    |   Lateral  |    Fringe  |    Total   \n";
    cout << setw(7) << area_mapping.size() << setw(6) << "|";
    cout << setw(7) << layer_num << setw(6) << "|";
    cout << setw(7) << fringe_mapping.size() - layer_num << setw(6) << "|";
    cout << setw(7) << total_Cap_List.size() << endl;
    cout << "=== Finish parsing porcess file \"" << fileName << "\" ===\n";
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

void chipManager::init_polygon(string &filename, unordered_set<int> &cnet_set)
{
    cout<<"init poly..."<<endl;
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
    vector<int> tokens;
    Polygon* poly;
    int aa = 1;
    int bb = 1;
    while (token != ""){

        if (first_line){
            while ( (token = next_token(buff_beg, buff_end)) != ""){
                if (token[0] == '\n') {first_line = false; break;}
                if (myStr2Int(token, num)){
                    tokens.push_back(num);
                }
            }
            _bl_bound_x = tokens[0];
            _bl_bound_y = tokens[1];
            _tr_bound_x = tokens[2];
            _tr_bound_y = tokens[3];
            tokens.clear();
            for (int i = 0; i < layer_num; ++i){
                _LayerList[i].initialize_layer(_bl_bound_x, _bl_bound_y, _tr_bound_x, _tr_bound_y);
                #ifdef DEBUG
                cout<<"layer num = "<<bb<<endl;
                bb++;
                #endif

            }
        }
        else {
            bool if_new=false;
            while ( (token = next_token(buff_beg, buff_end)) != ""){
                if (token[0] == '\n') break;
                if_new=false;
                if (myStr2Int(token, num)){
                    tokens.push_back(num);
                }
                else {
                    #ifdef DEBUG
                    //cout<<"start new.....tokens size = "<<tokens.size()<<endl;
                    #endif
                    if_new=true;
                    poly = new Polygon(token);
                    poly->set_coordinate(tokens);
                    poly->setToSolid();
                    tokens.clear();
                }
            }
            if (cnet_set.count(tokens[5])){
                poly->setToCNet();
            }
            
            _LayerList[poly->get_layer_id()-1].insert(poly,true);
            cout<<"parse poly....number of poly = "<<setw(6)<<aa<<"...."<<"\r";
            #ifdef DEBUG
            //cout<<"..............layer id = "<<poly->get_layer_id()<<" .................."<<endl;
            //cout<<".................finish poly....................."<<endl;
            #endif
            aa++;
            /*
            if(if_new){
                delete poly;
                poly=NULL;
            }*/
            
        }
    }
    cout << "===    Finish inserting "<< aa << " polygon    ===" << endl;
}
void chipManager::insert_tile(){
    int x, y, wnd_num;
    double density = 0;
    double count[9] = {0}, count2[9] = {0};
    int horizontal_cnt = (_tr_bound_x - window_size - _bl_bound_x) * 2 / window_size + 1;
    int vertical_cnt = (_tr_bound_y - window_size - _bl_bound_y) * 2 / window_size + 1;
    vector<Polygon*> critical_nets;

    for (int i = 0; i < layer_num; ++i){
        for(int row = 0; row < vertical_cnt; ++row){
            for (int col = 0; col < horizontal_cnt; ++col){
                x = _bl_bound_x + col * window_size / 2;
                y = _bl_bound_y + row * window_size / 2;
                wnd_num = i * horizontal_cnt * vertical_cnt + row * horizontal_cnt + col;
                //cout<<"density cal"<<x<<","<<y<<endl;
                density = _LayerList[i].density_calculate(x, y, window_size, critical_nets);
                //total_Cnet_List.emplace(wnd_num, critical_nets);
                
                if(density < _LayerList[i].get_min_density()){   
                    double new_density=density;
                    //cout<<endl<<"密度 "<<density<<x<<","<<y<<" windownum= "<<wnd_num<<endl;
                    _LayerList[i].insert_dummy(x,y,window_size,new_density,i+1);
                    //cout<<"新的密度 "<<new_density<<" "<<x<<","<<y<<" windownum= "<<wnd_num<<" layer id= "<<i+1<<endl;

                }
                else count[i]+=1;
                count2[i]++;
            }
        }       
    }
    cout<<endl;
    for(int i=0;i<9;i++)
        cout<<"幹你娘第"<<i+1<<"層只有"<<count[i]/count2[i]*100<<"%有滿足"<<endl;
}

