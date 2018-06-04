Name: Yixuan Zhou
USC ID: 3827-1583-76

g++ version: g++ (Ubuntu 7.2.0-8ubuntu3.2) 7.2.0

Compilation Steps:
1. g++ BellmanFord.cpp -o BellmanFord
2. g++ GraphGen.cpp -o GraphGen
3. g++ BellmanFordPipe.cpp -o BellmanFordPipe
4. g++ GraphGenPipe.cpp -o GraphGenPipe

Executing Steps:
1. ./BellmanFord Graphfile.csv
2. ./GraphGen num
3. First start server: ./BellmanFordPipe, then start client: ./GraphGenPipe  

The code for BellmanFordPipe.cpp and GraphGen.cpp are revised based on the sample code given by TA.
