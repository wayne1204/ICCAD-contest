#ifndef LAYOUT_H
#define LAYOUT_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <climits>
#include <iostream>
#include <iomanip> 
#include <vector>
#include <string>
#include <unordered_set>

using namespace std;

class Polygon
{
public:
    Polygon(string s = "normal",bool solid_ornot = false):_type(s),_is_solid(solid_ornot) {
        _is_critical_net = false;
    }
    void set_coordinate(vector<int> tokens);
    void set_layer_id(int a){_layer_id=a;}
    void set_net_id(int a){_net_id=a;}
    void set_polygon_id(int a){_polygon_id=a;}
    void set_xy(int x1,int y1,int x2, int y2);
    void set_tr(Polygon* a){tr=a;}
    void set_rt(Polygon* a){rt=a;}
    void set_lb(Polygon* a){lb=a;}
    void set_bl(Polygon* a){bl=a;}
    void setToglobalref(){ref=global_ref;}
    static void setGlobalref(){global_ref++;}
    Polygon* get_tr(){return tr;}
    Polygon* get_rt(){return rt;}
    Polygon* get_lb(){return lb;}
    Polygon* get_bl(){return bl;}
    string getType(){return _type;}
    inline void setToSolid(){_is_solid = true;}
    inline void setToCNet() { _is_critical_net = true; }
    inline void setType(string type){ _type = type;}
    inline int get_layer_id(){return _layer_id;}
    inline int get_net_id(){return _net_id;}
    inline int get_polygon_id(){return _polygon_id;}
    inline int _bottom_left_x(){return _b_left_x;}
    inline int _bottom_left_y(){return _b_left_y;}
    inline int _top_right_x(){return _t_right_x;}
    inline int _top_right_y(){return _t_right_y;}
    bool isglobalref(){return ref==global_ref;}
    bool is_solid(){return _is_solid;}
    bool is_critical(){return _is_critical_net;}
private:
    int _b_left_x;
    int _b_left_y;
    int _t_right_x;
    int _t_right_y;
    int _polygon_id;
    int _net_id;
    int _layer_id;
    Polygon* tr;
    Polygon* rt;
    Polygon* lb;
    Polygon* bl;
    string _type;
    bool _is_critical_net;
    bool _is_solid;
    unsigned ref;
    static unsigned global_ref;
};



class Layer
{
public:
    void initialize_layer(int x_bl, int y_bl, int x_tr, int y_tr);
    void print_Polygon(Polygon* T);
    void insert_dummy(const int& x, const int& y,const double& windowsize, double& density,const int& layer_id);
    void initRule(int, int, int, double, double);
    void init_polygon(string &filename, unordered_set<int> &cnet_set);
    void region_query(Polygon *start, int x1, int y1, int x2, int y2, vector<Polygon *>& query_Polygon);
    void region_query(Polygon *start, Polygon *T, vector<Polygon *> &query_Polygon);
    bool insert(Polygon *T,bool is_myinset);
    double density_calculate(const int &x, const int &y, const double &windowsize, vector<Polygon *>& vec);
    Polygon *point_search(Polygon *start, int x, int y);
    Polygon *split_Y(Polygon *bigGG, int y, bool is_top);
    Polygon *split_X_left(Polygon *bigGG, int x_left, int x_right);
    Polygon *split_X_right(Polygon *bigGG, int x_left, int x_right);
    Polygon* get_dummy(){return dummy_left;}
    inline int get_gap() { return min_space; }
    inline int get_width() { return min_width; }
    inline int get_bl_boundary_x() { return _bl_boundary_x; }
    inline int get_bl_boundary_y() { return _bl_boundary_y; }
    inline int get_tr_boundary_x() { return _tr_boundary_x; }
    inline int get_tr_boundary_y() { return _tr_boundary_y; }
    inline double get_min_density() { return min_density; }
private:
    int _bl_boundary_x;
    int _bl_boundary_y;
    int _tr_boundary_x;
    int _tr_boundary_y;
    Polygon* dummy_top;
    Polygon* dummy_right;
    Polygon* dummy_left;
    Polygon* dummy_bottom;
    Polygon* dummy_bottom_right;
    Polygon* dummy_right_top;
    Polygon* dummy_bottom_left;
    Polygon* dummy_top_left;

    int min_width;
    int min_space;
    int max_fill_width;
    double min_density;
    double max_density;
    vector<Polygon*> _polygonlist;

};

#endif