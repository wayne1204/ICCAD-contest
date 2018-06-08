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
    Polygon(string s = "",bool solid_ornot = false):_type(s),_is_solid(solid_ornot) {
        _is_critical_net = false;
    }
    void set_coordinate(vector<int> tokens){
        _polygon_id = tokens[0];
        _b_left_x = tokens[1];
        _b_left_y = tokens[2];
        _t_right_x = tokens[3];
        _t_right_y = tokens[4];
        _net_id = tokens[5];
        _layer_id = tokens[6];
    }
    void set_xy(size_t x1,size_t y1,size_t x2, size_t y2){
        _b_left_x=x2;
        _b_left_y=y2;
        _t_right_x=x1;
        _t_right_y=y1;
    }
    void setToCNet();
    void setToSolid(){_is_solid = true;}
    void setType(string type);
    void set_tr(Polygon* a){tr=a;}
    void set_rt(Polygon* a){rt=a;}
    void set_lb(Polygon* a){lb=a;}
    void set_bl(Polygon* a){bl=a;}
    static void setGlobalref(){global_ref++;}
    Polygon* get_tr(){return tr;}
    Polygon* get_rt(){return rt;}
    Polygon* get_lb(){return lb;}
    Polygon* get_bl(){return bl;}
    void setToglobalref(){ref=global_ref;}
    string getType(){return _type;}
    bool isglobalref(){return ref==global_ref;}
    inline size_t _bottom_left_x(){return _b_left_x;}
    inline size_t _bottom_left_y(){return _b_left_y;}
    inline size_t _top_right_x(){return _t_right_x;}
    inline size_t _top_right_y(){return _t_right_y;}
    inline size_t get_layer_id(){return _layer_id;}
    bool is_solid(){return _is_solid;}
private:
    size_t _b_left_x;
    size_t _b_left_y;
    size_t _t_right_x;
    size_t _t_right_y;
    static unsigned global_ref;
    size_t _polygon_id;
    unsigned ref;
    size_t _net_id;
    size_t _layer_id;
    Polygon* tr;
    Polygon* rt;
    Polygon* lb;
    Polygon* bl;

    string _type;
    bool _is_critical_net;
    bool _is_solid;
};

inline void Polygon::setToCNet() {
    _is_critical_net = true;
}
inline void Polygon::setType(string type) {
    _type = type;
}

class Layer
{
public:
    void initRule(size_t, size_t, size_t, double, double);
    Polygon* point_search(Polygon* start,size_t x,size_t y);
    vector<Polygon*> region_query(Polygon* start,size_t x1,size_t y1,size_t x2,size_t y2);
    vector<Polygon*> region_query(Polygon* start,Polygon* T);
    bool insert(Polygon* T);
    Polygon* split_Y(Polygon* &bigGG,size_t y,bool is_top,Polygon* &inserted);
    Polygon* split_X_left(Polygon* &bigGG, size_t x_left,size_t x_right ,Polygon* &  inserted);
    Polygon* split_X_right(Polygon* &bigGG, size_t x_left,size_t x_right ,Polygon* &  inserted);
    void initialize_layer(size_t x_bl, size_t y_bl, size_t x_tr, size_t y_tr);
    inline size_t get_gap() { return min_space; }
    inline size_t get_width() { return min_width; }
    inline size_t get_bl_boundary_x(){return _bl_boundary_x;}
    inline size_t get_bl_boundary_y(){return _bl_boundary_y;}
    inline size_t get_tr_boundary_x(){return _tr_boundary_x;}
    inline size_t get_tr_boundary_y(){return _tr_boundary_y;}
private:
    size_t _bl_boundary_x;
    size_t _bl_boundary_y;
    size_t _tr_boundary_x;
    size_t _tr_boundary_y;
    Polygon* dummy_top;
    Polygon* dummy_right;
    Polygon* dummy_left;
    Polygon* dummy_bottom;
    Polygon* dummy_bottom_right;
    Polygon* dummy_right_top;
    Polygon* dummy_bottom_left;
    Polygon* dummy_top_left;

    size_t min_width;
    size_t min_space;
    size_t max_fill_width;
    double min_density;
    double max_density;
    vector<Polygon*> _polygonlist;

};

#endif