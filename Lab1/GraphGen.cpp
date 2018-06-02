#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>
#include <algorithm>

using namespace std;

/* randomly generate a directed graph. */
vector<vector<int>> generateDAG(int num) {
    vector<int> node;
    vector<vector<int>> map(num, vector<int> (num));

    int maxWeight = 100;
    for (int i = 0; i < num; i++) node.push_back(i);

    random_shuffle (node.begin(), node.end());
    random_shuffle (node.begin(), node.end());

    srand(time(NULL));

    int m = num * (num-1) / 2; // number of edges

    for (int i = 0; i < m; i++) {
        int p1 = rand() % num;
        int p2 = rand() % num;

        if (p1 == p2) continue;

        int x = node[p1];
        int y = node[p2];

        int l = rand() % maxWeight + 1; // weight
        map[x][y] = l;
    }

    return map;
}

/* write generated graph into csv file. */
void writeCSV(int n) {

    vector<vector<int>> data = generateDAG(n);
    ofstream myfile;
    myfile.open ("N"+to_string(n)+".csv");

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (data[i][j] == 0) myfile << "*";
            else myfile << data[i][j];
            if (j != n-1) myfile << ",";
        }
        myfile << endl;
    }

    myfile.close();
}

int main(int argc, char **argv) {
    int num = stoi(argv[1]);
    writeCSV(num);

    return 0;
}