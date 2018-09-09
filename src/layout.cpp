#include <iostream>
#include <sstream>
#include <fstream>
#include <climits>
#include <iostream>
#include <iomanip> 
#include <vector>
#include <algorithm>
#include <string>
#include <unordered_set>
#include <cassert>
#include "layout.h"
#include "util.h"
#include "polygon.h"
using namespace std;
//#define DEBUG
//#define PRINT
struct Compare {
    Compare(int x,int y,int windowsize){
        this->x = x;
        this->y = y;
        this->windowsize = windowsize;
    }
    bool operator () (const Polygon* const i, const Polygon* const j) const {
        int area_i = classify(i->_top_right_x(), i->_bottom_left_x(),x+windowsize,x) * 
            classify(i->_top_right_y(),i->_bottom_left_y(),y+windowsize,y);
        int area_j = classify(j->_top_right_x(),j->_bottom_left_x(),x+windowsize,x) *
            classify(j->_top_right_y(),j->_bottom_left_y(),y+windowsize,y);
        return area_i > area_j; 
    }
    int x,y,windowsize;
};

void Layer::layer_rotate()
{
    swap(_bl_boundary_x, _bl_boundary_y);
    swap(_tr_boundary_x, _tr_boundary_y);
    swap(dummy_bottom, dummy_left);
    swap(dummy_top, dummy_right);
    swap(dummy_bottom_right, dummy_top_left);
}

