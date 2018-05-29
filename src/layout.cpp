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

using namespace std;

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
            if (cnet_set.count(tokens[5])){
                poly->setToCNet();
            }
            _polygonlist.push_back(poly);
        }
    }
}