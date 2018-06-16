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
//#define DEBUG
unsigned Polygon::global_ref=0;
double classify(int xy1,int xy2,int query_xy1,int query_xy2)
{   
    //xy1>>xy2
    //in range  被包在裡面
    if(xy1<=query_xy1&&xy2>=query_xy2)
        return (xy1 - xy2);
    //not in 橫跨
    else if(xy1>query_xy1&&xy2<query_xy2)
        return (query_xy1 - query_xy2 );
    //half 在上面或是在右邊
    else if(xy1>query_xy1&&xy2>=query_xy2&&xy2<query_xy1)
        return (query_xy1 - xy2);
    //half 在下面或是在左邊
    else if(xy2<query_xy2&&xy1<=query_xy1&&xy1>query_xy2)
        return (xy1 - query_xy2);
    else{
        cout<<endl<<"幹你娘找到囉"<<endl;
        cout<<xy1<<" "<<xy2<<" "<<query_xy1<<" "<<query_xy2<<endl;
        return 0;
    }
}
struct Local {
    Local(int x,int y,int windowsize){
    this->x= x;
    this->y= y;
    this->windowsize= windowsize;
    }
    bool operator () (Polygon* i, Polygon* j) {
    int area_i=classify(i->_top_right_x(),i->_bottom_left_x(),x+windowsize,x)*classify(i->_top_right_y(),i->_bottom_left_y(),y+windowsize,y);
    int area_j=classify(j->_top_right_x(),j->_bottom_left_x(),x+windowsize,x)*classify(j->_top_right_y(),j->_bottom_left_y(),y+windowsize,y);
    return (area_i <= area_j)? i<j : j<i; 
    }
    int x,y,windowsize;
};
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
double Layer::density_calculate(const int &x, const int &y, const double &windowsize, vector<Polygon *> &vec)
{   
    //region 右上/左下
    vector<Polygon*> query_list;
    region_query(dummy_bottom,x+windowsize,y+windowsize,x,y, query_list);
    vec.clear();
    double area=0,x_area=0,y_area=0;
    for(int i=0;i<query_list.size();i++){
        if(query_list[i]->is_solid()){
            x_area=classify(query_list[i]->_top_right_x(),query_list[i]->_bottom_left_x(),x+windowsize,x);
            y_area=classify(query_list[i]->_top_right_y(),query_list[i]->_bottom_left_y(),y+windowsize,y);
            if(x_area<0||y_area<0) ;//cout<<query_list[i]->_top_right_x()<<" "<<query_list[i]->_top_right_y()<<"in window"<<x<<","<<y<<"有問題\n";
            else area+=x_area*y_area;
        }
        if (query_list[i]->is_critical())
        {
            vec.push_back(query_list[i]);
        }
    }
    return area/(windowsize*windowsize);
}
void Layer::print_Polygon(Polygon* T)
{

    cerr<<T->getType()<<" ("<<T->_top_right_x()<<","<<T->_top_right_y()<<") ("<<T->_bottom_left_x()<<","<<T->_bottom_left_y()<<")\n";
    int a;
    //if(T->_top_right_x()==3407008&&T->_bottom_left_x()==3407008)cin>>a;
    if(T->getType()[0]=='d')return;
    cerr<<"tr "<<T->get_tr()->getType()<<" ("<<T->get_tr()->_top_right_x()<<","<<T->get_tr()->_top_right_y()<<") ("<<T->get_tr()->_bottom_left_x()<<","<<T->get_tr()->_bottom_left_y()<<")\n";
    //if(T->get_tr()->_bottom_left_x()!=T->_top_right_x()||T->get_tr()->_bottom_left_y()>T->_top_right_y())cin>>a;
    cerr<<"rt "<<T->get_rt()->getType()<<" ("<<T->get_rt()->_top_right_x()<<","<<T->get_rt()->_top_right_y()<<") ("<<T->get_rt()->_bottom_left_x()<<","<<T->get_rt()->_bottom_left_y()<<")\n";
    //(T->get_rt()->_bottom_left_y()!=T->_top_right_y()||T->get_rt()->_bottom_left_x()>T->_top_right_x())cin>>a;
    cerr<<"lb "<<T->get_lb()->getType()<<" ("<<T->get_lb()->_top_right_x()<<","<<T->get_lb()->_top_right_y()<<") ("<<T->get_lb()->_bottom_left_x()<<","<<T->get_lb()->_bottom_left_y()<<")\n";
    //if(T->get_lb()->_top_right_y()!=T->_bottom_left_y()||T->get_lb()->_top_right_x()<T->_bottom_left_x())cin>>a;
    cerr<<"bl "<<T->get_bl()->getType()<<" ("<<T->get_bl()->_top_right_x()<<","<<T->get_bl()->_top_right_y()<<") ("<<T->get_bl()->_bottom_left_x()<<","<<T->get_bl()->_bottom_left_y()<<")\n";
    //if(T->get_bl()->_top_right_x()!=T->_bottom_left_x()||T->get_bl()->_top_right_y()<T->_bottom_left_y())cin>>a;
}
void Layer::initRule(int n1, int n2, int n3, double min, double max)
{

    min_width = n1;
    min_space = n2;
    max_fill_width = n3;
    min_density = min;
    max_density = max;
}

