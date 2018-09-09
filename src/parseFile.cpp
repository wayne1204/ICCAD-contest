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
        // cout << str.substr(left, comma - left) <<endl;
        int right = str.find(')', left);
        ++comma;
        b = stod(str.substr(comma, right - comma));
        // cout << str.substr(comma, right - comma) << endl;
        weights.push_back(w);
        bias.push_back(b);
        left = right;
    }

}

// case Area & Fringe
// get capacitance from corresponding area
double Capacitance::getCapacitance(int area)
{
    for(int i = 1; i < weights.size(); ++i)
    {
        if (area < range[i])
            return area * weights[i] + bias[i];
    }
    // exceed maximun area
    double ret = range.back() * weights.back() + bias.back();
    return ret * area / range.back() * pow(10, 12);
}

double Capacitance::getCapacitance(int overlap, int space)
{
    // lateral capacitance per unit kength
    double lateral_cap = 0.0;
    for (int i = 1; i < weights.size(); ++i)
    {
        if (space < range[i]){
            lateral_cap = space * weights[i] + bias[i];
            break;
        }
    }
    // exceed maximun area
    if (space > range.back())
        lateral_cap = (range.back() * weights.back() + bias.back()) * space / range.back();
    
    assert(lateral_cap != 0);
    return lateral_cap * overlap * pow(10, 12);
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
        _LayerList[index-1].init_rule(num1, num2, num3, d1, d2, index);
        vector<Polygon*> cnet_list;
        total_Cnet_List.push_back(cnet_list);
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
