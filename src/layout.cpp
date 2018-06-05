#include <iostream>
#include <sstream>
#include <fstream>
#include <climits>
#include <iostream>
#include <iomanip> 
#include <vector>
#include <string>
#include <unordered_set>
#include "layout.h"
#include "util.h"

using namespace std;
unsigned Polygon::global_ref=0;
void Layer::init_polygon(string &filename, unordered_set<int> &cnet_set)
{
    ifstream ifs(filename);
    size_t filesize;
    ifs.seekg(0, ios::end);
    filesize = ifs.tellg();
    ifs.seekg(0, ios::beg);
    char* buff = new char[filesize+1];
    ifs.read(buff, filesize);
    char* buff_beg = buff;
    char* buff_end = buff + filesize;
    string token;
    int num;

    bool first_line = true;
    vector<int> tokens;
    Polygon* poly;
    while (token != ""){
        if (first_line){
            while ( (token = next_token(buff_beg, buff_end)) != ""){
                if (token[0] == '\n') {first_line = false; break;}
                if (myStr2Int(token, num)){
                    tokens.push_back(num);
                }
            }
            _bl_boundary_x = tokens[0];
            _bl_boundary_y = tokens[1];
            _tr_boundary_x = tokens[2];
            _tr_boundary_y = tokens[3];
            tokens.clear();
        }
        else {
            while ( (token = next_token(buff_beg, buff_end)) != ""){
                if (token[0] == '\n') break;
                if (myStr2Int(token, num)){
                    tokens.push_back(num);
                }
                else {
                    poly = new Polygon(token); 
                }
            }
            poly->set_coordinate(tokens);
            poly->setToSolid();
            if (cnet_set.count(tokens[5])){
                poly->setToCNet();
            }
            _polygonlist.push_back(poly);
        }
    }
}
void Layer::initialize_layer(){
    //一開始空的大space
    Polygon *a=new Polygon("space");
    a->set_xy(_tr_boundary_x,_tr_boundary_y,_bl_boundary_x,_tr_boundary_y);
    size_t x1=_tr_boundary_x;
    size_t y1=_tr_boundary_y;
    size_t x2=_bl_boundary_x;
    size_t y2=_bl_boundary_y;
    //dummy left
    dummy_left=new Polygon("dummy");
    dummy_left->set_xy(x2,y1,x2-1,y2);
    a->set_bl(dummy_left);
    //dummy right
    dummy_right=new Polygon("dummy");
    dummy_right->set_xy(x1+1,y1,x1,y2);
    a->set_tr(dummy_right);
    //dummy top
    dummy_top=new Polygon("dummy");
    dummy_top->set_xy(x1,y1+1,x2,y1);
    a->set_rt(dummy_top);
    dummy_top->set_lb(a);
    //dummy bottom
    dummy_bottom=new Polygon("dummy");
    dummy_bottom->set_xy(x1,y2,x2,y2-1);
    a->set_lb(dummy_bottom);
    dummy_bottom->set_rt(a);
}
Polygon* Layer::point_search(Polygon* start,size_t x,size_t y){
    /* 當x,y有切齊的時候 我們會選被包在框框裡面的tile 因此是
    top y = y  or left x = x 的時候
    */
    Polygon* current=start;
    while((x>=current->_top_right_x()||x<=current->_bottom_left_x())&&(y>=current->_top_right_y()||y<=current->_bottom_left_y())){
        while((y>=current->_top_right_y()||y<=current->_bottom_left_y())){
            if(y==current->_top_right_y())break;
            if(y>current->_top_right_y())current=current->get_rt();
            else current=current->get_lb();
        }
        while((x>=current->_top_right_x()||x<=current->_bottom_left_x())){
            if(x==current->_bottom_left_x())break;
            if(x>current->_top_right_x())current=current->get_tr();
            else current=current->get_bl();
        }
    }
    return current;
}
void neighbor_find_own(Polygon* T,vector<Polygon*> &v){
    //上到下找
    Polygon* current=T->get_tr();
    while(current->_bottom_left_y()>=T->_bottom_left_y()){
        v.push_back(current);
        current=current->get_lb();
    }
}
void enumerate(Polygon* T,vector<Polygon*> &v,int max_x){
    if(T->isglobalref())return;
    v.push_back(T);
    T->setToglobalref();
    if(T->_top_right_y()>=max_x)
        return;
    vector<Polygon*>neighbor;
    neighbor_find_own(T,neighbor);
    for(int i=0;i<neighbor.size();i++){
        enumerate(neighbor[i],v,max_x);
    }
    return;
}
void neighbor_find_right(Polygon* T,vector<Polygon*> &v){
    //上到下找
    Polygon* current=T->get_tr();
    int top=T->_top_right_y();
    int bottom=T->_bottom_left_y();
    while((current->_bottom_left_y()>=bottom&&current->_bottom_left_y()<=top)
        ||(current->_top_right_y()>=bottom&&current->_top_right_y()<=top)){
        v.push_back(current);
        current=current->get_lb();
    }
}
void neighbor_find_left(Polygon* T,vector<Polygon*> &v){
    //下到上找
    Polygon* current=T->get_bl();
    int top=T->_top_right_y();
    int bottom=T->_bottom_left_y();
    while((current->_bottom_left_y()>=bottom&&current->_bottom_left_y()<=top)
        ||(current->_top_right_y()>=bottom&&current->_top_right_y()<=top)){
        v.push_back(current);
        current=current->get_rt();
    }
}

