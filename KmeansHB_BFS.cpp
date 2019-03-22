#include <algorithm>
#include <cstdlib>
#include <limits>
#include <random>
#include <vector>
#include <cmath>
#include <map>
#include <random>
#include <stdio.h>      
#include <stdlib.h>
#include <iostream>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include "dataset.h"
#include "Graph.h"



using namespace std;

class Solution
{ 
    public:
      double* solution;
      std::vector<int> assignments;
      Solution(double* sol, std::vector<int> assign){
        solution = sol;
        assignments = assign;
      }
};

class Constraints{
    public:
      vector<vector<int> > connectedComponents;
      vector<vector<int> > mlAdjacencyList;
      vector<vector<int> > clAdjacencyList;
      float** mustLinks;
      float** cannotLinks;
      int cannotLinksAmount;
      int mustLinksAmount;
      int points;

      Constraints(int n){
        points = n;
        mustLinks = new float*[n];
        cannotLinks = new float*[n];
        mustLinksAmount = 0;
        cannotLinksAmount = 0;


        for (int i = 0; i < n; i++){
          vector<int> v1;
          mlAdjacencyList.push_back(v1);

          vector<int> v2;
          clAdjacencyList.push_back(v2);

          mustLinks[i] = new float[n];
          cannotLinks[i] = new float[n];

          for (int j = 0; j < n; j++){
            mustLinks[i][j] = 0;
            cannotLinks[i][j] = 0;

          }
        }

      }

      void addCannotLink(int a, int b, float weight){
        cannotLinks[a][b] = weight;
        cannotLinks[b][a] = weight;
        clAdjacencyList[a].push_back(b);
        clAdjacencyList[b].push_back(a);
        cannotLinksAmount++;
      }

      // Add pairs of contraints

      void addMustLink(int a, int b, float weight){

        //fullList to check the weight of specific broken constraint later
        mustLinks[a][b] = weight;
        mustLinks[b][a] = weight;
        mlAdjacencyList[a].push_back(b);
        mlAdjacencyList[b].push_back(a);
        mustLinksAmount++;
  
      }

      void findConnectedComponents() {

        Graph g(points);

        for (int i = 0; i < mlAdjacencyList.size(); i++) {
          for (int j = 0; j < mlAdjacencyList[i].size(); j++) {
            g.addEdge(i, mlAdjacencyList[i][j]);
          }
        }

        g.connectedComponents(); 
        vector<int> component = g.getComponent();
        connectedComponents = vector< vector<int> >(g.getNbComponents());

        for(int i = 0; i < points; i++) {
          connectedComponents[component[i]].push_back(i);
        }
      }
};

// Calculates the sum-of-squares of the Euclidean distance between 2 points

double pointToPointDistance(double* data, double* solution, int p, int cluster, int d) {

  double ret = 0.0;
  for (int i = 0; i < d; i++){

    double a = data[p * d + i];
    double b = solution[cluster * d + i];

    double diff = a - b;
    ret += diff * diff;
  }

  return ret;
}
double getCostWithoutConstraints(const Dataset* dataset, double* solution, std::vector<int> assignments){
    double* data = dataset->data;
    int n = dataset->n;
    int d = dataset->d;

    double cost = 0.0;

    for (int p = 0; p < n; p++){
      int cluster = assignments[p];
      cost += pointToPointDistance(data, solution, p, cluster, d);
    }

    return cost;
}
double getCost(const Dataset* dataset, double* solution, std::vector<int> assignments, Constraints* constraints){
    double* data = dataset->data;
    int n = dataset->n;
    int d = dataset->d;

    double cost = 0.0;

    for (int p = 0; p < n; p++){
      int cluster = assignments[p];
      cost += pointToPointDistance(data, solution, p, cluster, d);
    }

    // create a nxn matrix do
    int** doubleCheckML = new int*[n];
    int** doubleCheckCL = new int*[n];
    for(int i = 0; i < n; i++){
      doubleCheckML[i] = new int[n];
      doubleCheckCL[i] = new int[n];
      for (int j = 0; j < n; j++){
            doubleCheckML[i][j] = 0;
            doubleCheckCL[i][j] = 0;
      }
    }


    for(int i = 0; i < n; i++){
      for (int j = 0; j < n; j++){
        if(constraints->mustLinks[i][j] != 0.0){
          if(assignments[i] != assignments[j] && doubleCheckML[i][j] == 0 && doubleCheckML[j][i] == 0){
            cost += constraints->mustLinks[i][j];
            doubleCheckML[i][j] = 1;
            doubleCheckML[j][i] = 1;

          }
        }

        if(constraints->cannotLinks[i][j] != 0.0){
          if(assignments[i] == assignments[j] && doubleCheckCL[i][j] == 0 && doubleCheckCL[j][i] == 0){
            cost += constraints->cannotLinks[i][j];
            doubleCheckCL[i][j] = 1;
            doubleCheckCL[j][i] = 1;

          }
        }
      }
    }

    return cost;
}

