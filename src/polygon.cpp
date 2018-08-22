#include <algorithm>
#include "polygon.h"
#include <iostream>
#include "util.h"
using namespace std;

void Polygon::set_xy(int x1,int y1,int x2, int y2){
    //先右上再左下
    _b_left_x=x2;
    _b_left_y=y2;
    _t_right_x=x1;
    _t_right_y=y1;
}
void Polygon::set_coordinate_V(vector<int> tokens){
    _polygon_id = tokens[0];
    _b_left_x = tokens[1];
    _b_left_y = tokens[2];
    _t_right_x = tokens[3];
    _t_right_y = tokens[4];
    _net_id = tokens[5];
    _layer_id = tokens[6];
}
void Polygon::set_coordinate_H(vector<int> tokens){
    _polygon_id = tokens[0];
    _net_id = tokens[5];
    _layer_id = tokens[6];
    _b_left_y = tokens[1];
    _b_left_x = tokens[2];
    _t_right_y = tokens[3];
    _t_right_x = tokens[4];
}

void Polygon::rotate(){
    // cout<<"before rotate"<<_b_left_x<<" "<<_b_left_y<<endl;
    // print_Polygon(tr);
    // print_Polygon(rt);
    swap(_b_left_x, _b_left_y);
    swap(_t_right_x, _t_right_y);
    swap(tr, rt);
    swap(bl, lb);
    // cout<<"after rotate"<<_b_left_x<<" "<<_b_left_y<<endl;
    // print_Polygon(tr);
    // print_Polygon(rt);
    // Polygon* a = tr;
    // tr = rt;
    // rt = a;
    // a  = lb;
    // lb = bl;
    // bl = a;
}
void Polygon::swap_top_right(){
    swap(tr, rt);
}
void Polygon::swap_bottom_left(){
    swap(bl, lb);
}
void Polygon::swap_xy(){
    swap(_b_left_x, _b_left_y);
    swap(_t_right_x, _t_right_y);
}