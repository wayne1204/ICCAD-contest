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
#include "include/gurobi_c++.h"

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
    // chipManager *mgr = new chipManager();
    // mgr->parseProcessFile(process_file);
    // mgr->parseRuleFile(rule_file);
    // vector<bool>VorH;
    // mgr->init_polygon(design, cnets_set, VorH);
    // mgr->chip_rotate(VorH);

    double final_cap = 0.0;
    try{
        chipManager *mgr = new chipManager();
        mgr->parseProcessFile(process_file);
        mgr->parseRuleFile(rule_file);
        vector<bool> VorH;
        mgr->init_polygon(design, cnets_set, VorH);
        mgr->chip_rotate(VorH);
        for (int i = 0; i < mgr->getLayerNum(); ++i)
        {
            GRBEnv env = GRBEnv();
            GRBModel *model = new GRBModel(env);
            mgr->set_variable(model, i);
            int x = mgr->get_bl_boundary_x(); 
            int y = mgr->get_bl_boundary_y(); 
            mgr->layer_constraint(model, i, x, y, 1);
            mgr->minimize_cap(model, i);
            model->optimize();
            final_cap += model->get(GRB_DoubleAttr_ObjVal) ;
            mgr->write_output(model,i, x, y);            
        }
        string s = "";
        mgr->write_fill(output, s);
        mgr->final_check();
    }
    catch (GRBException e)
    {
        try{
            cout << "Error code = " << e.getErrorCode() << endl;
            cout << e.getMessage() << endl;
            chipManager *mgr = new chipManager();
            mgr->parseProcessFile(process_file);
            mgr->parseRuleFile(rule_file);
            vector<bool> VorH;
            mgr->init_polygon(design, cnets_set, VorH);
            mgr->chip_rotate(VorH);
            for (int i = 0; i < mgr->getLayerNum(); ++i)
            {
                GRBEnv env = GRBEnv();
                GRBModel *model = new GRBModel(env);
                mgr->set_variable(model, i);
                int x = mgr->get_bl_boundary_x();
                int y = mgr->get_bl_boundary_y();
                mgr->layer_constraint(model, i, x, y, 2);
                mgr->minimize_cap(model, i);
                model->optimize();
                final_cap += model->get(GRB_DoubleAttr_ObjVal);
                mgr->write_output(model, i, x, y);
            }
            string s = "";
            mgr->write_fill(output, s);
            mgr->final_check();
        }
        catch (GRBException e)
        {
            cout << "Error code = " << e.getErrorCode() << endl;
            cout << e.getMessage() << endl;
            chipManager *mgr = new chipManager();
            mgr->parseProcessFile(process_file);
            mgr->parseRuleFile(rule_file);
            vector<bool> VorH;
            mgr->init_polygon(design, cnets_set, VorH);
            mgr->chip_rotate(VorH);
            for (int i = 0; i < mgr->getLayerNum(); ++i)
            {
                GRBEnv env = GRBEnv();
                GRBModel *model = new GRBModel(env);
                mgr->set_variable(model, i);
                int x = mgr->get_bl_boundary_x();
                int y = mgr->get_bl_boundary_y();
                mgr->layer_constraint(model, i, x, y, 3);
                mgr->minimize_cap(model, i);
                model->optimize();
                final_cap += model->get(GRB_DoubleAttr_ObjVal);
                mgr->write_output(model, i, x, y);
            }
            string s = "";
            mgr->write_fill(output, s);
            mgr->final_check();
        }
    }
    cout <<"[Final Cap]" << final_cap <<endl;
    
    mu->report();
}
