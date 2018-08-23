#include <cassert>
#include <algorithm>
#include "polygon.h"

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
    swap(_b_left_x, _b_left_y);
    swap(_t_right_x, _t_right_y);
    swap(tr, rt);
    swap(bl, lb);
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

void Slot::setVariable(GRBModel *model)
{
    GRBLinExpr up, down;
    for (int i = 0; i < 8; ++i)
    {
        string name = 'w' + to_string(slot_id) + to_string(i + 1);
        GRBVar var = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, name);
        var_list.push_back(var);
        if (i < 4)
            up += var;
        else
            down += var;
    }

    string name = 'Y' + to_string(slot_id);
    Y_ij = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, name);

    model->addConstr(up * down == Y_ij, name);
    // model->addConstr(Y_ij )
}


GRBVar &Slot::getVariable(int i)
{
    assert(var_list.size() == 8);
    if(i == -1)
        return Y_ij;
    return var_list[i];
}