void Layer::initialize_layer(int x_bl, int y_bl, int x_tr, int y_tr)
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
    dummy_left=new Polygon("dummy left");
    dummy_left->set_xy(x2,y1,x2-2,y2);
    a->set_bl(dummy_left);
    dummy_left->set_tr(a);
    
    //dummy right
    dummy_right=new Polygon("dummy right");
    dummy_right->set_xy(x1+2,y1,x1,y2);
    dummy_right->set_bl(a);
    a->set_tr(dummy_right);
    //dummy top
    dummy_top=new Polygon("dummy top");
    dummy_top->set_xy(x1,y1+2,x2,y1);
    a->set_rt(dummy_top);
    dummy_top->set_lb(a);
    //dummy bottom
    dummy_bottom=new Polygon("dummy bottom");
    dummy_bottom->set_xy(x1,y2,x2,y2-2);
    a->set_lb(dummy_bottom);
    dummy_bottom->set_rt(a);

    dummy_bottom_right = new Polygon("dummy bottom right");
    dummy_bottom_right->set_xy(x1+2, y2, x1, y2-2);
    dummy_bottom_right->set_rt(dummy_right);
    dummy_bottom_right->set_bl(dummy_bottom);
    

    dummy_right_top = new Polygon("dummy right top");
    dummy_right_top->set_xy(x1+2, y1+2, x1, y1);
    dummy_right_top->set_lb(dummy_right);
    dummy_right_top->set_bl(dummy_top);
    

    dummy_bottom_left = new Polygon("dummy bottom left");
    dummy_bottom_left->set_xy(x2, y2, x2-2, y2-2);
    dummy_bottom_left->set_rt(dummy_left);
    dummy_bottom_left->set_tr(dummy_bottom);
    
    dummy_top_left = new Polygon("dummy top left");
    dummy_top_left->set_xy(x2, y1+2, x2-2, y1);
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
void neighbor_find_own(Polygon* T,vector<Polygon*> &v,const int& max_y,const int& min_y)
{
    //上到下找
    #ifdef DEBUG
        cout<<"find own "<<endl;
    #endif
    Polygon* current=T->get_tr();

    while(current->_bottom_left_y()>=T->_bottom_left_y()){
        //
        if(current->_bottom_left_y()<max_y&&current->_top_right_y()>min_y)
            v.push_back(current);
        current=current->get_lb();
    }
}
void enumerate(Polygon* T,vector<Polygon*> &v,const int& max_x,const int& max_y,const int& min_y)
{

    //找own
    #ifdef DEBUG
        cout<<"enumerate "<<endl;
    #endif
    if(T->isglobalref())return;
    v.push_back(T);
    T->setToglobalref();
    //not sure top_right_x or top_right_y
    if(T->_top_right_x()>=max_x)
        return;
    vector<Polygon*>neighbor;
    neighbor_find_own(T,neighbor,max_y,min_y);
    for(int i=0;i<neighbor.size();i++){
        enumerate(neighbor[i],v,max_x,max_y,min_y);
    }
    return;
}


void Layer::region_query(Polygon* start,int x1,int y1,int x2,int y2, vector<Polygon*>& query_Polygon)
{
    /*x1,y1 是右上   x2,y2 是左下
    左上角的座標是 x2,y1 是我們要query的
    */
    #ifdef DEBUG
        cout<<"region query "<<endl;
    #endif
    Polygon::setGlobalref();
    // vector<Polygon*> query_Polygon;
    vector<Polygon*>left_poly;
    start=point_search(start,x2,y1);
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
        enumerate(bottom_poly[i],query_Polygon,x1,y1,y2);
    }
    // return query_Polygon;
}

