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
    void set_xy(int x1,int y1,int x2, int y2){
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
    inline int _bottom_left_x(){return _b_left_x;}
    inline int _bottom_left_y(){return _b_left_y;}
    inline int _top_right_x(){return _t_right_x;}
    inline int _top_right_y(){return _t_right_y;}
    inline int get_layer_id(){return _layer_id;}
    bool is_solid(){return _is_solid;}
private:
    int _b_left_x;
    int _b_left_y;
    int _t_right_x;
    int _t_right_y;
    static unsigned global_ref;
    int _polygon_id;
    unsigned ref;
    int _net_id;
    int _layer_id;
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
    void init_polygon(string &filename, unordered_set<int> &cnet_set);
    Polygon* point_search(Polygon* start,int x,int y);
    vector<Polygon*> region_query(Polygon* start,int x1,int y1,int x2,int y2);
    vector<Polygon*> region_query(Polygon* start,Polygon* T);
    bool insert(Polygon* T);
    Polygon* split_Y(Polygon* &bigGG,int y,bool is_top,Polygon* &inserted);
    Polygon* split_X_left(Polygon* &bigGG, int x_left,int x_right ,Polygon* &  inserted);
    Polygon* split_X_right(Polygon* &bigGG, int x_left,int x_right ,Polygon* &  inserted);
    void initialize_layer(int x_bl, int y_bl, int x_tr, int y_tr);
    inline int get_gap(){return 30;}
    inline int get_width(){return mini_width;}
    inline int get_bl_boundary_x(){return _bl_boundary_x;}
    inline int get_bl_boundary_y(){return _bl_boundary_y;}
    inline int get_tr_boundary_x(){return _tr_boundary_x;}
    inline int get_tr_boundary_y(){return _tr_boundary_y;}
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

    int mini_gap=30;
    int mini_width=30;
    vector<Polygon*> _polygonlist;

};

#endif