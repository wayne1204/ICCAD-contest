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
    Polygon(string s = ""):_type(s) {
        _is_critical_net = false;
    }
    void set_coordinate(vector<int> tokens){
        _polygon_id = tokens[0];
        _bottom_left_x = tokens[1];
        _bottom_left_y = tokens[2];
        _top_right_x = tokens[3];
        _top_right_y = tokens[4];
        _net_id = tokens[5];
        _layer_id = tokens[6];
    }
    void setToCNet();
    void setType(string type);

private:
    size_t _bottom_left_x;
    size_t _bottom_left_y;
    size_t _top_right_x;
    size_t _top_right_y;

    size_t _polygon_id;
    size_t _net_id;
    size_t _layer_id;

    string _type;
    bool _is_critical_net;
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

private:
    size_t _bl_boundary_x;
    size_t _bl_boundary_y;
    size_t _tr_boundary_x;
    size_t _tr_boundary_y;

    vector<Polygon*> _polygonlist;

};

#endif