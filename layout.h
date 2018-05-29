#include <iostream>
#include <sstream>
#include <fstream>
#include <climits>
#include <iostream>
#include <iomanip> 
#include <vector>
#include <string>
#include <unordered_set>

using namespace std;

class Polygon
{
public:
    Polygon(string s = ""):_type(s) {
        _is_critical_net = false;
    }
    void set_coordinate(vector<int> tokens){
        _polygon_id = tokens[0];
        _bottom_left_x = tokens[1];
        _bottom_left_y = tokens[2];
        _top_right_x = tokens[3];
        _top_right_y = tokens[4];
        _net_id = tokens[5];
        _layer_id = tokens[6];
    }
    void setToCNet();
    void setType(string type);

private:
    size_t _bottom_left_x;
    size_t _bottom_left_y;
    size_t _top_right_x;
    size_t _top_right_y;

    size_t _polygon_id;
    size_t _net_id;
    size_t _layer_id;

    string _type;
    bool _is_critical_net;
};

inline void Polygon::setToCNet() {
    _is_critical_net = true;
}
inline void Polygon::setType(string type) {
    _type = type;
}

class Layer
{
public:

    void init_polygon(string &filename, unordered_set<int> &cnet_set);

private:
    size_t _bl_boundary_x;
    size_t _bl_boundary_y;
    size_t _tr_boundary_x;
    size_t _tr_boundary_y;

    vector<Polygon*> _polygonlist;

};


size_t
myStrGetTok(const string& str, string& tok, size_t pos = 0,
            const char del = ' ')
{
   size_t begin = str.find_first_not_of(del, pos);
   if (begin == string::npos) { tok = ""; return begin; }
   size_t end = str.find_first_of(del, begin);
   tok = str.substr(begin, end - begin);
   return end;
}

string next_token(char* &_buff, char* &buff_end)
{
	char separaters[] = {
		' ', ':', ';' 
	};
	char delimiter[] = {
		'\n'
	};
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
bool
myStr2Int(const string& str, int& num)
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