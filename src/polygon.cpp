#include <cassert>
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

void Polygon::setVariable(GRBModel *model)
{
    var_list.clear();
    GRBLinExpr up, down;
    for (int i = 0; i < 8; ++i)
    {
        string name = "slot" + to_string(_slot_id) + "_w" + to_string(i + 1);
        GRBVar var = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, name);
        var_list.push_back(var);
        if (i < 4)
            up += var;
        else
            down += var;
    }
    is_set = true;
    model->addConstr(up <= 1.0);
    model->addConstr(down <= 1.0);
    // string name = "slot" + to_string(_slot_id) + "_yij";
    // Y_ij = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, name);
    // model->addQConstr(Y_ij == up * down, name);

}


GRBVar &Polygon::getVariable(int i)
{
    assert(var_list.size() == 8);
    if(i == -1){
        return Y_ij;
    }
    return var_list[i];
}

// 0.5 w1 + 0.375 w2 + 0.25 w3 + 0.125 w4 + 0.125 w5 + ....
GRBLinExpr Polygon::getPortion(){
    GRBLinExpr lateral_portion;
    double slice = 1.0/8.0;
    for(int i = 0; i < 4; ++i)
        lateral_portion += (5 - i) * slice * var_list[i];
    for (int i = 4; i < 8; ++i)
        lateral_portion += (i - 4) * slice * var_list[i];
    return lateral_portion;
}

const int Polygon::get_Wi_coord(int i)
{
    int w_height = (_t_right_y - _b_left_y)/8;
    if(i == -1)
        return middle_y;
    else if (i < 4) 
        return middle_y + (4 - i) * w_height;
    else
        return middle_y - (i - 3) * w_height;
}