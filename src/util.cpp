#include <string>
#include <ctype.h>
#include <cstring>
#include <cassert>
#include <iostream>
#include <math.h>

#include "polygon.h"
#include "layout.h"
#include "include/gurobi_c++.h"
using namespace std;

// 1. strlen(s1) must >= n
// 2. The first n characters of s2 are mandatory, they must be case-
//    insensitively compared to s1. Return less or greater than 0 if unequal.
// 3. The rest of s2 are optional. Return 0 if EOF of s2 is encountered.
//    Otherwise, perform case-insensitive comparison until non-equal result
//    presents.
//
int myStrNCmp(const string& s1, const string& s2, unsigned n)
{
    assert(n > 0);
    unsigned n2 = s2.size();
    if (n2 == 0) return -1;
    unsigned n1 = s1.size();
    assert(n1 >= n);
    for (unsigned i = 0; i < n1; ++i) {
        if (i == n2)
            return (i < n)? 1 : 0;
        char ch1 = (isupper(s1[i]))? tolower(s1[i]) : s1[i];
        char ch2 = (isupper(s2[i]))? tolower(s2[i]) : s2[i];
        if (ch1 != ch2)
            return (ch1 - ch2);
    }
    return (n1 - n2);
}


// Parse the string "str" for the token "tok", beginning at position "pos",
// with delimiter "del". The leading "del" will be skipped.
// Return "string::npos" if not found. Return the past to the end of "tok"
// (i.e. "del" or string::npos) if found.
//
size_t myStrGetTok(const string& str, string& tok, size_t pos = 0,
            const char del = ' ')
{
   size_t begin = str.find_first_not_of(del, pos);
   if (begin == string::npos) { tok = ""; return begin; }
   size_t end = str.find_first_of(del, begin);
   tok = str.substr(begin, end - begin);
   return end;
}


// Convert string "str" to integer "num". Return false if str does not appear
// to be a number
bool myStr2Int(const string& str, int& num)
{
    num = 0;
    size_t i = 0;
    int sign = 1;
    if (str[0] == '-') { sign = -1; i = 1; }
    bool valid = false;
    for (; i < str.size(); ++i) {
        if (isdigit(str[i])) {
            num *= 10;
            num += int(str[i] - '0');
            valid = true;
        }
        else return false;
    }
    num *= sign;
    return valid;
}

string next_token(char* &_buff, char* &buff_end)
{
	char separaters[] = {' ', ':', ';' 	};
	char delimiter[] = {'\n'};
	size_t num_s = 3;
	size_t num_d = 1;
	string token;
	for (; _buff < buff_end; ++_buff){
		if(*_buff == '#'){
			for(; *_buff != '\n'; _buff++);
		}
		bool is_separater = false;
		for(size_t i = 0; i < num_s; ++i){
			if (*_buff == separaters[i]) is_separater = true;
		}
		bool is_delimiter = false;
		for(size_t i = 0; i < num_d; ++i){
			if(*_buff == delimiter[i]) is_delimiter = true;
		}
		if (is_separater){
			if(token.length() != 0){
				_buff++;
				break;
			}
		}
		else if (is_delimiter){
			if(token.length()==0){
				token.push_back(*_buff);
				_buff++;
			}
			break;
		}
		else {token.push_back(*_buff); }
	}
	return token;
}

string getDirName(const string& configFile)
{
    size_t pos = 0;
    while(configFile.find('/', pos) != string::npos){
        pos = configFile.find('/', pos);
        ++pos;
    }
    return configFile.substr(0, pos);
}

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
        // cout<<endl<<"幹你娘找到囉"<<endl;
        // cout<<xy1<<" "<<xy2<<" "<<query_xy1<<" "<<query_xy2<<endl;
        return 0;
    }
}

void print_Polygon(Polygon* T)
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

// retrun width and coordinates of a series of Y_ij
// boundary: minimun x or minimun y
// length: polygon width od height

GRBLinExpr overlap(Polygon* slot, const int & x1, const int & y1, const int & x2, const int & y2, int middle){
    assert(slot->is_slot());

    GRBLinExpr slot_exp = GRBLinExpr();
    
    //int middle = max(min(slot->get_Wi_coord(-1), y1), y2);
    //int width = min(x1, slot->_top_right_x()) - max(x2, slot->_bottom_left_x());

    for(int i = 0; i < slot->getVarSize(); ++i){
        int w = max(min(slot->get_Wi_coord(i), y1), y2);
        int height = abs(middle - w);
        slot_exp += slot->getVariable(i) * height;
    }
    return slot_exp;
}