bool Layer::expand( int& x1, int& y1, int& x2, int& y2, const int& edge_x, const int& edge_y, 
                    const int& windowsize, int num){
    vector<Polygon *> query_list;
    const int bl_x = (edge_x - get_gap()) < get_bl_boundary_x() ? get_bl_boundary_x() : edge_x - get_gap();
    const int bl_y = (edge_y - get_gap()) < get_bl_boundary_y() ? get_bl_boundary_y() : edge_y - get_gap();
    const int tr_x = (edge_x + windowsize + get_gap()) > get_tr_boundary_x() ? get_tr_boundary_x() : edge_x + windowsize + get_gap();
    const int tr_y = (edge_y + windowsize + get_gap()) > get_tr_boundary_y() ? get_tr_boundary_y() : edge_y + windowsize + get_gap();
    // const int bl_x = get_bl_boundary_x();
    // const int bl_y = get_bl_boundary_y();
    // const int tr_x = get_tr_boundary_x();
    // const int tr_y = get_tr_boundary_y();
    x1 = (x1 > tr_x) ? tr_x : x1;
    x2 = (x2 < bl_x) ? bl_x : x2;
    y1 = (y1 > tr_y) ? tr_y : y1;
    y2 = (y2 < bl_y) ? bl_y : y2;
    cout<<"nonono....."<<endl;
    if(region_query_bool(dummy_bottom,x1,y1,x2,y2,query_list)){
        if (y1 + num <= tr_y)
        {
            while(region_query_bool(dummy_bottom,x1,y1+num,x2,y1,query_list)){
                y1+=num;
                if(y1>=tr_y) break;
            }
        }
        if (y2 - num >= bl_y)
        {
            while(region_query_bool(dummy_bottom,x1,y2,x2,y2-num,query_list)){
                y2-=num;
                if(y2<=bl_y) break;
            }
        }
        if (x1 + num <= tr_x)
        {
            while(region_query_bool(dummy_bottom,x1+num,y1,x1,y2,query_list)){ 
                x1+=num;
                if(x1>=tr_x) break;
            }
        }
        if (x2 - num >= bl_x)
        {
            while(region_query_bool(dummy_bottom,x2,y1,x2-num,y2,query_list)){
                x2-=num;
                if(x2<=bl_x)break; 
            }
        }
        if((x1-x2-2*get_gap()>=get_width())&&(y1-y2-2*get_gap()>=get_width())){
            return true;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }   

}

// Input: a space polygon T
// Output: one or many fill polygon with max width and min space
void Layer::insert_dummies(Polygon* T, const int &layer_id, double &density, const int &edge_x, 
                const int &edge_y, const double &windowsize, int type, stringstream& output, int& fillnum)
{
    int x_next, y_next, y, x;
    int x_cur = (T->_bottom_left_x() < edge_x - get_gap()) ? edge_x : T->_bottom_left_x() + get_gap();
    int x_left_bound = x_cur;
    int y_cur = (T->_bottom_left_y() < edge_y - get_gap()) ? edge_y : T->_bottom_left_y() + get_gap();
    const int x_bound = (T->_top_right_x() > edge_x + windowsize ) ? edge_x + windowsize : T->_top_right_x();
    const int y_bound = (T->_top_right_y() > edge_y + windowsize ) ? edge_y + windowsize : T->_top_right_y();
    double sum = 0;
    double area = (x_bound - T->_bottom_left_x()) * (y_bound - T->_bottom_left_y());
    double new_density = 0;
    // y到邊界至少要有一個 min width + min space 的距離
    while (y_cur + get_gap() + get_width() <= y_bound)
    {
        y_next = y_cur + get_max_width();
        y_next = (y_next > y_bound - get_gap()) ? y_bound - get_gap() : y_next;
        while(x_cur + get_gap() + get_width() <= x_bound)
        {
            x_next = x_cur + get_max_width();
            x_next = (x_next > x_bound - get_gap()) ? x_bound - get_gap() : x_next;
            //////小龜的優化/////
            double n_area = classify(x_next, x_cur, edge_x + windowsize, edge_x) * 
                            classify(y_next, y_cur, edge_y + windowsize, edge_y);
            new_density = (density * windowsize * windowsize + n_area) / (windowsize * windowsize);
            if (new_density > get_min_density()){
                double temp = get_min_density() - density;
                y = y_next - y_cur;
                x = temp * windowsize * windowsize/y + 1;
                if (x < get_width()) x = get_width();
                x_next = x_cur + x;
            }
            Polygon* fill = new Polygon("filled", true);
            fill->set_layer_id(layer_id);
            fill->set_xy(x_next, y_next, x_cur, y_cur);
            
            sum += (x_next - x_cur) * (y_next - y_cur);
            if(insert(fill, false, dummy_bottom)){
                output<<fillnum<<" "<<fill->_bottom_left_x()<<" "<<fill->_bottom_left_y()<<" "<<fill->_top_right_x()<<" "<<fill->_top_right_y()<<" 0 "<<layer_id<<" Fill"<<endl;
                fillnum++;
                double new_area = classify(x_next, x_cur, edge_x + windowsize, edge_x) * 
                                  classify(y_next, y_cur, edge_y + windowsize, edge_y);
                density = (density * windowsize * windowsize + new_area) / (windowsize * windowsize);
            }
            delete fill;
            fill = NULL;
            if (density >= get_min_density())
            {
                #ifdef PRINT
                // cout << setprecision(4) << sum / area * 100 << "%         " << sum << " / " << area << "\n";
                if (type == 1)
                    cout << "............... finish in stage 1.............." 
                         << " density= " << density << "/" << get_min_density() << endl;
                else if(type == 2)
                    cout << "............... finish in expand .............."
                         << " density= " << density << "/" << get_min_density() << endl;
                else
                    cout << "............... stupid  stage ................."
                         << " density= " << density << "/" << get_min_density() << endl;
                #endif
                return;
            }
            x_cur += (get_gap() + get_max_width());
        }
        y_cur += (get_gap() + get_max_width());
        x_cur = x_left_bound;
    }
    // cout << setprecision(4) << sum/area*100 << "%         " << sum << " / " << area << "\n";
}

void Layer::layer_fill(const int &edge_x, const int &edge_y, const int &windowsize, 
                    double &density, const int &layer_id, string &out, int &fillnum)
{    
    stringstream output;
    vector<Polygon*> query_list;
    vector< vector<int> > rest;
    region_query(dummy_bottom,edge_x+windowsize-get_gap(),edge_y+windowsize-get_gap(),edge_x+get_gap(),edge_y+get_gap(),query_list);// (start,跟要插進去得tile)
    sort(query_list.begin(), query_list.end(), Compare(edge_x,edge_y,windowsize));
    for(int i=0;i < query_list.size();i++){
        if (density >= get_min_density()){
            out = output.str();
            return;
        }
        #ifdef PRINT
        int w = query_list[i]->_top_right_x() - query_list[i]->_bottom_left_x();
        int h = query_list[i]->_top_right_y() - query_list[i]->_bottom_left_y();
        cout << "polygon# " << setw(3) << i+1 << "/" << query_list.size() << " | width:";
        cout << setw(6) << w << " | height:" << setw(6) << h << "| density: " << density <<'\r';
        #endif
        
        if(query_list[i]->getType()=="space" ){
            //query 要內縮getgap 因為等一下插入的tile是會內縮過的 所以這裡query先縮才會找到合法的tile
            if(query_list[i]->_top_right_x() - get_gap()-query_list[i]->_bottom_left_x()-get_gap() >= get_width()
                &&query_list[i]->_top_right_y()-get_gap()-query_list[i]->_bottom_left_y()-get_gap()>=get_width()){
                // //the two aboves are to verify the validility of the the region will be inserted
                insert_dummies(query_list[i], layer_id, density, edge_x, edge_y, windowsize, 1, output, fillnum);
            }
            else{
                vector<int> coordinate;
                coordinate.push_back(query_list[i]->_top_right_x());
                coordinate.push_back(query_list[i]->_top_right_y());
                coordinate.push_back(query_list[i]->_bottom_left_x());
                coordinate.push_back(query_list[i]->_bottom_left_y());
                rest.push_back(coordinate);
            }
        }
    }

    // cout << endl;
    int cnt = 0;
    // for (int i = 0 ; i < query_list.size(); ++i){
    for(int i=0;i<rest.size();i++){
        if (density >= get_min_density()){
            out = output.str();
            return;
        }
        // int t_x = query_list[i]->_top_right_x(), t_y = query_list[i]->_top_right_y();
        // int b_x = query_list[i]->_bottom_left_x(), b_y = query_list[i]->_bottom_left_y();
        int t_x = rest[i][0], t_y = rest[i][1], b_x = rest[i][2], b_y = rest[i][3];
        if (expand(t_x, t_y, b_x, b_y, edge_x, edge_y, windowsize, 8))
        {
            // cout << "..............expanding " << cnt  << "/" << rest.size() <<'\r';
            Polygon* T = new Polygon("filled",true);
            T->set_layer_id(layer_id);
            T->set_xy(t_x, t_y, b_x, b_y);
            insert_dummies(T, layer_id, density, edge_x, edge_y, windowsize, 2, output, fillnum);
            delete T;
            T=NULL;
            ++cnt;
        }
    }

    #ifdef PRINT
    // cout << "\n..............brute force | density = " << density <<"\n";
    #endif
    int bl_y, bl_x, tr_x, tr_y;     
    int x = edge_x;
    int y = edge_y;
    int x1,x2,y1,y2;
    while(density<get_min_density()){
        //不確定是不是可以是正方形
        //先設最小的面積的來塞
        y2=edge_y;
        y1=y2+get_width();
        while(y1 <= edge_y + windowsize)
        {
            x2=edge_x;
            x1=x2+get_width();
            while(x1 <= edge_x + windowsize)
            {
                (x2 - get_gap() >= get_bl_boundary_x()) ? bl_x = x2 - get_gap()      : bl_x = get_bl_boundary_x();
                (y2 - get_gap() >= get_bl_boundary_y()) ? bl_y = y2 - get_gap()      : bl_y = get_bl_boundary_y();
                (x1 + get_gap() >  get_tr_boundary_x()) ? tr_x = get_tr_boundary_x() : tr_x = x1 + get_gap()     ;
                (y1 + get_gap() >  get_tr_boundary_y()) ? tr_y = get_tr_boundary_y() : tr_y = y1 + get_gap()     ;
                // cout << "Wei Kai is " << x1 << " " << y1 <<endl;
                // if(expand(tr_x,tr_y,bl_x,bl_y, edge_x, edge_y, windowsize, 2))
                // if (edge_x == 3670000 && edge_y == 1965000){
                //     cout << "xxx: "<< x2 << endl;
                //     cout << "yyy: "<< y2 << endl;
                // }
                if(expand(x1, y1, x2, y2, edge_x, edge_y, windowsize, 5))
                {
                    // if(tr_x-bl_x-2*get_gap()>=get_width()&&tr_y-bl_y-2*get_gap()>=get_width()){
                    // cout << "Wei Kai is __!!!" << endl;
                    Polygon* T = new Polygon("filled",true);
                    T->set_xy(x1, y1, x2, y2);
                    insert_dummies(T, layer_id, density, edge_x, edge_y, windowsize, 3, output, fillnum);
                
                    if(density>=get_min_density()){
                        out = output.str();
                        return;
                    }
                    delete T;
                    T = NULL;
                    x2=tr_x;
                    x1=x2+get_width();
                    // }
                }
                else {
                    x2 += 10;
                    x1 += 10;
                }
            }
            y2 += 10;
            y1 += 10;
        }
        // cout<<"QQ塞不滿 | den =" << density;
        out = output.str();
        return;
    }
    if(density>=get_min_density()){
        out = output.str();
        return;
    }
    
    // cout<<"QQ塞不滿\n";
    // out = output.str();
    // return;
}

void Layer::insert_slots(GRBModel *model, Polygon *p, const int &poly_w, const int &poly_h, int &slot_id)
{
    vector<int> coordinate_y;
    vector<int> coordinate_x;
    int w_y, w_x = 0;
    vector<Polygon *> v;
    int count = 0;
    int count_2 = 0;
    if (p->_top_right_x() + get_gap() <= get_tr_boundary_x())
        region_query(get_dummy(), p->_top_right_x() + get_gap(), p->_top_right_y(), p->_top_right_x(), p->_bottom_left_y(), v);
    else
        count = 1;
    for (int i = 0; i < v.size(); i++)
    {
        if (v[i]->getType() != "space")
            count++;
    }
    v.clear();
    if (p->_bottom_left_x() - get_gap() >= get_bl_boundary_x())
        region_query(get_dummy(), p->_bottom_left_x(), p->_top_right_y(), p->_bottom_left_x() - get_gap(), p->_bottom_left_y(), v);
    else
        count_2 = 1;
    for (int i = 0; i < v.size(); i++)
    {
        if (v[i]->getType() != "space")
            count_2++;
    }
    if (count == 0 && count_2 != 0)
        w_x = find_optimal_width(p->_bottom_left_x(), poly_w + get_gap(), coordinate_x);
    else if (count == 0 && count_2 == 0)
        w_x = find_optimal_width(p->_bottom_left_x() - get_gap(), poly_w + 2 * get_gap(), coordinate_x);
    else if (count != 0 && count_2 == 0)
        w_x = find_optimal_width(p->_bottom_left_x() - get_gap(), poly_w + get_gap(), coordinate_x);
    else
        w_x = find_optimal_width(p->_bottom_left_x(), poly_w, coordinate_x);
    vector<Polygon *> vv;
    count = 0;
    if (p->_top_right_y() + get_gap() <= get_tr_boundary_y())
        region_query(get_dummy(), p->_top_right_x(), p->_top_right_y() + get_gap(), p->_bottom_left_x(), p->_top_right_y(), vv);
    else
        count = 1;
    for (int i = 0; i < vv.size(); i++)
    {
        if (vv[i]->getType() != "space")
            count++;
    }
    if (count == 0)
        w_y = find_optimal_width(p->_bottom_left_y(), poly_h + get_gap(), coordinate_y);
    else
        w_y = find_optimal_width(p->_bottom_left_y(), poly_h, coordinate_y);

    for (int j = 0; j < coordinate_y.size(); j++)
    {
        for (int k = 0; k < coordinate_x.size(); k++)
        {
            int x1 = coordinate_x[k] + w_x / 2;
            if (w_x % 2 != 0)
                x1++;
            int y1 = coordinate_y[j] + w_y / 2;
            if (w_y % 2 != 0)
                y1++;

            int x2 = coordinate_x[k] - w_x / 2;
            int y2 = coordinate_y[j] - w_y / 2;
            Polygon *S = new Polygon(coordinate_y[j], model, ++slot_id);
            S->set_layer_id(layer_id);
            S->set_xy(x1, y1, x2, y2);
            // S->setALL(coordinate_y[j], model, ++slot_id);
            insert(S, true, dummy_bottom);
            // if(layer_id == 4){
            //     if (x1 == 3426907 && x2 == 3425657 && y1 == 1802562 && y2 == 1801889)
            //         cout <<"gurobi is shit=================================\n";
            // }
            // assert();

            //  3425657 1801889 3426907 1802562
            // cout << S->get_slot_id() << endl;
            // vector<Polygon*> pp ;
            // region_query(dummy_bottom, x1, y1, x2, y2, pp);
            // for(int i = 0; i< pp.size(); ++i)
            //     cout << pp[i]->getType() << " id:" << pp[i]->get_slot_id()
            //      << " Y_ij "<< pp[i]->get_Wi_coord(-1) << endl;
            // slot_list.push_back(S);
            delete S;
            S = NULL;
        }
    }
}

//     void
//     Layer::insert_slots(GRBModel *model, Polygon *p, const int &poly_w, const int &poly_h, int &slot_id)
// {
//     vector<int> coordinate_y;
//     vector<int> coordinate_x;
//     int w_y = find_optimal_width(p->_bottom_left_y(), poly_h, coordinate_y);
//     int w_x = find_optimal_width(p->_bottom_left_x(), poly_w, coordinate_x);

//     for (int j = 0; j < coordinate_y.size(); j++)
//     {
//         for (int k = 0; k < coordinate_x.size(); k++)
//         {
//             int x1 = coordinate_x[k] + w_x / 2;
//             if( w_x%2 != 0) x1++;
//             int y1 = coordinate_y[j] + w_y / 2;
//             if( w_y%2 != 0) y1++;
            
//             int x2 = coordinate_x[k] - w_x / 2;
//             int y2 = coordinate_y[j] - w_y / 2;
//             Polygon *S = new Polygon(coordinate_y[j], model, ++slot_id);
//             S->set_layer_id(layer_id);
//             S->set_xy(x1, y1, x2, y2);
//             // S->setALL(coordinate_y[j], model, ++slot_id);
//             insert(S, true, dummy_bottom);
//             if(layer_id == 4){
//                 if (x1 == 3426907 && x2 == 3425657 && y1 == 1802562 && y2 == 1801889)
//                     cout <<"gurobi is shit=================================\n";
//             }
//                 // assert();

//                 //  3425657 1801889 3426907 1802562
//             // cout << S->get_slot_id() << endl;
//             // vector<Polygon*> pp ;
//             // region_query(dummy_bottom, x1, y1, x2, y2, pp);
//             // for(int i = 0; i< pp.size(); ++i)
//             //     cout << pp[i]->getType() << " id:" << pp[i]->get_slot_id()
//             //      << " Y_ij "<< pp[i]->get_Wi_coord(-1) << endl;
//             // slot_list.push_back(S);
//             delete S;
//             S = NULL;
//         }
//     }
// }

int Layer::find_optimal_width(const int &boundary, const int &length, vector<int> &coordinates)
{
    //boundary 是下面的y或左邊的x
    int optimal_N = 0;
    int optimal_W = 0;

    int min_N = ((length - min_space) / (max_fill_width + min_space));
    int max_N = ((length - min_space) / (min_width + min_space));
    //cout<<"len= "<<length<<" min_N "<<min_N<<" max_N "<<max_N<<endl;
    if (max_N <= 0) 
        return 0;

    for (int n = max(min_N, 1); n <= max_N; ++n)
    {
        int width = floor((length - min_space) / n - min_space);
        if(width > max_fill_width )width = max_fill_width;
        if(width < min_width ){
            cout<<"-----------------error-------------------\n";
            width = min_width;}
        if (n * width > optimal_N * optimal_W)
        {
            optimal_W = width;
            //cout<<"opt w = " << optimal_W <<endl;
            optimal_N = n;
        }
    }
    for(int i = 0; i < optimal_N; ++i){
        coordinates.push_back(boundary + (i+1) * (optimal_W + min_space) - 0.5 *optimal_W);
    }
    if(optimal_W < min_width ){
            cout<<"-----------------error-------------------\n";
            optimal_W = min_width;}
    return optimal_W;
}


void Layer::critical_find_lr(Polygon *critical, vector<Polygon *> & neighbor_list)
{
    assert(critical->is_critical());
    neighbor_list.clear();

    int x_start = critical->_top_right_x() + min_space;
    int y_start = critical->_top_right_y();
    Polygon *current = point_search(dummy_bottom, x_start, y_start);
    if (x_start > _bl_boundary_x && x_start < _tr_boundary_x && y_start > _bl_boundary_y && y_start <_tr_boundary_y){
        while(current->_top_right_y() > critical->_bottom_left_y()){
            if(current->getType() == "slot"){
                neighbor_list.push_back(current);
            }
            if(current->_bottom_left_y() - min_space > _bl_boundary_y){
                current = point_search(current, x_start, current->_bottom_left_y() - min_space);
            }
            else break;
        }
    }

    x_start = critical->_bottom_left_x() - min_space;
    y_start = critical->_top_right_y();
    if (x_start > _bl_boundary_x && x_start < _tr_boundary_x && y_start > _bl_boundary_y && y_start <_tr_boundary_y){
        current = point_search(dummy_bottom, x_start, y_start);
        while(current->_top_right_y() > critical->_bottom_left_y()){
            if(current->is_slot()){
                neighbor_list.push_back(current);
            }
            if(current->_bottom_left_y() - min_space > _bl_boundary_y){
                current = point_search(current, x_start, current->_bottom_left_y() - min_space);
            }
            else break;
        }
    }
}

void Layer::critical_find_top(Polygon *critical, vector<Polygon *> &neighbor_list)
{
    assert(critical->is_critical());
    int x_start = critical->_bottom_left_x();
    int y_start = critical->_top_right_y() + min_space + 1 ;
    Polygon *current = point_search(dummy_bottom, x_start, y_start);
    neighbor_list.clear();

    while(current->_bottom_left_x() < critical->_top_right_x()){
        if(current->is_slot()){
            neighbor_list.push_back(current);
        }
        if(current->_top_right_x() + min_space < _tr_boundary_x){
            current = point_search(current, current->_top_right_x() + min_space, y_start);
            assert(current != NULL);
        }
        else break;
    }
}

void Layer::critical_find_bottom(Polygon *critical, vector<Polygon *> &neighbor_list)
{
    assert(critical->is_critical());
    int x_start = critical->_bottom_left_x();
    int y_start = critical->_bottom_left_y() - min_space -1;
    Polygon *current = point_search(dummy_bottom, x_start, y_start);
    neighbor_list.clear();
    while(current->_bottom_left_x() < critical->_top_right_x()){
        if(current->is_slot())
            neighbor_list.push_back(current);
        if(current->_top_right_x() + min_space < _tr_boundary_x){
            current = point_search(current, current->_top_right_x() + min_space, y_start);
            assert(current != NULL);
        }
        else break;
    }
}

void Layer::critical_find_vertical(Polygon *critical, vector<Polygon *> & slot_list, int space){
    slot_list.clear();
    int x1 = min(critical->_top_right_x() + space, _tr_boundary_x);
    int y1 = min(critical->_top_right_y() + space, _tr_boundary_y);
    int x2 = max(critical->_bottom_left_x() - space, _bl_boundary_x);
    int y2 = max(critical->_bottom_left_y() - space, _bl_boundary_y);
    
    vector<Polygon*> poly_list;
    region_query(get_dummy(), x1, y1, x2, y2, poly_list);
    for(int i = 0; i < poly_list.size(); ++i){
        if(poly_list[i]->is_slot())
            slot_list.push_back(poly_list[i]);
    }

}