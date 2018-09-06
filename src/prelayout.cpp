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
unsigned Polygon::global_ref=0;


double Layer::slot_area(const int &x, const int &y, const double &windowsize, vector<Polygon *> &slot_vec)
{
    //region 右上/左下
    vector<Polygon *> query_list;
    // cout<<"(slot area) start query..."<<endl;
    // cout<<"window size = "<<windowsize<<endl; 
    // cout<<"X = "<<x<<" Y = "<<y<<" Xr = "<<x+windowsize<<" Yr = "<<y+windowsize<<endl;
    // print_Polygon(dummy_bottom);
    region_query(dummy_bottom, x + windowsize, y + windowsize, x, y, query_list);
    slot_vec.clear();

    double area = 0, x_area = 0, y_area = 0;
    for (int i = 0; i < query_list.size(); i++)
    {
        if (query_list[i]->is_solid())
        {
            x_area = classify(query_list[i]->_top_right_x(), query_list[i]->_bottom_left_x(), x + windowsize, x);
            y_area = classify(query_list[i]->_top_right_y(), query_list[i]->_bottom_left_y(), y + windowsize, y);
            if (x_area < 0 || y_area < 0)
                ; //cout<<query_list[i]->_top_right_x()<<" "<<query_list[i]->_top_right_y()<<"in window"<<x<<","<<y<<"有問題\n";
            else
                area += x_area * y_area;
        }
        if (query_list[i]->getType() == "slot")
            slot_vec.push_back(query_list[i]);
    }
    return area;
}


// calculate density in a window
// return: density, critical nets and slots in window
double Layer::density_calculate(const int &x, const int &y, const double &windowsize, vector<Polygon *> &query_list)
{   
    //region 右上/左下

    double area = 0, x_area = 0, y_area = 0;
    for (int i = 0; i < query_list.size(); i++)
    {
        if (query_list[i]->is_solid())
        {

            x_area = classify(query_list[i]->_top_right_x(), query_list[i]->_bottom_left_x(), x + windowsize, x);
            y_area = classify(query_list[i]->_top_right_y(), query_list[i]->_bottom_left_y(), y + windowsize, y);
            if (x_area < 0 || y_area < 0)
                ; //cout<<query_list[i]->_top_right_x()<<" "<<query_list[i]->_top_right_y()<<"in window"<<x<<","<<y<<"有問題\n";
            else
                area += x_area * y_area;
        }
        // if (query_list[i]->is_critical())
        //     critical_vec.push_back(query_list[i]);
    }
    return area / (windowsize * windowsize);
}

void Layer::init_rule(int n1, int n2, int n3, double min, double max, int index)
{
    min_width = n1;
    min_space = n2;
    max_fill_width = n3;
    min_density = min;
    max_density = max;
    layer_id = index;
}

void Layer::init_layer(int x_bl, int y_bl, int x_tr, int y_tr)
{
    #ifdef DEBUG
        cout<<"init layer "<<endl;
    #endif
    _tr_boundary_x = x_tr;
    _tr_boundary_y = y_tr;
    _bl_boundary_x = x_bl;
    _bl_boundary_y = y_bl;
    //一開始空的大space
    Polygon *a=new Polygon("space");
    a->set_xy(_tr_boundary_x,_tr_boundary_y,_bl_boundary_x,_bl_boundary_y);
    int x1=_tr_boundary_x;
    int y1=_tr_boundary_y;
    int x2=_bl_boundary_x;
    int y2=_bl_boundary_y;
    //dummy left
    dummy_left=new Polygon("dummy left",true);
    dummy_left->set_xy(x2,y1,x2-3-get_gap(),y2);
    a->set_bl(dummy_left);
    dummy_left->set_tr(a);
    
    //dummy right
    dummy_right=new Polygon("dummy right",true);
    dummy_right->set_xy(x1+3+get_gap(),y1,x1,y2);
    dummy_right->set_bl(a);
    a->set_tr(dummy_right);
    //dummy top
    dummy_top=new Polygon("dummy top",true);
    dummy_top->set_xy(x1,y1+3+get_gap(),x2,y1);
    a->set_rt(dummy_top);
    dummy_top->set_lb(a);
    //dummy bottom
    dummy_bottom=new Polygon("dummy bottom",true);
    dummy_bottom->set_xy(x1,y2,x2,y2-3-get_gap());
    a->set_lb(dummy_bottom);
    dummy_bottom->set_rt(a);

    dummy_bottom_right = new Polygon("dummy bottom right",true);
    dummy_bottom_right->set_xy(x1+3+get_gap(), y2, x1, y2-3-get_gap());
    dummy_bottom_right->set_rt(dummy_right);
    dummy_bottom_right->set_bl(dummy_bottom);

    dummy_right_top = new Polygon("dummy right top",true);
    dummy_right_top->set_xy(x1+3+get_gap(), y1+3+get_gap(), x1, y1);
    dummy_right_top->set_lb(dummy_right);
    dummy_right_top->set_bl(dummy_top);

    dummy_bottom_left = new Polygon("dummy bottom left",true);
    dummy_bottom_left->set_xy(x2, y2, x2-3-get_gap(), y2-3-get_gap());
    dummy_bottom_left->set_rt(dummy_left);
    dummy_bottom_left->set_tr(dummy_bottom);
    
    dummy_top_left = new Polygon("dummy top left",true);
    dummy_top_left->set_xy(x2, y1+3+get_gap(), x2-3-get_gap(), y1);
    dummy_top_left->set_lb(dummy_left);
    dummy_top_left->set_tr(dummy_top);
    
    dummy_bottom->set_bl(dummy_bottom_left);
    dummy_bottom->set_tr(dummy_bottom_right);
    dummy_top->set_tr(dummy_right_top);
    dummy_top->set_bl(dummy_top_left);
    dummy_right->set_lb(dummy_bottom_right);
    dummy_right->set_rt(dummy_right_top);
    dummy_left->set_lb(dummy_bottom_left);
    dummy_left->set_rt(dummy_top_left);
}

