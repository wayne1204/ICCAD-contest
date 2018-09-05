#ifndef POLYGON_H
#define POLYGON_H

#include <string>
#include <vector>
#include "include/gurobi_c++.h"
using namespace std;

class Polygon
{
	public:
		Polygon(string s = "normal", bool solid_ornot = false) : _type(s), _is_solid(solid_ornot)
		{
			//assert(s != "slot");
			_is_critical_net = false;
			_slot_id = -1;
			middle_y = 0;

		}
		Polygon(int m_y, GRBModel *model, int id=-10) : _type("slot"), _is_solid(false), _slot_id(id)
		{
			middle_y = m_y;
			_is_critical_net = false;
			G_model = model;
		}
		void setALL(int m_y, GRBModel *model, int id){
			_type = "slot";
			_is_solid = false;
			_slot_id = id ;
			middle_y = m_y;
			_is_critical_net = false;
			// setVariable(model);
			// cout<<"ALL"<<middle_y<<endl;
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
		inline int get_slot_id() { return _slot_id; }
		inline int get_polygon_id() { return _polygon_id; }
		inline GRBModel* get_model(){return G_model;}
		inline int _bottom_left_x() const { return _b_left_x; }
		inline int _bottom_left_y() const { return _b_left_y; }
		inline int _top_right_x() const { return _t_right_x; }
		inline int _top_right_y() const { return _t_right_y; }
		bool isglobalref() { return ref == global_ref; }
		bool is_solid() { return _is_solid; }
		bool is_slot() { return (_type == "slot"); }
		bool is_critical() { return _is_critical_net; }

		// slot variable
		void setVariable(GRBModel *model, int );
		GRBVar &getVariable(int i);
		int getVarSize() {return var_list.size(); }
		GRBLinExpr getPortion();
		const int get_Wi_coord(int);
		void reset_var(){var_list.clear();}

		// preprocessing member functions
		bool setornot(){return is_set;}
		void rotate();
		void swap_top_right();
		void swap_xy();
		void swap_bottom_left();
	private:
		int _b_left_x;
		int _b_left_y;
		int _t_right_x;
		int _t_right_y;
		int middle_y;
		int _polygon_id;
		int _slot_id;
		int _net_id;
		int _layer_id;
		Polygon *tr;
		Polygon *rt;
		Polygon *lb;
		Polygon *bl;
		bool is_set;
		string _type;
		bool _is_critical_net;
		bool _is_solid;
		unsigned ref;
		static unsigned global_ref;
		vector<GRBVar> var_list;
		vector<int> w_coord;
		GRBVar Y_ij;
		GRBModel* G_model;
};

#endif