vector<Polygon*> Layer::region_query(Polygon* start,size_t x1,size_t y1,size_t x2,size_t y2){
    /*x1,y1 是右上   x2,y2 是左下
    左上角的座標是 x2,y1 是我們要query的
    */
    Polygon::setGlobalref();
    vector<Polygon*> query_Polygon;
    vector<Polygon*>left_poly;
    start=point_search(start,x2,y1);
    while(start->_bottom_left_y()>=y2){
        left_poly.push_back(start);
        start=point_search(start,x2,start->_bottom_left_y());
    }
    left_poly.push_back(start);//最後那塊跳出來的時候 也算在裡面 參照region query 8號tile
    for(int i=0;i<left_poly.size()-1;i++){
        enumerate(left_poly[i],query_Polygon,x1);
    }
    //當region query最下面那排要特別處理
    //找最下面那排有哪些tile
    vector<Polygon*>bottom_poly;
    Polygon* T=left_poly[left_poly.size()-1];
    while(T->_bottom_left_x()<x1){
        bottom_poly.push_back(T);
        T=point_search(T,T->_top_right_x(),y2);
    }
    for(int i=0;i<bottom_poly.size();i++){
        enumerate(bottom_poly[i],query_Polygon,x1);
    }
    return query_Polygon;
}
vector<Polygon*> Layer::region_query(Polygon* start,Polygon* T){
    return region_query(start,T->_top_right_x(),T->_top_right_y(),T->_bottom_left_x(),T->_bottom_left_y());
}
Polygon* Layer::split_Y(Polygon* &bigGG,size_t y,bool is_top){
    Polygon* new_poly=new Polygon(bigGG->getType(),bigGG->is_solid());
    if(is_top){
        new_poly->set_xy(bigGG->_top_right_x(),bigGG->_top_right_y(),bigGG->_bottom_left_x(),y);
        new_poly->set_rt(bigGG->get_rt());
        new_poly->set_tr(bigGG->get_tr());
        new_poly->set_lb(bigGG);
        new_poly->set_bl(bigGG->get_bl());
        //point_search(bigGG,bigGG->_bottom_left_x()-1,y)
        //把上面的東西指回來新的
        Polygon *tp;
        for(tp=bigGG->get_rt();tp->get_lb()==bigGG;tp=tp->get_bl())
            tp->set_lb(new_poly);
        bigGG->set_rt(new_poly);
        /*
        for(tp=bigGG->get_tr();tp->_bottom_left_y()>=y;tp=tp->get_lb())
            tp->set_bl(new_poly);
            */
        //bigGG->set_tr(point_search(new_poly,bigGG->_top_right_x()+1,y));
        bigGG->set_xy(bigGG->_top_right_x(),y,bigGG->_bottom_left_x(),bigGG->_bottom_left_y());
    }
    else{   
        new_poly->set_xy(bigGG->_top_right_x(),y,bigGG->_bottom_left_x(),bigGG->_bottom_left_y());
        new_poly->set_rt(bigGG);
        new_poly->set_lb(bigGG->get_lb());
        new_poly->set_bl(bigGG->get_bl());
        new_poly->set_tr(bigGG->get_tr());//point_search(bigGG,bigGG->_top_right_x()+1,y)
        Polygon *tp;
        for(tp=bigGG->get_lb();tp->get_rt()==bigGG;tp=tp->get_tr())
            tp->set_rt(new_poly);

        bigGG->set_xy(bigGG->_top_right_x(),bigGG->_top_right_y(),bigGG->_bottom_left_x(),y);
        bigGG->set_lb(new_poly);
        //bigGG->set_bl(point_search(bigGG,bigGG->_bottom_left_x()-1,y));
    }
    return new_poly;

}
vector<Polygon*> Layer::split_X(Polygon* &bigGG, size_t x_left,size_t x_right ,Polygon* inserted){
    Polygon* new_poly_left = new Polygon(bigGG->getType(), bigGG->is_solid());
    Polygon* new_poly_right = new Polygon(bigGG->getType(), bigGG->is_solid());
    Polygon* tp;
    new_poly_left->set_xy(x_left,bigGG->_top_right_y(),bigGG->_bottom_left_x(),bigGG->_bottom_left_y());
    new_poly_left->set_rt(point_search(bigGG,x_left,bigGG->_top_right_y()+1));
    new_poly_left->set_tr(inserted);
    new_poly_left->set_lb(bigGG->get_lb());
    new_poly_left->set_bl(bigGG->get_bl());
    bigGG->get_bl()->set_tr(new_poly_left);
    new_poly_right->set_xy(bigGG->_top_right_x(),bigGG->_top_right_y(),x_right,bigGG->_bottom_left_y());
    new_poly_right->set_rt(bigGG->get_rt());
    new_poly_right->set_tr(bigGG->get_tr());
    new_poly_right->set_bl(inserted);
    bigGG->get_tr()->set_bl(new_poly_right);
    for(tp=bigGG->get_rt();tp->_bottom_left_x()>=x_right;tp=tp->get_bl())
        tp->set_lb(new_poly_right);
    for(tp=bigGG->get_lb();tp->_top_right_x()<=x_right;tp=tp->get_tr()){
        if(tp->_top_right_x()<=x_left){
            tp->set_rt(new_poly_left);
        }
    }
    new_poly_right->set_lb(tp);
    while(tp->get_rt()==bigGG){
        tp->set_rt(new_poly_right);
        tp=tp->get_tr();
    }
    vector<Polygon*>split;
    split.push_back(new_poly_right);
    split.push_back(new_poly_left);
    return split;
}
bool Layer::insert(Polygon* T){
    //x y 是要給左上角的座標
    vector<Polygon*> query_list=region_query(dummy_bottom,T);
    for(size_t i=0;i<query_list.size();i++){
        if(query_list[i]->is_solid())
            return false;
    }

    //case 1 
    if(query_list.size()==1){

    }
    //case 2
    else if (query_list.size()==2){

    }
    //case 3 
    else{
        vector<Polygon*>edge_lilst;//存在邊上的那些的tile就好 只有那些要切

    }

return true;

}
void join(Polygon* T1,Polygon *T2){
    Polygon* tp;
    if(T1->_bottom_left_x()==T2->_bottom_left_x()&&T1->_top_right_x()==T2->_top_right_x()){
        T1->set_xy(T1->_top_right_x(),T1->_top_right_y(),T2->_bottom_left_x(),T2->_bottom_left_y());

        for(tp=T2->get_lb();tp->get_rt()==T2;tp=tp->get_tr()){
            tp->set_rt(T1);//把下面網上指導t2的指導t1
        }
        if(T2->get_tr()->get_bl()==T2)T2->get_tr()->set_bl(T1);
        delete T2;
        return;
    }
    else return;
}