Polygon* Layer::point_search(Polygon* start,int x,int y)
{
    /* 當x,y有切齊的時候 我們會選被包在框框裡面的tile 因此是
    top y = y  or left x = x 的時候
    */
    #ifdef DEBUG
        cout<<"point search "<<endl;
    #endif
    Polygon* current=start;
    //如果要query的座標比現在的x還要大或還要小且 要query的座標比現在的y還要大或還要小
    while((x>=current->_top_right_x()||x<current->_bottom_left_x())||(y>current->_top_right_y()||y<=current->_bottom_left_y())){
        while((y>=current->_top_right_y()||y<=current->_bottom_left_y())){
            if(y==current->_top_right_y())break;
            if(y>current->_top_right_y())current=current->get_rt();
            else current=current->get_lb();
        }
        while((x>=current->_top_right_x()||x<=current->_bottom_left_x())){
            if(x==current->_bottom_left_x())break;
            if(x>=current->_top_right_x())current=current->get_tr();
            else current=current->get_bl();
        }
        //current== 2200000 2477214
    }
    return current;
}

void Layer::region_query(Polygon* start,int x1,int y1,int x2,int y2, vector<Polygon*>& query_Polygon)
{
    /*x1,y1 是右上   x2,y2 是左下
    左上角的座標是 x2,y1 是我們要query的
    */
    int a;
    if(x1 <0 ||y1<0 ) cin>>a;
    #ifdef DEBUG
        cout<<"region query "<<endl;
    #endif
    Polygon::setGlobalref();

    vector<Polygon*> left_poly;
    start = point_search(start,x2,y1);
    // cout << start << endl;
    while(start->_top_right_y()>y2){
        left_poly.push_back(start);
        start=point_search(start,x2,start->_bottom_left_y());
    }
    
    //left_poly.push_back(start);//最後那塊跳出來的時候 也算在裡面 參照region query 8號tile
    for(int i=0;i<left_poly.size();i++){
        enumerate(left_poly[i],query_Polygon,x1,y1,y2);
    }
    #ifdef DEBUG
        cout<<"query"<<query_Polygon.size()<<endl;
        cout<<"le"<<left_poly.size()<<endl;
    #endif
    //當region query最下面那排要特別處理
    //找最下面那排有哪些tile
    vector<Polygon*>bottom_poly;
    Polygon* T=left_poly[left_poly.size()-1];
    //Polygon* T=left_poly[0];
    while(T->_bottom_left_x()<x1){
        bottom_poly.push_back(T);
        T=point_search(T,T->_top_right_x(),y2+1);
    }
    #ifdef DEBUG
        cout<<"bot"<<bottom_poly.size()<<endl;
        Polygon* aaa=left_poly[0];
        Polygon* bbb=bottom_poly[0];
        cout<<"query2"<<query_Polygon.size()<<endl;
    #endif
    for(int i=0;i<bottom_poly.size();i++){
        (enumerate(bottom_poly[i],query_Polygon,x1,y1,y2));
    }
    // for (int i = 0; i < query_Polygon.size(); ++i)
        // cout << setw(3) << i << " " << query_Polygon[i] << " " << query_Polygon[i]->getType() <<  endl;
}

