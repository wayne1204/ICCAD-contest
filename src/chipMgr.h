#ifndef PROCESS_H
#define PROCESS_H
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include "polygon.h"
#include "layout.h"
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
    double getCapacitance(int area);
    double getCapacitance(int overylap, int space);

private:
    int cap_type;
    int layer_id;
    int other_layer;

    vector<int> range;
    vector<double> weights;
    vector<double> bias;
};

// file parsing manager
class chipManager{
public:
    chipManager()
    {
      window_size = 0;
      layer_num = 0;
    }
    // parseFile.cpp
    void parseRuleFile(const string& fileName);
    void parseProcessFile(const string &fileName);
    void parseTable(ifstream& );
    void parseCapRules(ifstream &, int);
    void setWindow(int num){ window_size = num; }
    int getLayerNum() { return layer_num; }

    // chipMgr.cpp
    double calCapicitance(double area, int type, int layer1, int layer2);
    double calCapicitance(int overlap, int space, int layer);
    void init_polygon(string &filename, unordered_set<int> &cnet_set, vector<bool> &VorH_v);
    void insert_tile(string&);
    void report_density(bool);
    void write_fill(string, string);
    void check_layer(string &filename);
    double get_windowsize(){return window_size;}

    void chip_rotate(vector<bool> VorH);
    void rotate_dummy(Layer layer);
    void set_variable(GRBModel* model, int layer);
    void layer_constraint(GRBModel *model, int layer_id, int x ,int y, int cons);
    GRBLinExpr slot_constraint(GRBModel *model, const int &x, const int &y, vector<Polygon *> &slots, int layer_id);
    void minimize_cap(GRBModel *model, int layer_id);
    GRBLinExpr minimize_area_cap(GRBModel *model, int layer_id, bool);
    void write_output(GRBModel* g, int layer, int x, int y);
    inline int get_bl_boundary_x() { return _bl_bound_x; }
    inline int get_bl_boundary_y() { return _bl_bound_y; }
    inline int get_tr_boundary_x() { return _tr_bound_x; }
    inline int get_tr_boundary_y() { return _tr_bound_y; }
    void final_check();


  private:
    double window_size;
    int layer_num;
    unordered_map<string, pair<int, int>> area_mapping;
    unordered_map<string, pair<int, int>> fringe_mapping;
    unordered_map<int, Capacitance *> total_Cap_List;
    vector< vector<Polygon *>> total_poly_List;
    vector< vector<Polygon *>> total_Cnet_List;
    Layer *_LayerList;
    int _bl_bound_x;
    int _bl_bound_y;
    int _tr_bound_x;
    int _tr_bound_y;
};


#endif