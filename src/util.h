#ifndef UTIL_H
#define UTIL_H

#include <istream>
#include <vector>
#include "polygon.h"
#include "layout.h"
#include "include/gurobi_c++.h"

using namespace std;

// string function
extern int myStrNCmp(const string& s1, const string& s2, unsigned n);
extern size_t myStrGetTok(const string& str, string& tok, size_t pos = 0,
                          const char del = ' ');
extern bool myStr2Int(const string& str, int& num);
extern string next_token(char *&_buff, char *&buff_end);
extern string getDirName(const string&);

// polygon function
extern double classify(int xy1, int xy2, int query_xy1, int query_xy2);
extern void print_Polygon(Polygon *T);
extern void neighbor_find_own(Polygon*, vector<Polygon *> &, const int &, const int &);
extern void enumerate(Polygon*, vector<Polygon *> &, const int &, const int &, const int &);

GRBLinExpr overlap(Polygon *, const int &, const int &, const int &, const int &, int);
#endif // UTIL_H