void Layer::region_query(Polygon *start, Polygon *T, vector<Polygon *> &query_Polygon)
{
    return region_query(start, T->_top_right_x(), T->_top_right_y(), T->_bottom_left_x(), T->_bottom_left_y(), query_Polygon);
}
bool neighbor_find_own_bool(Polygon* T,vector<Polygon*> &v,const int& max_y,const int& min_y)
{
    //上到下找
    #ifdef DEBUG
        cout<<"find own "<<endl;
    #endif
    Polygon* current=T->get_tr();

    while(current->_bottom_left_y()>=T->_bottom_left_y()){
        //
        if(current->_bottom_left_y()<max_y&&current->_top_right_y()>min_y)
            {
                v.push_back(current);
                if(current->is_solid())return false;
            }
        current=current->get_lb();
    }
    return true;
}
bool enumerate_bool(Polygon* T,vector<Polygon*> &v,const int& max_x,const int& max_y,const int& min_y)
{

    //找own
    #ifdef DEBUG
        cout<<"enumerate "<<endl;
    #endif
    if(T->isglobalref())return true;
    v.push_back(T);
    T->setToglobalref();
    if(T->is_solid())return false;
    //not sure top_right_x or top_right_y
    if(T->_top_right_x()>=max_x)
        return true;
    vector<Polygon*>neighbor;
    if(!neighbor_find_own_bool(T,neighbor,max_y,min_y))
        return false;
    for(int i=0;i<neighbor.size();i++){
        if(!enumerate_bool(neighbor[i],v,max_x,max_y,min_y))
            return false;
    }
    return true;
}

bool Layer::region_query_bool(Polygon* start,int x1,int y1,int x2,int y2, vector<Polygon*>& query_Polygon)
{
    if(x1 > get_tr_boundary_x() || x2 < get_bl_boundary_x()|| y1 > get_tr_boundary_y() || y2 < get_bl_boundary_y())
        return false;
    Polygon::setGlobalref();
    int a;
    vector<Polygon*>left_poly;
    start=point_search(start,x2,y1);
    while(start->_top_right_y()>y2){
        left_poly.push_back(start);
        start=point_search(start,x2,start->_bottom_left_y());
    }

    for(int i=0;i<left_poly.size();i++){
        if(!enumerate_bool(left_poly[i],query_Polygon,x1,y1,y2))
            return false;
    }
    vector<Polygon*>bottom_poly;
    Polygon* T=left_poly[left_poly.size()-1];
    while(T->_bottom_left_x()<x1){
        bottom_poly.push_back(T);
        T=point_search(T,T->_top_right_x(),y2+1);
    }
    for(int i=0;i<bottom_poly.size();i++){
        if(!enumerate_bool(bottom_poly[i],query_Polygon,x1,y1,y2))
            return false;
    }
    return true;
}

bool Layer::region_query_bool(Polygon *start, Polygon *T, vector<Polygon *> &query_Polygon)
{
    return region_query_bool(start, T->_top_right_x(), T->_top_right_y(), T->_bottom_left_x(), T->_bottom_left_y(), query_Polygon);
}