string replaceString(string s, const string &toReplace, const string &replaceWith) {
    return(s.replace(s.find(toReplace), toReplace.length(), replaceWith));
}

double getCrand(std::vector<int> bestSolution, std::vector<int> foundSolution, int n){
  int trueNegative = 0;
  int falseNegative = 0;
  int truePositive = 0;
  int falsePositive = 0;
  for(int i = 0; i < n; i++){
    for(int j = 0; j < n; j++){
      if(i != j){

        if(bestSolution[i] == bestSolution[j]){
          if(foundSolution[i] == foundSolution[j]){
            truePositive++;
          }
          else{
            falseNegative++;
          }
        }else{
          if(bestSolution[i] != bestSolution[j]){
            if(foundSolution[i] != foundSolution[j]){
              trueNegative++;  
            }else{
              falsePositive++;
            }
          }
        }
      }
    }
  }
  double rand = (trueNegative + truePositive)/(truePositive + trueNegative + falsePositive + falseNegative);
  int total = truePositive + falseNegative + falsePositive + trueNegative;
  double crand = (truePositive - (1.0*(falseNegative + truePositive)*(falsePositive + truePositive))/total)/((1.0*(falseNegative + truePositive + falsePositive + truePositive))/2 - (1.0*(falseNegative + truePositive)*(falsePositive + truePositive))/total);

  return crand;
}

