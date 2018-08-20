#ifndef POLYGON_H
#define POLYGON_H

#include <string>
#include <vector>
using namespace std;

class Polygon
{
  public:
    // Polygon(string s = "normal", bool solid_ornot = false) : _type(s), _is_solid(solid_ornot)
    Polygon(string s = "normal", bool solid_ornot = false) : _type(s), _is_solid(solid_ornot)
    {
        _is_critical_net = false;
    }
    void set_coordinate_H(vector<int> tokens);
    void set_coordinate_V(vector<int> tokens);
    void set_layer_id(int a) { _layer_id = a; }
    void set_net_id(int a) { _net_id = a; }
    void set_polygon_id(int a) { _polygon_id = a; }
    void set_xy(int x1, int y1, int x2, int y2);
    void set_tr(Polygon *a) { tr = a; }
    void set_rt(Polygon *a) { rt = a; }
    void set_lb(Polygon *a) { lb = a; }
    void set_bl(Polygon *a) { bl = a; }
    void setToglobalref() { ref = global_ref; }
    static void setGlobalref() { global_ref++; }
    Polygon *get_tr() { return tr; }
    Polygon *get_rt() { return rt; }
    Polygon *get_lb() { return lb; }
    Polygon *get_bl() { return bl; }
    string getType() { return _type; }
    inline void setToSolid() { _is_solid = true; }
    inline void setToCNet() { _is_critical_net = true; }
    inline void setType(string type) { _type = type; }
    inline int get_layer_id() { return _layer_id; }
    inline int get_net_id() { return _net_id; }
    inline int get_polygon_id() { return _polygon_id; }
    inline int _bottom_left_x() const { return _b_left_x; }
    inline int _bottom_left_y() const { return _b_left_y; }
    inline int _top_right_x() const { return _t_right_x; }
    inline int _top_right_y() const { return _t_right_y; }
    bool isglobalref() { return ref == global_ref; }
    bool is_solid() { return _is_solid; }
    bool is_critical() { return _is_critical_net; }

    /*
    preprocessing member functions
    */
    void rotate();
    void swap_top_right();
    void swap_xy();
    void swap_bottom_left();


  private:
    int _b_left_x;
    int _b_left_y;
    int _t_right_x;
    int _t_right_y;
    int _polygon_id;
    int _net_id;
    int _layer_id;
    Polygon *tr;
    Polygon *rt;
    Polygon *lb;
    Polygon *bl;
    string _type;
    bool _is_critical_net;
    bool _is_solid;
    unsigned ref;
    static unsigned global_ref;
};

#endif