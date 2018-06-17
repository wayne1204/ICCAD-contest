
#include "polygon.h"

void Polygon::set_xy(int x1,int y1,int x2, int y2){
    //先右上再左下
    _b_left_x=x2;
    _b_left_y=y2;
    _t_right_x=x1;
    _t_right_y=y1;
}
void Polygon::set_coordinate(vector<int> tokens){
    _polygon_id = tokens[0];
    _b_left_x = tokens[1];
    _b_left_y = tokens[2];
    _t_right_x = tokens[3];
    _t_right_y = tokens[4];
    _net_id = tokens[5];
    _layer_id = tokens[6];
}