Polygon* Layer::split_Y(Polygon* bigGG,int y,bool is_top)
{
    #ifdef DEBUG
        cout<<"split y "<<endl;
    #endif     
    Polygon* new_poly=new Polygon(bigGG->getType(),bigGG->is_solid());
    if(is_top){
        new_poly->set_xy(bigGG->_top_right_x(),bigGG->_top_right_y(),bigGG->_bottom_left_x(),y);
        new_poly->set_rt(bigGG->get_rt());
        new_poly->set_tr(bigGG->get_tr());
        new_poly->set_lb(bigGG);

        //把上面的東西指回來新的
        Polygon *tp=bigGG->get_rt();
        while(tp->get_lb()==bigGG){
            //cout<<tp->_top_right_x()<<" "<<tp->_top_right_y()<<" "<<tp->getType()<<endl;
            tp->set_lb(new_poly);
            tp=tp->get_bl();
            //cout<<tp->_top_right_x()<<" "<<tp->_top_right_y()<<endl;
        }
        bigGG->set_rt(new_poly);
        
        //把左邊的人的tr指回來
        for(tp=bigGG->get_bl();tp->_top_right_y()<=y;tp=tp->get_rt())
            ;
        new_poly->set_bl(tp);
        while(tp->get_tr()==bigGG){
            tp->set_tr(new_poly);
            tp=tp->get_rt();
        }
        //把右邊的bl指回來
        for(tp=bigGG->get_tr();tp->_bottom_left_y()>=y;tp=tp->get_lb())
            tp->set_bl(new_poly);
        bigGG->set_tr(tp);

        bigGG->set_xy(bigGG->_top_right_x(), y ,bigGG->_bottom_left_x(),bigGG->_bottom_left_y());
    }
    else{   
        new_poly->set_xy(bigGG->_top_right_x(),y,bigGG->_bottom_left_x(),bigGG->_bottom_left_y());
        new_poly->set_rt(bigGG);
        new_poly->set_lb(bigGG->get_lb());
        new_poly->set_bl(bigGG->get_bl());
        //new_poly->set_tr(point_search(bigGG,bigGG->_top_right_x(),y));//point_search(bigGG,bigGG->_top_right_x()+1,y)
        //把下面的指回來上面
        Polygon *tp;
        for(tp=bigGG->get_lb();tp->get_rt()==bigGG;tp=tp->get_tr())
            tp->set_rt(new_poly);
        //把右邊的指回來
        for(tp=bigGG->get_tr();tp->_bottom_left_y()>=y;tp=tp->get_lb())
            ;
        new_poly->set_tr(tp);
        for(;tp->get_bl()==bigGG;tp=tp->get_lb())
            tp->set_bl(new_poly);
        //左邊的指回來
        for(tp=bigGG->get_bl();tp->_top_right_y()<=y;tp=tp->get_rt())
            tp->set_tr(new_poly);
        //set bigGG
        bigGG->set_bl(tp);
        bigGG->set_xy(bigGG->_top_right_x(),bigGG->_top_right_y(),bigGG->_bottom_left_x(),y);
        bigGG->set_lb(new_poly);
    }
    return new_poly;
}

Polygon* Layer::split_X_left(Polygon* bigGG, int x_left,int x_right)
{

    #ifdef DEBUG
        cout<<"split x_left "<<endl;
    #endif
    Polygon* tp;
    Polygon *new_poly = new Polygon(bigGG->getType(), bigGG->is_solid());
    new_poly->set_xy(x_left,bigGG->_top_right_y(),bigGG->_bottom_left_x(),bigGG->_bottom_left_y());
    new_poly->set_tr(bigGG);
    new_poly->set_lb(bigGG->get_lb());
    new_poly->set_bl(bigGG->get_bl());
    bigGG->set_xy(bigGG->_top_right_x(),bigGG->_top_right_y(),x_left,bigGG->_bottom_left_y());
    //把左邊的指回來 
    for(tp=new_poly->get_bl();tp->get_tr()==bigGG;tp=tp->get_rt())
        tp->set_tr(new_poly);
    //把上面的接回來
    for(tp=bigGG->get_rt();tp->_bottom_left_x()>=x_left;tp=tp->get_bl())
        ;
    new_poly->set_rt(tp);
    for(;tp->get_lb()==bigGG;tp=tp->get_bl())
        tp->set_lb(new_poly);
    //下面指回來
    for(tp=bigGG->get_lb();tp->_top_right_x()<=x_left;tp=tp->get_tr())
        tp->set_rt(new_poly);
    bigGG->set_lb(tp);
    bigGG->set_bl(new_poly);
    return new_poly;
}

Polygon* Layer::split_X_right(Polygon* bigGG, int x_left,int x_right )
{
    #ifdef DEBUG
        cout<<"split x_right "<<endl;
    #endif
    Polygon* tp;
    Polygon *new_poly = new Polygon(bigGG->getType(), bigGG->is_solid());
    new_poly->set_xy(bigGG->_top_right_x(),bigGG->_top_right_y(),x_right,bigGG->_bottom_left_y());
    new_poly->set_rt(bigGG->get_rt());
    new_poly->set_tr(bigGG->get_tr());
    new_poly->set_bl(bigGG);
    bigGG->set_xy(x_right,bigGG->_top_right_y(),bigGG->_bottom_left_x(),bigGG->_bottom_left_y());
    //上面接回來
    for(tp=bigGG->get_rt();tp->_bottom_left_x()>=x_right;tp=tp->get_bl())
        tp->set_lb(new_poly);
    bigGG->set_rt(tp);
    // 把下面指回來
    for(tp=bigGG->get_lb();tp->_top_right_x()<=x_right;tp=tp->get_tr())
        ;
    new_poly->set_lb(tp);
    while(tp->get_rt()==bigGG){
        tp->set_rt(new_poly);
        tp=tp->get_tr();
    }
    //右邊指回來
    for(tp=bigGG->get_tr();tp->get_bl()==bigGG;tp=tp->get_lb())
        tp->set_bl(new_poly);
    bigGG->set_tr(new_poly);
    return new_poly;
}

