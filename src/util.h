#ifndef UTIL_H
#define UTIL_H

#include <istream>
#include <vector>

using namespace std;

// In myString.cpp
extern int myStrNCmp(const string& s1, const string& s2, unsigned n);
extern size_t myStrGetTok(const string& str, string& tok, size_t pos = 0,
                          const char del = ' ');
extern bool myStr2Int(const string& str, int& num);
extern bool isValidVarName(const string& str);
extern string next_token(char *&_buff, char *&buff_end);

#endif // UTIL_H
