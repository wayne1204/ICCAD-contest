#include <iostream>
#include <sstream>
#include <fstream>
#include <climits>
#include <iostream>
#include <iomanip> 
#include <vector>
#include <string>
#include "process.h"
#include "util.h"
#include <unordered_set>

using namespace std;

int main(int argc, char** argv)
{
    string ConfigFile = argv[1];
    string dirName = getDirName(ConfigFile);
    
    ifstream ifs(ConfigFile);
    size_t filesize;
    ifs.seekg(0, ios::end);
    filesize = ifs.tellg();
    ifs.seekg(0, ios::beg);
    char* buff = new char[filesize+1];
    ifs.read(buff, filesize);
    char* buff_beg = buff;
    char* buff_end = buff + filesize;

    ProcessFile* pf = new ProcessFile();
    string token;
    string design, output, rulefile, process_file;
    unordered_set<int> cnets_set;
    int num;
    while( (token = next_token(buff_beg, buff_end)) != "" ){
        if (token == "design"){
            design = dirName + next_token(buff_beg, buff_end);
            next_token(buff_beg, buff_end);
        }
        else if (token == "output"){
            output = dirName + next_token(buff_beg, buff_end);
            next_token(buff_beg, buff_end);
        }
        else if (token == "rule_file"){
            rulefile = dirName + next_token(buff_beg, buff_end);
            next_token(buff_beg, buff_end);
        }
        else if (token == "process_file"){
            process_file = dirName + next_token(buff_beg, buff_end);
            pf->readFile(process_file.c_str());
            next_token(buff_beg, buff_end);
        }
        else if (token == "critical_nets"){
            while ( (token = next_token(buff_beg, buff_end)) != "" ){
                if (token[0] == '\n') break;
                myStr2Int(token, num);
                cnets_set.insert(num);
            }
        }
        else if (token == "power_nets"){

        }
        else if (token == "ground_nets"){

        }
    }
}