void join(Polygon* T1,Polygon *T2)
{
    Polygon* tp;
    if(T1->_bottom_left_x()==T2->_bottom_left_x()&&T1->_top_right_x()==T2->_top_right_x()&&T1->_bottom_left_y()==T2->_top_right_y()){
        T1->set_xy(T1->_top_right_x(),T1->_top_right_y(),T2->_bottom_left_x(),T2->_bottom_left_y());
        //沿著右邊指回來
        #ifdef DEBUG
        cout<<"joining"<<endl;
        #endif
        //cout<<"joining"<<endl;
        for(tp=T2->get_tr();tp->get_bl()==T2;tp=tp->get_lb()){
            tp->set_bl(T1);
        }
        for(tp=T2->get_bl();tp->get_tr()==T2;tp=tp->get_rt()){
            tp->set_tr(T1);
        }
        //把下面網上指導t2的指導t1
        for(tp=T2->get_lb();tp->get_rt()==T2;tp=tp->get_tr()){
            tp->set_rt(T1);
        }
        T1->set_lb(T2->get_lb());
        T1->set_bl(T2->get_bl());
        delete T2;
        T2 = NULL;
        return;
    }
    else return;
}
bool Layer::insert(Polygon* T, bool first_inset, Polygon* start){
    /*  
        query 是給 先右上 再左下
        因為soild 會有最小間距 所以我們query看能不能夠塞的時候 
        要看大塊一點 四周都加最小間距
    */
    //3414815,1800065 3414750,1800000
    #ifdef DEBUG
        cout<<"insert "<<T->_top_right_x()<<" "<<T->_top_right_y()<<endl;
    #endif
    int bl_x,bl_y,tr_x,tr_y;
    vector<Polygon*> split_x_right;
    vector<Polygon*> split_x_left;
    Polygon*aa;
    bool is_left= ( T->_bottom_left_x() != get_bl_boundary_x() );
    bool is_right= ( T->_top_right_x() != get_tr_boundary_x() );
    vector<Polygon*> query_list;
    // if(!first_inset){
    //     //決定query的邊界
    //     //左下角
    //     (T->_bottom_left_x()-get_gap()>=get_bl_boundary_x()) ? bl_x = T->_bottom_left_x() - get_gap() : bl_x = get_bl_boundary_x();
    //     (T->_bottom_left_y()-get_gap()>=get_bl_boundary_y()) ? bl_y = T->_bottom_left_y() - get_gap() : bl_y = get_bl_boundary_y();
    //     //右上角
    //     (T->_top_right_x() + get_gap() >get_tr_boundary_x() ) ? tr_x = get_tr_boundary_x() : tr_x = T->_top_right_x() + get_gap();
    //     (T->_top_right_y() + get_gap() >get_tr_boundary_y() ) ? tr_y = get_tr_boundary_y() : tr_y = T->_top_right_y() + get_gap();
        
    //     // 如果要插進去的這塊裡面有solid就不能插 就像有男友的女生一樣
    //     if (!region_query_bool(start, tr_x, tr_y, bl_x, bl_y, query_list))
    //         return false;// (start,跟要插進去得tile)
    // }
    // else {
    //     if (!region_query_bool(start, T->_top_right_x(), T->_top_right_y(), T->_bottom_left_x(), T->_bottom_left_y(), query_list))
    //         return false;
    //     }
    /*for(int i=0;i<query_list.size();i++){
        if(query_list[i]->is_solid())
            return false;
    }*/
    // if(!first_inset){
    //     query_list.clear();
    //     region_query(start, T->_top_right_x(), T->_top_right_y(), T->_bottom_left_x(), T->_bottom_left_y(), query_list);
    // }
    #ifdef DEBUG
        cout<<"query_list num= "<<query_list.size()<<endl;
    #endif
    region_query_bool(start, T->_top_right_x(), T->_top_right_y(), T->_bottom_left_x(), T->_bottom_left_y(), query_list);
    Polygon* queryzero=query_list[0];
    /*

    split y

    */
    bool is_top    = (T->_top_right_y()   != get_tr_boundary_y()) ? true : false;
    bool is_bottom = (T->_bottom_left_y() != get_bl_boundary_y()) ? true : false;
    for(int i=0; i<query_list.size(); i++){
        if(query_list[i]->_top_right_y() > T->_top_right_y())
            split_Y(query_list[i], T->_top_right_y(), true);
        //上面貼著
        if(query_list[i]->_bottom_left_y()<T->_bottom_left_y())
            split_Y(query_list[i],T->_bottom_left_y(),false);
        //下面貼著
    }
    /*

    split x

    */
    #ifdef DEBUG
        cout<<"query_list num= "<<query_list.size()<<endl;
    #endif
    for(int i=0;i<query_list.size();i++)
    {
        if(is_left){
            if(query_list[i]->_bottom_left_x() < T->_bottom_left_x()){
                Polygon* a = split_X_left(query_list[i],T->_bottom_left_x(),T->_top_right_x());
                //cout<<"split left"<<query_list[i]->_top_right_x()<<" "<<query_list[i]->_top_right_y()<<endl;
                split_x_left.push_back(a);
            }
        }
        if(is_right){
            if(query_list[i]->_top_right_x() > T->_top_right_x()){
                //cout<<"split right"<<query_list[i]->_top_right_x()<<" "<<query_list[i]->_top_right_y()<<endl;
                Polygon* a = split_X_right(query_list[i],T->_bottom_left_x(),T->_top_right_x());
                split_x_right.push_back(a);
            }
        }
    }
    /*
    after splitting we join 
    */
    for(int i=query_list.size()-1;i>=1;i--)
        join(query_list[i-1],query_list[i]);
    for(int i=split_x_left.size()-1;i>=1;i--)
        join(split_x_left[i-1],split_x_left[i]);
    for(int i=split_x_right.size()-1;i>=1;i--)
        join(split_x_right[i-1],split_x_right[i]);
    #ifdef DEBUG
        cout<<"query_list num= "<<query_list.size()<<endl;
        print_Polygon(query_list[0]);
    #endif

    //set somthing
    if(T->is_critical())
        query_list[0]->setToCNet();
    if(T->getType()=="slot"){
        query_list[0]->setALL(T->get_Wi_coord(-1),T->get_model(),T->get_slot_id());
        slot_list.push_back(query_list[0]);
    }
    else{ 
        query_list[0]->setToSolid();
    }
    query_list[0]->setType(T->getType());
    query_list[0]->set_layer_id(T->get_layer_id());
    if(first_inset){
        query_list[0]->set_net_id(T->get_net_id());
        query_list[0]->set_polygon_id(T-> get_polygon_id());
    }
    #ifdef DEBUG
    if(T->_top_right_y()!=query_list[0]->_top_right_y()||T->_top_right_x()!=query_list[0]->_top_right_x()
        ||T->_bottom_left_y()!=query_list[0]->_bottom_left_y()||T->_bottom_left_x()!=query_list[0]->_bottom_left_x()){
        cout<<endl;
        cout<<"T"<<T->_top_right_x()<<","<<T->_top_right_y()<<endl;
        cout<<"Q"<<query_list[0]->_top_right_x()<<","<<query_list[0]->_top_right_y()<<endl;
    }
    #endif
    return true;

}
// void chipManager::insert_tile(string& output_fill){ 
//     vector<Polygon*> critical_nets;
//     int fillnum = 1;
//     stringstream out_fill;