Solution* kmeans(const Dataset* dataset, int clusters, int numIt, int n, int d, Constraints* constraints) {


  static std::random_device seed;
  static std::mt19937 rng(1607);
  std::uniform_int_distribution<int> indices(0, n - 1);

  double* data = dataset->data;

  // START OF STEP 1) INITIALIZATION


  // Creates the clusters vector based on the number of desired clusters
  double* solution = new double[clusters * d];


  //- Pick K random samples to define the initial center positions
  for (int i = 0; i < clusters; i++) {
    int p = indices(rng);
    for (int j = 0; j < d; j++){
      solution[i * d + j] = data[p * d + j];
    }
  }

  std::vector<int> assignments(n);
  std::vector<double> bestDistances(n);




  //- For each sample, in random order, calculate the minimum cost of an assignment of this sample to a center 

  std::vector<int> randomizer;
  for (int i = 0; i < n; i++) 
    randomizer.push_back(i); 


  // using built-in random generator:
  std::random_shuffle(randomizer.begin(), randomizer.end());

  for (int k = 0; k < n; k++) {

      int p = randomizer[k];

      double bestDistance = std::numeric_limits<double>::max();

      int bestCluster = 0;

      for (int cluster = 0; cluster < clusters; cluster++) {
        // For each point, calculate the nearest cluster
        double newDistance = pointToPointDistance(data, solution, p, cluster, d);


        for(int j = 0; j < constraints->mlAdjacencyList[p].size(); j++){
          int i = constraints->mlAdjacencyList[p][j];
          if(assignments[i] != cluster){
            newDistance += constraints->mustLinks[p][i];
          }
        }

        for(int j = 0; j < constraints->clAdjacencyList[p].size(); j++){
          int i = constraints->clAdjacencyList[p][j];
          if(assignments[i] == cluster){
              newDistance += constraints->cannotLinks[p][i];
          }
        }

        if (newDistance < bestDistance){
          // If the distance is better, updates the best cluster and distance  
          bestDistance = newDistance;
          bestCluster = cluster;
        }


      }
      assignments[p] = bestCluster;
      bestDistances[p] = bestDistance;

  }

  // END OF STEP 1



  // Creates the assignment vector based on the number of points

  for (int iteration = 0; iteration < numIt; ++iteration) {

    // START OF STEP 2)  Update centers positions to be in the barycenter of the points currently assigned to it

    // Creates the vector for the new clusters
    std::vector<double> newSolution(clusters * d, 0);

    // Creates a vector to count how many points in a cluster
    std::vector<int> pointsInCluster(clusters, 0);

    for (int p = 0; p < n; p++) {
      // Get the right cluster for each point
      int cluster = assignments[p];

      for (int i = 0; i < d; i++){
        //Sums the points belonging to a cluster
        newSolution[cluster * d + i] += data[p * d + i];
      }

      pointsInCluster[cluster] += 1;
    }

    // Divide sums by number of points to get new centroids.
    for (int cluster = 0; cluster < clusters; cluster++) {
      // Turn 0/0 into 0/1 to avoid zero division.
      int count = std::max<int>(1, pointsInCluster[cluster]);

      for (int i = 0; i < d; i++){
          solution[cluster * d + i] = newSolution[cluster * d + i]/count;
      }
    }



    // END OF STEP 2
    

    // START OF STEP 3) Update assignment of samples to centers



    // Find assignments.

    std::random_shuffle(randomizer.begin(), randomizer.end());
    std::vector<int> oldAssignments(n);
    oldAssignments = assignments;


    for (int k = 0; k < n; k++) {

      int p = randomizer[k];
      double bestDistance = std::numeric_limits<double>::max();;
      int bestCluster = 0;

      for (int cluster = 0; cluster < clusters; cluster++) {
        // For each point, calculate the nearest cluster


        // - For each sample, in random order, calculate the minimum cost of change of assignment from its current center to 
        //another (considering the other samples as well assigned and the penalties associated to them in case we violate must-link 
        //or cannot-link constraints), if the difference of cost is smaller than 0, apply the change.

        double newDistance = pointToPointDistance(data, solution, p, cluster, d);    


        for(int i = 0; i < constraints->mlAdjacencyList[p].size(); i++){
          int j = constraints->mlAdjacencyList[p][i];
          if(cluster != assignments[j]){
            newDistance += constraints->mustLinks[p][j];
          }
        }

        for(int i = 0; i < constraints->clAdjacencyList[p].size(); i++){
          int j = constraints->clAdjacencyList[p][i];
          if(cluster == assignments[j]){
              newDistance += constraints->cannotLinks[p][j];
          }
        }

        
        if (newDistance < bestDistance){
          // If the distance is better, updates the best cluster and distance 
          bestDistance = newDistance;
          bestCluster = cluster;
        }
      }
      assignments[p] = bestCluster;
      bestDistances[p] = bestDistance;
    }


    // ##TODO: -  In addition, calculate the connected components using BFS relative to the must-link constraints. And for each connected component,
    // calculate the cost of relocating all the samples of the component to a different cluster. If the difference of cost is smaller than 0, apply the change


    for(int i = 0; i < constraints->connectedComponents.size(); i++){
      for(int k = 0; k < clusters; k++){
        double currentDistance = 0;
        double newDistance = 0;

        for(int j = 0; j < constraints->connectedComponents[i].size(); j++){
          int p = constraints->connectedComponents[i][j];
          newDistance += pointToPointDistance(data, solution, p, k, d);
          currentDistance += pointToPointDistance(data, solution, p, assignments[p], d);
          for(int i = 0; i < constraints->clAdjacencyList[p].size(); i++){
            int j = constraints->clAdjacencyList[p][i];
            if(k == assignments[j]){
              newDistance += constraints->cannotLinks[p][j];
            }
            if(assignments[p] == assignments[j]){
              currentDistance += constraints->cannotLinks[p][j];
            }
          }
        }

        if (newDistance < currentDistance){
          for(int j = 0; j < constraints->connectedComponents[i].size(); j++){
            assignments[constraints->connectedComponents[i][j]] = k;
          }
        }
      }
    }

    // If there was no change, stop 
    if(oldAssignments == assignments){
    //cout << "There were " << iteration << " iterations." << endl;
      break;

    }

    // END OF STEP 3


  }

  Solution* s = new Solution(solution, assignments);
  return s;
}


