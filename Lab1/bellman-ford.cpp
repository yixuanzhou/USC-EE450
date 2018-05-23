#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;


vector<vector<string>> readCSV(const string filename) {
	vector<vector<string>> graph;
	ifstream infile(filename);
	string line;

    if(!infile) { cerr << "Cannot open the File : " << filename <<endl; }

	while (infile) {

        string line;
		if (!getline(infile, line, '\n')) break;

		istringstream iss(line);
		vector<string> vertices;

		while (iss) {
			string s;
			if(!getline(iss, s, ',')) break;
			cout << s;
			vertices.push_back(s);
		}

		graph.push_back(vertices);
	}

	return graph;
}

int main() {
	vector<vector<string>> data = readCSV("N7.csv");

	for (auto row : data) {
		for (auto col : row)
			cout << col << " ";
		cout << endl;
	}

	return 0;
}