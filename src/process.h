#ifndef PROCESS_H
#define PROCESS_H
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
using namespace std;

enum CapacitanceType
{
    UNDEF = 0,
    AREA = 1,
    LATERAL = 2,
    FRINGE = 3
};

// record capacitance formula
class Capacitance{
public:
    Capacitance(int type, int n1, int n2){
        cap_type = type;
        layer_id = n1;
        other_layer = n2;
    }

    void setRange(const string&);
    void setParameter(const string&);
    double getCapacitance(double area);

private:
    int cap_type;
    int layer_id;
    int other_layer;

    vector<int> range;
    vector<double> weights;
    vector<double> bias;
};

// process file parser
class ProcessFile{
public:
    ProcessFile(){
        window_size = 0;
        layer_num = 0;
    }
    void readFile(const char*);
    void parseTable(ifstream& );
    void parseCapRules(ifstream &, int);
    void setWindow(int num){ window_size = num;   }
    double calCapicitance(double, int, int, int = -1);

private:
    int window_size;
    int layer_num;
    unordered_map<string, pair<int, int> > area_mapping;
    unordered_map<string, pair<int, int> > fringe_mapping;
    unordered_map<int, Capacitance*> total_Cap_List;
};


#endif