//     for (int _time = 0; _time < 4; ++_time){
//         int x, y, wnd_num;
//         double density = 0;
//         int half_wnd = window_size / 2;
//         int horizontal_cnt = (_tr_bound_x - _bl_bound_x) / window_size;
//         int vertical_cnt = (_tr_bound_y - _bl_bound_y) / window_size;
//         if(_time == 1 || _time == 3)
//             horizontal_cnt -= 1;
//         if(_time == 2 || _time == 3)
//             vertical_cnt -= 1;
//         for (int i = 0; i < layer_num; ++i){
//             for(int row = 0; row < vertical_cnt; ++row){
//                 for (int col = 0; col < horizontal_cnt; ++col){
//                     x = _bl_bound_x + col * window_size;
//                     y = _bl_bound_y + row * window_size;
//                     if(_time == 1 or _time == 3)
//                         x += half_wnd;
//                     if(_time == 2 or _time == 3)
//                         y += half_wnd;
//                     wnd_num = i * horizontal_cnt * vertical_cnt + row * horizontal_cnt + col;                
//                     density = _LayerList[i].density_calculate(x, y, window_size, critical_nets);
                    
//                     if(density < _LayerList[i].get_min_density()){
//                         cout << "error\n";
//                         double new_density = density;
//                         // cout << "\n==========[ Layer: " << i + 1 << " | Window: " << row * horizontal_cnt + col << "/"
//                         //    << horizontal_cnt * vertical_cnt << "]=========="<< endl;
//                         // cout << "\n=============[ Layer: " << i + 1 << " | Row: " << row/2 << "| Col:"
//                         //     << col/2 << " | Pos:" << (row%2)*2+col%2 << "]=============" << endl;
//                         string out = "";
//                         //cout<<"insert dummy in Layer: "<<setw(1)<<i+1<<".......................\r";
//                         _LayerList[i].layer_fill(x, y, window_size, new_density, i + 1, out, fillnum);
//                         out_fill<<out;
//                         // cout<<"新的密度 "<<new_density<<" "<<x<<","<<y<<" windownum= "<<wnd_num<<" layer id= "<<i+1<<endl;
//                     }
//                 }
//             }       
//         }
//     }
//     //cout << endl;
//     int horizontal_cnt = (_tr_bound_x - window_size - _bl_bound_x) * 2 / window_size + 1;
//     int vertical_cnt = (_tr_bound_y - window_size - _bl_bound_y) * 2 / window_size + 1;

