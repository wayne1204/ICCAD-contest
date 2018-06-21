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
#include "polygon.h"

using namespace std;

class Layer
{
public:
    void init_layer(int x_bl, int y_bl, int x_tr, int y_tr);
    void init_rule(int, int, int, double, double);
    void region_query(Polygon *start, int x1, int y1, int x2, int y2, vector<Polygon *>& query_Polygon);
    void region_query(Polygon *start, Polygon *T, vector<Polygon *> &query_Polygon);
    bool expand( int& x1,  int& y1,int& x2,  int& y2, const int& edge_x, const int& edge_y, const int& windowsize, int num);
    bool region_query_bool(Polygon *start, int x1, int y1, int x2, int y2, vector<Polygon *>& query_Polygon);
    bool region_query_bool(Polygon *start, Polygon *T, vector<Polygon *> &query_Polygon);
    void insert_dummies(Polygon* T, const int &, double &, const int &, const int &, const double &, int, stringstream&, int&);
    void layer_fill(const int& x, const int& y,const int& windowsize, double& density,const int& layer_id, string& out, int& fillnum);
    bool insert(Polygon *T, bool is_myinset, Polygon *start);
    double density_calculate(const int &x, const int &y, const double &windowsize, vector<Polygon *>& vec);
    Polygon *point_search(Polygon *start, int x, int y);
    Polygon *split_Y(Polygon *bigGG, int y, bool is_top);
    Polygon *split_X_left(Polygon *bigGG, int x_left, int x_right);
    Polygon *split_X_right(Polygon *bigGG, int x_left, int x_right);
    inline int get_gap() { return min_space; }
    inline int get_width() { return min_width; }
    inline Polygon* get_dummy() { return dummy_bottom; }
    inline int get_bl_boundary_x() { return _bl_boundary_x; }
    inline int get_bl_boundary_y() { return _bl_boundary_y; }
    inline int get_tr_boundary_x() { return _tr_boundary_x; }
    inline int get_tr_boundary_y() { return _tr_boundary_y; }
    inline int get_max_width() {return max_fill_width;}
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