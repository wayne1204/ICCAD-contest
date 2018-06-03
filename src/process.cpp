#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cassert>
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
            else if(header == "TableName:"){
                token = line.substr(pos);
                c = newCapRule(token);
                getline(ifs, line);
                getline(ifs, line);
                c->setRange(line);
                getline(ifs, line);
                c->setParameter(line);

            }
        }
    }
    cout << "Finish parsing porcess file \"" << fileName << "\"\n";
    cout << "Total capacitance rules : " << total_Cap_List.size() << endl;
    ifs.close();
}

// parse the line into different types of capacitance (area, lateral, fringe)
Capacitance* ProcessFile::newCapRule(const string& token)
{
    int layer1, layer2, key;
    string s1, s2;
    Capacitance *c;

    size_t pos = token.find('_');
    string type = token.substr(1, pos - 1);
    pos = token.find('_', ++pos);
    size_t end = token.find('_', ++pos);
    s1 = token.substr(pos, end - pos);
    s2 = token.substr(++end);
    myStr2Int(s1, layer1);
    myStr2Int(s2, layer2);
            
    if(type == "area"){
        c = new Capacitance(AREA, layer1, layer2);
        key = layer1 * (layer2 + 1);
    }else if (type == "lateral"){
        layer2 = layer1;
        c = new Capacitance(LATERAL, layer1, layer2); //same layer
        key = LATERAL * layer1 * (layer2 + 1);
    }else if (type == "fringe"){
        c = new Capacitance(FRINGE, layer1, layer2);
        key = FRINGE * layer1 * (layer2 + 1);
    }else{
        cout << "[Error] Unknown type of capacitance " << type << endl;
    }
    // cout << "parsed " << type << " " << layer1 << " " << layer2 << " " <<endl;

    pair<int, Capacitance *> mapping(key, c);
    total_Cap_List.insert(mapping);
    return c;
}

// find the corrseponding type of capacitance by using a hashmap
double ProcessFile::calCapicitance(double area, int type, int layer1, int layer2)
{
    layer2 = layer2 < 0 ? layer1 +1 : layer2 +1;
    int key = type * layer1 * layer2;

    if (total_Cap_List.find(key) == total_Cap_List.end())
    {
        cout << "[Error] can't find correspond cap rule\n";
        return 0;
    }
    Capacitance* c = total_Cap_List[key];
    return c->getCapacitance(area);
}