//     int x, y, wnd_num;
//     double density = 0;
//     int half_wnd = window_size / 2;

//     for (int i = 0; i < layer_num; ++i){
//         for(int row = 0; row < vertical_cnt; ++row){
//             for (int col = 0; col < horizontal_cnt; ++col){
//                 x = _bl_bound_x + col * window_size / 2;
//                 y = _bl_bound_y + row * window_size / 2;
//                 wnd_num = i * horizontal_cnt * vertical_cnt + row * horizontal_cnt + col;                
//                 density = _LayerList[i].density_calculate(x, y, window_size, critical_nets);
//                 if(density < _LayerList[i].get_min_density()){  
//                     cout<<"error\n";
//                     double new_density = density;
//                     cout << "\n==========[ Layer: " << i + 1 << " | Window: " << row * horizontal_cnt + col << "/"
//                          << horizontal_cnt * vertical_cnt << "]==========" << endl;
//                     string out = "";
//                     //cout<<"insert dummy in Layer: "<<setw(1)<<i+1<<".......................\r";
//                     _LayerList[i].layer_fill(x, y, window_size, new_density, i + 1, out, fillnum);
//                     out_fill<<out;
//                     // cout<<"新的密度 "<<new_density<<" "<<x<<","<<y<<" windownum= "<<wnd_num<<" layer id= "<<i+1<<endl;
//                 }
//             }
//         }       
//     }
//     ///cout << endl;
//     output_fill = out_fill.str();
// }
// void chipManager::report_density(bool init_cnet){
//     int x, y, wnd_num;
//     double density = 0;
//     double count[9] = {0}, count2[9] = {0};
//     int half_wnd = window_size / 2;
//     int horizontal_cnt = (_tr_bound_x - _bl_bound_x)  / half_wnd -1;
//     int vertical_cnt = (_tr_bound_y - _bl_bound_y)  / half_wnd -1 ;
//     vector<Polygon*> critical_nets;

//     for (int i = 0; i < layer_num; ++i){
//         for(int row = 0; row < vertical_cnt; ++row){
//             for (int col = 0; col < horizontal_cnt; ++col){
//                 x = _bl_bound_x + col * half_wnd;
//                 y = _bl_bound_y + row * half_wnd;
//                 wnd_num = i * horizontal_cnt * vertical_cnt + row * horizontal_cnt + col;
//                 density = _LayerList[i].density_calculate(x, y, window_size, critical_nets);
//                 if(init_cnet)
//                     // total_Cnet_List[].pus(critical_nets);
//                 if(density >= _LayerList[i].get_min_density() && density < _LayerList[i].get_min_density()+0.01)
//                     count[0]+=1;
//                 else if (density >= _LayerList[i].get_min_density()+0.01 && density < _LayerList[i].get_min_density()+0.04)
//                     count[1]+=1;
//                 else if (density >= _LayerList[i].get_min_density()+0.04 && density < _LayerList[i].get_min_density()+0.07)
//                     count[2]+=1;
//                 else if (density >= _LayerList[i].get_min_density()+0.07 && density < _LayerList[i].get_min_density()+0.1)
//                     count[3]+=1;
//                 else if (density >= _LayerList[i].get_min_density()+0.1)
//                     count[4]+=1;
//                 else{
//                     //cout << i+1 << " row " <<row/2 << " col "<<col/2 <<" pos " <<(row%2)*2 + (col%2) <<endl;
//                 }
//                 if (density >= _LayerList[i].get_min_density())
//                     ++count2[0];
//             }
//         }       
//     }
//     cout << "\n============[denity report]============\n";
//     for (int i = 0; i < 5; ++i){
//         cout<<"Density "<<0.4 + 0.01*i<<" - "<< 0.4 + 0.01*(i+1) <<" : "<<count[i]/count2[0]*100<<"% \n";
//     }
//     // for (int i = 0; i < 9; i++)
//     //     cout << "Layer #" << i + 1 << " | density:" << count[i] / count2[i] * 100 << "% \n";
// }
// void chipManager::check_layer(string &filename)
// {
//     ifstream ifs(filename);
//     int filesize;
//     ifs.seekg(0, ios::end);
//     filesize = ifs.tellg();
//     ifs.seekg(0, ios::beg);
//     char* buff = new char[filesize+1];
//     ifs.read(buff, filesize);
//     char* buff_beg = buff;
//     char* buff_end = buff + filesize;
//     string token = "haha";
//     int num;
//     vector<int> tokens;

