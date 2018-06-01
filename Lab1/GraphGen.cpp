#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>
#include <algorithm>

using namespace std;

vector<vector<int>> graphGenerator(int num) {
    vector<int> node;
    vector<vector<int>> map(num, vector<int> (num));

    int maxWeight = 100;

    for (int i = 0; i < 5; i++) node.push_back(i);

    random_shuffle (node.begin(), node.end());
    random_shuffle (node.begin(), node.end());

    srand(time(NULL));

    int m = rand() % (num * num) + 1;

    for (int i = 0; i < m; i++) {
        int p1 = rand() % num;
        int p2 = rand() % (num-p1) + p1 + 1;
        cout << p1 << " " << p2 << endl;

        int x = node[p1];
        int y = node[p2];
        int l = rand() % maxWeight + 1;
        map[x][y] = l;
    }

    for (int i = 0; i < num; i++) {
        for (int j = 0; j < num; j++) {
            cout << map[i][j] << " ";
        }
        cout << endl;
    }

    return map;
}

void writeCSV(int n) {

    vector<vector<int>> data = graphGenerator(n);
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