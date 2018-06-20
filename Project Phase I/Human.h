#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

string readFile(string filename) {
    string res = "";
    string seller = filename.substr(0, 7);
    ifstream infile(filename);    
    string line;
    while (getline(infile, line)) {
        stringstream iss(line);
        string s;
        while (iss.good()) {
            getline(iss, s, ':');
            res += (s+",");
        }
    }

    return seller+","+res.substr(0, res.length()-1);
}

