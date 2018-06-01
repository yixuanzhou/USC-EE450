#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <climits>
#include <regex>

using namespace std;

vector<vector<int>> readfile(const string filename) {
	vector<vector<int>> graph;
	ifstream infile(filename);

    if(!infile) { cerr << "Error when opening file: " << filename << endl; }

    string line;
    while (getline(infile, line)) {

        stringstream iss(line);
        vector<string> chars;
        vector<int> vertices;
        string s;

        while (iss.good()) {
            getline(iss, s, ','); // read each line delimited by comma
            chars.push_back(s);
        }

        for (string curr : chars) {
            if (curr.find('*') != string::npos) vertices.push_back(-1); // if char is "*", then store -1 as distance value
            else {
                if (!regex_match(curr, regex("[+-]?[0-9]+"))) curr = curr.substr(0, curr.size()-1);
                vertices.push_back(stoi(curr)); // store distance value
            }
        }

        graph.push_back(vertices); // store values into graph
    }

	return graph;
}

/* Implement Bellman-Ford algorithm to find shortest path from start node to all other node in a DAG. */
vector<int> BellmanFordSP(vector<vector<int>> graph) {
    auto numOfNode = (int) graph.size();
    int ct = 0; // count number of iterations
    vector<int> dist;
	dist.push_back(0);
	for (int i = 1; i < numOfNode; i++) dist.push_back(INT_MAX); // initialization
	vector<int> updated_dist(dist);
    vector<int> pred(numOfNode, 0);

    for (int i = 0; i < numOfNode-1; i++) {
        ct++;
        for (int j = 0; j < numOfNode; j++) {
            for (int k = 0; k < numOfNode; k++) {
                if (graph[j][k] < 0) continue; // if there is not path...
                if (dist[k] > dist[j] + graph[j][k] and dist[j] != INT_MAX) {
                    dist[k] = dist[j] + graph[j][k]; // update value when new distance < current distance
                    pred[k] = j;  //
                }
            }
        }

        if (dist == updated_dist) break; // early termination, when there is no update to dist vector
        else updated_dist = dist;
    }

    vector<string> paths(numOfNode, "");
    paths[0] = "0";
    for (int i = 1; i < numOfNode; i++) {
        int p = pred[i];
        while (p != 0) {
            paths[i] = "->" + to_string(p) + paths[i];
            p = pred[p];
        }
        paths[i] = "0" + paths[i];
        paths[i] += ("->" + to_string(i));
    }

    /* write results into text file */
    ofstream res;
    res.open ("output-N" + to_string(numOfNode) +".txt");
    for (int d : dist) res << d << ",";
    res << endl;
    for (const string &p : paths) res << p << endl;
    res << "Iteration: " << ct << endl;
    res.close();

	return dist;
}

int main(int argc, char **argv) {
    vector<vector<int>> data = readfile(argv[1]);
    vector<int> sp = BellmanFordSP(data);

    return 0;
}