//     while (token != ""){
//         while ( (token = next_token(buff_beg, buff_end)) != "" ){
//             if (token[0] == '\n') { break; }
//             if (myStr2Int(token, num)){
//                 tokens.push_back(num);
//             }
//             else{
//                 int a = tokens[6]-1;
//                 cout<<"layer id = "<<a<<endl;
//                 int count = 0;
//                 vector<Polygon*> query_list;
//                 int gap = _LayerList[a].get_gap();
//                 int y2 = tokens[4], y1 = tokens[2], x2 = tokens[1], x1 = tokens[3];
//                 int bl_x, bl_y, tr_x, tr_y;
//                 (x2 - _LayerList[a].get_gap() >= _LayerList[a].get_bl_boundary_x()) ? bl_x = x2 - _LayerList[a].get_gap()      : bl_x = _LayerList[a].get_bl_boundary_x();
//                 (y2 - _LayerList[a].get_gap() >= _LayerList[a].get_bl_boundary_y()) ? bl_y = y2 - _LayerList[a].get_gap()      : bl_y = _LayerList[a].get_bl_boundary_y();
//                 (x1 + _LayerList[a].get_gap() >  _LayerList[a].get_tr_boundary_x()) ? tr_x = _LayerList[a].get_tr_boundary_x() : tr_x = x1 + _LayerList[a].get_gap()     ;
//                 (y1 + _LayerList[a].get_gap() >  _LayerList[a].get_tr_boundary_y()) ? tr_y = _LayerList[a].get_tr_boundary_y() : tr_y = y1 + _LayerList[a].get_gap()     ; 
                       
//                 _LayerList[a].region_query( _LayerList[a].get_dummy(), tr_x, tr_y, bl_x, bl_y, query_list);
//                 for (int i = 0; i < query_list.size(); ++i){
//                     if (query_list[i]->is_solid())
//                     {
//                         int area = (query_list[i]->_top_right_x()-query_list[i]->_bottom_left_x())*(query_list[i]->_top_right_y()-query_list[i]->_bottom_left_y());
//                         cout<<"..............area = "<<area<<endl;
//                         cout<<"..............maxa = "<< _LayerList[a].get_max_width()*_LayerList[a].get_max_width()<<endl;
//                         assert(area < _LayerList[a].get_max_width()*_LayerList[a].get_max_width());
//                         cout<<"a "<<query_list[i]->_top_right_x() - query_list[i]->_bottom_left_x()<<endl;
//                         cout<<"b "<<_LayerList[a].get_max_width()<<endl;
//                         assert(query_list[i]->_top_right_x() - query_list[i]->_bottom_left_x() <= _LayerList[a].get_max_width());
//                         assert(query_list[i]->_top_right_y() - query_list[i]->_bottom_left_y() <= _LayerList[a].get_max_width());
//                         assert(query_list[i]->_top_right_x() - query_list[i]->_bottom_left_x() >= _LayerList[a].get_width());
//                         assert(query_list[i]->_top_right_y() - query_list[i]->_bottom_left_y() >= _LayerList[a].get_width());
//                         assert(area >= _LayerList[a].get_width()*_LayerList[a].get_width());
//                         count++;
//                         if (count > 1){
//                             cout<<".............gap error.....Layer id = "<<a<<"  ............."<<endl;
//                             print_Polygon(query_list[i]);
//                         }
//                     }
//                 }
//             }

//         }
//     }
// }