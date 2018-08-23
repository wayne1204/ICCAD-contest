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
//#include "include/gurobi_c++.h"

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
    // try{
    //     GRBEnv env = GRBEnv();
    //     GRBModel model = GRBModel(env);

    //     GRBVar x = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x");
    //     GRBVar y = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "y");
    //     GRBVar z = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "z");

    //     model.setObjective(x + y + 2 * z, GRB_MAXIMIZE);
    //     model.addConstr(x + 2 * y + 3 * z <= 4, "c0");
    //     model.addConstr(x + y >= 1, "c1");

    //     // Optimize model
    //     model.optimize();

    //     cout << x.get(GRB_StringAttr_VarName) << " "
    //          << x.get(GRB_DoubleAttr_X) << endl;
    //     cout << y.get(GRB_StringAttr_VarName) << " "
    //          << y.get(GRB_DoubleAttr_X) << endl;
    //     cout << z.get(GRB_StringAttr_VarName) << " "
    //          << z.get(GRB_DoubleAttr_X) << endl;

    //     cout << "Obj: " << model.get(GRB_DoubleAttr_ObjVal) << endl;
    // }
    // catch (GRBException e)
    // {
    //     cout << "Error code = " << e.getErrorCode() << endl;
    //     cout << e.getMessage() << endl;
    // }

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
    vector<bool>VorH;
    mgr->init_polygon(design, cnets_set, VorH);
    cerr<<"start preproccess......"<<endl;
    GRBEnv env = GRBEnv();
    GRBModel* model = new GRBModel(env);
    for (int i = 0; i < 9; ++i){
        mgr->preprocess(model, i, VorH);
    }
    //mu->report();

    string output_fill = "";
    // //mgr->report_density(true);
    // mgr->insert_tile(output_fill);
    // cout << "finish insert tile" << endl;
    // // mgr->insert_tile(output_fill);
    // mgr->report_density(false);
    // // mgr->insert_tile(output_fill);
    // mgr->write_fill(output, output_fill);
    // //mgr->check_layer(output);
    // mu->report();
}
