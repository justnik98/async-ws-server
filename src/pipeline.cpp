//
// Created by justnik on 06.03.2021.
//
//#include "json.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include "pipeline.h"


using json = nlohmann::json;

void runPipeline(std::string &str) {
    std::string s = R"({"UID": 0,"language": "cpp","version": 17,"taskUID": 1,"program": "int main(){return 0;}"})"; //mock
    json n = json::parse(s);
    std::string res = n["program"];
    std::stringstream ss;
    if (res.starts_with('\"')) {
        res.erase(str.begin());
    }
    if (res.ends_with('\"')) {
        res.erase(str.end() - 1);
    }
    ss << n["UID"] << "_" << n["taskUID"] << ".";
    std::string lang = n["language"];
    lang.erase(remove_if(lang.begin(), lang.end(), [](unsigned char x) { return x == '\"'; }),
               lang.end());
    ss << lang;
    std::string filename = ss.str();
    std::ofstream out(filename);
    out << res << std::endl;
}