int main(int argc, char** argv){

  //Get the data file name
  string fileName = argv[1];

  //Get the solution file name
  string solutionName = argv[2];
  
  //Create a variable to hold the constraints amount multiplying factor
  //Possible choices are 0.1, 0.05 and 0.01
  double cAmountMultiplier = atof(argv[3]);

  //Create a variable to hold the constraints weight multiplying factor
  //possible choices are 0.1, 1, 10
  double cWeightMultiplier = atof(argv[4]);

  //Amount of KmeansHB calls in each loop
  int runs = atoi(argv[5]);

  //The amount of loops
  int cruns = atoi(argv[6]);

  // Open the data file
  ifstream datasetFile(fileName);
  //datasetFile.open("dataset/fisher/fisher.txt");
    
  // Read the parameters
  int n, d;
  datasetFile >> n;
  datasetFile >> d;

    // Allocate storage
  Dataset *x = new Dataset(n, d);
    
    // Read the data values directly into the dataset
  for (int i = 0; i < n * d; ++i) {
      datasetFile >> x->data[i];
  }

  // Open a file with a good solution
  ifstream solutionFile(solutionName);
  //solutionFile.open("solutions/fisher/solution10.txt");

  //Read the parameters
  int clusters;
  double solutionCost;
  double f_best;
  solutionFile >> clusters;
  solutionFile >> solutionCost;
  solutionFile >> f_best;

  std::vector<int> solutionAssignments(n);

  // Read the assignments
  for(int i = 0; i < n; i++){
      solutionFile >> solutionAssignments[i];
  }

  //Decides the amount of constraints based on number of points in dataset
  int constraintsAmount = floor(n * cAmountMultiplier);

  //Decides the weight of each constraint 
  int constraintsWeight = (solutionCost/constraintsAmount) * cWeightMultiplier;

  //Create constraints based on the solution

  Constraints *constraints = new Constraints(n);


  // using built-in random generator to avoid skeweing results towards first points
  
  int currentAmount = 0;
  while(currentAmount < constraintsAmount){

    static std::random_device seed;
    static std::mt19937 rng(seed());
    std::uniform_int_distribution<int> points(0, n - 1);

    int selected = points(rng);
    int p = -1;
    do{p = points(rng);}while(p == selected);

    if(solutionAssignments[p] == solutionAssignments[selected]){
        constraints->addMustLink(p, selected, constraintsWeight);
    }else{
        constraints->addCannotLink(p, selected, constraintsWeight);
    }
    currentAmount++;
  }
  


  constraints->findConnectedComponents();



  double totalGap = 0.0;
  double totalTime = 0.0;
  double totalCrand = 0.0;
  int counter = 0;

  fileName = replaceString(fileName, "dataset/", "out/");
  fileName = replaceString(fileName, ".txt", "");


  while(counter < cruns){
    
    double best = std::numeric_limits<double>::max();; 

    clock_t begin = clock();

    Solution* bestSol;
    for(int i = 0; i < runs; i++){
      Solution* sol = kmeans(x, clusters, 100, n, d, constraints);
      double solutionCost = getCostWithoutConstraints(x, sol->solution, sol->assignments);
    
      if (solutionCost < best){
         best = solutionCost;
         bestSol = sol;
      }
    }

    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

    double gap = 100 * (best - f_best) / f_best;
    double crand = getCrand(solutionAssignments, bestSol->assignments, n);

    
    counter++;
    totalTime += elapsed_secs;
    totalGap += gap;
    totalCrand += crand;

  }

  double avgGap = totalGap/cruns;
  double avgTime = totalTime/cruns;
  double avgCrand = totalCrand/cruns;

  

  fileName = fileName + "_" + to_string(runs) + "_" + to_string(cruns) + "_" + to_string(cAmountMultiplier) + "_" + to_string(cWeightMultiplier) + "_" + to_string(clusters) + ".txt";

  std::ofstream myfile (fileName);

  myfile << fixed << avgTime << " "; 
  myfile << fixed << avgGap << " "; 
  myfile << runs << " ";
  myfile << clusters << " "; 
  myfile << cruns << " ";
  myfile << cWeightMultiplier << " ";
  myfile << cAmountMultiplier << " ";
  myfile << constraints->cannotLinksAmount << " "; 
  myfile << constraints->mustLinksAmount << " ";
  myfile << avgCrand << " ";
  myfile.close();  
}

