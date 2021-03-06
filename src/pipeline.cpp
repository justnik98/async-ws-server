//
// Created by justnik on 06.03.2021.
//
//

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include "pipeline.h"


using json = nlohmann::json;

void runPipeline(std::string &str) {
    std::string s = R"({"UID": 0,"language": "cpp","version": 17,"taskUID": 1,"program": "int main(){return 0;}"})"; //mock
    json n = json::parse(s);
    std::string code = n["program"];

    if (code.starts_with('\"')) {
        code.erase(str.begin());
    }
    if (code.ends_with('\"')) {
        code.erase(str.end() - 1);
    }

    //forming the file name
    std::stringstream ss;
    ss << "files/" << n["UID"] << "_" << n["taskUID"] << ".";
    std::string lang = n["language"];
    lang.erase(remove_if(lang.begin(), lang.end(), [](unsigned char x) { return x == '\"'; }),
               lang.end());
    std::string filename = ss.str();
    std::string outfile = filename + lang;
    std::string infile = filename + "res";
    std::ofstream out(outfile);
    out << code << std::endl;
    out.close();
    std::string command = "./script.sh " + outfile + " " + infile;
    system(command.c_str());//mock
    std::ifstream in(infile);
    if (!in.is_open()) {
        std::cerr << "Time exceeded";
        return;
    }
    std::string res;
    in >> res;
    std::cout << res;
}