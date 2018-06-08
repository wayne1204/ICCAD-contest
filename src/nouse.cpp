
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