#include <iostream>
#include <sstream>
#include <fstream>
#include <climits>
#include <iostream>
#include <iomanip> 
#include <vector>
#include <string>
#include <unordered_set>
#include "chipMgr.h"
#include "util.h"
#include "usage.h"
#include "polygon.h"

using namespace std;

int main(int argc, char** argv)
{
    MyUsage* mu = new MyUsage();
    string ConfigFile = argv[1];
    string dirName = getDirName(ConfigFile);
    
    ifstream ifs(ConfigFile);
    if(!ifs.is_open()){
        cerr << "[Error] confile file \"" << ConfigFile<< " not found !!\n";
        return -1;
    }

    int filesize;
    ifs.seekg(0, ios::end);
    filesize = ifs.tellg();
    ifs.seekg(0, ios::beg);
    char* buff = new char[filesize+1];
    ifs.read(buff, filesize);
    char* buff_beg = buff;
    char* buff_end = buff + filesize;

    chipManager *mgr = new chipManager();
    string token;
    string design, output, rule_file, process_file;
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
            rule_file = dirName + next_token(buff_beg, buff_end);
            next_token(buff_beg, buff_end);
        }
        else if (token == "process_file"){
            process_file = dirName + next_token(buff_beg, buff_end);
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
    mgr->parseProcessFile(process_file);
    mgr->parseRuleFile(rule_file);
    mgr->init_polygon(design, cnets_set);
    mu->report();

    cout << "int size: " << sizeof(int) <<endl;
    cout << "double size: " << sizeof(double) << endl;
    cout << "polygon ptr:" << sizeof(Polygon*) << endl;
    mgr->insert_tile();
    // mgr->insert_tile();
    // mgr->insert_tile();
    // mgr->insert_tile();
    // mgr->insert_tile();
    // mgr->insert_tile();
    
    mu->report();
}
