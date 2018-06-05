#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cassert>
#include <iomanip>
#include <typeinfo>
#include "process.h"
#include "util.h"

using namespace std;


// record the valid area range
void Capacitance::setRange(const string& str)
{
    int num;
    string token;
    size_t pos = 0;
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
    size_t left = 0;
    while (str.find('(', left) != string::npos)
    {
        left = str.find('(', left);
        size_t comma = str.find(',', ++left);
        w = stod(str.substr(left, comma - left));
        size_t right = str.find(')', left);
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

// parse file
void ProcessFile::readFile(const char* fileName)
{
    string line, header, token;
    Capacitance *c;
    ifstream ifs(fileName, ios::in);
    if(!ifs.is_open()){
        cout<<"[Error] can't open file \"" << fileName << "\" \n";
        return;
    }
    
    while(getline(ifs, line))
    {
        if(line != "") {
            size_t pos = line.find(' ', 0);          
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
    cout << "Finish parsing porcess file \"" << fileName << "\"\n";
    cout << "    Area    |   Lateral  |    Fringe  |    Total   \n";
    cout << setw(7) << area_mapping.size() << setw(6) << "|";
    cout << setw(7) << layer_num << setw(6) << "|";
    cout << setw(7) << fringe_mapping.size() - layer_num << setw(6) << "|";
    cout << setw(7) << total_Cap_List.size() << endl;
    ifs.close();
}

// parse table mapping rules
void ProcessFile::parseTable(ifstream& ifs)
{
    int layer_n;
    size_t left, right, comma, pos = 0;
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
void ProcessFile::parseCapRules(ifstream &ifs, int type)
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
        size_t pos = line.find(' ');
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
double ProcessFile::calCapicitance(double area, int type, int layer1, int layer2)
{
    int key = (type * 100) + (layer_1 * 10) + layer_2;

    if (total_Cap_List.find(key) == total_Cap_List.end())
    {
        cout << "[Error] can't find correspond cap rule\n";
        return 0;
    }
    Capacitance* c = total_Cap_List[key];
    return c->getCapacitance(area);
}