void Layer::region_query(Polygon *start, Polygon *T, vector<Polygon *> &query_Polygon)
{
    return region_query(start, T->_top_right_x(), T->_top_right_y(), T->_bottom_left_x(), T->_bottom_left_y(), query_Polygon);
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
    Polygon* new_poly = new Polygon(bigGG->getType(), bigGG->is_solid());
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
    Polygon* new_poly = new Polygon(bigGG->getType(), bigGG->is_solid());
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

void Layer::insert_dummy(const int& edge_x, const int& edge_y,const double& windowsize, double& density,const int& layer_id){
    // x,y 左下角座標 黃平瑋要看喔
    vector<Polygon*> query_list;
    //cout<<"query\n";
    region_query(dummy_bottom,edge_x+windowsize-get_gap(),edge_y+windowsize-get_gap(),edge_x+get_gap(),edge_y+get_gap(),query_list);// (start,跟要插進去得tile)
    //cout<<"query_list size= "<<query_list.size()<<endl;
    //cout<<"sort\n";
    sort (query_list.begin(), query_list.end(),  Local(edge_x,edge_y,windowsize));
    //cout<<"begin insert\n";
    for(int i=0;i<query_list.size();i++){
        //cout<<query_list[i]->getType()<<endl;
        if(query_list[i]->getType()=="space"){
            //query 要內縮getgap 因為等一下插入的tile是會內縮過的 所以這裡query先縮才會找到合法的tile
            if(query_list[i]->_top_right_x()-get_gap()-query_list[i]->_bottom_left_x()-get_gap()>=get_width()){
                if(query_list[i]->_top_right_y()-get_gap()-query_list[i]->_bottom_left_y()-get_gap()>=get_width()){
                    //the two aboves are to verify the validility of the the region will be inserted
                    Polygon* T = new Polygon("filled",true);
                    T->set_layer_id(layer_id);
                    T->set_xy(query_list[i]->_top_right_x()-get_gap(),query_list[i]->_top_right_y()-get_gap(),query_list[i]->_bottom_left_x()+get_gap(),query_list[i]->_bottom_left_y()+get_gap());
                    if(insert(T,false)){
                        //cout<<"now insert "<<T->_top_right_x()<<","<<T->_top_right_y()<<" "<<T->_bottom_left_x()<<","<<T->_bottom_left_y()<<endl;
                        double new_area=classify(T->_top_right_x(),T->_bottom_left_x(),edge_x+windowsize,edge_x)*classify(T->_top_right_y(),T->_bottom_left_y(),edge_y+windowsize,edge_y);
                        density=(density*windowsize*windowsize + new_area)/(windowsize*windowsize);

                    }
                    else cout<<"幹你娘錯了拉幹\n";
                    delete T;
                    T=NULL;
                    if(density>=get_min_density()){
                        break;
                    }
                }
            }
        }
    }
    cout<<"QQ塞不滿"<<endl;

}

bool Layer::insert(Polygon* T,bool first_inset){
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
    if(!first_inset){
        //決定query的邊界
        //左下角
        (T->_bottom_left_x()-get_gap()>=get_bl_boundary_x()) ? bl_x = T->_bottom_left_x() - get_gap() : bl_x = get_bl_boundary_x();
        (T->_bottom_left_y()-get_gap()>=get_bl_boundary_y()) ? bl_y = T->_bottom_left_y() - get_gap() : bl_y = get_bl_boundary_y();
        //右上角
        (T->_top_right_x() + get_gap() >get_tr_boundary_x() ) ? tr_x = get_tr_boundary_x() : tr_x = T->_top_right_x() + get_gap();
        (T->_top_right_y() + get_gap() >get_tr_boundary_y() ) ? tr_y = get_tr_boundary_y() : tr_y = T->_top_right_y() + get_gap();
        
        // 如果要插進去的這塊裡面有solid就不能插 就像有男友的女生一樣
        region_query(dummy_bottom,tr_x,tr_y,bl_x,bl_y,query_list);// (start,跟要插進去得tile)
    }
    else region_query(dummy_bottom, T->_top_right_x(), T->_top_right_y(), T->_bottom_left_x(), T->_bottom_left_y(), query_list);
    for(int i=0;i<query_list.size();i++){
        if(query_list[i]->is_solid())
            return false;
    }
    if(!first_inset){
        query_list.clear();
        region_query(dummy_bottom, T->_top_right_x(), T->_top_right_y(), T->_bottom_left_x(), T->_bottom_left_y(), query_list);
    }
    #ifdef DEBUG
        cout<<"query_list num= "<<query_list.size()<<endl;
    #endif
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
    if(T->is_critical())
        query_list[0]->setToCNet();
    //set somthing
        query_list[0]->setToSolid();
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



